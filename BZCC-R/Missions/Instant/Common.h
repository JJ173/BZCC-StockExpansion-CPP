#pragma once

#include <string>
#include "../../Shared/DllBase.h"

class Common : public DLLBase
{
    static constexpr int max_players = 4;

    int game_tps_;

    int first_int_;
    int mission_time_;
    int elapsed_game_time_;
    int player_team_;
    int strat_team_;
    int comp_team_;
    int difficulty_;
    int intro_state_;
    int intro_delay_;
    int player_count_;
    int vsr_taunt_easter_egg_time_;
    int cpu_scrap_amount_;
    int cpu_scrap_delay_;
    int last_int_;
        
    char player_race_char_;
    char cpu_race_char_;
    
    std::string map_name_;
    
    bool first_bool_;
    bool start_done_;
    bool intro_done_;
    bool game_over_;
    bool is_team_setup[MAX_TEAMS];
    bool is_mpi_;
    bool intro_enemies_spawned_;
    bool can_respawn_;
    bool intro_cutscene_enabled_;
    bool wildlife_enabled_;
    bool snipeable_enemies_;
    bool last_bool_;
    
    Handle first_handle_;
    Handle recycler_;
    Handle enemy_recycler_;
    Handle intro_enemy_1_;
    Handle intro_enemy_2_;
    Handle intro_enemy_3_;
    Handle intro_ship_1_;
    Handle intro_ship_2_;
    Handle intro_ship_3_;
    Handle intro_turret_1_;
    Handle intro_turret_2_;
    Handle scion_intro_hangar_;
    Handle scion_intro_matriarch_;
    Handle scion_intro_turret_1_;
    Handle scion_intro_turret_2_;
    Handle last_handle_;

protected:
    bool *b_array_;
    int b_count_;

    float *f_array_;
    int f_count_;

    int *h_array_;
    int h_count_;

    int *i_array_;
    int i_count_;
    
public:
    // Constructor
    Common();
    
    // Destructor
    ~Common() override;
    
    // Initial setup of the mission.
    void setup_mission();
    
    // Runs each frame.
    void Execute() override;
private:
    // Handle building any starting vehicles when the game begins.
    Handle build_starting_vehicle(int team, char race, const std::string& first_odf, const std::string& second_odf, const Vector& spawn_pos) const;
    
    // Remove the ISDF intro units.
    void remove_isdf_intro_units() const;
    
    // Remove the Scion intro units.
    void remove_scion_intro_units() const;
    
    // Disables the intro sequence.
    void disable_intro();
    
    // Handles checking the "Game Over" sequence.
    void game_conditions();
    
    // Handles calling the carrier manager to build carriers.
    void build_carriers();
    
    // If the intro is enabled, check to see if the intro enemies are dead.
    void check_intro_enemies_killed();
    
    // Handle building the player Recycler when the game begins.
    void build_player_recycler(const Vector& position);
    
    // Set up any players in the game.
    Handle setup_player(int team);
    
    // Clean any player spawns that are lingering at the start.
    void clean_spawns() const;
};
