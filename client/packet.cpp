#include "packet.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>

#include <sys/socket.h>
#include <unistd.h>

bool receive_packet(SOCKET_FD sockfd, MESSAGE_PACKET* buffer) {
    int msgSize;
    if (recv(sockfd, &msgSize, sizeof(int), 0) < 0) {
        std::cout << "ERROR RECEIVING PACKET\n" << std::endl;
        close(sockfd);
        return false;
    }

    char *msgBuffer = new char[msgSize + 1];
    msgBuffer[msgSize] = '\0';

    if (recv(sockfd, msgBuffer, msgSize, 0) < 0) {
        std::cout << "ERROR RECEIVING PACKET\n" << std::endl;
        close(sockfd);
        return false;
    }

    buffer->msg = std::string(msgBuffer);

    delete[] msgBuffer;
    
    return true;
}

void send_packet(SOCKET_FD sockfd, MESSAGE_PACKET* message) {
    int msgSize = message->msg.size();
	send(sockfd, &msgSize, sizeof(msgSize), 0);
	send(sockfd, message->msg.c_str(), msgSize, 0);

	return;
}