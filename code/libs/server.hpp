/**
 * @author Pietro do Couto Freitas
 */

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <cstring>
#include <unistd.h>

class Server
{
public:
    Server(int port);
    ~Server();

    int start_server();
    void receive_message();
    void send_message(char *message);
private:
    int server_sock;
    int client_sock;
    sockaddr_in server_address;
    sockaddr_in client_address;
    socklen_t client_address_size;
    char buffer[1024];
    std::mutex mtx;

    int has_valid_message();
};

/**
 *
 */
Server::Server(int port)
{
    memset(&this->buffer, 0, sizeof(this->buffer));

    this->server_address.sin_family = AF_INET;
    this->server_address.sin_port = htons(port);
    this->server_address.sin_addr.s_addr = INADDR_ANY;
}

/**
 * Destructor
 */
Server::~Server()
{
    // Validate that the socket is open
    if (this->server_sock != -1)
    {
        // Close the socket
        close(this->server_sock);

        // Invalidate the use
        this->server_sock = -1;
    }
};

/**
 *
 */
int Server::start_server()
{
    this->server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (this->server_sock == -1)
        return -1;

    if (bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        return -1;

    if (listen(server_sock, 2) == -1)
        return -1;

    this->client_sock = accept(this->server_sock, (struct sockaddr*)&this->client_address, &this->client_address_size);

    if (this->client_sock == -1)
        return -1;
        
    return 0;
}

/**
 *
 */
void Server::receive_message()
{
    // Keep the function alive
    while (true)
    {
        std::lock_guard<std::mutex> lock(mtx);
        recv(this->client_sock, this->buffer, sizeof(this->buffer), 0);
    }
}

/**
 * 
 */
void Server::send_message(char *message)
{
    send(this->server_sock, message, strlen(message), 0);
}

/**
 * 
 */
int Server::has_valid_message() {
    for (int i = 0; i < 1024; i++)
        if (buffer[i] == '\n')
            return 0;

    return -1;
}

#endif