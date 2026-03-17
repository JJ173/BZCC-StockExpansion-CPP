#include "Common.h"

#include "../../Shared/DLLUtils.h"
#include "../../Shared/TRNAllies.h"
#include "../../Utilities/Helpers.h"

// ==================================================
// Construction / Initialization
// ==================================================
Common::Common()
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

    h_count_ = &last_handle_ - &first_handle_ - 1;
    h_array_ = &first_handle_ + 1;

    i_count_ = &last_int_ - &first_int_ - 1;
    i_array_ = &first_int_ + 1;

    // Set up the terrain allies if they apply.
    TRNAllies::SetupTRNAllies(GetMapTRNFilename());
}

Common::~Common()
{

}

// ==================================================
// Spawn / Build Helpers
// ==================================================
Handle Common::build_starting_vehicle(const int team, const char race, const std::string& first_odf, const std::string& second_odf, const Vector& spawn_pos) const
{
    // Replace the first ODF character with the race character.
    std::string odf = first_odf;
    odf[0] = race;

    // Check if the first ODF exists.
    if (!DoesODFExist(odf.c_str()))
    {
        odf = second_odf;
        odf[0] = race;
    }

    // Build the object at the desired position.
    const Handle handle = BuildObject(odf.c_str(), team, spawn_pos);

    // If we're on a player team, then set the best group.
    if (team == player_team_)
    {
        SetBestGroup(handle);
    }

    // Return to caller.
    return handle;
}

// ==================================================
// Intro Cleanup
// ==================================================
void Common::remove_isdf_intro_units() const
{
    RemoveObject(intro_ship_1_);
    RemoveObject(intro_ship_2_);
    RemoveObject(intro_ship_3_);
    RemoveObject(intro_turret_1_);
    RemoveObject(intro_turret_2_);

    for (int i = 0; i < max_players; ++i)
    {
        // Create the string for the handle we need to find.
        std::string handle_name = "player_spawn_i_" + std::to_string(i);

        // Grab the player spawn handles and remove them during cleanup.
        const Handle player_spawn_handle = GetHandle(handle_name.c_str());

        // If found, then remove it.
        RemoveObject(player_spawn_handle);
    }
}

void Common::remove_scion_intro_units() const
{
    RemoveObject(scion_intro_hangar_);
    RemoveObject(scion_intro_matriarch_);
    RemoveObject(scion_intro_turret_1_);
    RemoveObject(scion_intro_turret_2_);

    for (int i = 0; i < max_players; ++i)
    {
        // Create the string for the handle we need to find.
        std::string handle_name = "player_spawn_f_" + std::to_string(i);

        // Grab the player spawn handles and remove them during cleanup.
        const Handle player_spawn_handle = GetHandle(handle_name.c_str());

        // If found, then remove it.
        RemoveObject(player_spawn_handle);
    }
}

void Common::disable_intro()
{
    if (intro_done_)
    {
        return;
    }

    remove_isdf_intro_units();
    remove_scion_intro_units();

    intro_done_ = true;
}

// ==================================================
// Game State / Win-Loss Conditions
// ==================================================
void Common::game_conditions()
{
    if (game_over_)
    {
        return;
    }

    if (!IsAlive(enemy_recycler_))
    {
        const Handle dll_handle = GetObjectByTeamSlot(comp_team_, DLL_TEAM_SLOT_RECYCLER);

        if (IsAround(dll_handle))
        {
            return;
        }

        if (!is_mpi_)
        {
            FailMission(GetTime() + 5, "instantw.txt");
        }
        else
        {
            const int winning_team_group = WhichTeamGroup(strat_team_);
            NoteGameoverByLastTeamWithBase(winning_team_group);
        }

        // TODO: Add CPU Chat Team Taunt.

        game_over_ = true;
    }
    else if (!IsAlive(recycler_))
    {
        const Handle dll_handle = GetObjectByTeamSlot(strat_team_, DLL_TEAM_SLOT_RECYCLER);

        if (IsAround(dll_handle))
        {
            return;
        }

        if (!is_mpi_)
        {
            FailMission(GetTime() + 5, "instantl.txt");
        }
        else
        {
            const int winning_team_group = WhichTeamGroup(comp_team_);
            NoteGameoverByLastTeamWithBase(winning_team_group);
        }

        // TODO: Add CPU Chat Team Taunt.

        game_over_ = true;
    }
}

// ==================================================
// Carrier Logic
// ==================================================
void Common::build_carriers()
{

}

