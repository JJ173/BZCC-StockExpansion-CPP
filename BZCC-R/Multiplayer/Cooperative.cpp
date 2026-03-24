#include "Cooperative.h"

#include "../../Source/fun3d/ScriptUtils.h"
#include "../Shared/TRNAllies.h"
#include <cstdio>

// ==================================================
// Construction / Initialization
// ==================================================
Cooperative::Cooperative() 
{
	// Grab the BZCC default TPS (20).
	game_tps_ = BZCC_DEFAULT_TPS;

	// Enable High TPS.
	EnableHighTPS(game_tps_);

	// Random tracks are fine.
	AllowRandomTracks(true);

    // These are important for save/load.
    b_count_ = &last_bool_ - &first_bool_ - 1;
    b_array_ = &first_bool_ + 1;

    i_count_ = &last_int_ - &first_int_ - 1;
    i_array_ = &first_int_ + 1;

    // Set up the terrain allies if they apply.
    TRNAllies::SetupTRNAllies(GetMapTRNFilename());
}

Cooperative::~Cooperative() 
{

}

// ==================================================
// Player Spawn / Build Helpers
// ==================================================
Handle Cooperative::setup_player(int team_)
{
    if (team_ < 1 || team_ >= MAX_TEAMS) 
    {
        return 0;
    }

    total_player_count_++;

    // TODO: Add world manager player incrementing here.

    if (IsTeamplayOn())
    {
        int cmd_team_ = GetCommanderTeam(team_);

        if (!is_team_setup[cmd_team_])
        {
            char team_race_ = GetRaceOfTeam(cmd_team_);
            SetMPTeamRace(WhichTeamGroup(team_race_), team_race_);
            is_team_setup[cmd_team_] = true;
        }
    }

    char player_spawn_handle_[64] = { 0 };
    sprintf_s(player_spawn_handle_, "player_spawn_%d", total_player_count_);
    Handle player_h_ = GetHandle(player_spawn_handle_);

    if (player_h_ == 0)
    {

    }
}

// ==================================================
// Main Logic
// ==================================================
void Cooperative::setup_cooperative(const char* mission_name_, const char* player_ship_odf_, const char* player_pilot_odf, bool is_coop_, bool spawn_pilot_only_, float height_offset_)
{
    constexpr const char* difficulty_map_[3] =
    {
        "Easy",
        "Medium",
        "Hard"
    };

    int difficulty_ = 0;

    if (is_coop_) 
    {
        difficulty_ = GetVarItemInt("network.session.ivar102");
    }
    else
    {
        difficulty_ = GetVarItemInt("options.play.difficulty");
    }

    char mission_name_buffer_[75] = { 0 };
    sprintf_s(mission_name_buffer_, "Mission: %s", mission_name_);

    AddToMessagesBox("[WELCOME]");
    AddToMessagesBox(mission_name_buffer_);
    AddToMessagesBox("Author: AI_Unit");

    if (is_coop_) 
    {
        AddToMessagesBox("Cooperative: Yes");
    }
    else
    {
        AddToMessagesBox("Cooperative: No");
    }

    char difficulty_string_buffer_[75] = { 0 };
    sprintf_s(difficulty_string_buffer_, "Difficulty: %s", difficulty_map_[difficulty_]);
    
    AddToMessagesBox(difficulty_string_buffer_);
    AddToMessagesBox("Good luck and have fun :)");

    ClearTeamColors();
    SetTeamNameForStat(0, "Neutral");

    Handle player_entry_h_ = GetPlayerHandle(1);
    if (player_entry_h_ != 0)
    {
        RemoveObject(player_entry_h_);
    }

    if (ImServer() || !is_coop_)
    {
        elapsed_game_time_ = 0;
    }

    int local_team_num_ = GetLocalPlayerTeamNumber();
    Handle player_h_ = setup_player(local_team_num_);
    SetAsUser(player_h_, local_team_num_);
    AddPilotByHandle(player_h_);

    // TODO: Add Bane Mission Logic for the World Manager.
}