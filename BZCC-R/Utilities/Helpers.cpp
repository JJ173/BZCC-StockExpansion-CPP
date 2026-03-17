#include "Helpers.h"

bool helpers::is_alive_and_on_team(Handle handle, const int team)
{
    return IsAlive(handle) && GetTeamNum(handle) == team;
}
