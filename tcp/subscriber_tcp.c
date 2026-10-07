// La documentación COMPLETA de este código se encuentra en el informe de laboratorio.
#include <stdio.h> // Para imprimir en pantalla jaja
#include <stdlib.h> // Para control del sistema (hacer exit)
#include <string.h> //manipulación y manejo de cadenas de texto
#include <unistd.h> // Para cerrar sockets y otras funciones del sistema
#include <arpa/inet.h> // libreria para manejar direcciones de red (IP y puerto)

#include "protocolo.h"

int main() {
    int socket_fd; // número de socket del suscriptor
    struct sockaddr_in direccion_broker; // estructura para guardar la dirección IP y puerto del broker
    MensajeTCP mensaje; // estructura para enviar y recibir mensajes según el protocolo definido
    int bytes_leidos; // variable para almacenar la cantidad de bytes leídos desde el socket

    // Preconexión: Crear el socket TCP del suscriptor
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Error al crear el socket del suscriptor");
        exit(EXIT_FAILURE);
    }

    direccion_broker.sin_family = AF_INET;
    direccion_broker.sin_port = htons(PUERTO_BROKER);
    
    if (inet_pton(AF_INET, "127.0.0.1", &direccion_broker.sin_addr) <= 0) {
        perror("Dirección IP inválida o no soportada");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Three-way handshake: Conectar con el broker TCP
    if (connect(socket_fd, (struct sockaddr *)&direccion_broker, sizeof(direccion_broker)) < 0) {
        perror("Error al conectar con el Broker TCP");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("=== Conectado exitosamente al Broker TCP ===\n");

    // Creación y solicitud de registro de suscripción
    memset(&mensaje, 0, sizeof(MensajeTCP));
    mensaje.tipo = TIPO_REGISTRAR_SUB;

    printf("Ingrese el tema o partido al que desea suscribirse (ej. Colombia_vs_Brasil): ");
    if (fgets(mensaje.tema, TAM_TEMA, stdin) != NULL) {
        // Eliminar el salto de línea '\n' si fue capturado por fgets
        mensaje.tema[strcspn(mensaje.tema, "\n")] = '\0';
    }

    if (send(socket_fd, &mensaje, sizeof(MensajeTCP), 0) < 0) {
        perror("Error al enviar la solicitud de registro");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Suscripción registrada para el tema: '%s'\n", mensaje.tema);
    printf("Esperando eventos/notificaciones del Broker...\n\n");

    // Escucha pasiva para recibir noticias del broker
    while (1) {
        bytes_leidos = recv(socket_fd, &mensaje, sizeof(MensajeTCP), 0);

        if (bytes_leidos == 0) {
            printf("\n[AVISO] El Broker cerró la conexión TCP.\n");
            break;
        } else if (bytes_leidos < 0) {
            perror("\n[ERROR] Fallo en la lectura del socket");
            break;
        }

        // Si llegaron datos válidos y el tipo es una publicación
        if (mensaje.tipo == TIPO_PUBLICAR) {
            printf("----------------------------------------\n");
            printf(" NOTICIA RECIBIDA [%s]\n", mensaje.tema);
            printf(" Evento: %s\n", mensaje.mensaje);
            printf("----------------------------------------\n\n");
        }
    }

    close(socket_fd);
    printf("Suscriptor finalizado.\n");
    return 0;
}