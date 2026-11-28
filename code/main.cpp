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

// Maximum reconnection attempts
#define MAX_ATTEMPTS 15

/**
 * Prototypes
 */
bool reconnection_handler(std::thread *listen_t, Server *server);
bool ready_handler(std::thread *listen_t, Server *server);
bool greetings_handler(std::thread *listen_t, Server *server, std::string *name, std::string *enemy_name);
bool message_handler(std::thread *listen_t, Server *server, std::string message);
bool ally_play_handler(std::thread *listen_t, Server *server, int *sequence);
bool enemy_play_handler(std::thread *listen_t, Server *server, int *sequence);

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
        wait_for_enter("ERRO ao conectar ao servidor! Encerrando...");
        return -1;
    }

    // Server mode
    if (mode == 1 && server.start(std::stoi(argv[2])) == -1)
    {
        wait_for_enter("ERRO ao receber conexões! Encerrando...");
        return -1;
    }

    // Start the listening thread
    std::thread listen_t(&Server::receive_message, &server);

    // Strings to hold the players' names
    std::string name;
    std::string enemy_name;

    if (!greetings_handler(&listen_t, &server, &name, &enemy_name))
        return -1;

    // Start the board
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

    int sequence = 1;
    int last_received = 0;

    // Game loop
    while (!board.check_game_over())
    {
        system("clear");

        // Display the game board
        board.show_board();

        // Ally turn
        if (board.get_ally_turn() && !ally_play_handler(&listen_t, &server, &sequence))
            return -1;

        // Enemy turn
        if (!board.get_ally_turn() && !enemy_play_handler(&listen_t, &server, &sequence))
            return -1;
    }

    // End the communication
    message_handler(&listen_t, &server, "TCHAU\n");

    // Join the thread
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

            wait_for_enter("Conexão reestabelecida!");

            return true;
        }

        i++;

        // Put the thread to sleep to give some time between attempts
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    std::cout << "Limite de tentativas excedido!\n";

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
    {
        wait_for_enter("ERRO ao enviar mensagem de pronto! Encerrando...");
        return false;
    }

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
    {
        wait_for_enter("ERRO ao enviar mensagem de apresentação! Encerrando...");
        return false;
    }

    while (true)
    {
        if (server->has_message())
        {
            std::string message = server->get_message();

            if (message.find("OLA") != std::string::npos)
            {
                // Check if the protocol version is invalid
                if (message.size() < 7 || (message.at(4) != '1' || message.at(5) != ' '))
                {
                    message_handler(listen_t, server, "ERRO versão do protocolo incompatível!\n");
                    wait_for_enter("ERRO versão do protocolo incompatível! Encerrando...");
                    return false;
                }

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

/**
 * Performs the ally play
 * @param listen_t Pointer to the listening thread
 * @param server Pointer to the server instance
 * @param board Pointer to the game board
 * @param sequence Pointer to the sequence number counter
 * @returns Boolean representing the success of the operation
 */
bool ally_play_handler(std::thread *listen_t, Server *server, Board *board, int *sequence)
{
    char line = ' ';
    int column = -1;

    std::string message = "TIRO " + std::to_string(*sequence) + " ";

    while (true)
    {

        std::cout << "Linha (A-J): ";

        std::cin >> line;

        std::cout << "Coluna (1-10): ";

        if (!(std::cin >> column))
        {
            wait_for_enter("Coluna inválida!");
            continue;
        }

        if (!board->validate_ally_shot_coordinates(line, column))
        {
            wait_for_enter("Jogada inválida!");
            continue;
        }

        break;
    }

    message += line + std::to_string(column);

    if (!message_handler(listen_t, server, message))
    {
        return false;
    }

    bool hit = false;

    // TODO validar resultado

    board->ally_shot(line, column, hit);

    return true;
}

/**
 * Performs the enemy play
 * @param listen_t Pointer to the listening thread
 * @param server Pointer to the server instance
 * @param board Pointer to the game board
 * @param sequence Pointer to the sequence number counter
 * @returns Boolean representing the success of the operation
 */
bool enemy_play_handler(std::thread *listen_t, Server *server, Board *board, int *sequence)
{
    while (true)
    {
        break;
    }

    return true;
}