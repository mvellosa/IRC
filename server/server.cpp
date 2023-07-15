#include "server.hpp"

#include <iostream>
#include <string>
#include <cstdint>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "packet.hpp"

Server::Server(std::string ip, uint16_t port) {
    this->ip = ip;
    this->port = port;
    this->is_running = false;
    this->socket_fd = -1;
    this->channels = std::vector<Channel>();
    this->clients = std::vector<Client>();

    struct sockaddr_in serverAddr;
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddr.sin_port = htons(port);
    serverAddr.sin_family = AF_INET;

    SOCKET_FD sListener = socket(AF_INET, SOCK_STREAM, 0);  
    if (sListener < 0) {
        std::cout << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (bind(sListener, (SA*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Error binding socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(sListener, 10) < 0) {
        std::cout << "Error listening socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->socket_fd = sListener;
}