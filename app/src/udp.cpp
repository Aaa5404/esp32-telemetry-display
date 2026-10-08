// #define _WIN32_WINNT 0x0600

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 50000
#define RECEIVER_IP ""
#include <tchar.h>
#include "../common/protocol.h"
#include "udp.h"
using namespace std;

Udpclient::Udpclient()
{
    clientSocket = INVALID_SOCKET;
}

Udpclient::~Udpclient()
{
    if (clientSocket != INVALID_SOCKET)
    {
        closesocket(clientSocket);
    }

    WSACleanup();
}

bool Udpclient::initialize()
{
    // Load dll
    WSADATA wsaData; // struct containing settings for windows sockets
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaerr != 0)
    {
        cout << "Winsock dll not found" << endl;
        return false;
    }
    else
    {
        cout << "Winsock dll found" << endl;
        cout << "Status:" << wsaData.szSystemStatus << endl;
    }
    // Create socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // sock_dgram for udp
    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }
    else
    {
        cout << "Socket is ok" << endl;
    }

    // Set destination address
    address.sin_family = AF_INET;
    InetPton(AF_INET, _T(RECEIVER_IP), &address.sin_addr.s_addr);
    address.sin_port = htons(PORT);

    return true;
}
bool Udpclient::send(const PhysicsData &data, int size)
{
    int bytesSent;

    bytesSent = sendto(clientSocket, (const char *)&data, size, 0, (struct sockaddr *)&address, sizeof(address));

    if (bytesSent == -1)
    {
        cout << "Error transmitting data" << endl;
        return false;
    }

    // cout << "Packet sent, Size:" << bytesSent << endl;

    return true;
}
