#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include <list>
#include <mutex>
#include <algorithm>

#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <csignal>

#include "server.hpp"
#include "packet.hpp"
#include "defines.hpp"
#include "channel.hpp"

#define SA struct sockaddr

Server 					        server; // instancia global do servidor

std::map<std::string, Channel> 	channels;
std::mutex                      channels_mutex;

std::map<int, Client>           clients;
std::mutex                      clients_mutex;

int						        currGeneratedID = 0;

#define DEFAULT_PORT 8080
#define MAX_NICKNAME_SIZE 50

#define END_CONNECTION -1

#define MAX_CHANNEL_NAME_SIZE 200
#define BELL_CHAR 7

bool is_number(std::string str) {
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

/** Verifica se o nome do canal segue a especificação do RFC 1459
 * @param channelName nome do canal a ser verificado
 * @return true se for um nome de canal válido, false caso contrário
 */
bool validate_channel_name(std::string channelName) {
    if (channelName.size() > MAX_CHANNEL_NAME_SIZE || channelName.size() < 2) return false;

    if (channelName.at(0) != '#' && channelName.at(0) != '&') return false;

    for (char c : channelName) {
        if (c == BELL_CHAR || c == ',' || c == ' ') return false;
    }

    return true;
}

std::vector<std::string> stringSplit(std::string str, std::string delimiter) {
	size_t posStart = 0;
	size_t posEnd;
	std::vector<std::string> splitted;

	while ((posEnd = str.find(delimiter, posStart)) != std::string::npos) {
		splitted.push_back(str.substr(posStart, posEnd - posStart));
		posStart = posEnd + delimiter.length();
	}
	splitted.push_back(str.substr(posStart));

	return splitted;
}

void send_packet_channel(std::string channelName, int userID, std::string message) {
    MESSAGE_PACKET messageP;
    if (userID < 0) {
        messageP = MESSAGE_PACKET("SERVER: " + message);
    } else {
        messageP = MESSAGE_PACKET(clients[userID].nickname + ": " + message);
    }
    
	// manda mensagem para todos os clientes conectados no canal
    for (int user : channels[channelName].users) {
        if (user == userID) continue;
        send_packet(&clients[user].connection_s, &messageP);
    }
}

/** Envia uma mensagem para todos os usuários conectados no servidor
 *  @param userID ID do usuário que enviou a mensagem
 *  @param msg mensagem a ser enviada
 *  @return 0 se nada a mais precisa ser feito ou um código se precisar
 */
int message_controller(int userID, std::string msg) {
	std::vector<std::string> splittedMessage = stringSplit(msg, " ");

    std::lock_guard<std::mutex> client_lock(clients_mutex);
    std::lock_guard<std::mutex> channel_lock(channels_mutex);

    Client &senderUser = clients[userID];

    // verifica se o usuário está conectado a algum canal
	if (splittedMessage[0] == "/ping") {
		MESSAGE_PACKET messageP("SERVER: pong");
        send_packet(&senderUser.connection_s, &messageP);
		return 0;
	}

    // muda o nickname do usuário
    if (splittedMessage[0] == "/nickname") {
        if (splittedMessage.size() < 2) {
            MESSAGE_PACKET messageP("Usage: /nickname <new_nickname>");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // usuário não pode mudar o nickname se estiver conectado a um canal
        if (senderUser.connectedRoomName != "") {
            MESSAGE_PACKET messageP("You must leave the channel before changing your nickname");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // verifica se o novo nickname é válido
        std::string newNickname = splittedMessage[1];
        if (newNickname.size() > MAX_NICKNAME_SIZE) {
            MESSAGE_PACKET messageP("Nickname too long");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // verifica se o novo nickname já está em uso
        for (auto &client : clients) {
            if (client.second.nickname == newNickname) {
                MESSAGE_PACKET messageP("Nickname already in use");
                send_packet(&senderUser.connection_s, &messageP);
                return 0;
            }
        }

        senderUser.nickname = newNickname;

        MESSAGE_PACKET messageP("Nickname changed to " + newNickname);
        send_packet(&senderUser.connection_s, &messageP);
        return 0;
    }

    // entra em um canal ou cria ele se não existir
    if (splittedMessage[0] == "/join") {
        if (splittedMessage.size() < 2) {
            MESSAGE_PACKET messageP("Usage: /join <channel_name>");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        if (senderUser.nickname == "") {
            MESSAGE_PACKET messageP("You must set a nickname before joining a channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        if (senderUser.connectedRoomName != "") {
            MESSAGE_PACKET messageP("You are already connected to a channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        std::string channelName = splittedMessage[1];

        if (!validate_channel_name(channelName)) {
            MESSAGE_PACKET messageP("Invalid channel name");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // verifica se o canal existe, se existir conecta o usuário
        if (channels.find(channelName) != channels.end()) {
            
            senderUser.connectedRoomName = channelName;
            channels[channelName].users.push_back(userID);

            MESSAGE_PACKET messageP("Joined channel " + channelName);
            send_packet(&senderUser.connection_s, &messageP);

            send_packet_channel(channelName, userID, "joined the channel");
            return 0;
        }

        // se não existir, cria o canal e conecta o usuário
        Channel newChannel(channelName);
        newChannel.adminID = userID;

        channels[channelName] = newChannel;
        channels[channelName].users.push_back(userID);

        senderUser.connectedRoomName = channelName;

        MESSAGE_PACKET messageP("Created and joined channel " + channelName);
        send_packet(&senderUser.connection_s, &messageP);

        return 0;
    }

    // sai do canal em que o usuário está conectado
    if (splittedMessage[0] == "/leave") {
        // verifica se o usuário está conectado a um canal
        if (senderUser.connectedRoomName == "") {
            MESSAGE_PACKET messageP("You are not connected to a channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        std::string channelName = senderUser.connectedRoomName;
    
        // remove o usuário do canal
        senderUser.connectedRoomName = "";
        channels[channelName].users.erase(std::remove(channels[channelName].users.begin(), channels[channelName].users.end(), userID),
                                          channels[channelName].users.end());
        channels[channelName].muted_users.erase(std::remove(channels[channelName].muted_users.begin(), channels[channelName].muted_users.end(), userID),
                                                channels[channelName].muted_users.end());

        MESSAGE_PACKET messageP("Left channel " + channelName);
        send_packet(&senderUser.connection_s, &messageP);

        std::string message = senderUser.nickname + " left the channel";
        send_packet_channel(channelName, -1, message);
        return 0;
    }

    // kicka um usuario da sala (admin)
    if (splittedMessage[0] == "/kick") {
        if (splittedMessage.size() < 2) {
            MESSAGE_PACKET messageP("Usage: /kick <user_nickname>");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // verifica se o usuário está conectado a algum canal
        if (senderUser.connectedRoomName == "") {
            MESSAGE_PACKET messageP("You must be connected to a channel to kick users");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        Channel &userChannel = channels[senderUser.connectedRoomName];

        // verifica se o usuário é admin da sala
        if (userID != userChannel.adminID) {
            MESSAGE_PACKET messageP("You must be the admin of the channel to kick users");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        std::string kickedUserNickname = splittedMessage[1];
        int kickedUserID = -1;
        
        // verifica se o usuário a ser kickado está na sala
        // se estiver, pega o seu ID

        for (auto &user: userChannel.users) {
            if (clients[user].nickname == kickedUserNickname) {
                kickedUserID = user;
                break;
            }
        }

        if (kickedUserID == -1) {
            MESSAGE_PACKET messageP("User " + kickedUserNickname + " is not in the channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        std::cout << "Kicking user " << clients[kickedUserID].nickname << std::endl;
        // remove o usuário do canal  
        clients[kickedUserID].connectedRoomName = "";
        
        auto& channelUsers = userChannel.users;
        channelUsers.erase(std::remove(channelUsers.begin(), channelUsers.end(), kickedUserID), channelUsers.end());
        
        auto& channelMutedUsers = userChannel.muted_users;
        channelMutedUsers.erase(std::remove(channelMutedUsers.begin(), channelMutedUsers.end(), kickedUserID), channelMutedUsers.end());

        // manda mensagem para o usuário kickado
        MESSAGE_PACKET messagePKickedUser("You were kicked from the channel " + userChannel.name);
        send_packet(&clients[kickedUserID].connection_s, &messagePKickedUser);
        
        std::string message = kickedUserNickname + " was kicked from the channel";
        send_packet_channel(userChannel.name, -1, message);
        return 0;
    }

    // muta um usuario da sala (admin)
    if (splittedMessage[0] == "/mute") {
        if (splittedMessage.size() < 2) {
            MESSAGE_PACKET messageP("Usage: /mute <user_nickname>");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // verifica se o usuário está conectado a algum canal
        if (senderUser.connectedRoomName == "") {
            MESSAGE_PACKET messageP("You must be connected to a channel to mute users");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        Channel &userChannel = channels[senderUser.connectedRoomName];

        // verifica se o usuário é admin da sala
        if (userID != userChannel.adminID) {
            MESSAGE_PACKET messageP("You must be the admin of the channel to mute users");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        std::string mutedUserNickname = splittedMessage[1];

        int mutedUserID = -1;
        for (auto &user: userChannel.users) {
            if (clients[user].nickname == mutedUserNickname) {
                mutedUserID = user;
                break;
            }
        }

        if (mutedUserID == -1) {
            MESSAGE_PACKET messageP("User " + mutedUserNickname + " is not in the channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }
        
        // muta o usuário
        userChannel.muted_users.push_back(mutedUserID);

        // manda mensagem para o usuário mutado
        MESSAGE_PACKET messagePMutedUser("You were muted");
        send_packet(&clients[mutedUserID].connection_s, &messagePMutedUser);

        std::string message = mutedUserNickname + " was muted";
        send_packet_channel(userChannel.name, -1, message);
        return 0;

    }

    // desmuta um usuario mutado da sala (admin)
    if (splittedMessage[0] == "/unmute") {
        if (splittedMessage.size() < 2) {
            MESSAGE_PACKET messageP("Usage: /unmute <user_nickname>");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // verifica se o usuário está conectado a algum canal
        if (senderUser.connectedRoomName == "") {
            MESSAGE_PACKET messageP("You are not connected to a channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        Channel &userChannel = channels[senderUser.connectedRoomName];

        // verifica se o usuário é admin da sala
        if (userID != userChannel.adminID) {
            MESSAGE_PACKET messageP("You must be the admin of the channel to unmute users");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        std::string mutedUserNickname = splittedMessage[1];

        // verifica se o usuário a ser desmutado está na sala
        // se estiver, pega o seu ID e index na lista de usuários da sala
        int mutedUserID = -1;
        for (auto &user: userChannel.users) {
            if (clients[user].nickname == mutedUserNickname) {
                mutedUserID = user;
                break;
            }
        }

        if (mutedUserID == -1) {
            MESSAGE_PACKET messageP("User " + mutedUserNickname + " is not in the channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        // desmuta o usuário
        auto& mutedUsers = userChannel.muted_users;
        mutedUsers.erase(std::remove(mutedUsers.begin(), mutedUsers.end(), mutedUserID), mutedUsers.end());

        // manda mensagem para o usuário desmutado
        MESSAGE_PACKET messageP("You are no longer muted");
        send_packet(&clients[mutedUserID].connection_s, &messageP);

        std::string message = mutedUserNickname + " was unmuted";
        send_packet_channel(userChannel.name, -1, message);
        return 0;
    }

    // mostra informações de um usuário
    if (splittedMessage[0] == "/whois") {
        if (splittedMessage.size() < 2) {
            MESSAGE_PACKET messageP("Usage: /whois <user_nickname>");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }
    
        if (senderUser.connectedRoomName == "") {
            MESSAGE_PACKET messageP("You must be connected to a channel to use this command");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        Channel userChannel = channels[senderUser.connectedRoomName];

        std::string specifiedUserNickname = splittedMessage[1];

        int specifiedUserID = -1;
        for (auto &user: userChannel.users) {
            if (clients[user].nickname == specifiedUserNickname) {
                specifiedUserID = user;
                break;
            }
        }

        // verifica se o usuário especificado está na sala
        if (specifiedUserID == -1) {
            MESSAGE_PACKET messageP("User " + specifiedUserNickname + " is not in the channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }

        Client specifiedUser = clients[specifiedUserID];

        std::string message = "User " + specifiedUser.nickname + " is connected from " + specifiedUser.ip + ":" + std::to_string(specifiedUser.port);
        MESSAGE_PACKET messageP(message);
        send_packet(&senderUser.connection_s, &messageP);
        return 0;
    }

    if (splittedMessage[0] == "/quit" || splittedMessage[0] == "/disconnect") {
        
        // verifica se o usuário está conectado a algum canal
        if (senderUser.connectedRoomName != "") {
            Channel &senderChannel = channels[senderUser.connectedRoomName];
            // verifica se o usuário é admin da sala
            if (userID == senderChannel.adminID) {
                // remove a sala
                std::string message = "Channel " + senderChannel.name + " was closed by the admin";
                send_packet_channel(senderChannel.name, -1, message);
                // remove os usuários da sala
                for (auto &user: senderChannel.users) {
                    clients[user].connectedRoomName = "";
                }

                channels.erase(senderChannel.name);
            } else {
                // remove o usuário da sala
                auto& users = senderChannel.users;
                users.erase(std::remove(users.begin(), users.end(), userID), users.end());
                // manda mensagem para os usuários da sala
                std::string message = senderUser.nickname + " left the channel";
                send_packet_channel(senderChannel.name, -1, message);
            }
        }
        
        MESSAGE_PACKET messageP("DISCONNECTED");
        send_packet(&senderUser.connection_s, &messageP);

        sleep(1); // garante que mensagem chegará antes da conexão ser encerrada

        return END_CONNECTION;
    }

    // verifica se o usuário possui um nickname
    if (senderUser.nickname == "") {
        MESSAGE_PACKET messageP("You must set a nickname before sending messages");
        send_packet(&senderUser.connection_s, &messageP);
        return 0;
    }

    // verifica se o usuário está conectado a algum canal
    if (senderUser.connectedRoomName == "") {
        MESSAGE_PACKET messageP("You must be connected to a channel to send messages");
        send_packet(&senderUser.connection_s, &messageP);
        return 0;
    }

    Channel senderChannel = channels[senderUser.connectedRoomName];
    
    // verifica se o usuário está mutado
    for (int user : senderChannel.muted_users) {
        if (user == userID) {
            MESSAGE_PACKET messageP("You are muted in this channel");
            send_packet(&senderUser.connection_s, &messageP);
            return 0;
        }
    }

    // envia mensagem para membros do canal
    send_packet_channel(senderUser.connectedRoomName, userID, msg);
    return 0;
}

/** função que lida com o cliente, chamada em uma thread separada
 *  @param userID id do usuário
 */
void handle_client(int userID) {
    MESSAGE_PACKET message;
    while (server.is_running) {

        clients_mutex.lock();
        
        std::cout << userID << " waiting for message on socket " << clients[userID].connection_s << std::endl;
        SOCKET_FD client_socket = clients[userID].connection_s;
        
        clients_mutex.unlock();

        if (!receive_packet(client_socket, &message)) { // aguarda mensagem do cliente
            std::cout << userID << ": Error receiving message" << std::endl;
            break;
        }
        
        std::cout << "Received message from " << userID <<": " + message.msg << std::endl;

        // processa a mensagem recebida
        int return_code = message_controller(userID, message.msg);

		if (return_code == END_CONNECTION) {
            break;
        }
    }
    std::lock_guard<std::mutex> client_lock(clients_mutex);

    close(clients[userID].connection_s);
    clients.erase(userID);
}

/** cuida do sinal de interrupção (ctrl+c)
 * fecha o socket do servidor e encerra o programa
 */
void signalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << "SIGINT received" << std::endl;
        std::cout << "Closing server..." << std::endl;

        server.is_running = false;

        std::lock_guard<std::mutex> client_lock(clients_mutex);

        // envia mensagem de desconexão para todos os clientes
        for (auto &client: clients) {
            std::string message = "Server is shutting down";
            MESSAGE_PACKET messageP("SERVER: " + message);
            send_packet(&client.second.connection_s, &messageP);

            MESSAGE_PACKET messageP2("DISCONNECTED");
            send_packet(&client.second.connection_s, &messageP2);
        }
    }

    close(server.socket_fd);
    exit(EXIT_SUCCESS);  
}

int main(int argc, char* argv[]) {
    uint16_t port = DEFAULT_PORT;

    // verifica se foi passado um número de porta como argumento
    if (argc == 2) {
        if (!is_number(argv[1])) {
            std::cout << "Invalid port number" << std::endl;
            exit(EXIT_FAILURE);
        }
        port = atoi(argv[1]);
    }

    signal(SIGINT, signalHandler);

    server = Server("127.0.0.1", port);

    if (server.socket_fd < 0) {
        std::cout << "Error creating server socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << server.port << std::endl;

    server.is_running = true;

    SOCKET_FD new_connection_s;
    while (server.is_running) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(sockaddr_in);

        // aceita conexão com o cliente
        new_connection_s = accept(server.socket_fd, (SA*)&client_addr, &client_addr_size);

        if (new_connection_s == -1) {
            std::cout << "Error accepting client" << std::endl;
            continue;
        }
  
        Client new_client(new_connection_s, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        std::cout << "Client "<< new_client.ip <<" connected on port "<< new_client.port << std::endl;

        // adiciona o cliente na lista de clientes
        clients_mutex.lock();
        clients[currGeneratedID] = new_client;
        clients_mutex.unlock();

        // cria uma thread para tratar do cliente conectado
        std::thread t(handle_client, currGeneratedID);
        t.detach();

		currGeneratedID++;
    }

    // fecha o socket do servidor
    close(server.socket_fd);
	close(new_connection_s);
    server.is_running = false;
    return 0;
}