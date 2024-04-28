#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SERVER_PORT 8889
#define BUFFER_SIZE 1024

char* format_time(const char* format) {
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);

    char* buffer = (char*)malloc(BUFFER_SIZE);
    strftime(buffer, BUFFER_SIZE, format, timeinfo);

    return buffer;
}

char* handle_request(const char* request) {
    char* response = (char*)malloc(BUFFER_SIZE);

    char format[BUFFER_SIZE];
    sscanf(request, "%*s %s", format);

    if (strcmp(format, "dd/mm/yyyy") == 0) {
        strcpy(response, format_time("%d/%m/%Y"));
    } else if (strcmp(format, "dd/mm/yy") == 0) {
        strcpy(response, format_time("%d/%m/%y"));
    } else if (strcmp(format, "mm/dd/yyyy") == 0) {
        strcpy(response, format_time("%m/%d/%Y"));
    } else if (strcmp(format, "mm/dd/yy") == 0) {
        strcpy(response, format_time("%m/%d/%y"));
    } else {
        strcpy(response, "Invalid format");
    }
    char *res = response + '\n';
    return res;
}

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    printf("Connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    int flag = 1;
    while (flag == 1) {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Error reading from socket");
            break;
        }

        if (bytes_received == 0) {
            printf("Client %s:%d disconnected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            break;
        }

        buffer[bytes_received] = '\0';

        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "exit\n") == 0 || strcmp(buffer, "exit\r\n") == 0) {
            printf("Client %s:%d sent exit command. Closing connection.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            flag = 0;
            close(client_socket);
            break;
        }

        char* response = handle_request(buffer);
        response+='\n';
        ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);
        if (bytes_sent < 0) {
            perror("Error writing to socket");
        }

        free(response);
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error on binding");
        return EXIT_FAILURE;
    }

    listen(server_socket, 5);

    printf("Server is running...\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error on accept");
            return EXIT_FAILURE;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Error on fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) { // Child process
            close(server_socket);
            handle_client(client_socket, client_addr);
            exit(EXIT_SUCCESS);
        } else { // Parent process
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
