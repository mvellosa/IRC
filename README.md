# ICMC-connect
 Repositório criado para o projeto IRC da disciplina SSC0142

# Integrantes
- Matheus Vellosa de Andrade - 12421725
<!-- - Pedro -->
<!-- - Bruna -->

# Video apresentação
[Link](https://)

# Como executar
Cliente e servidor necessitam de terminais separados para serem executados.
## Cliente
Para compilar o cliente, dentro do diretorio principal:

> cd client \
make all

Para rodar o cliente:
> make run

&emsp; OU

> ./client

## Servidor
De mesma maneira, para compilar o servidor, dentro do diretorio principal:

> cd server \
make all


Para rodar o servidor:
> make run

OU

> ./server


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
