#include <iostream>
#include <cstring>

#include <thread>
#include <unistd.h>

#include "client.hpp"
#include "packet.hpp"

/** verifica se uma string é um número
 *  @param str string a ser verificada
 *  @return true se for um número, false caso contrário
 */
bool is_number(std::string str) {
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

/** imprime a mensagem de ajuda para o usuário */
void print_help() {
    std::cout << "Usage: ./client <host> <port>" << std::endl;
    std::cout << "host: server ip address" << std::endl;
    std::cout << "port: server port number" << std::endl;
}



int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    
    // verifica se a porta é válida
    uint16_t port;
    if (!is_number(argv[2]) || atoi(argv[2]) > 0xFFFF) {
        std::cout << "Invalid port number.\nHas to be a valid number in range (0, 65535)" << std::endl;
        return 1;
    }
    port = atoi(argv[2]);

    std::cout << "Connecting to " << ip << ":" << port << std::endl;
    
    SOCKET_FD sockfd = connect_server(ip, port);
    if (sockfd == -1) {
        std::cout << "Failed to connect to server" << std::endl;
        return 1;
    }

    std::thread client_thread(client_handler, sockfd);

    // loop principal, lê mensagens do usuário e envia para o servidor
    while (true) {
        MESSAGE_PACKET packet;
        
        std::string message;
        std::getline(std::cin, message);

        packet.msgSize = message.size();
        strcpy(packet.msg, message.c_str());

        send_packet(sockfd, &packet);
        sleep(1);
    }
    

    return 0;
}