#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "role_inverted_fb.h"

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

#include "basic_actions/body_go_to_point.h"

using namespace rcsc;

const std::string RoleInvertedFB::NAME("InvertedFB");

/*-------------------------------------------------------------------*/
/*!

 */
namespace {
    rcss::RegHolder role = SoccerRole::creators().autoReg(&RoleInvertedFB::create,
        RoleInvertedFB::NAME);
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
RoleInvertedFB::execute(PlayerAgent* agent)
{
    std::cout << "EXECUTE CALLED INVERTEDFB";
    bool kickable = agent->world().self().isKickable();
    if (agent->world().kickableTeammate()
        && agent->world().teammatesFromBall().front()->distFromBall()
        < agent->world().ball().distFromSelf())
    {
        kickable = false;
    }

    if (kickable)
    {
        doKick(agent);
    }
    else
    {
        std::cout << "INSIDE ELSE SECTION";
        bool tri = triangleSituation(agent);
        if (!tri) {
            doMove(agent);
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleInvertedFB::doKick(PlayerAgent* agent)
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
RoleInvertedFB::doMove(PlayerAgent* agent)
{
    Bhv_BasicMove().execute(agent);
}

bool RoleInvertedFB::triangleSituation(PlayerAgent* agent) {
    
    std::cout << "TRIANGLE ATTEMPTED PHASE 1\n"; 
    
    const rcsc::WorldModel& wm = agent->world();
    const rcsc::SelfObject& self = wm.self();
    
    // teammates nearby?
    const auto teammates = wm.teammatesFromSelf();
    std::vector<const rcsc::PlayerObject*> nearby;
    
    for (const auto* p : teammates) {
        if (p->unum() == self.unum()) continue;
        if (p->posValid() && p->distFromSelf() < 50.0) {
            nearby.push_back(p);
        }
    }

    if (nearby.size() < 2) {
        // Not enough players to form a triangle
        std::cout << "TRIANGLE ATTEMPTED - NOT ENOUGH PLAYERS \n"; 
        return false;
    }
    
    // have the ball?
    const rcsc::PlayerObject* ball_holder = nullptr;
    for (const auto* p : nearby) {
        if (p->distFromBall() < 1.5) {
            ball_holder = p;
            break;
        }
    }

    if (!ball_holder) {
        // No nearby teammate has the ball
        std::cout << "TRIANGLE ATTEMPTED - BALL NOT NEARBY\n"; 
        return false;
    }
    
    const rcsc::PlayerObject* second = nullptr;
    for (const auto* p : nearby) {
        if (p == ball_holder) continue;
        second = p;
        break;
    }

    if (!second) return false; // no second teammate to form triangle
    
    rcsc::Vector2D target_pos = (ball_holder->pos() + second->pos()) * 0.5;
    AngleDeg angle = (ball_holder->pos() - second->pos()).th();
    rcsc::Vector2D direction(angle.cos(), angle.sin());
    direction.setLength(2.5);
    target_pos += direction;
    
    agent->debugClient().addMessage("TRIANGLE SUPPORT");
    agent->debugClient().addCircle(target_pos, 0.5);
    Body_GoToPoint(target_pos,
        0.5,    // dist_thr
        100.0,  // dash_power
        360.0,  // max_dash_angle
        0,      // max_cycle
        false,  // back_dash
        0.0,    // min_dash_power
        100.0,  // max_dash_power
        true    // save_recovery
    ).execute(agent);

    
    std::cout << "TRIANGLE ATTEMPTED - FINAL PHASE \n"; 
    
    return true;
}