#include "packet.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <vector>

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

#define MAX_MESSAGE_SIZE 4096

void send_packet(SOCKET_FD sockfd, MESSAGE_PACKET* message) {
    int msgSize = message->msg.size();

    if (msgSize == 0) return;
    
    // se a mensagem for um comando e for maior que o tamanho maximo, nao envia
    if (msgSize > MAX_MESSAGE_SIZE && message->msg.at(0) == '/') {
        std::cout << "MESSAGE NOT SENT. Command too large." << std::endl;
        return;
    }

    // se a mensagem for maior que o tamanho maximo, divide ela em pedaços
    if (msgSize > MAX_MESSAGE_SIZE) {
        std::string messageCpy = message->msg;

        std::vector<std::string> splitMsg;
        while (messageCpy.size() > MAX_MESSAGE_SIZE) {
            splitMsg.push_back(messageCpy.substr(0, MAX_MESSAGE_SIZE));
            messageCpy = messageCpy.substr(MAX_MESSAGE_SIZE);
        }

        for (auto &sliceMsg : splitMsg) {
            int tamanhoSlice = sliceMsg.size();
            send(sockfd, &tamanhoSlice, sizeof(int), 0);
            send(sockfd, sliceMsg.c_str(), tamanhoSlice, 0);
        }
        return;
    }
	
    // se a mensagem for "normal", apenas envia ela
    send(sockfd, &msgSize, sizeof(msgSize), 0);
	send(sockfd, message->msg.c_str(), msgSize, 0);

	return;
}