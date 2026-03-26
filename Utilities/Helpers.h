#pragma once
#include "../Shared/DllBase.h"

class Helpers
{
public:
    static void AddObjectiveOverride(const char* name, long color, float show_time, bool clear_existing, bool is_coop);
        
    static bool IsPlayerWithinDistance(Handle handle, float distance, int total_players);
    static bool IsPlayerWithinDistance(const char* path, float distance, int total_players);
    static bool IsPlayerInBuilding(int total_players);
    static bool IsAliveAndOnTeam(Handle handle, int team);
    static bool IsAudioMessageFinished(int audio_handle, int audio_delay_time, int mission_time, bool is_coop);
};
