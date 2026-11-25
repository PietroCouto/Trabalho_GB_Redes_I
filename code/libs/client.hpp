/**
 * @author Pietro do Couto Freitas
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <cstring>
#include <unistd.h>

class Client
{
public:
    Client(char *ip, int port);
    ~Client();

    int start_client();
    void receive_message();
    void send_message(char *message);

private:
    int client_sock;
    sockaddr_in server_address;
    char buffer[1024];
    std::mutex mtx;
    bool connected;
};

/**
 *
 */
Client::Client(char *ip, int port)
{
    // Clear the buffer
    memset(&this->buffer, 0, sizeof(this->buffer));

    // Add the server IP
    inet_pton(AF_INET, ip, &this->server_address.sin_addr.s_addr);

    // Add the address family
    this->server_address.sin_family = AF_INET;

    // Add the port
    this->server_address.sin_port = htons(port);

    this->connected = false;
}

/**
 * Destructor
 */
Client::~Client()
{
    // Validate that the socket is open
    if (this->client_sock != -1)
    {
        // Close the socket
        close(this->client_sock);

        // Invalidate the use
        this->client_sock = -1;
    }
};

/**
 *
 */
int Client::start_client()
{
    this->client_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (this->client_sock == -1)
        return -1;

    if (connect(this->client_sock, (struct sockaddr*)&this->server_address, sizeof(this->server_address)) == -1)
        return -1;

    this->connected = true;

    return 0;
}

/**
 *
 */
void Client::receive_message()
{
    // Keep the function alive
    while (true)
    {
        std::lock_guard<std::mutex> lock(this->mtx);

        // Receive messages from the socket
        recv(this->client_sock, this->buffer, sizeof(this->buffer), 0);
    }
}

/**
 *
 */
void Client::send_message(char *message)
{
    if (this->client_sock == -1 || !this->connected)
        return;

    send(this->client_sock, message, strlen(message), 0);
}

#endif