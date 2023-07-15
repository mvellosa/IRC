#pragma once

#include <string>

#include "defines.hpp"
#include <thread>
#include <vector>

class Client {
    public:
    std::string username;
    std::string ip;
    uint16_t port;
    SOCKET_FD connection_s;
    std::thread handler_thread;

    Client(){};
};

class Channel {
    public:

    std::string name;
    std::vector<SOCKET_FD> client_sockets;
    uint32_t max_clients;
    bool is_invite_only;
};


