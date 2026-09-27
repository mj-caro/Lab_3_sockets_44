#include <stdio.h>      // Entrada y salida estándar
#include <stdlib.h>     // Funciones generales
#include <string.h>     // Manipulación de texto
#include <unistd.h>     // general (sleep para pausar y el close)
#include <arpa/inet.h>  // Funciones de red 

#define PUERTO 8080     // Puerto del broker
#define BUFFER_SIZE 1024// Tamaño del búfer

int main(int argc, char *argv[]) {
    if (argc < 2) {     // Verificamos que el usuario escriba el tema al ejecutar
        printf("Uso: %s <tema>\nEjemplo: %s deportes\n", argv[0], argv[0]);
        return 1;
    }

    char *tema = argv[1];                               // Guardamos el tema de la publicación
    int sock_fd;                                        // Identificador del socket
    struct sockaddr_in dir_broker;                      // Dirección del broker
    char buffer[BUFFER_SIZE];                           // Memoria para armar el mensaje

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);           // Creamos el socket UDP

    dir_broker.sin_family = AF_INET;                    // Red IPv4
    dir_broker.sin_port = htons(PUERTO);                // Puerto 8080 en formato de red
    inet_pton(AF_INET, "127.0.0.1", &dir_broker.sin_addr); // Dirección IP local (localhost)

    // Bucle para enviar exactamente 10 mensajes seguidos (requisito del laboratorio)
    for (int i = 1; i <= 10; i++) {
        // Armamos el mensaje con el formato correcto: "PUB <tema> <mensaje>"
        snprintf(buffer, sizeof(buffer), "PUB %s Gol_numero_%d", tema, i);

        // Enviamos el datagrama UDP directamente al broker
        sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&dir_broker, sizeof(dir_broker));

        printf("Enviado mensaje #%d para el tema: %s\n", i, tema); // Mensaje de control en consola
        
        sleep(1);    // Pausa de 1 segundo entre cada mensaje enviado pues porque tampoco queremos un desastrecon el broker con 10 mensajes en 1 segundo
    }

    close(sock_fd);         // Cerramos el socket al terminar los 10 envíos
    return 0;
}