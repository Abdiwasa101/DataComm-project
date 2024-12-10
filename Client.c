#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h> 

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

DWORD WINAPI handleServerCommunication(LPVOID param);

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], serverReply[BUFFER_SIZE];
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server.sin_family = AF_INET;
    server.sin_port = htons(8080); 

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to server.\n");

    // Create a thread for handling server communication
    DWORD threadId;
    HANDLE threadHandle = CreateThread(NULL, 0, handleServerCommunication, (LPVOID)&sock, 0, &threadId);

    if (threadHandle == NULL) {
        printf("Failed to create thread. Error: %d\n", GetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    while (1) {
        printf("\nEnter a command (GET <City>, ADD <City, Weather>, UPDATE <City, Weather>, or exit):\n");
        fgets(message, sizeof(message), stdin);

        
        message[strcspn(message, "\n")] = 0;

        // Send message to server
        if (send(sock, message, strlen(message), 0) == SOCKET_ERROR) {
            printf("Send failed. Error Code: %d\n", WSAGetLastError());
            break;
        }

        // Exit if the user types "exit"
        if (strncmp(message, "exit", 4) == 0) {
            break;
        }
    }

    // Clean up and close the socket
    closesocket(sock);
    WSACleanup();
    return 0;
}

// Thread function to handle receiving data from the server
DWORD WINAPI handleServerCommunication(LPVOID param) {
    SOCKET sock = *(SOCKET *)param;
    char serverReply[BUFFER_SIZE];

    while (1) {
        memset(serverReply, 0, BUFFER_SIZE);
        int recvSize = recv(sock, serverReply, sizeof(serverReply), 0);
        if (recvSize == SOCKET_ERROR) {
            printf("Recv failed. Error Code: %d\n", WSAGetLastError());
            break;
        }

        if (recvSize == 0) {
            printf("Server disconnected.\n");
            break;
        }

        serverReply[recvSize] = '\0'; 
        printf("\nServer reply: %s\n", serverReply);
    }

    return 0;
}
