#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include <list>
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
// std::vector<SOCKET_FD> 	connections;
// std::list<Client>       clients;
std::vector<Channel> 	rooms;
std::map<int, Client>   clients;
int						currGeneratedID = 0;

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

void messageHandler(int userID, std::string msg) {
	std::vector<std::string> splittedMessage = stringSplit(msg, " ");

	if (splittedMessage[0] == "/ping") {
		MESSAGE_PACKET messageP("pong");
        send_packet(&clients[userID].connection_s, &messageP);
		return;
	}

	MESSAGE_PACKET messageP(std::to_string(userID) + ": " + msg);

	// manda pra todos na rede (menos pro autor da mensagem)
	for (auto &currUser : clients) {
        int currID = currUser.first;

        Client currClient = currUser.second;

        if (currID == userID || currClient.connection_s == UNUSED_SOCKET)
            continue;
		
        send_packet(&currClient.connection_s, &messageP);
	}
}

void handle_client(int userID) {
    MESSAGE_PACKET message;
    while (server.is_running) {
        std::cout << userID << " waiting for message on socket " << clients[userID].connection_s << std::endl;

        if (!receive_packet(clients[userID].connection_s, &message)) {
            std::cout << userID << ": Error receiving message" << std::endl;
            break;
        }
        
        std::cout << "Received message from " << userID <<": " + message.msg << std::endl;

		messageHandler(userID, message.msg);
    }
    close(clients[userID].connection_s);
    clients.erase(userID);
}

#define DEFAULT_PORT 8080

int main(int argc, char* argv[]) {
    uint16_t port = DEFAULT_PORT;

    if (argc == 2) {
        port = atoi(argv[1]);
    }

    server = Server("127.0.0.1", port);

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

        clients[currGeneratedID] = new_client;
		// connections.push_back(new_connection_s);

        std::thread t(handle_client, currGeneratedID);
        t.detach();

		currGeneratedID++;
    }

    close(server.socket_fd);
	close(new_connection_s);
    server.is_running = false;
    return 0;
}