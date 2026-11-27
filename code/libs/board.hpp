/**
 * @author Pietro do Couto Freitas
 */

#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include "./ship.hpp"

inline constexpr std::string_view lines = "ABCDEFGHIJ";
inline constexpr std::string_view ships = "PA EN CR DE";

class Board
{
public:
    // Constructor
    Board(bool ally_starts);

    // Methods
    void show_board();
    int insert_ship(char line, int column, std::string type, char direction);
    void show_available_ships();
    int get_available_ships(std::string type);
    int get_ship_size(std::string type);
    std::string enemy_shot(char line, int column);
    int ally_shot(char line, int column, bool hit);
    bool check_game_over();
    std::string get_fleet();
    bool get_ally_turn();

private:
    // Attributes
    int ally_side[10][10];
    int enemy_side[10][10];
    std::string positions;
    Ship ships[6];
    bool ally_turn;

    // Methods
    int validate_coordinates(char line, int column);
    int validate_ship_placement(char line, int column, std::string type, char direction);
    int set_placed(std::string type);
};

/**
 * Constructor
 * @param ally_starts Whether the ally should start the game
 */
Board::Board(bool ally_starts)
{
    // Set the board to water
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            ally_side[i][j] = -1;
            enemy_side[i][j] = -1;
        }
    }

    // Create the ships
    ships[0] = Ship("PA", 5);
    ships[1] = Ship("EN", 4);
    ships[2] = Ship("CR", 3);
    ships[3] = Ship("CR", 3);
    ships[4] = Ship("DE", 2);
    ships[5] = Ship("DE", 2);

    // Set the starting player
    this->ally_turn = ally_starts;
}

/**
 * Displays the game board
 */
void Board::show_board()
{
    std::cout << "              JOGADOR                           INIMIGO\n\n";
    std::cout << "   01 02 03 04 05 06 07 08 09 10     01 02 03 04 05 06 07 08 09 10\n\n";

    // Displays the lines
    for (int line = 0; line < lines.size(); line++)
    {
        std::cout << lines.at(line) << "  ";

        // Displays the ally line
        for (int j = 0; j < 10; j++)
        {
            if (ally_side[line][j] >= 0 && ally_side[line][j] < 6)
                std::cout << ships[ally_side[line][j]].get_type();

            if (ally_side[line][j] == -1)
                std::cout << "~~";

            if (ally_side[line][j] == 6)
                std::cout << "XX";

            if (ally_side[line][j] == 7)
                std::cout << "**";

            std::cout << " ";
        }

        std::cout << " " << lines.at(line) << "  ";

        // Displays the enemy line
        for (int j = 0; j < 10; j++)
        {
            if (enemy_side[line][j] == -1)
                std::cout << "~~";

            if (enemy_side[line][j] == 6)
                std::cout << "XX";

            if (enemy_side[line][j] == 7)
                std::cout << "**";

            std::cout << " ";
        }

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
    if (get_available_ships(type) == 0)
        return -1;

    // Validate horizontally
    if (direction == 'H')
    {
        // Check if it is out of bounds
        if (column > (11 - size))
            return -1;

        // Validate if the slot is empty
        for (int j = column - 1; j < column + size - 1; j++)
            if (ally_side[lines.find(line)][j] != -1)
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
            if (ally_side[i][column - 1] != -1)
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

    int ship_index = set_placed(type);

    // Decrease this type's available ships
    if (ship_index == -1)
        return -1;

    // Insert horizontally
    if (direction == 'H')
        for (int j = column - 1; j < column + ships[ship_index].get_size() - 1; j++)
            ally_side[lines.find(line)][j] = ship_index;

    // Insert vertically
    if (direction == 'V')
        for (int i = lines.find(line); i < lines.find(line) + ships[ship_index].get_size(); i++)
            ally_side[i][column - 1] = ship_index;

    // Save the position
    this->positions.append(type + ":" + line + std::to_string(column) + ":" + direction);

    if (get_available_ships("") > 0)
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
    std::cout << "PA - Porta-aviões: " << get_available_ships("PA") << "\n";
    std::cout << "EN - Encouraçado: " << get_available_ships("EN") << "\n";
    std::cout << "CR - Cruzador: " << get_available_ships("CR") << "\n";
    std::cout << "DE - Destroyer: " << get_available_ships("DE") << "\n\n";
}

/**
 * Get the amount of available ships
 * @returns The total amount of available ships
 */
int Board::get_available_ships(std::string type)
{
    int available = 0;

    for (int i = 0; i < 6; i++)
        if (!ships[i].is_placed() && (ships[i].get_type().compare(type) == 0 || type.compare("") == 0))
            available++;

    return available;
}

/**
 * Validates an enemy shot
 * @param line The line shot
 * @param column The column shot
 * @returns String with the result
 */
std::string Board::enemy_shot(char line, int column)
{
    // Check if it is the enemy turn
    if (this->ally_turn)
        return std::string("ERRO turno errado!");

    // Check if the coordinates are valid
    if (validate_coordinates(line, column) == -1)
        return std::string("ERRO Coordenada inválida!");

    // Check if it is a repeated shot
    if (ally_side[lines.find(line)][column - 1] >= 6)
        return std::string("ERRO Coordenada Repetida!");

    this->ally_turn = true;

    // Check if it hit the water
    if (ally_side[lines.find(line)][column - 1] == -1)
    {
        ally_side[lines.find(line)][column - 1] = 6;

        return std::string("AGUA");
    }

    // Increment ship's shot count
    ships[ally_side[lines.find(line)][column - 1]].shot();

    // Get the hitten ship
    int ship_index = ally_side[lines.find(line)][column - 1];

    // Register the shot
    ally_side[lines.find(line)][column - 1] = 6;

    // Check if it was a non-sunking hit
    if (!ships[ship_index].has_sunken())
        return std::string("ACERTOU");

    std::string result;

    result.append("AFUNDOU " + ships[ship_index].get_type());

    // Check if the game has ended
    if (check_game_over())
        result.append(" FIM");

    return result;
};

/**
 * Checks if the game has ended
 * @returns boolean indicating the game over
 */
bool Board::check_game_over()
{
    for (int i = 0; i < 6; i++)
        if (!ships[i].has_sunken())
            return false;

    return true;
}

/**
 * Registers a ship from type as placed
 * @param type The type of ship to be registered
 * @returns -1 for error, index of the ship for success
 */
int Board::set_placed(std::string type)
{
    for (int i = 0; i < 6; i++)
    {
        // Check if it is the same type and has not been placed
        if (!ships[i].is_placed() && ships[i].get_type().compare(type) == 0)
        {
            ships[i].place();
            return i;
        }
    }

    return -1;
}

/**
 * Register the ally shot result
 * @param line The line coordinate
 * @param column The column coordinate
 * @param hit If the shot hit a ship
 */
int Board::ally_shot(char line, int column, bool hit)
{
    // Check if it is the ally turn
    if (!this->ally_turn)
        return -1;

    // Validate the coordinates
    if (validate_coordinates(line, column) == -1)
        return -1;

    // Check if it is a repeated shot
    if (enemy_side[lines.find(line)][column - 1] >= 6)
        return -1;

    enemy_side[lines.find(line)][column - 1] = hit ? 6 : 7;

    this->ally_turn = false;

    return 0;
}

/**
 * Get the fleet position
 * @returns String containing the positions
 */
std::string Board::get_fleet()
{
    return this->positions;
}

/**
 * Get the turn's player
 * @returns bool representing the ally turn
 */
bool Board::get_ally_turn()
{
    return this->ally_turn;
}

#endif