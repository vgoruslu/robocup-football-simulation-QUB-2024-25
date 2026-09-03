#ifndef AGENT2D_ROLE_INVERTEDFB_H
#define AGENT2D_ROLE_INVERTEDFB_H

#include "soccer_role.h"

class RoleInvertedFB
    : public SoccerRole {
private:

public:

    static const std::string NAME;

    RoleInvertedFB()
    { }

    ~RoleInvertedFB()
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
        SoccerRole::Ptr ptr(new RoleInvertedFB());
        return ptr;
    }

private:

    void doKick(rcsc::PlayerAgent* agent);
    void doMove(rcsc::PlayerAgent* agent);
    bool triangleSituation(rcsc::PlayerAgent* agent);
};


#endif
#pragma once
