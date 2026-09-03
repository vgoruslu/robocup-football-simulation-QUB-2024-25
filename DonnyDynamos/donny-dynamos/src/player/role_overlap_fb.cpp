#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "role_overlap_fb.h"

#include "strategy.h"

#include "bhv_basic_move.h"

#include "planner/bhv_planned_action.h"
#include "basic_actions/body_hold_ball.h"
#include "basic_actions/neck_scan_field.h"

#include <rcsc/formation/formation.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

using namespace rcsc;

const std::string RoleOverlapFB::NAME("OverlapFB");

/*-------------------------------------------------------------------*/
/*!

 */
namespace {
    rcss::RegHolder role = SoccerRole::creators().autoReg(&RoleOverlapFB::create,
        RoleOverlapFB::NAME);
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
RoleOverlapFB::execute(PlayerAgent* agent)
{
    bool kickable = agent->world().self().isKickable();
    if (agent->world().kickableTeammate()
        && agent->world().teammatesFromBall().front()->distFromBall()
        < agent->world().ball().distFromSelf())
    {
        kickable = false;
    }
    // OLD LOGIC
    /*
    if (kickable)
    {
        doKick(agent);
    }
    else
    {
        doMove(agent);
    }
    */
    
    const WorldModel & wm = agent->world();
    const Vector2D pos = wm.self().pos();
    
    // NEW LOGIC
    // if kickable and in good position for cross then do that, else default to doKick
    if (kickable){
        if (crossOn(pos, wm)) {
            doCross(agent);
        } else {
            doKick(agent);
        } 
    } else {
        doMove(agent);
    }   

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleOverlapFB::doKick(PlayerAgent* agent)
{
    if (Bhv_PlannedAction().execute(agent))
    {
        dlog.addText(Logger::TEAM,
            __FILE__": (execute) do planned action");
        agent->debugClient().addMessage("PlannedAction");
        return;
    }

    Body_HoldBall().execute(agent);
    agent->setNeckAction(new Neck_ScanField());
}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleOverlapFB::doMove(PlayerAgent* agent)
{
    Bhv_BasicMove().execute(agent);
}

// NEW FUNCTIONS for checking situations
// is a cross on? - check fullbacks positioning
bool RoleOverlapFB::crossOn(const Vector2D & pos, const WorldModel & wm) const {
    const bool is_left = wm.ourSide() == rcsc::LEFT;
    double y_limit = is_left
        ? 25.0
        : -25.0;
    
    double x_limit = is_left
        ? 35.0
        : -35.0;
    
    
    if (pos.y > y_limit && pos.x < x_limit ) {
        std::cout << "CROSS ON \n";
        return true;
    }  else {
        return false;
    }
}

// NEW FUNCTIONS for executing actions
// doCross - play a lofted ball towards the target
void RoleOverlapFB::doCross(PlayerAgent* agent) {
    const rcsc::WorldModel& wm = agent->world();
    const bool is_left = wm.ourSide() == rcsc::LEFT;

    // Target the penalty spot on the opponent's side
    rcsc::Vector2D target = is_left
        ? rcsc::Vector2D(42.0, 0.0)   // Opponent's penalty spot (when we are LEFT)
        : rcsc::Vector2D(-42.0, 0.0); // Opponent's penalty spot (when we are RIGHT)

    // Turn towards target
    rcsc::AngleDeg angle = (target - wm.self().pos()).th();
    agent->doTurn(angle);

    // Kick ball in that direction
    rcsc::Vector2D relTarget = target - wm.ball().pos();
    agent->doKick(100.0, relTarget.th());

    dlog.addText(Logger::TEAM,
        __FILE__": (doCross) CROSS ATTEMPTED");
    agent->debugClient().addMessage("CROSS ATTEMPTED");

    std::cout << "CROSS ATTEMPTED toward " << target << "\n";
    
    return;
}


