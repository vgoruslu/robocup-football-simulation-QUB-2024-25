#include "Simulator.h"
#include <iostream>

Simulator::Simulator(const std::string& teamname, int number_of_players, bool use_coach)
    : teamname(teamname), use_coach(use_coach) {
    for (int i = 1; i <= number_of_players; ++i) {
        players.push_back(std::make_unique<Player>(i, teamname));
    }
    if (use_coach) {
        coach = std::make_unique<Coach>(teamname);
    }
}

void Simulator::run() {
    for (auto& player : players) {
        player->act();
    }
    if (use_coach && coach) {
        coach->strategize();
    }
}
