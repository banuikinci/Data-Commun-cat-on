#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_LENGTH 1024

SOCKET socketId;

void sendMessage() {
    char buffer[BUFFER_LENGTH];
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Yeni satır karakterini sil
        if (strcmp(buffer, "quit") == 0) {
            send(socketId, "quit", 4, 0);  // "quit" komutu gönder
            break;
        }
        send(socketId, buffer, strlen(buffer), 0);
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return EXIT_FAILURE;
    }

    socketId = socket(AF_INET, SOCK_STREAM, 0);
    if (socketId == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        WSACleanup();
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Sunucu IP'si
    serverAddr.sin_port = htons(PORT);

    if (connect(socketId, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Connection failed.\n");
        closesocket(socketId);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Connected to the server.\n");

    // Kullanıcıdan isim alınır
    char name[50];
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';  // Yeni satır karakterini sil
    send(socketId, name, strlen(name), 0);  // Sunucuya ismini gönder

    // Mesaj göndermek için
    sendMessage();

    closesocket(socketId);
    WSACleanup();
    return 0;
}