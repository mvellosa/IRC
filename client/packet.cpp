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

bool receive_packet(SOCKET_FD sockfd, MESSAGE_PACKET* buffer) {
    if (recv(sockfd, &buffer->msgSize, sizeof(buffer->msgSize), 0) < 0) {
        printf("SERVER DISCONNECT\n\n");
        close(sockfd);
        return false;
    }

    memset(buffer->msg, 0, sizeof(buffer->msg));

    if (recv(sockfd, buffer->msg, buffer->msgSize, 0) < 0) {
        printf("SERVER DISCONNECT\n\n");
        close(sockfd);
        return false;
    }
    
    return true;
}

void send_packet(SOCKET_FD sockfd, MESSAGE_PACKET* message) {
	send(sockfd, &message->msgSize, sizeof(message->msgSize), 0);
	send(sockfd, message->msg, message->msgSize, 0);

	return;
}