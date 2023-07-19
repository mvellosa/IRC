#pragma once

#include <string>
#include <vector>

#include "defines.hpp"
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