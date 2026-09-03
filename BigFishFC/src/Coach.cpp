#include "Coach.h"
#include <iostream>

Coach::Coach(const std::string& teamname) : teamname(teamname) {}

void Coach::strategize() {
    std::cout << "Coach of team " << teamname << " is strategizing." << std::endl;
}
