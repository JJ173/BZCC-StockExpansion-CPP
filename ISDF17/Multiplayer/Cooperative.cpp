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
Handle Cooperative::setup_player(const int team)
{
    if (team < 1 || team >= MAX_TEAMS) 
    {
        return 0;
    }

    total_player_count_++;

    // TODO: Add world manager player incrementing here.

    if (IsTeamplayOn())
    {
        const int cmd_team = GetCommanderTeam(team);

        if (!is_team_setup[cmd_team])
        {
            const char team_race = GetRaceOfTeam(cmd_team);
            SetMPTeamRace(WhichTeamGroup(team_race), team_race);
            is_team_setup[cmd_team] = true;
        }
    }

    char player_spawn_handle[64];
    const int written = sprintf_s(player_spawn_handle, sizeof(player_spawn_handle), "player_spawn_%d", total_player_count_);
    if (written < 0)
    {
        player_spawn_handle[0] = '\0';
    }
    
    const Handle player_h = GetHandle(player_spawn_handle);

    if (player_h == 0)
    {

    }
}

// ==================================================
// Main Logic
// ==================================================
void Cooperative::setup_cooperative(const char* mission_name, const char* player_ship_odf_, const char* player_pilot_odf, const bool is_coop, bool spawn_pilot_only, float height_offset)
{
    constexpr const char* difficulty_map[3] =
    {
        "Easy",
        "Medium",
        "Hard"
    };

    int difficulty;

    if (is_coop) 
    {
        difficulty = GetVarItemInt("network.session.ivar102");
    }
    else
    {
        difficulty = GetVarItemInt("options.play.difficulty");
    }

    char mission_name_buffer[100];
    int written = sprintf_s(mission_name_buffer, sizeof(mission_name_buffer), "Mission: %s", mission_name ? mission_name : "<unknown>");
    if (written < 0)
    {
        mission_name_buffer[0] = '\0';
    }
    
    AddToMessagesBox("[WELCOME]");
    AddToMessagesBox(mission_name_buffer);
    AddToMessagesBox("Author: AI_Unit");

    if (is_coop) 
    {
        AddToMessagesBox("Cooperative: Yes");
    }
    else
    {
        AddToMessagesBox("Cooperative: No");
    }

    char difficulty_string_buffer[100];
    written = sprintf_s(difficulty_string_buffer, sizeof(difficulty_string_buffer), "Difficulty: %s", difficulty_map[difficulty] ? difficulty_map[difficulty] : "<unknown>");
    if (written < 0)
    {
        difficulty_string_buffer[0] = '\0';
    }
    
    AddToMessagesBox(difficulty_string_buffer);
    AddToMessagesBox("Good luck and have fun :)");

    ClearTeamColors();
    SetTeamNameForStat(0, "Neutral");

    const Handle player_entry_h = GetPlayerHandle(1);
    if (player_entry_h != 0)
    {
        RemoveObject(player_entry_h);
    }

    if (ImServer() || !is_coop)
    {
        elapsed_game_time_ = 0;
    }

    const int local_team_num = GetLocalPlayerTeamNumber();
    const Handle player_h = setup_player(local_team_num);
    SetAsUser(player_h, local_team_num);
    AddPilotByHandle(player_h);

    // TODO: Add Bane Mission Logic for the World Manager.
}