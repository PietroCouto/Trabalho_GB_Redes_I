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
#include <atomic>

#define BUFFER_SIZE 1024

class Server
{
public:
    // Constructor
    Server(int mode);

    // Destructor
    ~Server();

    // Methods
    void receive_message();
    int send_message(const std::string &message);
    bool has_message();
    int start(int port, const char *ip = "127.0.0.1");
    int reconnect();
    std::string get_message();
    bool is_connected();

private:
    // Variables
    int sock_fd;
    int peer_sock;
    int last_index;
    int mode;

    sockaddr_in address;
    sockaddr_in peer_address;
    socklen_t peer_address_size;

    char buffer[BUFFER_SIZE];

    std::atomic<bool> has_connection;
    std::mutex mtx;

    // Methods
    int start_server(int port);
    int start_client(const char *ip, int port);
};

/**
 * Constructor
 */
Server::Server(int mode)
{
    // Clear the buffer memory
    memset(&this->buffer, 0, sizeof(this->buffer));

    this->has_connection = false;
    this->mode = mode;
    this->last_index = 0;

    // Invalidate the sockets
    this->sock_fd = -1;
    this->peer_sock = -1;
}

/**
 * Destructor
 */
Server::~Server()
{
    // Check if the socket is open
    if (this->sock_fd != -1)
    {
        // Close the socket
        close(this->sock_fd);

        // Invalidate the use
        this->sock_fd = -1;
    }

    // Check if the peer socket is open
    if (this->peer_sock != -1)
    {
        // Close the socket
        close(this->peer_sock);

        // Invalidate the use
        this->peer_sock = -1;
    }
};

/**
 * Starts the Server/Client
 * @param port The IP port to
 * @param ip The IP address of the server
 * @returns -1 for error, 0 for success
 */
int Server::start(int port, const char *ip)
{
    // Start the server mode
    if (this->mode == 1)
        return start_server(port);

    // Start the client mode
    return start_client(ip, port);
}

/**
 * Starts the server
 * @param port The port to be openned
 * @returns -1 for error, 0 for success
 */
int Server::start_server(int port)
{
    // Set the address family
    this->address.sin_family = AF_INET;

    // Set the IP to any
    this->address.sin_addr.s_addr = INADDR_ANY;

    // Set the port to open
    this->address.sin_port = htons(port);

    // Try to open a socket
    this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Check if the socket was openned successfully
    if (this->sock_fd == -1)
        return -1;

    // Try to bind the socket
    if (bind(sock_fd, (struct sockaddr *)&this->address, sizeof(this->address)) == -1)
        return -1;

    // Try to listen for one connection
    if (listen(sock_fd, 1) == -1)
        return -1;

    this->peer_address_size = sizeof(this->peer_address);

    // Await for a connection
    this->peer_sock = accept(this->sock_fd, (struct sockaddr *)&this->peer_address, &this->peer_address_size);

    // Check if a connection was accepted successfully
    if (this->peer_sock == -1)
        return -1;

    this->has_connection = true;

    return 0;
}

/**
 * Starts the client version of the server
 * @param ip The IP address to connect to
 * @param port The IP port to connect to
 * @returns -1 for error, 0 for success
 */
int Server::start_client(const char *ip, int port)
{
    // Try to open a socket
    this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Check if the socket was openned successfully
    if (this->sock_fd == -1)
        return -1;

    // Add the server IP
    inet_pton(AF_INET, ip, &this->peer_address.sin_addr.s_addr);

    // Add the address family
    this->peer_address.sin_family = AF_INET;

    // Add the port
    this->peer_address.sin_port = htons(port);

    // Try to connect to the server
    if (connect(this->sock_fd, (struct sockaddr *)&this->peer_address, sizeof(this->peer_address)) == -1)
        return -1;

    // Set the connection to true
    this->has_connection = true;

    return 0;
}

/**
 * Receives messages from the peer
 */
