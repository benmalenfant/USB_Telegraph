#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <vector>
#include <thread>

#define PORT 8888
#define MAX_CLIENTS 10

#define RCV_TIMEOUT_MS 200
#define SEND_TIMEOUT_MS 700

struct ClientData
{
    SOCKET clientSocket;
    sockaddr_in clientAddr;
    std::thread *clientThread;
    bool alive;
};

std::vector<ClientData> clientList;

SOCKET initTCPSocket(int port);

int update_connections(SOCKET listenSocket);

int clientThreadHandler(ClientData *clientData, int state);

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

int main()
{
    SOCKET sockfd = initTCPSocket(PORT);

    while (1)
    {
        update_connections(sockfd);

        for (size_t i = 0; i < clientList.size(); i++)
        {
            if (clientList.at(i).alive)
            {
                clientList.at(i).clientThread = new std::thread(clientThreadHandler, &clientList.at(i),1);
            }
            else
            {
                closesocket(clientList.at(i).clientSocket);
                clientList.erase(clientList.begin() + i);
                printf("KILLINGGG\n");
            }
        }

        for (size_t i = 0; i < clientList.size(); i++)
        {
            clientList.at(i).clientThread->join();
        }

        //printf("Sockets : %d\n", clientList.size());
        //Sleep(1000); // Adjust the sleep duration as needed
    }

    closesocket(sockfd);
    WSACleanup();
    return (0);
}

int clientThreadHandler(ClientData *clientData, int state)
{
    char buffer[2];
    buffer[0] = 0;
    int bytesSent = send(clientData->clientSocket, (char *)&buffer, 1, 0);
    if (bytesSent == -1)
    {
        std::cerr << "send failed: " << GetLastErrorAsString() << "\n";
        clientData->alive = false;
        return (-1);
    }

    int bytesReceived = recv(clientData->clientSocket, (char *)&buffer, 1, 0);
    if (bytesReceived == -1)
    {
        std::cerr << "recv failed: " << GetLastErrorAsString() << "\n";
        clientData->alive = false;
        return (-1);
    }
    return (0);
}

int update_connections(SOCKET listenSocket)
{
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listenSocket, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0; // 5 seconds
    timeout.tv_usec = 0;

    // Wait for incoming connection
    int activity = select(0, &readfds, NULL, NULL, &timeout);
    if (activity == SOCKET_ERROR)
    {
        std::cerr << "Socket error\n";
        closesocket(listenSocket);
        WSACleanup();
        return -1;
    }
    else if (activity > 0)
    {
        SOCKET clientSocket = accept(listenSocket, reinterpret_cast<SOCKADDR *>(&clientAddr), &addrLen);
        if (clientSocket != INVALID_SOCKET)
        {
            std::cout << "New connection accepted." << std::endl;

            int timeout = RCV_TIMEOUT_MS;
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeout), sizeof(timeout));
            timeout = SEND_TIMEOUT_MS;
            setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&timeout), sizeof(timeout));

            ClientData clientData;
            clientData.clientSocket = clientSocket;
            clientData.clientAddr = clientAddr;
            clientList.push_back(clientData);
        }
        else
        {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK)
            {
                std::cerr << "Accept failed with error: " << error << std::endl;
                return (-1);
            }
        }
    }
    return (0);
}

SOCKET initTCPSocket(int port)
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return INVALID_SOCKET;
    }

    // Create a TCP socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Set up the server address struct
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    // Bind the socket to the address and port
    if (bind(sockfd, reinterpret_cast<SOCKADDR *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    /* // Set the socket to non-blocking mode
     u_long mode = 1;
     if (ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR)
     {
         std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << std::endl;
         closesocket(sockfd);
         WSACleanup();
         return 1;
     }
 */
    // Start listening for incoming connections
    if (listen(sockfd, MAX_CLIENTS) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return sockfd;
}