#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

void *receive_messages(void *arg) {
    int socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            printf("Server disconnected\n");
            exit(1);
        }

        printf("\n%s\n", buffer);
        printf("Enter command(LIST/SEND username:message): ");
        fflush(stdout);
    }

    return NULL;
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    pthread_t receive_thread;
    char buffer[BUFFER_SIZE];
    char username[50];

    // create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(1);
    }

    // set server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    // connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    printf("Connected to server\n");

    while (1) {
        printf("Enter username: ");
        fgets(username, 50, stdin);
        username[strcspn(username, "\n")] = 0;

        send(client_socket, username, strlen(username), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "Authenticated", 13) == 0) {
            printf("Authentication successful\n");
            break;
        } else {
            printf("Authentication failed: %s\n", buffer);
        }
    }

    if (pthread_create(&receive_thread, NULL, receive_messages, &client_socket) != 0) {
        perror("Error creating thread");
        exit(1);
    }

    while (1) {
        printf("Enter command(LIST/SEND username:message): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "LIST") == 0) {
            send(client_socket, "LIST", 4, 0);
        } else if (strncmp(buffer, "SEND ", 5) == 0) {
            // Format should be: SEND username:message
            char *message_part = buffer + 5;
            send(client_socket, message_part, strlen(message_part), 0);
        } else {
            printf("Invalid command. Use LIST or SEND username:message\n");
        }
    }

    close(client_socket);
    return 0;
}
