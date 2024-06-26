#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <windows.h>

#define SERVER_PORT 8888

std::string GetLastErrorAsString()
{
    DWORD errorMessageID = GetLastError();
    if (errorMessageID == 0)
        return std::string(); // No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);

    // Free the buffer.
    LocalFree(messageBuffer);

    return message;
}


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
    char buffer[2];

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

    if (inet_pton(AF_INET, argv[2], &server.sin_addr.s_addr) == 0) {
        fprintf(stderr, "Invalid address\n");
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    // Connect to remote server
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("connect error");
        return 1;
    }

    int timeout1 = 700;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeout1), sizeof(timeout1));
    timeout1 = 700;
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&timeout1), sizeof(timeout1));

    printf("Connected to server\n");

    // Open COM port
    com_port = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (com_port == INVALID_HANDLE_VALUE)
    {
        printf("Error opening COM port\n");
        return 1;
    }

    DCB serialParams;
    SecureZeroMemory(&serialParams, sizeof(DCB));
    serialParams.DCBlength = sizeof(DCB);

    BOOL fSuccess = GetCommState(com_port, &serialParams);

    if (!fSuccess) 
    {
      //  Handle the error.
      printf ("GetCommState failed with error %d.\n", GetLastError());
      return (-1);
    }
    serialParams.BaudRate = CBR_9600;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    serialParams.fDtrControl=1;
    fSuccess = SetCommState(com_port, &serialParams);

    if (!fSuccess) 
    {
      //  Handle the error.
      printf ("SetCommState failed with error %d.\n", GetLastError());
      return (-1);
    }
    

    // Set timeouts
    COMMTIMEOUTS timeout = {0};
    timeout.ReadIntervalTimeout = 10;
    timeout.ReadTotalTimeoutConstant = 10;
    timeout.ReadTotalTimeoutMultiplier = 10;
    timeout.WriteTotalTimeoutConstant = 10;
    timeout.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(com_port, &timeout);

    while (1)
    {
        if (recv(client_socket, (char*)&buffer, 1, 0) < 0)
        {
            std::cerr << "Recv from server failed: " << GetLastErrorAsString() << "\n";
            return(-1);
        }

        DWORD bytesWritten;
        if (!WriteFile(com_port, &buffer, 1, &bytesWritten, NULL))
        {
            printf("COM Send failed\n");
            return(-1);
        }

        DWORD bytesRead;
        //PurgeComm(com_port,PURGE_RXCLEAR);
        if (!ReadFile(com_port, (char*)buffer, 1, &bytesRead, NULL))
        {
            printf("Error reading from COM port\n");
            return(-1);
        }
        if(bytesRead == 0){
            printf("Error reading from COM port\n");
            return(-1);
        }

        int bytesSent = send(client_socket, (char*)&buffer, 1, 0);
        if(bytesSent == -1){
            printf("SEND ERROR\n");
            return(-1);
        }
    }

    // Close the socket and COM port
    closesocket(client_socket);
    CloseHandle(com_port);
    WSACleanup();

    return 0;
}
