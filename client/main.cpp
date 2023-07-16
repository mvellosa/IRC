#include <iostream>
#include <cstring>

#include <thread>
#include <unistd.h>
#include <csignal>
#include <vector>

#include "client.hpp"
#include "packet.hpp"

bool client_connected = false;
SOCKET_FD client_s = UNUSED_SOCKET;

/** verifica se uma string é um número
 *  @param str string a ser verificada
 *  @return true se for um número, false caso contrário
 */
bool is_number(std::string str) {
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

void print_help(void) {
    std::cout << "Available commands:\n"
              << "/connect <host> <port>\n"
              << "/disconnect\n"
              << "/help\n"
              << "/quit\n";
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    if (signum == SIGINT) {
        std::cout << "Ignoring Signal..." << std::endl;
        return;
    }

    exit(signum);  
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

bool process_message(MESSAGE_PACKET *message) {
    // split message
    std::vector<std::string> splittedMessage = stringSplit(message->msg, " ");
    
    if (splittedMessage[0] == "/help") {
        print_help();
        return true;
    }

    if (splittedMessage[0] == "/connect") {
        if (splittedMessage.size() != 3) {
            std::cout << "Usage: /connect <host> <port>" << std::endl;
            return false;
        }

        if (client_connected) {
            std::cout << "Already connected to a server" << std::endl;
            return false;
        }

        std::string ip = splittedMessage[1];
        uint16_t port;
        if (!is_number(splittedMessage[2]) || atoi(splittedMessage[2].c_str()) > 0xFFFF) {
            std::cout << "Invalid port number.\nHas to be a valid number in range (0, 65535)" << std::endl;
            return false;
        }
        port = atoi(splittedMessage[2].c_str());

        SOCKET_FD sockfd = connect_server(ip, port);
        if (sockfd == -1) {
            std::cout << "Failed to connect to server" << std::endl;
            return false;
        }

        client_connected = true;
        client_s = sockfd;

        std::cout << "Connected to server" << ip << ":" << port << "\n" << std::endl;
        std::thread connection_thread(client_handler, sockfd);
        connection_thread.detach();

        return true;
    }

    if (splittedMessage[0] == "/disconnect") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        send_packet(client_s, message);

        close(client_s);
        client_connected = false;

        return true;
    }

    // if (splittedMessage[0] == "/ping") {
    //     if (!client_connected) {
    //         std::cout << "Not connected to a server" << std::endl;
    //         return false;
    //     }

    //     send_packet(client_s, message);
    //     return true;
    // }

    if (splittedMessage[0] == "/quit") {
        if (client_connected) {
            send_packet(client_s, message);

            close(client_s);
            client_connected = false;
            sleep(1);
        }

        exit(EXIT_SUCCESS);
        return true;
    }

    // caso onde nenhum comando foi reconhecido
    // if (splittedMessage[0].at(0) == '/') {
    //     std::cout << "Invalid command" << std::endl;
    //     return false;
    // }

    // caso onde não é um comando, apenas uma mensagem
    if (!client_connected || client_s == UNUSED_SOCKET) {
        std::cout << "Not connected to a server" << std::endl;
        return false;
    }

    send_packet(client_s, message);

    return true;
}

int main(void) {
    signal(SIGINT, signalHandler);

    std::cout << "Welcome to ICMC-connect, connect to a server to start.\n" 
     "Type /help to see the available commands" << std::endl;
    // loop principal, lê mensagens do usuário e envia para o servidor
    while (true) {
        
        std::string message;
        std::getline(std::cin, message);

        MESSAGE_PACKET packet(message);  

        process_message(&packet);
        sleep(1);
    }

    return 0;
}