#include "server.hpp"

#include <iostream>
#include <string>
#include <cstdint>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "defines.hpp"

SOCKET_FD create_server_socket(const std::string ip, uint16_t port) {
    struct sockaddr_in serverAddr;
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddr.sin_port = htons(port);
    serverAddr.sin_family = AF_INET;

    SOCKET_FD sListener = socket(AF_INET, SOCK_STREAM, 0);  
    if (sListener < 0) {
        std::cout << "Error creating socket" << std::endl;
        return -1;
    }

    if (bind(sListener, (SA*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Error binding socket" << std::endl;
        return -1;
    }

    if (listen(sListener, 10) < 0) {
        std::cout << "Error listening socket" << std::endl;
        return -1;
    }

    return sListener;
}