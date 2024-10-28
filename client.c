#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    char ip[50];
    int port;
} Client;

void register_client(int sock) {
    Client client;
    printf("Enter username: ");
    scanf("%s", client.username);
    printf("Enter password: ");
    scanf("%s", client.password);
    printf("Enter IP address: ");
    scanf("%s", client.ip);
    printf("Enter port: ");
    scanf("%d", &client.port);

    send(sock, &client, sizeof(Client), 0);

    char buffer[BUFFER_SIZE];
    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);
}

void receive_file(int sock) {
    char receiver_username[50], password[50], filename[100];
    char buffer[BUFFER_SIZE];
    FILE *fp;

    printf("Enter your username: ");
    scanf("%s", receiver_username);
    printf("Enter your password: ");
    scanf("%s", password);
    printf("Enter filename to receive: ");
    scanf("%s", filename);

    // Send receiver username, password, and filename to server
    send(sock, receiver_username, sizeof(receiver_username), 0);
    send(sock, password, sizeof(password), 0);
    send(sock, filename, sizeof(filename), 0);

    // Open file to save received data
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("Error opening file to save received data.\n");
        return;
    }

    // Receive file data and write to file
    while (1) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) break;
        fwrite(buffer, sizeof(char), bytes_received, fp);
    }
    fclose(fp);

    printf("File %s received and saved successfully.\n", filename);
}


void request_client_list(int sock) {
    char buffer[BUFFER_SIZE];
    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);
}

void send_file(int sock) {
    char sender_username[50], password[50], receiver_username[50], filename[100];
    char buffer[BUFFER_SIZE];
    FILE *fp;

    printf("Enter your username: ");
    scanf("%s", sender_username);
    printf("Enter your password: ");
    scanf("%s", password);
    printf("Enter receiver's username: ");
    scanf("%s", receiver_username);
    printf("Enter filename to send: ");
    scanf("%s", filename);

    send(sock, sender_username, sizeof(sender_username), 0);
    send(sock, password, sizeof(password), 0);
    send(sock, receiver_username, sizeof(receiver_username), 0);
    send(sock, filename, sizeof(filename), 0);

    // Send file data
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("File not found.\n");
        return;
    }

    while (1) {
        int bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
        if (bytes_read == 0) break;
        send(sock, buffer, bytes_read, 0);
    }
    fclose(fp);
    printf("File %s sent.\n", filename);
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char option;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("Choose option:\n1. Register\n2. List Clients\n3. Send File\n");
    scanf(" %c", &option);
    send(sock, &option, sizeof(option), 0);

    switch (option) {
        case '1':
            register_client(sock);
            break;
        case '2':
            request_client_list(sock);
            break;
        case '3':
            send_file(sock);
            break;
        case '4':
            receive_file(sock);
            break;
        default:
            printf("Invalid option.\n");
    }

    close(sock);
    return 0;
}
