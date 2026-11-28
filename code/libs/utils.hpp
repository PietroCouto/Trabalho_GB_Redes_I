/**
 * @author Pietro do Couto Freitas
 */

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <limits>

#include "board.hpp"

int show_usage();
int validate_options(int argc, char *argv[]);
int validate_port(char *port);
int validate_ip(char *string_ip);
void insert_ships(Board *board);
void wait_for_enter(std::string message);

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

    if (int_port < 1024 || int_port > 65535)
    {
        std::cout << "Porta " << port << " inválida!\n\n";
        return -1;
    }

    return 0;
}

/**
 * Displays the correct command line usage
 * @returns -1 indicating the incorrect usage
 */
int show_usage()
{
    std::cout << "Utilização: ./main [ -l | --listen <port> ] | [ -c | --connect <ip> <port> ]\n";

    return -1;
}

/**
 * Get the fleet position from user input
 * @param board Pointer to the board
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
            wait_for_enter("Tipo Inválido!");

            continue;
        }
        char line;

        std::cout << "Linha (A-J): ";

        std::cin >> line;

        int column;

        std::cout << "Coluna (1-10): ";

        if (!(std::cin >> column))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            wait_for_enter("Coluna inválida!");
            continue;
        }

        char direction;

        std::cout << "Direção do navio (H ou V): ";

        std::cin >> direction;

        if (board->insert_ship(line, column, type, direction) == -1)
        {
            wait_for_enter("Falha ao inserir!");
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
    std::cout << "\n" << message << " [Enter]";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    system("clear");
}

#endif