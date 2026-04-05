#pragma once
#include <unordered_map>

#include "../Shared/DllBase.h"

class Helpers
{
public:
    static void AddObjectiveOverride(const char* name, long color, float show_time, bool clear_existing, bool is_coop);
    static void TeleportOut(Handle handle);
        
    static bool IsPlayerWithinDistance(Handle handle, float distance, int total_players);
    static bool IsPlayerWithinDistance(const char* path, float distance, int total_players);
    static bool IsPlayerInBuilding(int total_players);
    static bool IsAliveAndOnTeam(Handle handle, int team);
    static bool IsAudioMessageFinished(int audio_handle, int audio_delay_time, int mission_time, bool is_coop);
    static bool Teleport(Handle handle, const Vector& destination, float offset);
    static bool IsRecycler(Handle handle);
    
    static int GetRandomInt(float max);
    
    static Handle TeleportIn(const char* odf, int team, const Vector& position);
    
    static Vector GetPathPosition(const char* path);
};
