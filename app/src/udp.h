#ifndef UDP_H
#define UDP_H

#include <winsock2.h>
#include <ws2tcpip.h>

class Udpclient
{
public:
    Udpclient();
    ~Udpclient();

    bool initialize();
    bool send(const PhysicsData &data, int size); // sendto function wrapper

private:
    SOCKET clientSocket;
    sockaddr_in address;
};

#endif