void Server::receive_message()
{
    char aux_buffer[BUFFER_SIZE];

    int read_bytes = 0;

    // Keep the function alive
    while (true)
    {
        // Try to read a stream
        read_bytes = recv(this->mode ? this->peer_sock : this->sock_fd, aux_buffer, sizeof(aux_buffer), 0);

        // Check if the socket was closed or had an error
        if (read_bytes <= 0)
        {
            // Drop the connection
            this->has_connection = false;

            break;
        }

        // Lock the buffer
        std::lock_guard<std::mutex> lock(mtx);

        int i = 0;

        // Write from the aux buffer to the buffer
        for (; i < read_bytes; i++)
        {
            // Check if the buffer is full
            if (this->last_index + i >= BUFFER_SIZE)
                break;

            this->buffer[this->last_index + i] = aux_buffer[i];
        }

        // Update the last index
        this->last_index += i;
    }
}

/**
 * Sends a message to the peer
 * @param message The message to be sent
 * @returns -1 for error, number of bytes sent for success
 */
int Server::send_message(const std::string &message)
{
    if (!this->has_connection)
        return -1;

    return send(this->mode ? this->peer_sock : this->sock_fd, message.c_str(), strlen(message.c_str()), 0);
}

/**
 * Check if the buffer has a full message available
 * @returns boolean representing the message availability
 */
bool Server::has_message()
{
    // Lock the buffer
    std::lock_guard<std::mutex> lock(mtx);

    // Check for a full message
    for (int i = 0; i < this->last_index; i++)
        if (this->buffer[i] == '\n')
            return true;

    return false;
}

/**
 * Get a message from the buffer
 * @returns String with the message
 */
std::string Server::get_message()
{
    std::string message;

    // Lock the buffer
    std::lock_guard<std::mutex> lock(mtx);

    int i = 0;

    // Copy the buffer to the message string
    for (; i < this->last_index; i++)
    {
        if (this->buffer[i] == '\n')
            break;

        message.push_back(this->buffer[i]);
    }

    // Check if the end of the buffer was reached
    if (i == this->last_index)
        return "";

    int j = 0;

    // Clear the buffer
    for (i += 1; i < this->last_index; i++)
    {
        this->buffer[j] = this->buffer[i];
        j++;
    }

    // Set the last index
    this->last_index = j;

    return message;
}

/**
 * Tries to reconnect to the peer
 * @returns -1 for error, 0 for success
 */
int Server::reconnect()
{
    // Reconnection for client mode
    if (mode == 0)
    {
        // Check if the socket is not invalid
        if (this->sock_fd != -1)
        {
            close(this->sock_fd);

            this->sock_fd = -1;
        }

        // Try to open a socket
        this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);

        // Check if the socket was openned successfully
        if (this->sock_fd == -1)
            return -1;

        // Try to connect to the server
        if (connect(this->sock_fd, (struct sockaddr *)&this->peer_address, sizeof(this->peer_address)) == -1)
            return -1;
    }

    // Reconnection for server mode
    if (mode == 1)
    {
        // Check if the socket is not invalid
        if (this->peer_sock != -1)
        {
            close(this->peer_sock);

            this->peer_sock = -1;
        }

        // Reset the address size
        this->peer_address_size = sizeof(this->peer_address);

        // Await for a connection
        this->peer_sock = accept(this->sock_fd, (struct sockaddr *)&this->peer_address, &this->peer_address_size);

        // Check if a connection was accepted successfully
        if (this->peer_sock == -1)
            return -1;
    }

    // Lock the mutex
    std::lock_guard<std::mutex> lock(mtx);

    // Reset the last index
    this->last_index = 0;

    // Reset the buffer
    memset(this->buffer, 0, sizeof(this->buffer));

    this->has_connection = true;

    return 0;
}

/**
 * Get the connection status
 * @returns Boolean representing the connection status
 */
bool Server::is_connected()
{
    return this->has_connection;
}

#endif