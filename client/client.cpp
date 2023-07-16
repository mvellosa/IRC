#include "client.hpp"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <csignal>

#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/socket.h>

#include "packet.hpp"

int connect_server(const std::string ip, uint16_t port) {
    // cria socket
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        // se falhar, retorna -1
        std::cout << "Socket creation failed..." << std::endl;
        return -1;
    }
    // bind do socket
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    servaddr.sin_port = htons(port);

    // tenta conectar ao servidor
    if (connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) != 0) {
        // se falhar, retorna -1
        std::cout << "Connection with the server failed..." << std::endl;
        return -1;
    }

    return sockfd;
}

void client_handler(SOCKET_FD sockfd) {
    MESSAGE_PACKET buffer;

	while (1) {
        if(!receive_packet(sockfd, &buffer)) {
            break;
        }
        
        std::cout << buffer.msg << std::endl;
	}
    close(sockfd);
}
