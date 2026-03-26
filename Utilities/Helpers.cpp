#include "Helpers.h"

void Helpers::AddObjectiveOverride(const char* name, const long color, const float show_time, const bool clear_existing,
                                   const bool is_coop)
{
    if (clear_existing)
    {
        ClearObjectives();
    }

    if (!is_coop)
    {
        AddObjective(name, color, show_time);
    }
    else
    {
        AddToMessagesBox2(name, color);
    }
}

bool Helpers::IsPlayerWithinDistance(const char* path, const float distance, const int total_players)
{
    for (int i = 0; i < total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);

        if (IsAlive(player) && GetDistance(player, path) <= distance)
        {
            return GetDistance(player, path) <= distance;
        }
    }

    return false;
}

bool Helpers::IsPlayerWithinDistance(Handle handle, const float distance, const int total_players)
{
    for (int i = 1; i <= total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);

        if (IsAlive(player) && GetDistance(player, handle) <= distance)
        {
            return true;
        }
    }

    return false;
}

bool Helpers::IsPlayerInBuilding(const int total_players)
{
    for (int i = 1; i <= total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);

        if (IsAlive(player) && InBuilding(player))
        {
            return true;
        }
    }

    return false;
}

bool Helpers::IsAliveAndOnTeam(Handle handle, const int team)
{
    return IsAlive(handle) && GetTeamNum(handle) == team;
}

bool Helpers::IsAudioMessageFinished(const int audio_handle, const int audio_delay_time, const int mission_time,
                                     const bool is_coop)
{
    if (audio_handle == 0)
    {
        return true;
    }

    if (is_coop)
    {
        return audio_delay_time < mission_time;
    }

    return IsAudioMessageDone(audio_handle);
}
