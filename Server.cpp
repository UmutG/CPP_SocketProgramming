// @author: Umut Geyik | github.com/UmutG
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <string>
#include <iostream>

// Link with ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

// Define constants
#define PORT "1234"
#define BACKLOG 3
#define DATA_LEN 64

using namespace std;

// Required functions
class Server {
public:
    void StartServer();
    string getTime();
};


int main() {
    Server server;
    server.StartServer();
}

 string Server::getTime() {
    struct tm newtime;
    time_t now = time(0);
    localtime_s(&newtime, &now);
    short hour = newtime.tm_hour;
    short min = newtime.tm_min;
    short sec = newtime.tm_sec;
    string myString = "\t" + to_string(hour) + ":" + to_string(min) + ":" + to_string(sec);
    return myString;
}


void Server::StartServer() {
    printf("Server-side initializing...\n");
    // Windows Sockets implementation
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo addr;
    int value, new_value;
    char recv_buffer[DATA_LEN] = "";

    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_protocol = IPPROTO_TCP;
    addr.ai_flags = AI_PASSIVE;

    // Initialize Winsock => generate the versioning word expected by WSAStartup
    value = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (value != 0) {
        printf("WSAStartup has failed: %d\n", value);
        exit(-1);
    }

    ZeroMemory(&addr, sizeof(addr));

    value = getaddrinfo(NULL, PORT, &addr, &result);
    if (value != 0) {
        printf("Error at getting address: %d\n", value);
        WSACleanup();
        exit(0);
    }

    // Server's workflow: socket() -> bind() -> listen() -> accept() -> recv() -> send()
    // Finally, server shutdowns and closes all sockets.

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == SOCKET_ERROR) {
        printf("Error at socket: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(-1);
    }

    printf("Socket opened!\n");

    value = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (value == SOCKET_ERROR) {
        printf("Error at bind: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(-1);
    }

    printf("Bind successful!\n");
    freeaddrinfo(result);

    value = listen(ListenSocket, BACKLOG);
    if (value == SOCKET_ERROR) {
        printf("Listening has failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        exit(-1);
    }

    else printf("Server is listening...\n");
    SOCKET socketForClient;
    socketForClient = accept(ListenSocket, NULL, NULL);
    if (socketForClient == SOCKET_ERROR) {
        closesocket(ListenSocket);
        printf("Server cannot accept client: %d", WSAGetLastError());
        WSACleanup();
        exit(-1);
    }
    else printf("Connection accepted!\n");
    closesocket(ListenSocket);


    do {
        value = recv(socketForClient, recv_buffer, DATA_LEN, 0);

        if (value > 0) {
            cout << "Data has received:\n" << recv_buffer << "\n";
            string new_buffer = recv_buffer + getTime();
            int len = sizeof(new_buffer);
            new_value = send(socketForClient, new_buffer.c_str(), len, 0);
            if (new_value == SOCKET_ERROR) {
                closesocket(socketForClient);
                printf("Data send failed: %ld\n", WSAGetLastError());
                WSACleanup();
                exit(-1);
            }
            printf("Data has sent to client!\n");
        }

        else if (value == 0) 
            printf("Closing the connection...");

        else {
            closesocket(socketForClient);
            closesocket(ListenSocket);
            printf("Data receive failed: %d\n", WSAGetLastError());
            WSACleanup();
            exit(-1);
        }

    } while (value > 0);

    value = shutdown(socketForClient, SD_SEND);
    if (value == SOCKET_ERROR) {
        printf("Shutdown failed: %d\n", WSAGetLastError());
        closesocket(socketForClient);
        WSACleanup();
        exit(-1);
    }

    else printf("Shutting down sockets...\n");
    closesocket(socketForClient);

    printf("Shutting down server...\n");
    WSACleanup();
}
