#pragma once

#include "../Shared/DllBase.h"
#include "../../Source/fun3d/ScriptUtils.h"

class Cooperative : public DLLBase
{
    int game_tps_;

    int first_int_;
    int total_player_count_;
    int elapsed_game_time_;
    int last_int_;
    
    bool first_bool_;
    bool is_team_setup[4];
    bool last_bool_;

    Handle setup_player(int team_);

protected:
    bool* b_array_;
    int b_count_;

    int* i_array_;
    int i_count_;

public:
    // Constructor
    Cooperative();

    // Destructor
    ~Cooperative();

    // Setup
    void setup_cooperative(const char* mission_name_, const char* player_ship_odf_, const char* player_pilot_odf, bool is_coop_, bool spawn_pilot_only_, float height_offset_);
};
