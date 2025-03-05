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
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

// Fonction pour envoyer la liste des clients connectés
void send_client_list(int client_sock) {
    char list_message[BUFFER_SIZE] = "Clients connectés : ";
    for (int i = 0; i < client_count; i++) {
        strcat(list_message, clients[i].pseudo);
        if (i < client_count - 1) {
            strcat(list_message, ", ");
        }
    }
    strcat(list_message, "\n");

    send(client_sock, list_message, strlen(list_message), 0);
}

void remove_client(int client_sock) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_sock) {
            printf("Client %s disconnected\n", clients[i].pseudo);
            close(clients[i].socket);
            clients[i] = clients[client_count - 1]; // Éviter le décalage
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
     printf("Bind OK\n");                                                                                                   
    printf("IP Serveur : 103.252.88.62 sur le port : %s \n", argv[1]);

    if (listen(server_sock, 5) < 0) {
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
                    if (read(client_sock, clients[client_count].pseudo, 31) <= 0) {
                        printf("Erreur lors de la réception du pseudo\n");
                        close(client_sock);
                        continue;
                    }
                    clients[client_count].pseudo[31] = '\0';
                    clients[client_count].address = client_addr;

                    printf("Client %s connected from %s\n", clients[client_count].pseudo, inet_ntoa(client_addr.sin_addr));

                    send_client_list(client_sock); // Envoi de la liste des clients

                    client_count++;
                } else {
                    close(client_sock);
                }
            }
        }
    }

    close(server_sock);
    return 0;
}
