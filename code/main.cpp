/**
 * @author Pietro do Couto Freitas
 */

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <thread>

#include "./libs/board.hpp"
#include "./libs/server.hpp"
#include "./libs/client.hpp"

/**
 * Prototypes
 */
int show_usage();
int validate_options(int argc, char *argv[]);
int validate_port(char *port);
int validate_ip(char *string_ip);
void insert_ships(Board *board);
void wait_for_enter(std::string message);

int main(int argc, char *argv[])
{
    int mode = validate_options(argc, argv);

    // Check if the selected mode is valid
    if (mode == -1)
        return show_usage();

    Board board;

    board.insert_ship('A', 3, "PA", 'H');
    board.insert_ship('B', 1, "DE", 'V');
    board.insert_ship('D', 1, "DE", 'V');
    board.insert_ship('C', 6, "EN", 'V');
    board.insert_ship('I', 4, "CR", 'H');
    board.insert_ship('H', 10, "CR", 'V');

    // Insert the ships into the board
    insert_ships(&board);

    // Game loop
    // while (!board.check_game_over()) {
    //     board.show_board();

    // Check turn

    // }

    // Client mode
    if (mode == 0)
    {
        Client client = Client(argv[2], std::stoi(argv[3]));

        client.start_client();

        client.send_message("TESTEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    }

    // Server mode
    if (mode == 1)
    {
        Server server = Server(std::stoi(argv[2]));

        server.start_server();

        std::cout << "deu certo";
        
        std::thread listen_t(&Server::receive_message, &server);

        std::cout << "deu certo";

        listen_t.join();
    }

    return 0;
}

/**
 * Validates the user's command line input
 * @param argc The argument count
 * @param argv The provided arguments
 * @returns -1 for error, 0 for client mode, 1 for server mode
 */
int validate_options(int argc, char *argv[])
{
    // Validate the argument count
    if (argc < 3 || argc > 4)
        return -1;

    std::string flag = std::string(argv[1]);

    // Check if the user provided the connect option
    if (flag.compare("-c") == 0 || flag.compare("--connect") == 0)
    {
        // Validate the argument count
        if (argc != 4)
            return -1;

        // Validate the provided IP address
        if (validate_ip(argv[2]) == -1)
            return -1;

        // Validate the provided IP port
        if (validate_port(argv[3]) == -1)
            return -1;

        return 0;
    }

    // Check if the user provided the listen option
    if (flag.compare("-l") == 0 || flag.compare("--listen") == 0)
    {
        // Validate the argument count
        if (argc != 3)
            return -1;

        // Validate the provided IP port
        if (validate_port(argv[2]) == -1)
            return -1;

        return 1;
    }

    return -1;
}

/**
 * Checks if the provided IP address is valid
 * @param ip C string containing the address
 * @returns -1 for invalid, 0 for valid
 */
int validate_ip(char *ip)
{
    char str[INET_ADDRSTRLEN];

    if (inet_pton(AF_INET, ip, str) != 1)
    {
        std::cout << "Endereço IP " << ip << " inválido!\n\n";

        return -1;
    }

    return 0;
}

/**
 * Checks if the provided IP port is valid
 * @param port
 */
int validate_port(char *port)
{
    int int_port = -1;

    try
    {
        int_port = std::stoi(port);
    }
    catch (...)
    {
    }

    if (int_port < 1023 || int_port > 65535)
    {
        std::cout << "Porta " << port << " inválida!\n\n";
        return -1;
    }

    return 0;
}

/**
 * Displays the correct command line usage
 */
int show_usage()
{
    std::cout << "Utilização: ./main [ -l | --listen <port> ] | [ -c | --connect <ip> <port> ]\n";

    return -1;
}

/**
 *
 */
void insert_ships(Board *board)
{
    while (board->get_available_ships("") > 0)
    {
        board->show_board();

        board->show_available_ships();

        std::cout << "Digite o tipo de navio (2 letras): ";

        std::string type;

        std::cin >> type;

        int size = board->get_ship_size(type);

        if (size == -1)
        {
            wait_for_enter("\nTipo Inválido! [Enter]");

            continue;
        }
        char line;

        std::cout << "Linha (A-J): ";

        std::cin >> line;

        int column;

        std::cout << "Coluna (1-10): ";

        std::cin >> column;

        char direction;

        std::cout << "Direção do navio (H ou V): ";

        std::cin >> direction;

        if (board->insert_ship(line, column, type, direction) == -1)
        {
            wait_for_enter("\nFalha ao inserir! [Enter]");
            continue;
        }

        system("clear");
    }
}

/**
 * Displays a message and waits for the user to press enter
 * @param message The message to be displayed
 */
void wait_for_enter(std::string message)
{
    std::cout << message;

    std::cin.ignore();

    while (std::cin.get() != 10)
    {
    };

    system("clear");
}