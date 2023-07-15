#include "packet.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>

#include <sys/socket.h>
#include <unistd.h>

bool process_packet(MESSAGE_PACKET *message) {
    std::cout << message->msg << std::endl;
    return true;
}

bool receive_packet(SOCKET_FD sockfd, MESSAGE_PACKET* receivedMessage) {
    int msgSize;
    if (recv(sockfd, &msgSize, sizeof(msgSize), 0) < 0) {
        printf("ERROR RECEIVING PACKET\n\n");
        close(sockfd);
        return false;
    }

    char *msgBuffer = new char[msgSize + 1];
    msgBuffer[msgSize] = '\0';
    if (recv(sockfd, msgBuffer, msgSize, 0) < 0) {
        printf("ERROR RECEIVING PACKET\n\n");
        close(sockfd);
        return false;
    }

    receivedMessage->msg = std::string(msgBuffer);

    delete[] msgBuffer;
    
    return true;
}

void send_packet(SOCKET_FD *sockfd, MESSAGE_PACKET* message) {
    int msgSize = message->msg.size();

	send(*sockfd, &msgSize, sizeof(msgSize), 0);
	send(*sockfd, message->msg.c_str(), msgSize, 0);
	return;
}