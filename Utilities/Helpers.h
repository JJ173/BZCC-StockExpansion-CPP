#pragma once
#include <unordered_map>

#include "../Shared/DllBase.h"

struct UnitTracker
{
    Handle handle = 0;
    bool active = false;
    int command = 0;

    bool IsValid() { return active && IsAlive(handle); }
    bool IsAround() { return active && ::IsAround(handle); }
    bool IsIdle() { return active && ::IsIdle(handle); }

    void Register(const Handle h)
    {
        handle = h;
        active = true;
        command = CMD_NONE;
    }
    
    void Reset()
    {
        handle = 0;
        active = false;
        command = CMD_NONE;
    }
    
    void InitAttack(const Handle him)
    {
        Attack(handle, him, 0);
        command = CMD_ATTACK;
    }
    
    void InitFollow(const Handle him)
    {
        Follow(handle, him, 0);
        command = CMD_FOLLOW;
    }
    
    void InitPatrol(const char* path_name)
    {
        Patrol(handle, path_name, 0);
        command = CMD_PATROL;
    }
};

struct MissionTimer
{
    int expiry = -1;
    void Start(const float duration, const int missionTurn) { expiry = missionTurn + SecondsToTurns(duration); }
    void Stop() { expiry = -1; }
    bool IsExpired(const int missionTurn) { return expiry > 0 && missionTurn >= expiry; }
    bool IsRunning() { return expiry > 0; }
};

class Helpers
{
public:
    static void AddObjectiveOverride(const char* name, long color, float show_time, bool clear_existing, bool is_coop);
    static void TeleportOut(Handle handle);

    static bool TryTrackUnit(Handle h, const char* odf, UnitTracker* array, int count);
    
    static bool IsPlayerWithinDistance(Handle handle, float distance, int total_players);
    static bool IsPlayerWithinDistance(const char* path, float distance, int total_players);
    static bool IsPlayerInBuilding(int total_players);
    static bool IsAliveAndOnTeam(Handle handle, int team);
    static bool IsAudioMessageFinished(int audio_handle, int audio_delay_time, int mission_time, bool is_coop);
    static bool Teleport(Handle handle, const Vector& destination, float offset);
    static bool IsRecycler(Handle handle);
    static bool IsVectorZero(const Vector& vec);

    static int GetRandomInt(float max);

    static Handle TeleportIn(const char* odf, int team, const Vector& position);

    static Vector GetPathPosition(const char* path);

    static const char* GetODFStringFromChain(const char* odf_path, const char* section, const char* key);
    static float GetODFFloatFromChain(const char* odf_path, const char* section, const char* key);
};
