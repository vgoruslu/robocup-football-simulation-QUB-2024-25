#ifndef ROLE_FALSE_NINE_H
#define ROLE_FALSE_NINE_H

#include "soccer_role.h"

#include "basic_actions/body_go_to_point.h"


class RoleFalseNine
    : public SoccerRole {
private:

public:

    static const std::string NAME;

    RoleFalseNine()
    { }

    ~RoleFalseNine()
    { }

    virtual
        bool execute(rcsc::PlayerAgent* agent);


    static
        const
        std::string& name()
    {
        return NAME;
    }

    static
        SoccerRole::Ptr create()
    {
        SoccerRole::Ptr ptr(new RoleFalseNine());
        return ptr;
    }
private:

    void doKick(rcsc::PlayerAgent* agent);
    void doMove(rcsc::PlayerAgent* agent);
    bool findSpace(rcsc::PlayerAgent* agent);
    void dribbleFirst(rcsc::PlayerAgent* agent);
};


#endif
