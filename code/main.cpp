/**
 * @author Pietro do Couto Freitas
 */

#include <iostream>
#include <string>
#include <arpa/inet.h>

/**
 * Prototypes
 */
int show_usage();
int validate_options(int argc, char *argv[]);
int validate_port(char *port);
int validate_ip(char *string_ip);

int main(int argc, char *argv[])
{
    int mode = validate_options(argc, argv);

    // Check if the selected mode is valid
    if (mode == -1)
        return show_usage();

    // Client mode
    if (mode == 0)
        return 0;

    // Server mode
    if (mode == 1)
        return 0;

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