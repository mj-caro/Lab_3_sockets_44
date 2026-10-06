#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#define PUERTO_BROKER 8081  // Usamos 8081 para no chocar con el 8080 de UDP

// Tipos de acciones del protocolo de aplicación
#define TIPO_REGISTRAR_SUB 1   // Mensaje que envía el Suscriptor para unirse a un partido
#define TIPO_PUBLICAR      2   // Mensaje que envía el Publicador con la novedad

#define TAM_TEMA    128
#define TAM_MENSAJE 1024

// Estructura del mensaje que se enviará por TCP
typedef struct {
    int tipo;                   // TIPO_REGISTRAR_SUB o TIPO_PUBLICAR
    char tema[TAM_TEMA];        // Ej: "Champions_League_Final_Real_Madrid_vs_Barcelona"
    char mensaje[TAM_MENSAJE];  // Ej: Descripción o resumen de la noticia (1KB)
} MensajeTCP;

#endif