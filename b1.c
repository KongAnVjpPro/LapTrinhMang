#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_PORT 8888
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buf[BUFFER_SIZE];
    int ret = recv(client_socket, buf, sizeof(buf), 0);
    if (ret < 0) {
        perror("Error reading from socket");
        close(client_socket);
        return;
    }

    buf[ret] = '\0';

    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client_socket, response, strlen(response), 0);

    close(client_socket);
}

int main() {
    int listener, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return EXIT_FAILURE;
    }

    if (listen(listener, MAX_CLIENTS) < 0) {
        perror("Error listening for connections");
        return EXIT_FAILURE;
    }

    printf("Server is running...\n");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            while (1) {
                client_socket = accept(listener, (struct sockaddr *)&client_addr, &client_len);
                if (client_socket < 0) {
                    perror("Error accepting connection");
                    exit(EXIT_FAILURE);
                }

                handle_client(client_socket);
            }
        } else if (pid < 0) {
            perror("Error forking process");
            return EXIT_FAILURE;
        }
    }

    while (wait(NULL) > 0);

    close(listener);

    return 0;
}
