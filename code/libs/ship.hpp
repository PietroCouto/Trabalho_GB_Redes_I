/**
 * @author Pietro do Couto Freitas
 */

#ifndef SHIP_H
#define SHIP_H

#include <iostream>

class Ship
{
public:
    Ship();
    Ship(std::string type, int size);

    void shot();
    bool has_sunken();
    int get_size();
    std::string get_type();
    bool is_placed();
    void place();

private:
    std::string type;
    bool placed;
    int size;
    int shots_taken;
};

/**
 * Constructor
 */
Ship::Ship()
{
    this->placed = false;
    this->shots_taken = 0;

    this->type = "PA";
    this->size = 5;
}

Ship::Ship(std::string type, int size)
{
    this->placed = false;
    this->shots_taken = 0;

    this->type = type;
    this->size = size;
}

void Ship::shot()
{
    this->shots_taken++;
};

bool Ship::has_sunken()
{
    return this->size == this->shots_taken;
}

bool Ship::is_placed()
{
    return this->placed;
}

int Ship::get_size()
{
    return this->size;
}

std::string Ship::get_type()
{
    return this->type;
}

void Ship::place()
{
    this->placed = true;
}

#endif