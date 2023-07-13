#include <iostream>
#include <cstdint>
#include <cstring>

#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "client.hpp"

#define SA struct sockaddr

/** Formato da mensagem que o cliente troca com o servidor
 * 
 */
typedef struct {
    int msgSize;
    char msg[4097];
} MessagePacket;



/** conecta ao servidor e retorna o socket fd
 *  @param ip  ip do servidor
 *  @param port porta do servidor
 * 
 *  @return socket fd ou -1 em caso de erro
 */
int connect_server(const std::string ip, uint16_t port) {
    // cria socket
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        // se falhar, retorna -1
        std::cout << "Socket creation failed..." << std::endl;
        return -1;
    }
    // bind do socket
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    servaddr.sin_port = htons(port);

    // tenta conectar ao servidor
    if (connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) != 0) {
        // se falhar, retorna -1
        std::cout << "Connection with the server failed..." << std::endl;
        return -1;
    }

    return sockfd;
}

/** verifica se uma string é um número
 *  @param str string a ser verificada
 *  @return true se for um número, false caso contrário
 */
bool is_number(std::string str) {
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

void print_help() {
    std::cout << "Usage: ./client <host> <port>" << std::endl;
    std::cout << "host: server ip address" << std::endl;
    std::cout << "port: server port number" << std::endl;
}

bool processPacket(MessagePacket *message) {
    std::cout << message->msg << std::endl;
    return true;
}


void send_message(int socket, MessagePacket* message) {
	int messageSize = strlen(message->msg);
	send(socket, (char*)&messageSize, sizeof(int), 0);
	send(socket, (char*)&message->msg, messageSize, 0);

	return;
}

void client_handler(int sockfd) {
    MessagePacket buffer;

	while (1) {
        if (recv(sockfd, (char*) &buffer.msgSize, sizeof(buffer.msgSize), 0) < 0) {
            printf("SERVER DISCONNECT\n\n");
            close(sockfd);
            break;
        }

        memset(buffer.msg, 0, sizeof(buffer.msg));

		if (recv(sockfd, (char*) &buffer.msg, buffer.msgSize, 0) < 0) {
            printf("SERVER DISCONNECT\n\n");
            close(sockfd);
            break;
		}
        
		processPacket(&buffer);
	}
    close(sockfd);
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
    
    int sockfd = connect_server(ip, port);
    if (sockfd == -1) {
        std::cout << "Failed to connect to server" << std::endl;
        return 1;
    }

    std::thread client_thread(client_handler, sockfd);

    // loop principal, lê mensagens do usuário e envia para o servidor
    while (true) {
        MessagePacket packet;
        
        std::string message;
        std::getline(std::cin, message);

        packet.msgSize = message.size();
        strcpy(packet.msg, message.c_str());

        send_message(sockfd, &packet);
        sleep(1);
    }
    

    return 0;
}