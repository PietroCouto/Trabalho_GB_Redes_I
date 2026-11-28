/**
 * @author Pietro do Couto Freitas
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

/**
 * Handmade Libraries
 */
#include "./libs/board.hpp"
#include "./libs/server.hpp"
#include "./libs/utils.hpp"

#define MAX_ATTEMPTS 15

/**
 * Prototypes
 */
bool reconnection_handler(std::thread *listen_t, Server *server);
bool ready_handler(std::thread *listen_t, Server *server);
bool greetings_handler(std::thread *listen_t, Server *server, std::string *name, std::string *enemy_name);
bool message_handler(std::string message, Server *server);

/**
 * Battleship game
 * @param argc Argument count
 * @param argv Argument array
 * @returns -1 for error, 0 for success
 */
int main(int argc, char *argv[])
{
    int mode = validate_options(argc, argv);

    // Check if the selected mode is valid
    if (mode == -1)
        return show_usage();

    // Create the server
    Server server = Server(mode);

    // Client mode
    if (mode == 0 && server.start(std::stoi(argv[3]), argv[2]) == -1)
    {
        std::cout << "Falha ao conectar ao servidor!\n\n";
        return -1;
    }

    // Server mode
    if (mode == 1 && server.start(std::stoi(argv[2])) == -1)
    {
        std::cout << "Falha ao receber conexões!\n\n";
        return -1;
    }

    // Start the listening thread
    std::thread listen_t(&Server::receive_message, &server);

    std::string name;
    std::string enemy_name;

    if (!greetings_handler(&listen_t, &server, &name, &enemy_name))
        return -1;

    Board board(mode);

    // TODO remove after tests
    board.insert_ship('A', 3, "PA", 'H');
    board.insert_ship('B', 1, "DE", 'V');
    board.insert_ship('D', 1, "DE", 'V');
    board.insert_ship('C', 6, "EN", 'V');
    board.insert_ship('I', 4, "CR", 'H');
    board.insert_ship('H', 10, "CR", 'V');

    // Insert the ships into the board
    insert_ships(&board);

    if (!ready_handler(&listen_t, &server))
        return -1;

    // Game loop
    while (!board.check_game_over())
    {
        system("clear");

        board.show_board();

        if (!server.is_connected() && !reconnection_handler(&listen_t, &server))
            return -1;

        // Check turn
    }

    if (listen_t.joinable())
        listen_t.join();

    return 0;
}

/**
 * Handles the reconnection process
 * @param listen_t Pointer to the listening thread
 * @param server Pointer to the server instance
 * @returns Boolean representing the success
 */
bool reconnection_handler(std::thread *listen_t, Server *server)
{
    std::cout << "Conexão perdida!\n";

    int i = 1;

    // Try to reconnect
    while (!server->is_connected() && i <= MAX_ATTEMPTS)
    {
        std::cout << "[" << i << "] Reconectando...\n";

        // Check if the listening thread is joinable
        if (listen_t->joinable())
            listen_t->join();

        // Try to reconnect
        if (server->reconnect() == 0)
        {
            *listen_t = std::thread(&Server::receive_message, server);

            return true;
        }

        i++;

        // Put the thread to sleep to give some time between attempts
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "Limite de tentativas excedido! Encerrando...\n";

    return false;
}

/**
 * Handles the ready process
 * @param listen_t Pointer to the listening thread
 * @param server Pointer to the server instance
 * @returns Boolean representing the success
 */
bool ready_handler(std::thread *listen_t, Server *server)
{
    if (!message_handler(listen_t, server, "PRONTO\n"))
        return false;

    while (true)
    {
        if (server->has_message() && server->get_message().find("PRONTO") != std::string::npos)
            return true;

        if (!server->is_connected() && !reconnection_handler(listen_t, server))
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/**
 * Handles the greeting process
 * @param listen_t Pointer to the listening thread
 * @param server Pointer to the server instance
 * @param name String pointer to hold the player's name
 * @param enemy_name String pointer to hold the enemy player's name
 * @returns Boolean representing the success
 */
bool greetings_handler(std::thread *listen_t, Server *server, std::string *name, std::string *enemy_name)
{

    std::cout << "Digite seu nome: ";
    std::cin >> *name;

    if (!message_handler(listen_t, server, "OLA 1 " + *name + "\n"))
        return false;

    while (true)
    {
        if (server->has_message())
        {
            std::string message = server->get_message();

            if (message.find("OLA 1 ") != std::string::npos)
            {
                *enemy_name = message.substr(6, message.size() - 1);
                return true;
            }
        }

        if (!server->is_connected() && !reconnection_handler(listen_t, server))
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/**
 * Send a message to the peer
 * @param listen_t Pointer to the listening thread
 * @param server Pointer to the server instance
 * @param message The message to be sent
 * @returns Boolean representing the success
 */
bool message_handler(std::thread *listen_t, Server *server, std::string message)
{
    while (server->send_message(message) == -1)
    {
        if (!server->is_connected() && !reconnection_handler(listen_t, server))
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return true;
}