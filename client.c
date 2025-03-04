#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

void clear_line() {
    printf("\33[2K\r"); // Efface la ligne actuelle
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <port> <pseudo>\n", argv[0]);
        return 1;
    }

    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set read_fds;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    write(client_sock, argv[3], strlen(argv[3]));
    printf("Bienvenu %s sur MyTeams\n", argv[3]);
    printf("Tapez vos messages\n");
    printf("%s (Me) : ", argv[3]);
    fflush(stdout);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_sock, &read_fds);

        if (select(client_sock + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            continue;
        }

        if (FD_ISSET(client_sock, &read_fds)) {
            int bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                clear_line(); // Efface l'invite actuelle
                printf("%s\n", buffer);
                printf("%s (Me) : ", argv[3]);
                fflush(stdout);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input[BUFFER_SIZE];
            if (fgets(input, BUFFER_SIZE, stdin) != NULL) {
                input[strcspn(input, "\n")] = '\0';
                write(client_sock, input, strlen(input));
                printf("%s (Me) : ", argv[3]); // Réaffiche l'invite après l'envoi
                fflush(stdout);
            }
        }
    }

    close(client_sock);
    return 0;
}