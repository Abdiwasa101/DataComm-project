#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct WeatherData {
    char city[50];
    char weather[50];
} WeatherData;

WeatherData weatherDB[100];
int weatherCount = 5;

const char *defaultWeather[][2] = {
    {"Bosaso", "Sunny, 25°C"},
    {"Borame ", "Cloudy, 22°C"},
    {"Karabuk", "Rainy, 18°C"},
    {"Berbera", "Hot, 30°C"},
    {"Phoenix", "Dry, 35°C"}
};

// Function to initialize default weather data
void initializeWeatherDB() {
    for (int i = 0; i < weatherCount; i++) {
        strcpy(weatherDB[i].city, defaultWeather[i][0]);
        strcpy(weatherDB[i].weather, defaultWeather[i][1]);
    }
}

// Function to handle each client connection
unsigned __stdcall handle_client(void *client_socket) {
    int socket = *(int *)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;

        buffer[strcspn(buffer, "\n")] = 0; 
        printf("Client sent: %s\n", buffer);

        if (strncmp(buffer, "exit", 4) == 0) {
            snprintf(response, BUFFER_SIZE, "Goodbye!");
            send(socket, response, strlen(response), 0);
            break;
        }

        char command[20], city[50], weather[50];
        int parsed = sscanf(buffer, "%s %49[^,], %49[^\n]", command, city, weather);

        if (strcmp(command, "GET") == 0 && parsed == 2) {
            // Search for the city
            int found = 0;
            for (int i = 0; i < weatherCount; i++) {
                if (strcasecmp(city, weatherDB[i].city) == 0) {
                    snprintf(response, BUFFER_SIZE, "Weather in %s: %s", weatherDB[i].city, weatherDB[i].weather);
                    found = 1;
                    break;
                }
            }
            if (!found) snprintf(response, BUFFER_SIZE, "Weather data for '%s' not found.", city);
        } else if (strcmp(command, "ADD") == 0 && parsed == 3) {
            strcpy(weatherDB[weatherCount].city, city);
            strcpy(weatherDB[weatherCount].weather, weather);
            weatherCount++;
            snprintf(response, BUFFER_SIZE, "Weather data added for %s.", city);
        } else if (strcmp(command, "UPDATE") == 0 && parsed == 3) {
            int updated = 0;
            for (int i = 0; i < weatherCount; i++) {
                if (strcasecmp(city, weatherDB[i].city) == 0) {
                    strcpy(weatherDB[i].weather, weather);
                    updated = 1;
                    snprintf(response, BUFFER_SIZE, "Weather data updated for %s.", city);
                    break;
                }
            }
            if (!updated) snprintf(response, BUFFER_SIZE, "No existing data for '%s'. Use ADD instead.", city);
        } else {
            snprintf(response, BUFFER_SIZE, "Invalid command. Use GET <City>, ADD <City, Weather>, or UPDATE <City, Weather>.");
        }

        send(socket, response, strlen(response), 0);
    }

    printf("Client disconnected.\n");
    closesocket(socket);
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_len = sizeof(client_address);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error\n");
        return 1;
    }

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed with error\n");
        WSACleanup();
        return 1;
    }

    // Set up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Bind failed with error\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen failed with error\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    printf("Server is listening on port %d...\n", PORT);

    initializeWeatherDB();

    // Accept and handle clients
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed with error\n");
            continue;
        }
        printf("New client connected.\n");

        int *client_sock = malloc(sizeof(int));
        *client_sock = client_socket;
        _beginthreadex(NULL, 0, handle_client, client_sock, 0, NULL);
    }

    // Cleanup
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
