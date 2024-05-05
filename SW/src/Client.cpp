#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

#define SERVER_PORT 8888

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <COM port> <server IP address>\n", argv[0]);
        return 1;
    }

    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    HANDLE com_port;
    char buffer[2], recv_buffer;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_addr.s_addr = inet_addr(argv[2]);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    // Connect to remote server
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("connect error");
        return 1;
    }

    printf("Connected to server\n");

    // Open COM port
    com_port = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (com_port == INVALID_HANDLE_VALUE)
    {
        printf("Error opening COM port\n");
        return 1;
    }

    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    GetCommState(com_port, &serialParams);
    serialParams.BaudRate = 115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    SetCommState(com_port, &serialParams);

    // Set timeouts
    COMMTIMEOUTS timeout = {0};
    timeout.ReadIntervalTimeout = 200;
    timeout.ReadTotalTimeoutConstant = 200;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(com_port, &timeout);

    while (1)
    {
        if (recv(client_socket, &recv_buffer, 1, 0) < 0)
        {
            printf("recv failed\n");
            return 1;
        }

        DWORD bytesWritten;
        if (!WriteFile(com_port, &recv_buffer, 1, &bytesWritten, NULL))
        {
            printf("COM Send failed\n");
        }

        DWORD bytesRead;
        PurgeComm(com_port,PURGE_RXCLEAR);
        if (!ReadFile(com_port, &buffer, 1, &bytesRead, NULL))
        {
            printf("Error reading from COM port\n");
        }
        DWORD err = GetLastError();
        printf("Read byte from COM port: %d\n", buffer[0]);
        Sleep(500);
    }

    // Close the socket and COM port
    closesocket(client_socket);
    CloseHandle(com_port);
    WSACleanup();

    return 0;
}
