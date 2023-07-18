#include <iostream>
#include <cstring>

#include <thread>
#include <unistd.h>
#include <csignal>
#include <vector>
#include <map>

#include "client.hpp"
#include "packet.hpp"

bool client_connected = false;
SOCKET_FD client_s = UNUSED_SOCKET;

struct Command {
    std::string name;
    std::string description;
    std::string usage;

    Command() {
        this->name = "";
        this->description = "";
        this->usage = "";
    }
    Command(std::string name, std::string description, std::string usage) {
        this->name = name;
        this->description = description;
        this->usage = usage;
    }
};

std::map<std::string, Command> commands_info;

/** verifica se uma string é um número
 *  @param str string a ser verificada
 *  @return true se for um número, false caso contrário
 */
bool is_number(std::string str) {
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

void init_commands_info() {
    commands_info["/help"] = Command("/help", "Show this help message", "/help");
    commands_info["/connect"] = Command("/connect", "Connect to a server", "/connect <host> <port>");
    commands_info["/disconnect"] = Command("/disconnect", "Disconnect from the server", "/disconnect");
    commands_info["/ping"] = Command("/ping", "Ping the server and waits for pong", "/ping");
    commands_info["/join"] = Command("/join", "Join a chat room", "/join <room_name>");
    commands_info["/leave"] = Command("/leave", "Leave the current chat room", "/leave");
    commands_info["/nickname"] = Command("/nickname", "Change your nickname", "/nickname <new_nickname>");
    commands_info["/kick"] = Command("/kick", "Kick a user in the chat room (admin only)", "/kick <target_username>");
    commands_info["/mute"] = Command("/mute", "Mute a user in the chat room (admin only)", "/mute <target_username>");
    commands_info["/unmute"] = Command("/unmute", "Unmute a muted user in the chat room (admin only)", "/unmute <target_username>");
    commands_info["/whois"] = Command("/whois", "Show information about a user in the chat room (admin only)", "/whois <username>");
    commands_info["/quit"] = Command("/quit", "Quit the program", "/quit");
}

void print_help(void) {

    std::cout << "Available commands:" << std::endl;

    for (auto &currentCommand : commands_info) {
        std::cout << currentCommand.second.name << " - " << currentCommand.second.description << std::endl;
    }
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

void client_handler(SOCKET_FD sockfd) {
    MESSAGE_PACKET buffer;

	while (client_connected) {
        if (!receive_packet(sockfd, &buffer)) {
            break;
        }

        if (buffer.msg == "DISCONNECTED") {
            std::cout << "Disconnected from server" << std::endl;
            client_connected = false;
            break;
        }
        
        std::cout << buffer.msg << std::endl;
	}
    close(sockfd);
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

        MESSAGE_PACKET confirmation;
        receive_packet(client_s, &confirmation); // wait for disconnect confirmation

        return true;
    }

    if (splittedMessage[0] == "/ping") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/join") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        if (splittedMessage.size() != 2) {
            std::cout << "Usage: /join <room_name>" << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/leave") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/mute") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        if (splittedMessage.size() != 2) {
            std::cout << commands_info["/mute"].usage << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/unmute") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        if (splittedMessage.size() != 2) {
            std::cout << commands_info["/unmute"].usage << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/kick") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        if (splittedMessage.size() != 2) {
            std::cout << "Usage: /kick <nickname>" << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/whois") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        if (splittedMessage.size() != 2) {
            std::cout << "Usage: /whois <nickname>" << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/quit") {
        if (client_connected) {
            send_packet(client_s, message);

            MESSAGE_PACKET confirmation;
            receive_packet(client_s, &confirmation); // wait for disconnect confirmation

            if (confirmation.msg != "DISCONNECTED") {
                std::cout << "Failed to disconnect from server" << std::endl;
                std::cout << "received: " << confirmation.msg << std::endl;
            }

            std::cout << "Disconnected from server\n" << std::endl;

            close(client_s);
            client_connected = false;
            client_s = UNUSED_SOCKET;
        }

        exit(EXIT_SUCCESS);
        return true;
    }

    if (splittedMessage[0] == "/nickname") {
        if (!client_connected) {
            std::cout << "Not connected to a server" << std::endl;
            return false;
        }

        if (splittedMessage.size() != 2) {
            std::cout << "Usage: /nickname <nickname>" << std::endl;
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    // caso onde nenhum comando foi reconhecido
    if (splittedMessage[0].at(0) == '/') {
        std::cout << "Invalid command" << std::endl;
        return false;
    }

    // caso onde não é um comando, apenas uma mensagem
    if (!client_connected || client_s == UNUSED_SOCKET) {
        std::cout << "Not connected to a server" << std::endl;
        return false;
    }

    send_packet(client_s, message);

    return true;
}

int main(void) {
    // registra o handler para o sinal SIGINT
    signal(SIGINT, signalHandler);

    init_commands_info();

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