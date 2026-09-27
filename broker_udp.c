//Estas bibliotecas no son las de sockets que prohibe el lab, yo lo investigue.

#include <stdio.h>   // esta biblioteca es para para imprimir en pantalla y así tener claro que estoy haciendo
#include <stdlib.h>  // estabiblioteca es para control del sistema (hacer exit)
#include <string.h>  // esta la tuve que meter para manipular textos 
#include <unistd.h>     // Para que no me salgan errores de sistema operativo
#include <arpa/inet.h>  // esta es una biblioteca que usa C para manejar las direcciones de red (osea IP y puerto)

#define PUERTO 8080  // El puerto fijo donde el broker va a estar escuchando (esta es la que aparece en el lab, entonces por eso la puse)
#define MAX_SUBS 10     // Máximo de suscriptores que el broker va a recordar (igual se supone que son minimo 2 pero por si algo)
#define BUFFER_SIZE 1024// Tamaño máximo del mensaje que va a recibir

// Estructura para guardar la info de cada suscriptor
struct Suscriptor {
    struct sockaddr_in dir; // Guarda la IP y el puerto del suscriptor
    char tema[50];   // Guarda el tema/equipo que le interesa
};

int main() {
    int servidor_id;      // Identificador del socket del broker
    struct sockaddr_in dir_servidor, dir_cliente;  //  para las direcciones de red
    socklen_t dir_len = sizeof(dir_cliente);           // Tamaño de la dirección del cliente
    char buffer[BUFFER_SIZE];         // Memoria temporal para guardar el texto que llega
    
    struct Suscriptor subs[MAX_SUBS];           // Lista para almacenar a los suscriptores registrados
    int total_subs = 0;                    // Contador de cuántos suscriptores hay suscritos

    servidor_id = socket(AF_INET, SOCK_DGRAM, 0);       // crea el socket UDP (AF_INET=IPv4, SOCK_DGRAM=UDP)
    
    dir_servidor.sin_family = AF_INET;             // indica que usa IPv4
    dir_servidor.sin_addr.s_addr = INADDR_ANY;     // se escucha desde cualquier IP de la máquina
    dir_servidor.sin_port = htons(PUERTO);       // y aca convierte el puerto al formato estándar de red

    bind(servidor_id, (struct sockaddr*)& dir_servidor, sizeof(dir_servidor)); // Asignamos el puerto 8080 al socket

    printf("Broker UDP esta encendido y escuchando en el puerto ", PUERTO); //Para que el usuario sepa que el broker está activo y en qué puerto

    while(1) {                          // Acá hicimos un bucle infinito para que el broker no se vaya a dormir y siempre esté escuchando
        memset(buffer, 0, BUFFER_SIZE);               // limpa el buffer de memoria antes de leer
        
        // Recibimos un mensaje de cualquier cliente y guardamos su IP/puerto en 'dir_cliente'
        int bytes = recvfrom(servidor_id, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*) & dir_cliente, & dir_len);
        
        if (bytes > 0) {                     // Si llegaron datos...
            buffer[bytes] = '\0';            // Nos aseguramos de que el texto termine bien
            
            char comando[10], tema[50], mensaje[256];   // Variables para separar el mensaje recibido
            
            // Leemos el mensaje separándolo por espacios (ejemplo: "SUB deportes")
            sscanf(buffer, "%s %s %[^\n]", comando, tema, mensaje);

            if (strcmp(comando, "SUB") == 0) {          // Si el comando es "SUB" (Suscribirse)
                if (total_subs < MAX_SUBS) {            // Si hay espacio en la lista...
                    subs[total_subs].dir = dir_cliente; // Guardamos la dirección IP y puerto del suscriptor
                    strcpy(subs[total_subs].tema, tema);// Guardamos el tema que escogió
                    total_subs++;                       // Aumentamos en 1 el total de suscriptores
                    printf("Nuevo suscriptor registrado al equipo: %s\n", tema);
                }
            } 
            else if (strcmp(comando, "PUB") == 0) {     // Si el comando es "PUB" (Publicar noticia)
                printf("Noticia recibida para [%s]: %s\n", tema, mensaje);
                
                // Recorremos la lista de suscriptores para reenviarles la noticia si coincide el equipo
                for (int i = 0; i < total_subs; i++) {
                    if (strcmp(subs[i].tema, tema) == 0) { // Si el equipo del suscriptor es igual al de la noticia...
                        // Reenviamos el mensaje directamente a ese suscriptor usando su dirección guardada
                        sendto(servidor_id, mensaje, strlen(mensaje), 0, (struct sockaddr*)&subs[i].dir, sizeof(subs[i].dir));
                    }
                }
            }
        }
    }
    close(servidor_id);            // y cerramos el socket para que no quede abierto y genere problemas
    return 0;
}