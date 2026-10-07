#include <stdio.h>   // para entradas y salidas estándar 
#include <stdlib.h>     // todo muy general 
#include <string.h>     // Manipulación de textos
#include <unistd.h>     // mas funciones del sistema (para cerrar el socket ej)
#include <arpa/inet.h>  // Funciones de red:)

#define PUERTO 8080     // Puerto del broker al que se conectará el suscriptor
#define BUFFER_SIZE 1024// Tamaño máximo del mensaje

int main(int argc, char *argv[]) {
    if (argc < 2) {     // Verificamos que al ejecutar se escriba el tema (ej: ./sub deportes)
        printf("Uso: %s <tema>\nEjemplo: %s deportes\n", argv[0], argv[0]);
        return 1;
    }

    char *tema = argv[1];                               // Guardamos el tema ingresado por el usuario
    int sock_fd;                                        // Identificador del socket
    struct sockaddr_in dir_broker;                      // Estructura con la dirección del broker
    char buffer[BUFFER_SIZE];                           // Memoria temporal para mensajes

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);           // Creamos el socket UDP

    dir_broker.sin_family = AF_INET;                    // Indicamos red IPv4
    dir_broker.sin_port = htons(PUERTO);                // Puerto 8080 adaptado al formato de red
    inet_pton(AF_INET, "127.0.0.1", &dir_broker.sin_addr); // IP local (localhost) convertida a binario

    // Preparamos el mensaje de suscripción ("SUB deportes") y lo guardamos en el buffer
    snprintf(buffer, sizeof(buffer), "SUB %s", tema);

    // Enviamos el mensaje de suscripción al broker
    sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&dir_broker, sizeof(dir_broker));

    printf("Suscrito al tema [%s]. Esperando noticias...\n", tema);

    while(1) {                                          // Bucle infinito para escuchar de forma continua
        memset(buffer, 0, BUFFER_SIZE);                 // Limpiamos el buffer
        
        // Esperamos a recibir un mensaje (el programa se pausa aquí hasta que el broker envía algo)
        int bytes = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
        
        if (bytes > 0) {                                // Si llega información...
            buffer[bytes] = '\0';                       // Terminamos la cadena de texto de forma segura
            printf("Noticia recibida -> %s\n", buffer); // Mostramos la noticia en pantalla
        }
    }

    close(sock_fd);                                     // Cerramos el socket
    return 0;
}