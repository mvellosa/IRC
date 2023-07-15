#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

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

Server server; // instancia global do servidor

void handle_client(Client *client) {
    MESSAGE_PACKET message;
    while (server.is_running) {
        if (!receive_packet(client->connection_s, &message)) {
            std::cout << "Error receiving message" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Received message: " << message.msg << std::endl;

        process_packet(&message);

        // echo na mensagem
        send_packet(client->connection_s, &message);
        memset(message.msg, 0, sizeof(message.msg));
    }
    close(client->connection_s);
}


int main(void) {
    server = Server("127.0.0.1", 8080);

    if (server.socket_fd < 0) {
        std::cout << "Error creating server socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << server.port << std::endl;

    server.is_running = true;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(sockaddr_in);

        Client new_client;

        new_client.connection_s = accept(server.socket_fd, (SA*)&client_addr, &client_addr_size);

        if (new_client.connection_s == -1) {
            std::cout << "Error accepting client" << std::endl;
            exit(EXIT_FAILURE);
        }

        new_client.ip = inet_ntoa(client_addr.sin_addr);
        new_client.port = ntohs(client_addr.sin_port);

        std::cout << "Client "<< new_client.ip <<" connected on port "<< new_client.port << std::endl;

        new_client.handler_thread = std::thread(handle_client, &new_client);
        new_client.handler_thread.detach();

        // TODO: adicionar cliente na lista de clientes do servidor
        // server.clients.push_back(new_client); // ? não funciona
    }

    close(server.socket_fd);
    server.is_running = false;
    return 0;
}