#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
    int socket;
    char pseudo[32];
    struct sockaddr_in address;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

void broadcast_message(char *message, int sender_sock) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_sock) {
            size_t len = strlen(message);
            size_t sent = 0;
            while (sent < len) {
                ssize_t bytes = write(clients[i].socket, message + sent, len - sent);
                if (bytes > 0) {
                    sent += bytes;
                } else {
                    break;
                }
            }
        }
    }
}

void remove_client(int client_sock) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_sock) {
            printf("Client %s disconnected\n", clients[i].pseudo);
            close(clients[i].socket);
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    fd_set read_fds;
    int max_fd;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    printf("My Teams Connect...OK\n");

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("listen");
        return 1;
    }

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);
        max_fd = server_sock;

        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i].socket, &read_fds);
            if (clients[i].socket > max_fd) {
                max_fd = clients[i].socket;
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            continue;
        }

        if (FD_ISSET(server_sock, &read_fds)) {
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
            if (client_sock > 0) {
                if (client_count < MAX_CLIENTS) {
                    clients[client_count].socket = client_sock;
                    read(client_sock, clients[client_count].pseudo, 31);
                    clients[client_count].pseudo[31] = '\0';
                    clients[client_count].address = client_addr;
                    printf("Client %s connected from %s\n", clients[client_count].pseudo, inet_ntoa(client_addr.sin_addr));
                    client_count++;
                } else {
                    close(client_sock);
                }
            }
        }

        for (int i = 0; i < client_count; i++) {
            if (FD_ISSET(clients[i].socket, &read_fds)) {
                int bytes_read = read(clients[i].socket, buffer, BUFFER_SIZE - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    char message[BUFFER_SIZE + 32];
                    snprintf(message, sizeof(message), "%.31s: %.1020s", clients[i].pseudo, buffer);
                    broadcast_message(message, clients[i].socket);
                } else if (bytes_read == 0) {
                    remove_client(clients[i].socket);
                }
            }
        }
    }

    close(server_sock);
    return 0;
}
