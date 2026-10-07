// La documentación COMPLETA de este código se encuentra en el informe de laboratorio.
#include <stdio.h> // Para imprimir en pantalla jaja
#include <stdlib.h> // Para control del sistema (hacer exit)
#include <string.h> //manipulación y manejo de cadenas de texto
#include <unistd.h> // Para cerrar sockets y otras funciones del sistema
#include <arpa/inet.h> // libreria para manejar direcciones de red (IP y puerto)
#include <sys/socket.h> // API de sockets
#include <sys/select.h> //habilita la multiplexación de sockets con select()

#include "protocolo.h" // Importamos nuestro "contrato de datos"

#define MAX_SUBS 20

// Estructura interna del broker de suscriptor
typedef struct {
    int socket_fd;             // Identificador del socket TCP (File Descriptor)
    char tema[TAM_TEMA];       // Tema al que está suscrito
} Suscriptor;

int main() {
    int servidor_fd, nuevo_socket, max_fd, i, bytes_leidos;
    struct sockaddr_in direccion;
    socklen_t addrlen = sizeof(direccion);
    
    // Arreglo para gestionar las conexiones en la memoria del Broker
    Suscriptor subs[MAX_SUBS];
    for (i = 0; i < MAX_SUBS; i++) {
        subs[i].socket_fd = 0;
        subs[i].tema[0] = '\0';
    }

    // Conjunto de sockets para la función de multiplexación select()
    fd_set readfds;

    // Creación del socket
    servidor_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4, SOCK_STREAM = TCP
    if (servidor_fd == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }
    
    // Permitir reutilizar el puerto inmediatamente si se reinicia el Broker
    int opt = 1;
    setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configurar la IP y el puerto de escucha del Broker
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY; // Escucha en cualquier interfaz de red local
    direccion.sin_port = htons(PUERTO_BROKER);

    // Enlazar el puerto al socket y ponerlo en modo de escucha pasiva
    if (bind(servidor_fd, (struct sockaddr *)&direccion, sizeof(direccion)) < 0) {
        perror("Error en bind()");
        close(servidor_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(servidor_fd, 10) < 0) {
        perror("Error en listen()");
        close(servidor_fd);
        exit(EXIT_FAILURE);
    }

    printf("=== Broker TCP iniciado exitosamente en el puerto %d ===\n", PUERTO_BROKER);

    while (1) { // Mantiene al broker activo y escuchando conexiones entrantes

        FD_ZERO(&readfds); // Limpiamos el conjunto de sockets antes de agregar los nuevos
        FD_SET(servidor_fd, &readfds); // Agregamos el socket del servidor al conjunto de monitoreo
        max_fd = servidor_fd; // valor por defecto, en el siguiente bucle lo actualizamos de ser necesario

        // Agregar los sockets activos de los clientes al conjunto de monitoreo
        for (i = 0; i < MAX_SUBS; i++) {
            int sd = subs[i].socket_fd;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_fd) max_fd = sd;
        }

        // Esperar actividad de entrada en cualquiera de los sockets monitoreados
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Error en select()");
            break;
        }

        // CASO 1: Nueva solicitud de conexión TCP (Handshake de 3 vías completado)
        if (FD_ISSET(servidor_fd, &readfds)) {
            nuevo_socket = accept(servidor_fd, (struct sockaddr *)&direccion, &addrlen);
            if (nuevo_socket < 0) {
                perror("Error en accept()");
            } else {
                printf("[CONEXION] Nueva sesion TCP abierta (Socket FD: %d)\n", nuevo_socket);

                // Asignar el socket entrante en la primera posición libre de la tabla
                for (i = 0; i < MAX_SUBS; i++) {
                    if (subs[i].socket_fd == 0) {
                        subs[i].socket_fd = nuevo_socket;
                        subs[i].tema[0] = '\0'; // Inicia sin tema hasta recibir TIPO_REGISTRAR_SUB
                        break;
                    }
                }
            }
        }

        // CASO 2: Actividad de lectura en una conexión activa existente
        for (i = 0; i < MAX_SUBS; i++) {
            int sd = subs[i].socket_fd;

            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                MensajeTCP mensaje;

                // Leer la estructura exacta enviada a través del socket TCP
                bytes_leidos = recv(sd, &mensaje, sizeof(MensajeTCP), 0);

                if (bytes_leidos == 0) {
                    // El cliente cerró la conexión ordenadamente (Paquete TCP FIN)
                    printf("[DESCONEXION] Cliente desconectado (Socket FD: %d)\n", sd);
                    close(sd);
                    subs[i].socket_fd = 0;
                    subs[i].tema[0] = '\0'; // Se resetea el tema registrado
                } else if (bytes_leidos > 0) {

                    // Opción 1: Solicitud de registro a un tema/partido
                    if (mensaje.tipo == TIPO_REGISTRAR_SUB) {
                        strncpy(subs[i].tema, mensaje.tema, TAM_TEMA - 1);
                        subs[i].tema[TAM_TEMA - 1] = '\0'; // Asegurar fin de cadena
                        printf("[REGISTRO] Socket %d suscrito al tema: '%s'\n", sd, mensaje.tema);
                    } 

                    // Opción 2: Evento o noticia enviada por un publicador
                    else if (mensaje.tipo == TIPO_PUBLICAR) {
                        printf("[PUBLICACION] Evento en [%s]: %s\n", mensaje.tema, mensaje.mensaje);

                        // Reenviar la noticia ÚNICAMENTE a las conexiones interesadas en este tema
                        for (int j = 0; j < MAX_SUBS; j++) {
                            if (subs[j].socket_fd > 0 && subs[j].tema[0] != '\0') {
                                // Verificar si el tema de la noticia coincide con el del suscriptor
                                if (strcmp(subs[j].tema, mensaje.tema) == 0) {
                                    send(subs[j].socket_fd, &mensaje, sizeof(MensajeTCP), 0);
                                }
                            }
                        }
                    }

                }
            }
        }
    }

    close(servidor_fd);
    return 0;
}