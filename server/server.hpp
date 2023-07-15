#pragma once

#include <string>
#include <vector>

#include "defines.hpp"
// #include "client.hpp"
#include "channel.hpp"

class Server {
    public:

    std::string ip;
    uint16_t port;
    SOCKET_FD socket_fd;
    std::vector<Channel> channels;
    std::vector<Client> clients;
    bool is_running = false;

    Server(){};
    Server(std::string ip, uint16_t port);
    
};


// SOCKET_FD create_server_socket(const std::string ip, uint16_t port);