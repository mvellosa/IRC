#include <iostream>
#include <cstdint>
#include <cstring>

#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "server.hpp"
#include "packet.hpp"
#include "defines.hpp"

#define SA struct sockaddr

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <server_ip> <server_port>" << std::endl;
        // return 1;
    }

    std::string server_ip = "127.0.0.1";
    uint16_t server_port = 8080;

    struct sockaddr_in serverAddr;
    serverAddr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    serverAddr.sin_port = htons(server_port);
    serverAddr.sin_family = AF_INET;

    SOCKET_FD sListener = socket(AF_INET, SOCK_STREAM, 0);  
    if (sListener < 0) {
        std::cout << "Error creating socket" << std::endl;
        return 1;
    }

    if (bind(sListener, (SA*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Error binding socket" << std::endl;
        return 1;
    }

    if (listen(sListener, 1) < 0) {
        std::cout << "Error listening socket" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port " << server_port << std::endl;

    int connection_s;
    while (true) {
        connection_s = accept(sListener, NULL, NULL);
        if (connection_s < 0) {
            std::cout << "Error accepting client" << std::endl;
            return 1;
        }

        std::cout << "Client connected" << std::endl;

        MESSAGE_PACKET message;
        while (true) {
            if (receive_packet(connection_s, &message) < 0) {
                std::cout << "Error receiving message" << std::endl;
                return 1;
            }
            
            std::cout << "Received message: " << message.msg << std::endl;

            process_packet(&message);

            // echo na mensagem
            send_packet(connection_s, &message);
            memset(message.msg, 0, sizeof(message.msg));
        }
    }

    return 0;
}