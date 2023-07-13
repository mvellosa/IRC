#include <iostream>
#include <cstdint>
#include <cstring>

#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "server.hpp"

#define SA struct sockaddr

/** Formato da mensagem que o cliente troca com o servidor
 * 
 */
typedef struct {
    int msgSize;
    char msg[4097];
} MessagePacket;

bool processPacket(MessagePacket message) {
    std::cout << message.msg << std::endl;
    return true;
}


void send_message(int socket, MessagePacket* message) {
	int messageSize = strlen(message->msg);
	send(socket, (char*)&messageSize, sizeof(int), 0);
	send(socket, (char*)&message->msg, messageSize, 0);

	return;
}

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

    int sListener = socket(AF_INET, SOCK_STREAM, 0);  
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

        MessagePacket message;
        while (true) {
            int bytes = recv(connection_s, (char*)&message.msgSize, sizeof(int), 0);
            if (bytes < 0) {
                std::cout << "Error receiving message" << std::endl;
                return 1;
            }

            bytes = recv(connection_s, (char*)&message.msg, message.msgSize, 0);
            message.msg[message.msgSize] = '\0';

            if (bytes < 0) {
                std::cout << "Error receiving message" << std::endl;
                return 1;
            }
            if (bytes == 0) {
                std::cout << "Client disconnected" << std::endl;
                break;
            }

            std::cout << "Received message: " << message.msg << std::endl;

            processPacket(message);

            // echo na mensagem
            send_message(connection_s, &message);
            memset(message.msg, 0, sizeof(message.msg));
        }
    }

    return 0;
}