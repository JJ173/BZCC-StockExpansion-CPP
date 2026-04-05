#include "Instant.h"

#include <algorithm>
#include <chrono>

#include "../../shared/DLLUtils.h"
#include "../../shared/TRNAllies.h"
#include "../../Utilities/Helpers.h"
#include "../../Shared/GameConfig.h"
#include "../../Utilities/Subtitles.h"

// ==================================================
// Construction / Initialization
// ==================================================
Instant::Instant()
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
        "mcwing01",
        "pspwn_1"
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

    // Set up the terrain allies if they apply.
    TRNAllies::SetupTRNAllies(GetMapTRNFilename());
}

Instant::~Instant()
{
}

// ==================================================
// Spawn / Build Helpers
// ==================================================
Handle Instant::BuildStartingVehicle(const int team, const char race, const std::string& first_odf,
                                     const std::string& second_odf, const Vector& spawn_pos) const
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
// Game State / Win-Loss Conditions
// ==================================================
void Instant::GameConditions()
{
    if (game_over_)
    {
        return;
    }

    if (!IsAlive(enemy_recycler_))
    {
        const Handle dll_handle = GetObjectByTeamSlot(comp_team_, DLL_TEAM_SLOT_RECYCLER);

        if (dll_handle == 0)
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

        if (dll_handle == 0)
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
void Instant::BuildCarriers()
{
    carrier_manager_.SetupCarrier(strat_team_, human_race_char_);
    carrier_manager_.SetupCarrier(comp_team_, cpu_race_char_);
}

// ==================================================
// Intro Cleanup
// ==================================================
void Instant::RemoveISDFIntroUnits() const
{
    RemoveObject(intro_ship_1_);
    RemoveObject(intro_ship_2_);
    RemoveObject(intro_ship_3_);
    RemoveObject(intro_turret_1_);
    RemoveObject(intro_turret_2_);

    for (int i = 1; i <= max_players; ++i)
    {
        // Create the string for the handle we need to find.
        char handle_name[GameConfig::MAX_NAME_LENGTH] = {};
        if (sprintf_s(handle_name, sizeof(handle_name), "player_spawn_i_%d", i) < 0)
        {
            handle_name[0] = '\0';
            continue;
        }

        // Grab the player spawn handles and remove them during cleanup.
        const Handle player_spawn_handle = GetHandle(handle_name);

        // If found, then remove it.
        RemoveObject(player_spawn_handle);
    }
}

void Instant::RemoveScionIntroUnits() const
{
    RemoveObject(scion_intro_hangar_);
    RemoveObject(scion_intro_matriarch_);
    RemoveObject(scion_intro_turret_1_);
    RemoveObject(scion_intro_turret_2_);

    for (int i = 1; i <= max_players; ++i)
    {
        // Create the string for the handle we need to find.
        char handle_name[GameConfig::MAX_NAME_LENGTH] = {};
        if (sprintf_s(handle_name, sizeof(handle_name), "player_spawn_f_%d", i) < 0)
        {
            handle_name[0] = '\0';
            continue;
        }

        // Grab the player spawn handles and remove them during cleanup.
        const Handle player_spawn_handle = GetHandle(handle_name);

        // If found, then remove it.
        RemoveObject(player_spawn_handle);
    }
}

void Instant::DisableIntro()
{
    if (intro_done_)
    {
        return;
    }

    RemoveISDFIntroUnits();
    RemoveScionIntroUnits();

    intro_done_ = true;
}

void Instant::HandleDropshipRemoval()
{
    if (!dropship_takeoff_check_)
    {
        return;
    }

    if (!dropship_1_takeoff_)
    {
        const bool turret_1_clear = GetDistance(intro_turret_1_, intro_ship_1_) > 30;
        const bool turret_2_clear = GetDistance(intro_turret_2_, intro_ship_2_) > 30;
        const bool player_clear = !Helpers::IsPlayerWithinDistance(intro_ship_1_, 30, player_count_);

        if (turret_1_clear && turret_2_clear && player_clear)
        {
            SetAnimation(intro_ship_1_, "takeoff", 1);

            const int engine_sound = StartAudio3D("dropleav.wav", intro_ship_1_);
            SetVolume(engine_sound, 0.3f);

            dropship_1_takeoff_ = true;
            dropship_1_time_ = mission_time_ + SecondsToTurns(15.0f);
        }
    }
    else if (!dropship_1_removed_ && mission_time_ >= dropship_1_time_)
    {
        RemoveObject(intro_ship_1_);
        dropship_1_removed_ = true;
    }

    if (!dropship_2_takeoff_)
    {
        const bool player_clear = !Helpers::IsPlayerWithinDistance(intro_ship_2_, 30, player_count_);

        if (player_clear)
        {
            SetAnimation(intro_ship_2_, "takeoff", 1);

            const int engine_sound = StartAudio3D("dropleav.wav", intro_ship_2_);
            SetVolume(engine_sound, 0.3f);

            dropship_2_takeoff_ = true;
            dropship_2_time_ = mission_time_ + SecondsToTurns(15.0f);
        }
    }
    else if (!dropship_2_removed_ && mission_time_ >= dropship_2_time_)
    {
        RemoveObject(intro_ship_2_);
        dropship_2_removed_ = true;
    }

    if (!dropship_3_takeoff_)
    {
        const bool player_clear = !Helpers::IsPlayerWithinDistance(intro_ship_3_, 30, player_count_);

        if (player_clear)
        {
            SetAnimation(intro_ship_3_, "takeoff", 1);

            const int engine_sound = StartAudio3D("dropleav.wav", intro_ship_3_);
            SetVolume(engine_sound, 0.3f);

            dropship_3_takeoff_ = true;
            dropship_3_time_ = mission_time_ + SecondsToTurns(15.0f);
        }
    }
    else if (!dropship_3_removed_ && mission_time_ >= dropship_3_time_)
    {
        RemoveObject(intro_ship_3_);
        dropship_3_removed_ = true;
    }

    if (!dropship_takeoff_dialog_played_ && dropship_1_takeoff_ && dropship_2_takeoff_ && dropship_3_takeoff_)
    {
        intro_audio_ = subtitles_.AudioWithSubtitles("IA_Pilot_4.wav", GameConfig::SubtitlePanelSize::Small);
        dropship_takeoff_dialog_played_ = true;
    }

    if (dropship_1_removed_ && dropship_2_removed_ && dropship_3_removed_)
    {
        dropship_takeoff_check_ = false;
    }
}

// ==================================================
// Intro Helpers
// ==================================================
bool Instant::CanAdvanceIntro() const
{
    return mission_time_ >= intro_delay_;
}

bool Instant::IntroEnemiesKilled()
{
    if (!intro_enemies_spawned_)
    {
        // Set the standard ODF to spawn.
        std::string enemy_odf = "*vscout_c";

        for (int i = 1; i <= difficulty_; ++i)
        {
            // Replace the first character.
            enemy_odf[0] = cpu_race_char_;

            // Position that we're building at.
            char position_path[GameConfig::MAX_NAME_LENGTH] = {};
            if (sprintf_s(position_path, sizeof(position_path), "intro_attacker_%d", i) < 0)
            {
                position_path[0] = '\0';
            }

            // Build the enemy handle.
            const Handle enemy = BuildObject(enemy_odf.c_str(), comp_team_, position_path);

            // Pick a target.
            switch (i)
            {
            case 0:
                intro_enemy_1_ = enemy;
                Goto(intro_enemy_1_, "Recycler", 1);
                break;
            case 1:
                intro_enemy_2_ = enemy;
                Goto(intro_enemy_2_, "Recycler", 1);
                break;
            default:
                intro_enemy_3_ = enemy;
                Goto(intro_enemy_3_, "Recycler", 1);
                break;
            }
        }

        intro_enemies_spawned_ = true;
    }

    // Check to see if the intro enemies are alive.
    if (!Helpers::IsAliveAndOnTeam(intro_enemy_1_, comp_team_)
        && !Helpers::IsAliveAndOnTeam(intro_enemy_2_, comp_team_)
        && !Helpers::IsAliveAndOnTeam(intro_enemy_3_, comp_team_))
    {
        intro_delay_ = mission_time_ + SecondsToTurns(3.0f);
        return true;
    }

    return false;
}

// ==================================================
// Intro Progression
// ==================================================
void Instant::UpdateIntro()
{
    if (human_race_char_ == GameConfig::FACTION_ISDF)
    {
        UpdateISDFIntro();
    }
    else if (human_race_char_ == GameConfig::FACTION_SCION)
    {
        UpdateScionIntro();
    }
}

void Instant::UpdateISDFIntro()
{
    switch (isdf_intro_state_)
    {
    case ISDFIntroState::Cleanup:
        ISDFCleanup();
        break;
    case ISDFIntroState::PlayLine1:
        ISDFIntroLine1();
        break;
    case ISDFIntroState::Touchdown:
        ISDFTouchdown();
        break;
    case ISDFIntroState::PrepareDropship:
        ISDFPrepareDropship();
        break;
    case ISDFIntroState::PlayLine2:
        ISDFIntroLine2();
        break;
    case ISDFIntroState::DispatchUnits:
        ISDFDispatchUnits();
        break;
    case ISDFIntroState::PlayLine3:
        ISDFIntroLine3();
        break;
    case ISDFIntroState::CheckEnemiesAreDead:
        ISDFCheckEnemiesAreDead();
        break;
    case ISDFIntroState::SpawnCarriers:
        ISDFSpawnCarriers();
        break;
    case ISDFIntroState::CarrierLine2:
        ISDFCarrierLine2();
        break;
    case ISDFIntroState::End:
        ISDFEndIntro();
        break;
    }
}

void Instant::UpdateScionIntro()
{
}

void Instant::AdvanceISDFIntroState(const ISDFIntroState next, const float delay_seconds = 0.0f)
{
    isdf_intro_state_ = next;
    intro_delay_ = mission_time_ + SecondsToTurns(delay_seconds);
}

void Instant::AdvanceScionIntroState(const ScionIntroState next, const float delay_seconds = 0.0f)
{
    scion_intro_state_ = next;
    intro_delay_ = mission_time_ + SecondsToTurns(delay_seconds);
}

// ==================================================
// Player Spawn / Build Helpers
// ==================================================
void Instant::BuildPlayerRecycler(const Vector& position)
{
    std::string custom_human_recycler;

    if (is_mpi_)
    {
        custom_human_recycler = dll_utils::get_checked_network_svar(5, NETLIST_IAHumanRecyList);
    }
    else
    {
        char buffer[GameConfig::MAX_ODF_LENGTH] = {};
        IFace_GetString(GameConfig::RECYCLER_ODF, buffer, sizeof(buffer));
        custom_human_recycler = buffer;
    }

    if (!custom_human_recycler.empty())
    {
        recycler_ = BuildStartingVehicle(strat_team_, human_race_char_, custom_human_recycler, "*vrecy", position);
    }
    else
    {
        recycler_ = BuildStartingVehicle(strat_team_, human_race_char_, "*vrecy", "*vrecy", position);
    }

    SetBestGroup(recycler_);
    SetScrap(strat_team_, 40);
}

void Instant::BuildPlayerVehicles(const Vector& position, const bool is_intro) const
{
    if (difficulty_ >= GameConfig::Difficulty::HARD)
    {
        return;
    }

    const Handle tank1 = BuildStartingVehicle(player_team_, human_race_char_, "*vtank_xm", "*vtank", position);
    const Handle tank2 = BuildStartingVehicle(player_team_, human_race_char_, "*vtank_xm", "*vtank", position);

    SetRandomHeadingAngle(tank1);
    SetRandomHeadingAngle(tank2);
    SetBestGroup(tank1);
    SetBestGroup(tank2);

    if (is_intro)
    {
        Defend2(tank1, recycler_, 0);
        Defend2(tank2, recycler_, 0);
    }

    if (difficulty_ < GameConfig::Difficulty::MEDIUM)
    {
        if (human_race_char_ == GameConfig::FACTION_ISDF)
        {
            GiveWeapon(tank1, "gspstab_c");
            GiveWeapon(tank2, "gspstab_c");
        }
        else
        {
            GiveWeapon(tank1, "garc_c");
            GiveWeapon(tank1, "gabsorb");
            GiveWeapon(tank2, "garc_c");
            GiveWeapon(tank2, "gabsorb");
        }

        const Handle truck = BuildStartingVehicle(player_team_, human_race_char_, "*vserv_xm", "*vserv", position);
        SetRandomHeadingAngle(truck);
        SetBestGroup(truck);

        if (is_intro)
        {
            Follow(truck, recycler_, 0);
        }
    }
}

Handle Instant::SetupPlayer(const int team)
{
    if (team < 0 || team >= MAX_TEAMS)
    {
        return 0;
    }

    player_count_++;
    IFace_SetInteger(GameConfig::MPI_PLAYER_COUNT, player_count_);

    char player_count_message[GameConfig::MAX_CONSOLE_MSG_LENGTH];
    if (sprintf_s(player_count_message, sizeof(player_count_message), "[IA 2.0]: Registering New Player Count: %d",
                  player_count_) < 0)
    {
        player_count_message[0] = '\0';
    }
    else
    {
        PrintConsoleMessage(player_count_message);
    }

    if (IsTeamplayOn())
    {
        const int cmd_team = GetCommanderTeam(team);

        if (!is_team_setup_[cmd_team])
        {
            const char team_race = GetRaceOfTeam(cmd_team);
            SetMPTeamRace(WhichTeamGroup(team_race), team_race);
            is_team_setup_[cmd_team] = true;
        }
    }
    
    Vector team_spawn_pos = GetPositionNear(Helpers::GetPathPosition("Recycler"), 25.0f, 25.0f);
    Handle player_h = 0;

    if (intro_cutscene_enabled_)
    {
        char spawn_handle_name[GameConfig::MAX_NAME_LENGTH];

        if (sprintf_s(spawn_handle_name, sizeof(spawn_handle_name), "player_spawn_%c_%d",
                      is_mpi_ ? GetRaceOfTeam(team) : human_race_char_, team) < 0)
        {
            spawn_handle_name[0] = '\0';
        }
        else
        {
            player_h = GetHandle(spawn_handle_name);
        }
    }

    if (player_h == 0)
    {
        char player_spawn_handle_log[GameConfig::MAX_CONSOLE_MSG_LENGTH];
        if (sprintf_s(player_spawn_handle_log, sizeof(player_spawn_handle_log),
                      "[IA 2.0]: No pre-placed spawns found for team: %d. Spawning a new ship.", team) < 0)
        {
            player_spawn_handle_log[0] = '\0';
        }
        else
        {
            PrintConsoleMessage(player_spawn_handle_log);
        }

        const float cur_floor = TerrainFindFloor(team_spawn_pos.x, team_spawn_pos.z) + 2.5f;
        team_spawn_pos.y = std::max(team_spawn_pos.y, cur_floor);

        if (is_mpi_)
        {
            player_h = BuildObject(GetPlayerODF(team), team, team_spawn_pos);
        }
        else
        {
            char new_player_odf[GameConfig::MAX_ODF_LENGTH];
            if (sprintf_s(new_player_odf, sizeof(new_player_odf), "%cvscout_x", human_race_char_) < 0)
            {
                new_player_odf[0] = '\0';
            }

            player_h = BuildObject(new_player_odf, team, team_spawn_pos);
        }

        SetRandomHeadingAngle(player_h);
    }

    team_pos_[3 * team + 0] = team_spawn_pos.x;
    team_pos_[3 * team + 1] = team_spawn_pos.y;
    team_pos_[3 * team + 2] = team_spawn_pos.z;

    char player_pilot_odf[GameConfig::MAX_ODF_LENGTH];
    if (sprintf_s(player_pilot_odf, sizeof(player_pilot_odf), "%cspilo_x",
                  is_mpi_ ? GetRaceOfTeam(team) : human_race_char_) < 0)
    {
        player_pilot_odf[0] = '\0';
    }

    SetPilotClass(player_h, player_pilot_odf);
    AddPilotByHandle(player_h);
    is_team_setup_[team] = true;

    return player_h;
}

// ==================================================
// Spawn Cleanup
// ==================================================
void Instant::CleanSpawns() const
{
    for (int i = 1; i <= max_players; ++i)
    {
        if (i <= player_count_)
        {
            continue;
        }

        char spawn_handle_name[GameConfig::MAX_ODF_LENGTH];
        if (sprintf_s(spawn_handle_name, sizeof(spawn_handle_name), "player_spawn_%c_%d", human_race_char_, i) < 0)
        {
            spawn_handle_name[0] = '\0';
            continue;
        }

        const Handle spawn_handle = GetHandle(spawn_handle_name);
        RemoveObject(spawn_handle);
    }
}

// ==================================================
// Main Logic
// ==================================================
void Instant::SetupMission()
{
    // This might be cool as an option in the shell for ease.
    SetAutoGroupUnits(false);

    // We are a 1.3 DLL. Show bot kill messages.
    WantBotKillMessages();

    // Start initializing the variables that we need from the shell.
    is_mpi_ = IsNetworkOn();
    mission_time_ = 0;
    start_done_ = false;
    game_over_ = false;
    map_name_ = GetMapTRNFilename();

    // Check to see if we're in an MPI session and adjust variables.
    if (is_mpi_)
    {
        intro_cutscene_enabled_ = IFace_GetInteger(GameConfig::MPI_INTRO_SCENE_ENABLED) > 0;
        wildlife_enabled_ = IFace_GetInteger(GameConfig::MPI_WILDLIFE_ENABLED) > 0;
        difficulty_ = IFace_GetInteger(GameConfig::MPI_DIFFICULTY) + 1;
        cpu_race_char_ = static_cast<char>(IFace_GetInteger(GameConfig::MPI_CPU_TEAM_RACE));
        human_race_char_ = GetRaceOfTeam(strat_team_);
        snipeable_enemies_ = IFace_GetInteger(GameConfig::MPI_SNIPEABLE_ENEMIES) > 0;
        cpu_scrap_delay_ = (4 - difficulty_) * 2 / CountPlayers();
        cpu_recycler_odf_ = GetVarItemStr(GameConfig::MPI_CPU_RECYCLER_ODF);
    }
    else
    {
        intro_cutscene_enabled_ = IFace_GetInteger(GameConfig::INTRO_SCENE_ENABLED) > 0;
        wildlife_enabled_ = IFace_GetInteger(GameConfig::WILDLIFE_ENABLED) > 0;
        difficulty_ = IFace_GetInteger(GameConfig::DIFFICULTY) + 1;
        cpu_race_char_ = static_cast<char>(IFace_GetInteger(GameConfig::HIS_RACE));
        human_race_char_ = static_cast<char>(IFace_GetInteger(GameConfig::MY_RACE));
        can_respawn_ = IFace_GetInteger(GameConfig::CAN_RESPAWN) > 0;
        cpu_scrap_delay_ = (4 - difficulty_) * 2;
        cpu_recycler_odf_ = GetVarItemStr(GameConfig::CPU_RECYCLER_ODF);
    }

    vsr_taunt_easter_egg_time_ = mission_time_ + SecondsToTurns(600.0f);
    cpu_scrap_amount_ = difficulty_;

    ClearTeamColors();

    if (cpu_race_char_ == human_race_char_)
    {
        if (cpu_race_char_ == GameConfig::FACTION_ISDF)
        {
            SetTeamColor(comp_team_, 0, 127, 255);
        }
        else
        {
            SetTeamColor(comp_team_, 85, 255, 85);
        }
    }
    
    char cpu_recycler_odf[GameConfig::MAX_ODF_LENGTH] = {};
    if (cpu_recycler_odf_ != nullptr && cpu_recycler_odf_[0] != '\0')
    {
        strcpy_s(cpu_recycler_odf, sizeof(cpu_recycler_odf), cpu_recycler_odf_);
    }
    else
    {
        strcpy_s(cpu_recycler_odf, sizeof(cpu_recycler_odf), "*vrecycpu");
    }
        
    enemy_recycler_ = BuildStartingVehicle(comp_team_, cpu_race_char_, cpu_recycler_odf, "*vrecy", Helpers::GetPathPosition("RecyclerEnemy"));
    
    BuildStartingVehicle(comp_team_, cpu_race_char_, "*vturr_c", "vturr_x", Helpers::GetPathPosition("TurretEnemy1"));
    BuildStartingVehicle(comp_team_, cpu_race_char_, "*vturr_c", "vturr_x", Helpers::GetPathPosition("TurretEnemy2"));
    
    if (wildlife_enabled_)
    {
        // Need to check if the map_name_ variable is either a Mire map or a Bane map.
        const bool is_mire_map = std::find(std::begin(GameConfig::MIRE_MAPS), std::end(GameConfig::MIRE_MAPS),
                                           map_name_) !=
            std::end(GameConfig::MIRE_MAPS);

        if (is_mire_map)
        {
            animal_manager_.SetupMireMapHerds();
        }
        else
        {
            char mother_odf_buffer[GameConfig::MAX_ODF_LENGTH] = {};
            char baby_odf_buffer[GameConfig::MAX_ODF_LENGTH] = {};

            if (std::strcmp(map_name_, "dunesi.trn") == 0)
            {
                strcpy_s(mother_odf_buffer, sizeof(mother_odf_buffer), "bcrhino_s");
                strcpy_s(baby_odf_buffer, sizeof(baby_odf_buffer), "bcrhino_s_b");
            }
            else
            {
                strcpy_s(mother_odf_buffer, sizeof(mother_odf_buffer), "bcrhino_x");
                strcpy_s(baby_odf_buffer, sizeof(baby_odf_buffer), "bcrhino_x_b");
            }

            animal_manager_.SetupBaneMapHerds(mother_odf_buffer, baby_odf_buffer);
        }
    }
    
    cpu_manager_.RegisterNewTeam(comp_team_, cpu_race_char_, "RecyclerEnemy", false, human_race_char_);

    intro_ship_1_ = GetHandle("intro_drop_1");
    intro_ship_2_ = GetHandle("intro_drop_2");
    intro_ship_3_ = GetHandle("intro_drop_3");

    scion_intro_hangar_ = GetHandle("scion_intro_hangar");
    scion_intro_matriarch_ = GetHandle("intro_matriarch");
    scion_intro_turret_1_ = GetHandle("intro_turret_1_scion");
    scion_intro_turret_2_ = GetHandle("intro_turret_2_scion");

    intro_turret_1_ = GetHandle("turret1");
    intro_turret_2_ = GetHandle("turret2");

    Stop(intro_turret_1_);
    Stop(intro_turret_2_);

    if (!intro_cutscene_enabled_)
    {
        DisableIntro();
        BuildPlayerRecycler(Helpers::GetPathPosition("Recycler"));

        const Vector recycler_spawn_pos = GetPosition(recycler_);
        BuildStartingVehicle(player_team_, human_race_char_, "*vturr_xm", "*vturr_x", GetPositionNear(recycler_spawn_pos, 40.0f, 60.0f));
        BuildStartingVehicle(player_team_, human_race_char_, "*vturr_xm", "*vturr_x", GetPositionNear(recycler_spawn_pos, 40.0f, 60.0f));
        BuildPlayerVehicles(GetPositionNear(recycler_spawn_pos, 40.0f, 60.0f));
        BuildCarriers();
    }

    const Handle player_entry_h = GetPlayerHandle(1);
    if (player_entry_h != 0)
    {
        RemoveObject(player_entry_h);
    }

    if (ImServer() || !is_mpi_)
    {
        elapsed_game_time_ = 0;
    }

    const int local_team_num = GetLocalPlayerTeamNumber();
    const Handle player_h = SetupPlayer(local_team_num);
    SetAsUser(player_h, local_team_num);
    PrintConsoleMessage("[IA 2.0]: New DLL written by AI_Unit.");

    start_done_ = true;
}

void Instant::Execute()
{
    mission_time_++;
    subtitles_.Execute();
    animal_manager_.Execute(mission_time_);
    carrier_manager_.Execute(mission_time_);
    cpu_manager_.Execute(mission_time_);

    if (!start_done_)
    {
        SetupMission();
        return;
    }

    if (!intro_done_)
    {
        UpdateIntro();
        HandleDropshipRemoval();
        return;
    }

    GameConditions();

    if (next_cpu_scrap_time_ <= mission_time_)
    {
        AddScrap(comp_team_, cpu_scrap_amount_);
        next_cpu_scrap_time_ = mission_time_ + cpu_scrap_delay_;
    }
}

// ==================================================
// Delegates
// ==================================================
void Instant::PreOrdnanceHit(const Handle shooter_handle, const Handle victim_handle, int ordnance_team,
                             const char* ordnance_odf)
{
    char obj_class[GameConfig::MAX_ODF_LENGTH] = {};
    if (!GetObjInfo(victim_handle, Get_EntityType, obj_class))
    {
        return;
    }

    if (std::strcmp(obj_class, "CLASS_ID_ANIMAL") == 0)
    {
        const char* victim_label = GetLabel(victim_handle);
        int victim_index = 0;
        if (sscanf_s(victim_label, "%d", &victim_index) != 1)
        {
            return;
        }

        animal_manager_.AnimalShot(victim_index, mission_time_, shooter_handle);
    }
}

void Instant::AddObject(const Handle new_handle)
{
    char obj_class[GameConfig::MAX_ODF_LENGTH] = {};
    GetObjInfo(new_handle, Get_GOClass, obj_class);
    
    if (std::strcmp(obj_class, "CLASS_DEPOSIT") == 0)
    {
        cpu_manager_.AddMapPool(new_handle);
        return;
    }
    
    if (std::strcmp(obj_class, "CLASS_SCRAP") == 0)
    {
        cpu_manager_.AddMapScrap(new_handle);
        return;
    }
    
    if (std::strcmp(obj_class, "CLASS_POWERUP_SERVICE") == 0)
    {
        cpu_manager_.AddServicePod(new_handle);
        return;
    }
    
    const int team = GetTeamNum(new_handle);

    if (team == player_team_)
    {
        SetSkill(new_handle, 3);
    }
    else if (team == comp_team_)
    {
        SetSkill(new_handle, difficulty_);
        
        if (Helpers::IsRecycler(new_handle))
        {
            enemy_recycler_ = new_handle;
        }
    }
    
    char obj_odf[GameConfig::MAX_ODF_LENGTH] = {};
    if (!GetObjInfo(new_handle, Get_ODF, obj_odf))
    {
        return;
    }
    
    const bool is_open = OpenODF(obj_odf);
    if (!is_open)
    {
        return;
    }

    char ai_unit_type[GameConfig::MAX_ODF_LENGTH] = {};
    GetODFString(obj_odf, "GameObjectClass", "AIUnitType", GameConfig::MAX_ODF_LENGTH, ai_unit_type);

    if (strcmp(ai_unit_type, GameConfig::AIUnitType::LANDING_PAD) == 0)
    {
        carrier_manager_.RegisterLandingPad(new_handle, team);
    }
    else if (strcmp(ai_unit_type, GameConfig::AIUnitType::DROPSHIP_REQUEST) == 0)
    {
        carrier_manager_.RegisterDropshipRequest(new_handle, team,mission_time_ + SecondsToTurns(GameConfig::GetDropshipCooldownRequestTime(difficulty_)));
    }
            
    if (team == comp_team_ && strcmp(ai_unit_type, GameConfig::AIUnitType::CARRIER) != 0)
    {
        cpu_manager_.AddTeamObject(new_handle, mission_time_, team, ai_unit_type);
    }

    CloseODF(obj_odf);
}

void Instant::DeleteObject(const Handle dead_handle)
{
    char obj_class[GameConfig::MAX_ODF_LENGTH] = {};
    GetObjInfo(dead_handle, Get_GOClass, obj_class);
    
    if (std::strcmp(obj_class, "CLASS_SCRAP") == 0)
    {
        cpu_manager_.RemoveMapScrap(dead_handle);
        return;
    }
    
    if (std::strcmp(obj_class, "CLASS_POWERUP_SERVICE") == 0)
    {
        cpu_manager_.RemoveServicePod(dead_handle);
        return;
    }
}

// ==================================================
// ISDF Intro
// ==================================================
void Instant::ISDFCleanup()
{
    RemoveScionIntroUnits();

    if (player_count_ < 2)
    {
        RemoveObject(intro_ship_3_);
        dropship_3_removed_ = true;
        dropship_3_takeoff_ = true;
    }

    CleanSpawns();
    SetColorFade(1.0f, 0.5f, RGBA_MAKE(0, 0, 0, 255));
    StartEarthQuake(5.0f);

    music_option_value_ = GetVarItemInt(GameConfig::OPTIONS_AUDIO_MUSIC);
    IFace_SetInteger(GameConfig::OPTIONS_AUDIO_MUSIC, 0);

    SetAnimation(intro_ship_2_, "deploy", 1);

    intro_music_ = StartSoundEffect("IA_Intro.wav");
    intro_delay_ = mission_time_ + SecondsToTurns(4.0f);
    isdf_intro_state_ = ISDFIntroState::PlayLine1;
}

void Instant::ISDFIntroLine1()
{
    if (!set_intro_music_volume_)
    {
        SetVolume(intro_music_, intro_music_volume_);
        set_intro_music_volume_ = true;
    }

    if (!CanAdvanceIntro())
    {
        return;
    }

    intro_audio_ = subtitles_.AudioWithSubtitles("IA_Pilot_1.wav", GameConfig::SubtitlePanelSize::Small);
    intro_delay_ = mission_time_ + SecondsToTurns(10.0f);
    isdf_intro_state_ = ISDFIntroState::Touchdown;
}

void Instant::ISDFTouchdown()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    UpdateEarthQuake(30.0f);
    intro_delay_ = mission_time_ + SecondsToTurns(0.2f);
    isdf_intro_state_ = ISDFIntroState::PrepareDropship;
}

void Instant::ISDFPrepareDropship()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    StopEarthQuake();
    intro_delay_ = mission_time_ + SecondsToTurns(4.0f);
    isdf_intro_state_ = ISDFIntroState::PlayLine2;
}

