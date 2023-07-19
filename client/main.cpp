#include <iostream>
#include <cstring>

#include <thread>
#include <unistd.h>
#include <csignal>
#include <vector>
#include <map>
#include <mutex>

#include "client.hpp"
#include "packet.hpp"

bool client_connected = false;
std::mutex connection_mutex;
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

/** imprime uma mensagem na tela
 * @param message mensagem a ser impressa
 */
void show_message(std::string message) {
    std::cout << "\t" + message + "\n" << std::endl;
}

/** inicializa o mapa de comandos */
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

/** imprime a mensagem de ajuda */
void print_help(void) {
    show_message("Available commands:");

    for (auto &currentCommand : commands_info) {
        std::cout << "\t" << currentCommand.second.name << " - " << currentCommand.second.description << std::endl;
    }
}

/** divide uma string com base em um delimitador
 * @param str string a ser dividida
 * @param delimiter delimitador
 * @return vetor de strings com as partes da string original
 */
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
            show_message("Disconnected from server");

            client_connected = false;
            break;
        }
        
        show_message(buffer.msg);
	}

    close(sockfd);
}

bool process_message(MESSAGE_PACKET *message) {
    // split message
    std::vector<std::string> splittedMessage = stringSplit(message->msg, " ");

    std::string notConnectedMessage = "You are not connected to a server";
    std::string notInRoomMessage = "You are not in a chat room";
    std::string wrongUsageBaseMessage = "Incorrect Usage\nUsage: ";
    
    if (splittedMessage[0] == "/help") {
        print_help();
        return true;
    }

    if (splittedMessage[0] == "/connect") {
        if (splittedMessage.size() != 3) {
            show_message(wrongUsageBaseMessage + commands_info["/connect"].usage);
            return false;
        }

        if (client_connected) {
            show_message("Already connected to a server");
            return false;
        }

        std::string ip = splittedMessage[1];
        uint16_t port;
        if (!is_number(splittedMessage[2]) || atoi(splittedMessage[2].c_str()) > 0xFFFF) {
            show_message("Invalid port number.\nHas to be a valid integer smaller than 65535");
            return false;
        }
        port = atoi(splittedMessage[2].c_str());

        SOCKET_FD sockfd = connect_server(ip, port);
        if (sockfd == -1) {
            show_message("Failed to connect to server");
            return false;
        }

        client_connected = true;
        client_s = sockfd;

        show_message("Connected to server " + ip + ":" + std::to_string(port));
        std::thread connection_thread(client_handler, sockfd);
        connection_thread.detach();

        return true;
    }

    if (splittedMessage[0] == "/disconnect") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        send_packet(client_s, message);

        return true;
    }

    if (splittedMessage[0] == "/ping") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/nickname") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        if (splittedMessage.size() != 2) {
            show_message(wrongUsageBaseMessage + commands_info["/nickname"].usage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/join") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        if (splittedMessage.size() != 2) {
            show_message(wrongUsageBaseMessage + commands_info["/join"].usage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/leave") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/mute") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        if (splittedMessage.size() != 2) {
            show_message(wrongUsageBaseMessage + commands_info["/mute"].usage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/unmute") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        if (splittedMessage.size() != 2) {
            show_message(wrongUsageBaseMessage + commands_info["/unmute"].usage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/kick") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        if (splittedMessage.size() != 2) {
            show_message(wrongUsageBaseMessage + commands_info["/kick"].usage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/whois") {
        if (!client_connected) {
            show_message(notConnectedMessage);
            return false;
        }

        if (splittedMessage.size() != 2) {
            show_message(wrongUsageBaseMessage + commands_info["/whois"].usage);
            return false;
        }

        send_packet(client_s, message);
        return true;
    }

    if (splittedMessage[0] == "/quit") {
        if (client_connected) {
            send_packet(client_s, message);
        }

        int retry = 0;
        while (retry < 5) {
            if (client_connected) {
                sleep(1);
            } else {
                break;
            }
        }

        if (client_connected) {
            show_message("Failed to disconnect from server, unsafe quitting.");
        }

        exit(EXIT_SUCCESS);
        return true;
    }

    // caso onde o usuário digitou um comando inválido
    if (splittedMessage[0].at(0) == '/') {
        show_message("Invalid command.");
        return false;
    }

    // caso onde não é um comando, apenas uma mensagem
    if (!client_connected || client_s == UNUSED_SOCKET) {
        show_message(notConnectedMessage);
        return false;
    }

    send_packet(client_s, message);

    return true;
}

void signalHandler(int signum) {
    show_message("Interrupt signal (" + std::to_string(signum) + ") received.");

    if (signum == SIGINT) {
        show_message("Ignoring Signal...");
        return;
    }

    exit(signum);  
}

int main(void) {
    // registra o handler para o sinal SIGINT
    signal(SIGINT, signalHandler);

    init_commands_info();

    show_message("Welcome to ICMC-connect, connect to a server to start.");
    show_message("Type /help to see the available commands");
    
    // loop principal, lê mensagens do usuário e envia para o servidor
    while (true) {
        
        std::string message;
        std::getline(std::cin, message);
        std::cout << std::endl;

        // caso o usuário tenha apertado ctrl+d
        if (std::cin.eof()) {
            // por ter o mesmo efeito de /quit, é tratado da mesma forma
            MESSAGE_PACKET packet("/quit");
            process_message(&packet);
            continue;
        }

        MESSAGE_PACKET packet(message);  

        process_message(&packet);
    }

    return 0;
}