#include "Helpers.h"

void helpers::add_objective(const char* name, const long color, const float show_time, const bool clear_existing, const bool is_coop)
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

bool helpers::is_player_within_distance(const char* path, const float distance, const int total_players)
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

bool helpers::is_player_within_distance(Handle handle, const float distance, const int total_players)
{
    for (int i = 0; i < total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);
        
        if (IsAlive(player) && GetDistance(player, handle) <= distance)
        {
            return true;
        }
    }
    
    return false;   
}

bool helpers::is_player_in_building(const int total_players)
{
    for (int i = 0; i < total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);
        
        if (IsAlive(player) && InBuilding(player))
        {
            return true;
        }
    }
    
    return false;
}

bool helpers::is_alive_and_on_team(Handle handle, const int team)
{
    return IsAlive(handle) && GetTeamNum(handle) == team;
}

bool helpers::is_audio_message_finished(const int audio_handle, const int audio_delay_time, const int mission_time, const bool is_coop)
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