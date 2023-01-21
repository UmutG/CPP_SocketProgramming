/ @author: Umut Geyik | github.com/UmutG
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define PORT "1234"
#define DATA_LEN 64

class Client {
public:
    void StartClient();
};


int main() {
    
    Client client;
    client.StartClient();
}

void Client::StartClient() {
    printf("Client-side initializing...\n");

    WSADATA wsaData;
    SOCKET SocketConnection = INVALID_SOCKET;
    struct addrinfo* result = NULL, *ptr = NULL, hints;
    int value;
    char buffer [] = "Hello";
    char recv_buffer[DATA_LEN] = "";

    // WSAStartup function is called to initiate use of WS2_32.dll.
    value = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (value != 0) {
        printf("WSAStartup failed: %d\n", value);
        exit(-1);
    }

    printf("WSAStartup successful!\n");

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    value = getaddrinfo(NULL, PORT, &hints, &result);
    if (value != 0) {
        printf("Error at getting address: %d\n", value);
        WSACleanup();
        exit(0);
    }

    // Client's workflow: socket() -> connect() -> send() -> recv()
    // Finally, client shutdowns and closes all sockets.
    
    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        SocketConnection = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (SocketConnection == INVALID_SOCKET) {
            printf("Error at socket: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(-1);
        }

        // Connect to server.
        value = connect(SocketConnection, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (value == SOCKET_ERROR) {
            closesocket(SocketConnection);
            SocketConnection = INVALID_SOCKET;
            continue;
        }
        break;
    }

    printf("Socket opened!\n");

    freeaddrinfo(result);
    if (SocketConnection == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        exit(-1);
    }

    printf("Connection successful!\n");

    value = send(SocketConnection, buffer, 5, 0);
    if (value == SOCKET_ERROR) {
        closesocket(SocketConnection);
        printf("Failed sending the data: %d\n", WSAGetLastError());
        WSACleanup();
        exit(-1);
    }

    else printf("Data sent to server!\n");

    value = shutdown(SocketConnection, SD_SEND);
    if (value == SOCKET_ERROR) {
        printf("Shutdown failed: %d\n", WSAGetLastError());
        closesocket(SocketConnection);
        WSACleanup();
        exit(-1);
    }

    else printf("Shutting down socket...\nWaiting response from server...\n");

    do {
        value = recv(SocketConnection, recv_buffer, DATA_LEN, 0);
        if (value > 0) printf("Data has received:\n%s\n", recv_buffer);
        else if (value == 0) printf("Connection has closed.\n");
        else printf("Data did not receive: %d\n", WSAGetLastError());
    } while (value > 0);


    closesocket(SocketConnection);
    WSACleanup();
    
}
