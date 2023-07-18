#pragma once

#include <string>

#include "defines.hpp"

/** conecta ao servidor e retorna o socket fd
 *  @param ip  ip do servidor
 *  @param port porta do servidor
 * 
 *  @return socket fd ou -1 em caso de erro
 */
int connect_server(const std::string ip, uint16_t port);

// void client_handler(SOCKET_FD sockfd);