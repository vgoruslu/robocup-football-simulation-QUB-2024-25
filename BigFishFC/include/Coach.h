#ifndef COACH_H
#define COACH_H

#include <string>

class Coach {
public:
    Coach(const std::string& teamname);
    void strategize();

private:
    std::string teamname;
};

#endif // COACH_H
