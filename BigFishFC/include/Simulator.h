#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Player.h"
#include "Coach.h"
#include <vector>
#include <memory>

class Simulator {
public:
    Simulator(const std::string& teamname, int number_of_players, bool use_coach);
    void run();

private:
    std::string teamname;
    bool use_coach;
    std::vector<std::unique_ptr<Player>> players;
    std::unique_ptr<Coach> coach;
};

#endif // SIMULATOR_H
