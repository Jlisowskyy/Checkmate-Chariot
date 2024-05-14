//
// Created by wookie on 5/13/24.
//

#include "../include/ThreadManagement/GameTimeManagerUtils.h"
#include <tuple>

std::tuple<lli, lli, lli> GameTimeManagerUtils::ParseGoTimeInfo(const GoTimeInfo &tInfo, const Color &color)
{
    lli timeLimitClockMs   = (color == BLACK ? tInfo.bTime : tInfo.wTime);
    lli timeLimitPerMoveMs = tInfo.moveTime == GoTimeInfo::NotSet ? GoTimeInfo::Infinite : tInfo.moveTime;
    lli incrementMs =
        (color == BLACK ? tInfo.bInc : tInfo.wInc); // Get the time left for the engine to play (on the clock)
    timeLimitClockMs = timeLimitClockMs == GoTimeInfo::NotSet ? GoTimeInfo::Infinite : timeLimitClockMs;

    incrementMs = incrementMs == GoTimeInfo::NotSet ? 0 : incrementMs;
    return {timeLimitClockMs, timeLimitPerMoveMs, incrementMs};
}