void Instant::ISDFIntroLine2()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    intro_audio_ = subtitles_.AudioWithSubtitles("IA_Pilot_2.wav", GameConfig::SubtitlePanelSize::Small);
    intro_delay_ = mission_time_ + SecondsToTurns(6.0f);
    isdf_intro_state_ = ISDFIntroState::DispatchUnits;
}

void Instant::ISDFDispatchUnits()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    SetAnimation(intro_ship_1_, "deploy", 1);

    if (IsAround(intro_ship_3_))
    {
        SetAnimation(intro_ship_3_, "deploy", 1);
    }
    
    BuildPlayerRecycler(Helpers::GetPathPosition("Recycler"));
    BuildPlayerVehicles(GetPosition(recycler_), true);

    Goto(recycler_, "recycler_go", 0);

    intro_delay_ = mission_time_ + SecondsToTurns(2.5f);
    isdf_intro_state_ = ISDFIntroState::PlayLine3;
}

void Instant::ISDFIntroLine3()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    StartSoundEffect("dropdoor.wav", intro_ship_1_);
    if (IsAround(intro_ship_3_))
    {
        StartSoundEffect("dropdoor.wav", intro_ship_3_);
    }

    Goto(intro_turret_1_, "turret_1_go", 1);
    Goto(intro_turret_2_, "turret_2_go", 1);

    dropship_takeoff_check_ = true;
    intro_audio_ = subtitles_.AudioWithSubtitles("IA_Pilot_3.wav", GameConfig::SubtitlePanelSize::Small);
    isdf_intro_state_ = ISDFIntroState::CheckEnemiesAreDead;
}

