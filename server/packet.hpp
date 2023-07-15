#pragma once

#include <cstdint>
#include <string>

#include "defines.hpp"

/** estrutura de um pacote
 */
struct MESSAGE_PACKET{
    std::string msg;

    MESSAGE_PACKET() {}
    MESSAGE_PACKET(std::string message) {
        this->msg = message;
    }
};

/** processa um pacote
 *  @param message pacote a ser processado
 *  @return true se o pacote foi processado com sucesso, false caso contrário
 */
bool process_packet(MESSAGE_PACKET *message);

/** recebe um pacote
 *  @param sockfd socket fd
 *  @param buffer buffer para armazenar o pacote recebido
 *  @return true se o pacote foi recebido com sucesso, false caso contrário
 */
bool receive_packet(SOCKET_FD sockfd, MESSAGE_PACKET* buffer);

/** envia um pacote
 *  @param socket socket fd
 *  @param message pacote a ser enviado
 */
void send_packet(SOCKET_FD *sockfd, MESSAGE_PACKET* message);