// ==================================================
// Intro Progression
// ==================================================
void Common::check_intro_enemies_killed()
{
    if (!intro_enemies_spawned_)
    {
        // Set the standard ODF to spawn.
        std::string enemy_odf = "*vscout_c";

        for (int i = 0; i < difficulty_; ++i)
        {
            // Replace the first character.
            enemy_odf[0] = cpu_race_char_;

            // Position that we're building at.
            std::string pos = "intro_attacker_" + std::to_string(i);

            // Build the enemy handle.
            const Handle enemy = BuildObject(enemy_odf.c_str(), comp_team_, pos.c_str());

            // Pick a target.
            switch (i)
            {
            case 0:
                intro_enemy_1_ = enemy;
                break;
            case 1:
                intro_enemy_2_ = enemy;
                break;
            default:
                intro_enemy_3_ = enemy;
                break;
            }
        }

        intro_enemies_spawned_ = true;
    }

    // Check to see if the intro enemies are alive.
    if (!helpers::is_alive_and_on_team(intro_enemy_1_, comp_team_)
        && !helpers::is_alive_and_on_team(intro_enemy_2_, comp_team_)
        && !helpers::is_alive_and_on_team(intro_enemy_3_, comp_team_))
    {
        intro_delay_ += mission_time_ + SecondsToTurns(3.0f);
        intro_state_++;
    }
}

// ==================================================
// Player Recycler
// ==================================================
void Common::build_player_recycler(const Vector& position)
{
    std::string customHumanRecycler;

    if (is_mpi_)
    {
        customHumanRecycler = dll_utils::get_checked_network_svar(5, NETLIST_IAHumanRecyList);
    }
    else
    {
        char buffer[64]{};
        IFace_GetString("options.instant.string1", buffer, sizeof(buffer));
        customHumanRecycler = buffer;
    }

    if (customHumanRecycler.empty())
    {
        recycler_ = build_starting_vehicle(strat_team_, player_race_char_, customHumanRecycler, "*vrecy", position);
    }
    else
    {
        recycler_ = build_starting_vehicle(strat_team_, player_race_char_, "*vrecy", "*vrecy", position);
    }

    SetScrap(strat_team_, 40);
}

// ==================================================
// Spawn Cleanup
// ==================================================
void Common::clean_spawns() const
{
    for (int i = 1; i <= max_players; ++i)
    {
        if (i > player_count_)
        {
            const Handle spawn_handle = GetHandle(("player_spawn_i_" + std::to_string(i)).c_str());
            RemoveObject(spawn_handle);
        }
    }
}

// ==================================================
// Main Logic
// ==================================================
void Common::setup()
{
    // This might be cool as an option in the shell for ease.
    SetAutoGroupUnits(false);
    
    // We are a 1.3 DLL. Show bot kill messages.
    WantBotKillMessages();
    
    // Run through and preload a list of ODF names.
    // Generate a local list here.
    static constexpr const char* odfs_to_preload[] =
    {
        "ivrecy",
        "fvrecy",
        "ivrecycpu",
        "fvrecycpu",
        "ivrecy_x",
        "fvrecy_x",
        "ivrecy_c",
        "fvrecy_c",
        "ibcarrier_xm",
        "fbcarrier_xm",
        "ivpdrop_x",
        "fbhangar",
        "fbportb_ARK",
        "fbstro_ARK",
        "fbark2holo",
        "bcrhino",
        "mcjak01",
        "mcwing01"
    };
    
    for (const char* odf : odfs_to_preload)
    {
        PreloadODF(odf);
    }
    
    // Next, run through and preload a list of Audio names.
    // Generate a local list here.
    static constexpr const char* audio_to_preload[] = 
    {
        "IA_Intro.wav",
        "IA_Pilot_1.wav",
        "IA_Pilot_2.wav",
        "IA_Pilot_3.wav",
        "IA_Pilot_4.wav",
        "IA_Carrier_1.wav",
        "IA_Carrier_2.wav",
        "IA_Scion_Carrier_1.wav",
        "IA_Scion_Carrier_2.wav",
        "IA_Scion_Tech_1.wav",
        "IA_Scion_Tech_2.wav",
        "IA_Scion_Tech_3.wav",
        "IA_Scion_Tech_3A.wav",
        "IA_Scion_Tech_3B.wav",
        "IA_Scion_Tech_4.wav",
        "dropdoor.wav"
    };
    
    for (const char* audio : audio_to_preload)
    {
        PreloadAudioMessage(audio);
    }
    
    // Start initializing the variables that we need from the shell.
    is_mpi_ = IsNetworkOn();
    mission_time_ = 0;
    start_done_ = false;
    game_over_ = false;
    comp_team_ = 6;
    strat_team_ = 1;
    map_name_ = GetMapTRNFilename();
}

void Common::execute()
{
    // Works out the game time for us.
    mission_time_++;
}
