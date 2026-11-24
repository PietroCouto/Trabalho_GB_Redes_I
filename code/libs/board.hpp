/**
 * @author Pietro do Couto Freitas
 */

#ifndef BOARD_H
#define BOARD_H

#include <iostream>

inline constexpr std::string_view lines = "ABCDEFGHIJ";
inline constexpr std::string_view ships = "PA EN CR DE";

class Board
{
public:
    Board();

    void show_board();
    int insert_ship(char line, int column, std::string type, char direction);
    void show_available_ships();
    int get_available_ships();
    int get_ship_size(std::string type);
    std::string enemy_shot(char line, int column);

private:
    std::string ally_side[10][10];
    std::string enemy_side[10][10];
    std::string positions;
    int available_ships[4];

    int validate_coordinates(char line, int column);
    int validate_ship_placement(char line, int column, std::string type, char direction);
};

/**
 * Constructor
 */
Board::Board()
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            ally_side[i][j].assign("oo");
            enemy_side[i][j].assign("oo");
        }
    }

    available_ships[0] = 2;
    available_ships[1] = 2;
    available_ships[2] = 1;
    available_ships[3] = 1;
}

/**
 * Displays the game board
 */
void Board::show_board()
{
    std::cout << "              JOGADOR                           INIMIGO\n\n";
    std::cout << "   01 02 03 04 05 06 07 08 09 10     01 02 03 04 05 06 07 08 09 10\n\n";

    for (int line = 0; line < lines.size(); line++)
    {
        std::cout << lines.at(line) << "  ";

        for (int j = 0; j < 10; j++)
            std::cout << ally_side[line][j] << " ";

        std::cout << " " << lines.at(line) << "  ";

        for (int j = 0; j < 10; j++)
            std::cout << enemy_side[line][j] << " ";

        std::cout << " " << lines.at(line) << "\n";
    }

    std::cout << "\n   01 02 03 04 05 06 07 08 09 10     01 02 03 04 05 06 07 08 09 10\n\n";
}

/**
 * Checks if a given coordinate is valid
 * @param line The line letter
 * @param column The column number
 * @returns -1 for error, 0 for succcess
 */
int Board::validate_coordinates(char line, int column)
{
    if (column < 1 || column > 10)
        return -1;

    if (lines.find(line) == std::string::npos)
        return -1;

    return 0;
}

/**
 * Validates if it is possible to place a ship
 * @param line The line to insert into
 * @param column The column to insert into
 * @param type The ship's type
 * @param direction The direction of the insertion
 * @returns -1 for error, 0 for success
 */
int Board::validate_ship_placement(char line, int column, std::string type, char direction)
{
    // Validate the coordinates
    if (validate_coordinates(line, column) == -1)
        return -1;

    int size = get_ship_size(type);

    // Validate the ship's type
    if (size == -1)
        return -1;

    // Check if this type is available
    if (available_ships[size - 2] == 0)
        return -1;

    // Validate horizontally
    if (direction == 'H')
    {
        // Check if it is out of bounds
        if (column > (11 - size))
            return -1;

        // Validate if the slot is empty
        for (int j = column - 1; j < column + size - 1; j++)
            if (ally_side[lines.find(line)][j].compare("oo") != 0)
                return -1;

        return 0;
    }

    // Validate vertically
    if (direction == 'V')
    {
        // Check if it is out of bounds
        if (lines.substr(lines.size() - (size - 1)).find(line) != std::string::npos)
            return -1;

        // Validate if the slot is empty
        for (int i = lines.find(line); i < lines.find(line) + size; i++)
            if (ally_side[i][column - 1].compare("oo") != 0)
                return -1;

        return 0;
    }

    return -1;
}

/**
 * Inserts a ship into the board
 * @param line The line to insert into
 * @param column The column to insert into
 * @param type The ship's type
 * @param direction The direction of the insertion
 * @returns -1 for error, 0 for success
 */
int Board::insert_ship(char line, int column, std::string type, char direction)
{
    // Check if it can be inserted on the given coordinates
    if (validate_ship_placement(line, column, type, direction) == -1)
        return -1;

    int size = get_ship_size(type);

    // Decrease this type's available ships
    available_ships[size - 2]--;

    // Insert horizontally
    if (direction == 'H')
        for (int j = column - 1; j < column + size - 1; j++)
            ally_side[lines.find(line)][j].assign(type);

    // Insert vertically
    if (direction == 'V')
        for (int i = lines.find(line); i < lines.find(line) + size; i++)
            ally_side[i][column - 1].assign(type);

    // Save the position
    this->positions.append(type + ":" + line + std::to_string(column) + ":" + direction);

    if (get_available_ships() > 0)
        this->positions.append(" ");

    return 0;
}

/**
 * Get a ship's type size
 * @param type The two letter type
 * @returns -1 for invalid type, size for valid
 */
int Board::get_ship_size(std::string type)
{
    int size = -1;

    if (type == "PA")
        size = 5;

    if (type == "EN")
        size = 4;

    if (type == "CR")
        size = 3;

    if (type == "DE")
        size = 2;

    return size;
}

/**
 * Displays which ships are available to place
 */
void Board::show_available_ships()
{
    std::cout << "----- Disponível -----" << "\n";
    std::cout << "PA - Porta-aviões: " << available_ships[3] << "\n";
    std::cout << "EN - Encouraçado: " << available_ships[2] << "\n";
    std::cout << "CR - Cruzador: " << available_ships[1] << "\n";
    std::cout << "DE - Destroyer: " << available_ships[0] << "\n\n";
}

/**
 * Get the amount of available ships
 * @returns The total amount of available ships
 */
int Board::get_available_ships()
{
    return available_ships[0] + available_ships[1] + available_ships[2] + available_ships[3];
}

std::string Board::enemy_shot(char line, int column)
{
    if (validate_coordinates(line, column) == -1)
        return std::string("ERRO Coordenada inválida!");

    if (ally_side[lines.find(line)][column - 1] == "oo")
        return std::string("AGUA");

    if (ally_side[lines.find(line)][column - 1] == "XX")
        return std::string("REPETIDO");

    return std::string("AGUA");
};

#endif