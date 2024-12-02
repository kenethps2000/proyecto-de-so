#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"  // Cambiar a la IP del servidor
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void* monitor_logs(void* arg) {
    char* priority = (char*)arg;
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    FILE* pipe;

    // Construir comando journalctl
    snprintf(command, sizeof(command), "journalctl -p %s -o short-iso -f", priority);

    // Abrir pipe para leer logs en tiempo real 
    pipe = popen(command, "r");
    if (!pipe) {
        perror("Error al ejecutar journalctl");
        exit(1);
    }

    // Conectar al servidor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        exit(1);
    }

    // Leer logs en tiempo real y enviarlos al servidor
    while (fgets(buffer, sizeof(buffer), pipe)) {
        send(sockfd, buffer, strlen(buffer), 0);
    }

    pclose(pipe);
    close(sockfd);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <prioridad (alert, err, notice, emerg, crit, warning, info, debug)>\n", argv[0]);
        exit(1);
    }

    pthread_t thread;
    pthread_create(&thread, NULL, monitor_logs, argv[1]);
    pthread_join(thread, NULL);

    return 0;
}

