# Chat-connect

Um IRC (Internet Relay Chat) que segue (com ressalvas) o padrão RFC 1459, permitindo a comunicação entre usuários através de um servidor.

# Introdução

A troca de mensagens entre cliente e servidor é realizada por sockets, utilizando o protocolo TCP.

O servidor é responsável por gerenciar as conexões e salas de chat, enquanto o cliente é responsável por enviar e receber mensagens do servidor.

O servidor é capaz de gerenciar múltiplas conexões simultâneas através de threads, e possui um sistema de salas de chat, onde cada sala possui um nome e uma lista de usuários conectados.

# Autores
- Matheus Vellosa de Andrade - 12421725

# Video apresentação
[Link](https://drive.google.com/file/d/1bGqO4dkzKGwbmfiBefa2WVTEbxjBHJ-s/view?usp=sharing)

# Executando o projeto
Cliente e servidor necessitam de terminais separados para serem executados.
## Cliente
Para compilar o cliente, dentro do diretorio principal:
```sh
cd client
make all
```

Para rodar o cliente:
```sh
make run
```
&emsp; OU
```sh
./client
```
## Servidor
De mesma maneira, para compilar o servidor, dentro do diretorio principal:
```sh
cd server
make all
```


Para rodar o servidor:
```sh
make run
```

OU
```sh
./server
```


# Utilizando a aplicação
## Cliente
Uma vez rodando, o cliente poderá utilizar os comandos para conectar com o servidor desejado e iniciar a comunicação.\
Todos comandos são iniciados com o caracter '/', seguido do comando desejado.
## Servidor
O servidor irá rodar em uma porta \
padrão: **8080** \
caso outra seja desejada, pode ser passada como argumento na execução do servidor.
```
./server <porta>
```
# Comandos disponíveis
| Comando | Descrição | Uso | Restrição |
| --- | --- | --- | --- |
| /help | Mostra a lista de comandos disponíveis | /help | Nenhuma |
| /connect | Conecta com um servidor | /connect <host> <port> | Nenhuma |
| /disconnect | Desconecta do servidor | /disconnect | Conectado a um servidor |
| /ping | Envia um ping para o servidor | /ping | conectado em uma sala de chat |
| /join | Entra em uma sala de chat | /join <room_name> | Conectado a um servidor |
| /leave | Sai da sala de chat atual | /leave | Conectado a uma sala de chat |
| /nickname | Altera o nickname do usuário no servidor | /nickname <new_nickname> | Conectado a um servidor |
| /kick | Expulsa um usuário da sala de chat | /kick <target_username> | ser admin do chat |
| /mute | Muta um usuário na sala de chat | /mute <target_username> | ser admin do chat |
| /unmute | Desmuta um usuário na sala de chat | /unmute <target_username> | ser admin do chat |
| /whois | Mostra informações de um usuário do chat | /whois <targe_username> | ser admin do chat |
| /quit | Sai do programa | /quit | Nenhuma |

# Pacote
## estrutura:
```C++
int msg_size;
char message[msg_size];
```
&emsp; Onde:
- **msg_size**: tamanho da mensagem a ser enviada
- **message**: mensagem a ser enviada

O processamento de comandos é feito a partir da subdivisião da mensagem em tokens, separados por espaços.

# Ambiente de desenvolvimento
- Sistema operacional: Pop!_OS 22.04 LTS
- Linguagem: C++
- Compilador: g++ 11.3.0
- IDE: Visual Studio Code 1.80.1
- Bibliotecas utilizadas: 
  - Sockets:
    * sys/socket.h
    * arpa/inet.h
    * unistd.h
  - Threads: std::thread
    - Mutex: std::mutex
- Ferramentas de versionamento: Git e GitHub
