#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player {
public:
    Player(int id, const std::string& teamname);
    void act();

private:
    int id;
    std::string teamname;
};

#endif // PLAYER_H
