#pragma once

#include <string>

#include "defines.hpp"
#include <thread>
#include <vector>

class Client {
    public:
    std::string connectedRoomName;
    std::string nickname;
    std::string ip;
    uint16_t port;
    SOCKET_FD connection_s;

    Client(){};
    Client(SOCKET_FD connection_s, std::string ip, uint16_t port) {
        this->nickname = "";
        this->connectedRoomName = "";
        this->ip = ip;
        this->port = port;
        this->connection_s = connection_s;
    }
        
};

class Channel {
    public:

    int adminID;
    std::string name;
    bool is_invite_only;
    std::vector<int> users;
    std::vector<int> muted_users;

    Channel(){}
    Channel(std::string name) {
        this->name = name;
        this->is_invite_only = false;
    }
    Channel(std::string name, bool is_invite_only) {
        this->name = name;
        this->is_invite_only = is_invite_only;
    }
};


