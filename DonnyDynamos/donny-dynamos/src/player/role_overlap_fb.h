#ifndef AGENT2D_ROLE_OVERLAPFB_H
#define AGENT2D_ROLE_OVERLAPFB_H

#include "soccer_role.h"
#include <rcsc/geom/vector_2d.h>

class RoleOverlapFB
    : public SoccerRole {
private:

public:

    static const std::string NAME;

    RoleOverlapFB()
    { }

    ~RoleOverlapFB()
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
        SoccerRole::Ptr ptr(new RoleOverlapFB());
        return ptr;
    }

private:

    void doKick(rcsc::PlayerAgent* agent);
    void doMove(rcsc::PlayerAgent* agent);
    void doCross(rcsc::PlayerAgent* agent);                     
    bool crossOn(const rcsc::Vector2D & pos) const;
};


#endif
#pragma once
