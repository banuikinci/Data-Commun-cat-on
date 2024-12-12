#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_LENGTH 1024
#define MAX_CLIENTS 10
#define NAME_LENGTH 50

SOCKET clientSockets[MAX_CLIENTS];
char clientNames[MAX_CLIENTS][NAME_LENGTH];
int clientCount = 0;

void broadcastMessage(const char* message, SOCKET senderSocket) {
    for (int i = 0; i < clientCount; i++) {
        if (clientSockets[i] != senderSocket) {
            send(clientSockets[i], message, strlen(message), 0);
        }
    }
}

DWORD WINAPI handleClient(LPVOID clientSocket) {
    SOCKET socket = *(SOCKET*)clientSocket;
    char buffer[BUFFER_LENGTH];

    // Yeni gelen istemciden kullanıcı adını al
    send(socket, "Enter your name: ", 17, 0); // Kullanıcı adı sormak için mesaj gönder
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(socket, buffer, sizeof(buffer), 0); // Kullanıcı adını al
    buffer[strcspn(buffer, "\n")] = '\0'; // Yeni satır karakterini temizle

    if (bytesReceived > 0) {
        // İstemcinin ismini kaydet
        snprintf(clientNames[clientCount], sizeof(clientNames[clientCount]), "%s", buffer);
        const char* connectedMessage = "You are connected to the server.\n";
        send(socket, connectedMessage, strlen(connectedMessage), 0);
        printf("%s has connected.\n", buffer);
    }

    // İstemci bağlantısını kabul ettikten sonra mesajları al
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("%s disconnected.\n", clientNames[clientCount]);
            break;
        }

        // Gelen mesajı ekrana yazdır
        printf("%s: %s\n", clientNames[clientCount], buffer);

        // Mesajı diğer tüm istemcilere ilet
        broadcastMessage(buffer, socket);
    }

    // İstemci bağlantısını kapat
    closesocket(socket);
    for (int i = 0; i < clientCount; i++) {
        if (clientSockets[i] == socket) {
            for (int j = i; j < clientCount - 1; j++) {
                clientSockets[j] = clientSockets[j + 1];
                snprintf(clientNames[j], sizeof(clientNames[j]), "%s", clientNames[j + 1]);
            }
            clientCount--;
            break;
        }
    }

    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return EXIT_FAILURE;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        WSACleanup();
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Socket binding failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        fprintf(stderr, "Socket listening failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            fprintf(stderr, "Client connection failed.\n");
            continue;
        }

        printf("New client connected.\n");

        if (clientCount < MAX_CLIENTS) {
            clientSockets[clientCount++] = clientSocket;
            HANDLE thread = CreateThread(NULL, 0, handleClient, &clientSocket, 0, NULL);
            if (thread == NULL) {
                fprintf(stderr, "Failed to create thread for client.\n");
            }
        } else {
            const char* fullMessage = "Server is full. Try again later.\n";
            send(clientSocket, fullMessage, strlen(fullMessage), 0);
            closesocket(clientSocket);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}