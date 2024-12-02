#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>


void enviar_alerta_twilio(const char *numero_destino, const char *mensaje) {
    char comando[512];

    // Construir el comando para llamar al script de Python
    snprintf(comando, sizeof(comando), "python3 enviar_alerta.py %s \"%s\"", numero_destino, mensaje);

    // Ejecutar el comando
    int res = system(comando);

    if (res == 0) {
        printf("Mensaje enviado correctamente.\n");
    } else {
        printf("Error al enviar el mensaje.\n");
    }
}

#define PORT 8080
#define BUFFER_SIZE 1024

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    FILE* dashboard = fopen("dashboard.txt", "a");
    if (!dashboard) {
        perror("Error al abrir el archivo del dashboard");
        exit(1);
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;

        // Escribir los logs en el dashboard
        fprintf(dashboard, "%s", buffer);
        fflush(dashboard);

        // Mostrar en consola del servidor
        printf("Log recibido: %s", buffer);
    }

    fclose(dashboard);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error al crear el socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar el socket");
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Error al escuchar conexiones");
        exit(1);
    }

    printf("Servidor en espera de conexiones...\n");
    
    // Número del destinatario (debe estar autorizado en Twilio para pruebas)
    const char *numero_destino = "whatsapp:+593981773913";

    // Mensaje de alerta
    const char *mensaje = "Servidor en espera de conexiones...";

    // Enviar mensaje
    enviar_alerta_twilio(numero_destino, mensaje);

    while (1) {
        int* client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, NULL, NULL);
        if (*client_sock < 0) {
            perror("Error al aceptar conexión");
            free(client_sock);
            continue;
        }

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_sock);
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}
