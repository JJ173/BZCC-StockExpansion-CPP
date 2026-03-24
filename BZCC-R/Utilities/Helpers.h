#pragma once
#include "../Shared/DLLBase.h"

class helpers
{
public:
    static void add_objective(const char* name, long color, float show_time, bool clear_existing, bool is_coop);
        
    static bool is_player_within_distance(Handle handle, float distance, int total_players);
    static bool is_player_within_distance(const char* path, float distance, int total_players);
    static bool is_player_in_building(int total_players);
    static bool is_alive_and_on_team(Handle handle, int team);
    static bool is_audio_message_finished(int audio_handle, int audio_delay_time, int mission_time, bool is_coop);
};
