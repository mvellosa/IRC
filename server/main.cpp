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



int main(void) {
    std::string server_ip = "127.0.0.1";
    uint16_t server_port = 8080;

    SOCKET_FD sListener = create_server_socket(server_ip, server_port);
    
    if (sListener < 0) {
        std::cout << "Error creating server socket" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port " << server_port << std::endl;

    SOCKET_FD connection_s;
    while (true) {
        connection_s = accept(sListener, NULL, NULL);
        if (connection_s < 0) {
            std::cout << "Error accepting client" << std::endl;
            return 1;
        }

        std::cout << "Client connected" << std::endl;

        MESSAGE_PACKET message;
        while (true) {
            if (!receive_packet(connection_s, &message)) {
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