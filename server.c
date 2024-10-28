#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    char ip[50];
    int port;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

int authenticate_client(char *username, char *password) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, username) == 0 &&
            strcmp(clients[i].password, password) == 0) {
            return i;
        }
    }
    return -1;
}

void register_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    Client client;

    // Receive registration details
    recv(client_socket, &client, sizeof(Client), 0);

    // Store client details
    clients[client_count++] = client;
    snprintf(buffer, BUFFER_SIZE, "Client %s registered successfully!\n", client.username);
    send(client_socket, buffer, strlen(buffer), 0);
}

void send_client_list(int client_socket) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "Registered clients:\n");

    // Prepare and send the client list
    for (int i = 0; i < client_count; i++) {
        char client_info[100];
        snprintf(client_info, sizeof(client_info), "%d. %s (%s:%d)\n",
                 i + 1, clients[i].username, clients[i].ip, clients[i].port);
        strcat(buffer, client_info);
    }
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_file_transfer(int client_socket) {
    char sender_username[50], password[50], receiver_username[50], filename[100];
    char buffer[BUFFER_SIZE];
    FILE *fp;

    // Receive sender, receiver, and password
    recv(client_socket, sender_username, sizeof(sender_username), 0);
    recv(client_socket, password, sizeof(password), 0);
    recv(client_socket, receiver_username, sizeof(receiver_username), 0);

    int sender_index = authenticate_client(sender_username, password);
    if (sender_index == -1) {
        snprintf(buffer, BUFFER_SIZE, "Authentication failed for %s.\n", sender_username);
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }

    // Receive filename and open file for writing
    recv(client_socket, filename, sizeof(filename), 0);
    fp = fopen(filename, "wb");

    // Receive file data and save it
    while (1) {
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        fwrite(buffer, sizeof(char), bytes_read, fp);
    }
    fclose(fp);
    snprintf(buffer, BUFFER_SIZE, "File %s received and saved.\n", filename);
    send(client_socket, buffer, strlen(buffer), 0);
}
void handle_file_request(int client_socket) {
    char receiver_username[50], password[50], filename[100], buffer[BUFFER_SIZE];
    FILE *fp;

    // Receive receiver's username, password, and filename
    recv(client_socket, receiver_username, sizeof(receiver_username), 0);
    recv(client_socket, password, sizeof(password), 0);
    recv(client_socket, filename, sizeof(filename), 0);

    // Authenticate receiver
    int receiver_index = authenticate_client(receiver_username, password);
    if (receiver_index == -1) {
        snprintf(buffer, BUFFER_SIZE, "Authentication failed for %s.\n", receiver_username);
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }

    // Check if file exists
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        snprintf(buffer, BUFFER_SIZE, "File %s not found.\n", filename);
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }

    // Send file data to the client
    while (1) {
        int bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
        if (bytes_read == 0) break;
        send(client_socket, buffer, bytes_read, 0);
    }
    fclose(fp);

    // Notify client that file has been sent
    snprintf(buffer, BUFFER_SIZE, "\nFile %s received successfully.\n", filename);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_client(int client_socket) {
    char option;
    recv(client_socket, &option, sizeof(option), 0);

    switch (option) {
        case '1':
            register_client(client_socket);
            break;
        case '2':
            send_client_list(client_socket);
            break;
        case '3':
            handle_file_transfer(client_socket);
            break;
        case '4':
            handle_file_request(client_socket);
            break;
        default:
            break;
    }
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind and listen
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server started on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        handle_client(client_socket);
    }
    return 0;
}
