#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocolo.h"

int main() {
    int socket_fd;
    struct sockaddr_in direccion_broker;
    MensajeTCP mensaje;

    // Preconexión: Crear el socket TCP del publicador
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Error al crear el socket del publicador");
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

    // Publicación y envio de un evento/noticia
    memset(&mensaje, 0, sizeof(MensajeTCP));
    mensaje.tipo = TIPO_PUBLICAR;

    printf("Ingrese el tema o partido del evento (ej. Colombia_vs_Brasil): ");
    if (fgets(mensaje.tema, TAM_TEMA, stdin) != NULL) {
        mensaje.tema[strcspn(mensaje.tema, "\n")] = '\0'; // Limpiar '\n'
    }

    printf("Ingrese la noticia/evento a publicar: ");
    if (fgets(mensaje.mensaje, TAM_MENSAJE, stdin) != NULL) {
        mensaje.mensaje[strcspn(mensaje.mensaje, "\n")] = '\0'; // Limpiar '\n'
    }

    if (send(socket_fd, &mensaje, sizeof(MensajeTCP), 0) < 0) {
        perror("Error al enviar la publicación");
    } else {
        printf("\n[ÉXITO] Publicación enviada correctamente al Broker.\n");
    }

    close(socket_fd);
    printf("Publicador finalizado.\n");
    return 0;
}