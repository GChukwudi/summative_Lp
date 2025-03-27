#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 4
#define BUFFER_SIZE 1024

// client structure to track connections
typedef struct {
    int socket;
    char username[50];
    int is_authenticated;
} Client;

// global variables
Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int setup_server() {
    int server_socket;
    struct sockaddr_in server_address;

    // create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(1);
    }

    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        exit(1);
    }

    // set server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);
    return server_socket;
}

int authenticate_client(int client_socket, char *username) {
    pthread_mutex_lock(&clients_mutex);

    // Check if username already exists
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_authenticated &&
            strcmp(clients[i].username, username) == 0) {
            pthread_mutex_unlock(&clients_mutex);
            return 0;
        }
    }

    // Find an empty slot
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_socket) {
            strcpy(clients[i].username, username);
            clients[i].is_authenticated = 1;
            pthread_mutex_unlock(&clients_mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
    return 0;
}

int find_client_by_username(const char *username) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_authenticated &&
            strcmp(clients[i].username, username) == 0) {
            int client_socket = clients[i].socket;
            pthread_mutex_unlock(&clients_mutex);
            return client_socket;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

void broadcast_online_clients(int sender_socket) {
    char online_clients[BUFFER_SIZE] = "Online clients: ";
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_authenticated) {
            strcat(online_clients, clients[i].username);
            strcat(online_clients, ", ");
        }
    }

    pthread_mutex_unlock(&clients_mutex);
    send(sender_socket, online_clients, strlen(online_clients), 0);
}

void *handle_client(void *arg) {
    int *socket_ptr = (int *)arg;
    int client_socket = *socket_ptr;
    free(socket_ptr); // Free the allocated memory
    
    char buffer[BUFFER_SIZE];
    char username[50] = "";
    int is_authenticated = 0;

    // Register client in array
    pthread_mutex_lock(&clients_mutex);
    int client_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].is_authenticated) {
            clients[i].socket = client_socket;
            clients[i].is_authenticated = 0; // Not authenticated yet
            strcpy(clients[i].username, "");
            client_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (client_index == -1) {
        // No space for new client
        close(client_socket);
        return NULL;
    }

    while (!is_authenticated) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            close(client_socket);
            
            pthread_mutex_lock(&clients_mutex);
            clients[client_index].is_authenticated = 0;
            pthread_mutex_unlock(&clients_mutex);
            
            return NULL;
        }

        // Remove newline if present
        buffer[strcspn(buffer, "\n")] = 0;

        if (authenticate_client(client_socket, buffer)) {
            strcpy(username, buffer);
            is_authenticated = 1;
            printf("Client %s authenticated\n", username);
            
            send(client_socket, "Authenticated", 13, 0);
        } else {
            send(client_socket, "Username already taken", 22, 0);
        }
    }

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            break;
        }

        // Remove newline if present
        buffer[strcspn(buffer, "\n")] = 0;

        // Check if command is LIST
        if (strcmp(buffer, "LIST") == 0) {
            broadcast_online_clients(client_socket);
        } else {
            // Parse message format: username:message
            char *target_username = strtok(buffer, ":");
            char *message = strtok(NULL, "");

            if (target_username && message) {
                int target_socket = find_client_by_username(target_username);

                if (target_socket != -1) {
                    char message_buffer[BUFFER_SIZE];
                    snprintf(message_buffer, BUFFER_SIZE, "%s: %s", username, message);

                    send(target_socket, message_buffer, strlen(message_buffer), 0);
                } else {
                    char error_msg[BUFFER_SIZE];
                    snprintf(error_msg, BUFFER_SIZE, "User %s not found or not online", target_username);
                    send(client_socket, error_msg, strlen(error_msg), 0);
                }
            } else {
                send(client_socket, "Invalid message format. Use username:message", 43, 0);
            }
        }
    }

    pthread_mutex_lock(&clients_mutex);
    clients[client_index].is_authenticated = 0;
    strcpy(clients[client_index].username, "");
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    printf("Client %s disconnected\n", username);
    return NULL;
}

int main() {
    int server_socket = setup_server();
    pthread_t threads[MAX_CLIENTS];

    // Initialize client array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].is_authenticated = 0;
        strcpy(clients[i].username, "");
    }

    while(1) {
        struct sockaddr_in client_address;
        socklen_t client_length = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_length);

        if (client_socket < 0) {
            perror("Error accepting client connection");
            continue;
        }

        printf("New connection from %s:%d\n", 
               inet_ntoa(client_address.sin_addr), 
               ntohs(client_address.sin_port));

        // Allocate memory for the socket descriptor
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        if (pthread_create(&threads[client_socket % MAX_CLIENTS], NULL, handle_client, client_socket_ptr) != 0) {
            perror("Error creating thread");
            free(client_socket_ptr);
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}
