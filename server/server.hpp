#pragma once

#include <string>

#include "defines.hpp"

SOCKET_FD create_server_socket(const std::string ip, uint16_t port);