void Instant::ISDFCheckEnemiesAreDead()
{
    if (!IntroEnemiesKilled())
    {
        return;
    }

    isdf_intro_state_ = ISDFIntroState::SpawnCarriers;
}

void Instant::ISDFSpawnCarriers()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    SetBestGroup(intro_turret_1_);
    SetBestGroup(intro_turret_2_);
    Defend(intro_turret_1_, 0);
    Defend(intro_turret_2_, 0);

    BuildCarriers();

    intro_audio_ = subtitles_.AudioWithSubtitles("IA_Carrier_1.wav", GameConfig::SubtitlePanelSize::Small);
    isdf_intro_state_ = ISDFIntroState::CarrierLine2;
}

void Instant::ISDFCarrierLine2()
{
    if (!IsAudioMessageDone(intro_audio_))
    {
        return;
    }

    intro_audio_ = subtitles_.AudioWithSubtitles("IA_Carrier_2.wav", GameConfig::SubtitlePanelSize::Small);
    isdf_intro_state_ = ISDFIntroState::End;
}

void Instant::ISDFEndIntro()
{
    if (!CanAdvanceIntro())
    {
        return;
    }

    intro_music_volume_ -= 0.02f;
    intro_delay_ = mission_time_ + SecondsToTurns(0.3f);
    SetVolume(intro_music_, intro_music_volume_);

    if (intro_music_volume_ <= 0.0f)
    {
        StopAudio(intro_music_);
        IFace_SetInteger(GameConfig::OPTIONS_AUDIO_MUSIC, music_option_value_);
        intro_done_ = true;
    }
}

// ==================================================
// Scion Intro
// ==================================================

DLLBase* BuildMission()
{
    return new Instant();
}
