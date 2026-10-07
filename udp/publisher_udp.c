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

    char *tema = argv[1];       // Guardamos el tema de la publicación
    int sock_fd;                      // Identificador del socket
    struct sockaddr_in dir_broker;        // Dirección del broker
    char buffer[BUFFER_SIZE];     // Memoria para armar el mensaje

    const char *eventos[] = {
        "Inicio_del_partido",
        "Falta_a_favor",
        "Tarjeta_amarilla_al_jugador_10",
        "Disparo_a_puerta_salvado",
        "Tiro_de_esquina",
        "Gol_de_Equipo_A_al_minuto_32",
        "Cambio:_jugador_14_entra_por_jugador_8",
        "Tarjeta_roja_para_defensa",
        "Gol_anulado_por_fuera_de_lugar",
        "Fin_del_partido_1_a_1"
    };
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);           // Creamos el socket UDP

    dir_broker.sin_family = AF_INET;                    // Red IPv4
    dir_broker.sin_port = htons(PUERTO);                // Puerto 8080 en formato de red
    inet_pton(AF_INET, "127.0.0.1", &dir_broker.sin_addr); // Dirección IP local (localhost)

    // Bucle para enviar exactamente 10 mensajes seguidos (requisito del laboratorio)
    // Enviar los 10 mensajes variados
    for (int i = 0; i < 10; i++) {
        // Armamos el mensaje combinando el comando PUB, el tema y el evento deportivo
        snprintf(buffer, sizeof(buffer), "PUB %s %s", tema, eventos[i]);

        sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&dir_broker, sizeof(dir_broker));
        
        printf("Enviado evento #%d: %s\n", i + 1, eventos[i]);

        sleep(1); // espera de 1 segundo entre eventos porque no quiero que vayan como 10 mensajes por segundo
    }

    close(sock_fd);         // Cerramos el socket al terminar los 10 envíos
    return 0;
}