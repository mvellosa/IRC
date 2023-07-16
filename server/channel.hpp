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

    Client(){};
    Client(SOCKET_FD connection_s, std::string ip, uint16_t port) {
        this->username = username;
        this->ip = ip;
        this->port = port;
        this->connection_s = connection_s;
    }
};

class Channel {
    public:

    std::string name;
    std::string password;
    bool is_invite_only;
    std::vector<int> users;

    Channel(){}
    Channel(std::string name, std::string password) {
        this->name = name;
        this->password = password;
        this->is_invite_only = false;
    }
    Channel(std::string name, std::string password, bool is_invite_only) {
        this->name = name;
        this->password = password;
        this->is_invite_only = is_invite_only;
    }
    // uint32_t max_users;
};


