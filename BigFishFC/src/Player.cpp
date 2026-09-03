#include "Player.h"
#include <iostream>

Player::Player(int id, const std::string& teamname) : id(id), teamname(teamname) {}

void Player::act() {
    std::cout << "Player " << id << " from team " << teamname << " is playing." << std::endl;
}
