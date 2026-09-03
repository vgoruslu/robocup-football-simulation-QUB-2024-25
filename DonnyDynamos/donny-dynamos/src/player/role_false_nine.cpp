#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "role_false_nine.h"


#include "bhv_basic_move.h"

#include "planner/bhv_planned_action.h"
#include "basic_actions/body_hold_ball.h"
#include "basic_actions/neck_scan_field.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>

#include "basic_actions/body_go_to_point.h"

using namespace rcsc;

const std::string RoleFalseNine::NAME("FalseNine");

/*-------------------------------------------------------------------*/
/*!

 */
namespace {
    rcss::RegHolder role = SoccerRole::creators().autoReg(&RoleFalseNine::create,
        RoleFalseNine::NAME);
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
RoleFalseNine::execute(PlayerAgent* agent)
{
    bool kickable = agent->world().self().isKickable();
    if (agent->world().kickableTeammate()
        && agent->world().teammatesFromBall().front()->distFromBall()
        < agent->world().ball().distFromSelf())
    {
        kickable = false;
    }

    if (kickable)
    {
        dribbleFirst(agent);
        doKick(agent);
    }
    else
    {
        bool space = findSpace(agent);
        if (!space){
            doMove(agent);
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleFalseNine::doKick(PlayerAgent* agent)
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
RoleFalseNine::doMove(PlayerAgent* agent)
{
    Bhv_BasicMove().execute(agent);
}

bool RoleFalseNine::findSpace(PlayerAgent* agent) {
    const rcsc::WorldModel& wm = agent->world();
    const rcsc::SelfObject& self = wm.self();
    double half_length = rcsc::ServerParam::i().pitchLength() * 0.5;
    double half_width = rcsc::ServerParam::i().pitchWidth() * 0.5;

    const double radius_step = 2.0;
    const double angle_step = 15.0;
    const double max_radius = 10.0;
    const double min_opponent_dist = 5.0;

    rcsc::Vector2D best_point;
    double best_score = -1.0;

    for (double r = 2.0; r <= max_radius; r += radius_step)
    {
        for (double angle = -60.0; angle <= 60.0; angle += angle_step)
        {
            rcsc::Vector2D candidate = self.pos() + rcsc::Vector2D::polar2vector(r, angle);
            if (std::abs(candidate.x) > half_length || std::abs(candidate.y) > half_width)
            {
                continue;  // Out of bounds
            }   

            bool is_safe = true;
            for (const auto& opp : wm.opponentsFromSelf())
            {
                if (opp->posValid() && opp->pos().dist(candidate) < min_opponent_dist)
                {
                    is_safe = false;
                    break;
                }
            }

            if (is_safe)
            {
                double score = -candidate.dist(wm.ball().pos());
                if (score > best_score)
                {
                    best_score = score;
                    best_point = candidate;
                }
            }
        }
    }

    if (best_score < 0.0)
    {
        return false;
    }

    Body_GoToPoint(best_point,
                   0.5,
                   100.0,
                   360.0,
                   0,
                   false,
                   0.0,
                   100.0,
                   true).execute(agent);

    agent->debugClient().addCircle(best_point, 0.5);
    agent->debugClient().addMessage("F9 FindSpace");
    std::cout << "F9 MOVE TO SPACE at " << best_point << "\n";

    return true;
}

void
RoleFalseNine::dribbleFirst(PlayerAgent* agent)
{
    const rcsc::WorldModel& wm = agent->world();
    const rcsc::SelfObject& self = wm.self();
    
    // Check if any opponent is too close
     const double opponent_close_dist = 3.0;
    for (const auto& opp : wm.opponentsFromSelf())
    {
        if (opp->posValid() && opp->pos().dist(self.pos()) < opponent_close_dist)
        {
            agent->debugClient().addMessage("F9 Abort Dribble: Opponent too close");
            std::cout << "F9 CANCEL DRIBBLE: Opponent at " << opp->pos() << "\n";
            doMove(agent);  // fallback to default movement
            return;
        }
    }

    // Determine direction toward opponent goal
    const bool is_left = wm.ourSide() == rcsc::LEFT;
    rcsc::Vector2D goal_pos = is_left ? rcsc::Vector2D(52.5, 0.0)
                                      : rcsc::Vector2D(-52.5, 0.0);

    rcsc::Vector2D dir = (goal_pos - self.pos()).setLengthVector(3.0); // 3m ahead
    rcsc::Vector2D target_pos = self.pos() + dir;


    Body_GoToPoint(target_pos,
                   0.3,
                   100.0,
                   360.0,
                   0,
                   false,
                   0.0,
                   100.0,
                   true
    ).execute(agent);

    agent->setNeckAction(new Neck_ScanField());
    agent->debugClient().addMessage("F9 Dribble");
    std::cout << "F9 DRIBBLE TOWARD " << goal_pos << "\n";

    return;
}