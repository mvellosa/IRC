#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <mutex>

#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "server.hpp"
#include "packet.hpp"
#include "defines.hpp"
#include "channel.hpp"

#define SA struct sockaddr

Server 					server; // instancia global do servidor
std::vector<SOCKET_FD> 	connections;
std::vector<Channel> 	rooms;
int						userCounter = 0;

std::string getCurrentTime() {
	std::time_t currTime = std::time(0);
	char timeStr[50];
	std::strftime(timeStr, sizeof(timeStr), "%F, %T", std::localtime(&currTime));

	return std::string(timeStr);
}

std::vector<std::string> stringSplit(std::string str, std::string delimiter) {
	size_t posStart = 0;
	size_t posEnd;
	std::vector<std::string> splitted;

	while ((posEnd = str.find(delimiter, posStart)) != std::string::npos) {
		splitted.push_back(str.substr(posStart, posEnd - posStart));
		posStart = posEnd + delimiter.length();
	}
	splitted.push_back(str.substr(posStart));

	return splitted;
}

enum Packet {
	pChatMessage,
	pServerMessage,
	pWarningMessage
};

void messageHandler(int userID, std::string msg) {
	std::vector<std::string> splittedMessage = stringSplit(msg, " ");
	MESSAGE_PACKET messageP;

	if (splittedMessage[0] == "/ping") {
		messageP = MESSAGE_PACKET("pong");
		send_packet(&connections[userID], &messageP);
		return;
	}

	messageP = MESSAGE_PACKET(std::to_string(userID) + ": " + msg);

	// manda pra todos na rede (menos pro autor da mensagem)
	for (int currUser = 0; currUser < (int) connections.size(); currUser++) {
		
		if (currUser == userID || connections[currUser] == UNUSED_SOCKET) // autor da mensagem
			continue;
		
		send_packet(&connections[currUser], &messageP);
	}
}
void handle_client(int userID) {
    MESSAGE_PACKET message;
    while (server.is_running) {
        if (!receive_packet(connections[userID], &message)) {
            std::cout << "Error receiving message" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Received message: " << message.msg << std::endl;

		messageHandler(userID, message.msg);
    }
    close(connections[userID]);
}

int main(void) {
    server = Server("127.0.0.1", 8080);

    if (server.socket_fd < 0) {
        std::cout << "Error creating server socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << server.port << std::endl;

    server.is_running = true;

    SOCKET_FD new_connection_s;
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(sockaddr_in);


        new_connection_s = accept(server.socket_fd, (SA*)&client_addr, &client_addr_size);

        if (new_connection_s == -1) {
            std::cout << "Error accepting client" << std::endl;
            continue;
        }

        Client new_client(new_connection_s, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        std::cout << "Client "<< new_client.ip <<" connected on port "<< new_client.port << std::endl;

        new_client.handler_thread = std::thread(handle_client, connections.size());
        new_client.handler_thread.detach();

		connections.push_back(new_connection_s);
		userCounter++;

        // TODO: adicionar cliente na lista de clientes do servidor
        // server.clients.push_back(new_client); // ? não funciona
    }

    close(server.socket_fd);
	close(new_connection_s);
    server.is_running = false;
    return 0;
}