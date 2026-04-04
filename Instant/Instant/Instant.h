#pragma once

#include <memory>
#include <string>

#include "../../Managers/AnimalManager.h"
#include "../../Managers/CarrierManager.h"
#include "../../Managers/CPUManager.h"
#include "../../Shared/DllBase.h"
#include "../../Utilities/Subtitles.h"

class Instant : public DLLBase
{
    Subtitles subtitles_;
    AnimalManager animal_manager_;
    CarrierManager carrier_manager_;
    CPUManager cpu_manager_;

    enum class ISDFIntroState : std::uint8_t
    {
        Cleanup,
        PlayLine1,
        Touchdown,
        PrepareDropship,
        PlayLine2,
        DispatchUnits,
        PlayLine3,
        CheckEnemiesAreDead,
        SpawnCarriers,
        CarrierLine2,
        End
    };

    enum class ScionIntroState : std::uint8_t
    {
        Cleanup,
        FadeIn,
        StartMusic,
        PlayLine1,
        Shake,
        StopShake,
        PlayLine2,
        DeployPlayer,
    };

    static constexpr int max_players = 4;

    int game_tps_;

    int first_int_;
    int mission_time_;
    int elapsed_game_time_;
    int player_team_;
    int strat_team_;
    int comp_team_;
    int difficulty_;
    int intro_delay_;
    int player_count_;
    int vsr_taunt_easter_egg_time_;
    int cpu_scrap_amount_;
    int cpu_scrap_delay_;
    int next_cpu_scrap_time_;
    int music_option_value_;
    int intro_audio_;
    int intro_music_;
    int dropship_1_time_;
    int dropship_2_time_;
    int dropship_3_time_;
    int last_int_;

    ISDFIntroState isdf_intro_state_ = ISDFIntroState::Cleanup;
    ScionIntroState scion_intro_state_ = ScionIntroState::Cleanup;

    char human_race_char_;
    char cpu_race_char_;
    const char* map_name_;

    float first_float_;
    float team_pos_[3 * (MAX_TEAMS + 1)];
    float intro_music_volume_ = 1.0f;
    float last_float_;

    bool first_bool_;
    bool start_done_;
    bool intro_done_;
    bool game_over_;
    bool is_team_setup_[MAX_TEAMS];
    bool is_mpi_;
    bool intro_enemies_spawned_;
    bool can_respawn_;
    bool intro_cutscene_enabled_;
    bool wildlife_enabled_;
    bool snipeable_enemies_;
    bool dropship_takeoff_check_;
    bool dropship_takeoff_dialog_played_;
    bool dropship_1_removed_;
    bool dropship_2_removed_;
    bool dropship_3_removed_;
    bool dropship_1_takeoff_;
    bool dropship_2_takeoff_;
    bool dropship_3_takeoff_;
    bool set_intro_music_volume_;
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

    // Handle building any starting vehicles when the game begins.
    Handle BuildStartingVehicle(int team, char race, const std::string& first_odf, const std::string& second_odf,
                                const Vector& spawn_pos) const;

    // Remove the ISDF intro units.
    void RemoveISDFIntroUnits() const;

    // Remove the Scion intro units.
    void RemoveScionIntroUnits() const;

    // Disables the intro sequence.
    void DisableIntro();

    // Handles controlling Dropship takeoff and removal.
    void HandleDropshipRemoval();

    // Handles checking the "Game Over" sequence.
    void GameConditions();

    // Handles calling the carrier manager to build carriers.
    void BuildCarriers();

    // Handles updating the intros for each faction.
    void UpdateIntro();

    // Handles the ISDF intro sequence.
    void UpdateISDFIntro();

    // Handles the Scion intro sequence.
    void UpdateScionIntro();

    // Handles advancing the ISDF intro state.
    void AdvanceISDFIntroState(ISDFIntroState next, float delay_seconds);

    // Handles advancing the Scion intro state.
    void AdvanceScionIntroState(ScionIntroState next, float delay_seconds);

    // Returns true if the intro can be advanced.
    bool CanAdvanceIntro() const;

    // If the intro is enabled, check to see if the intro enemies are dead.
    bool IntroEnemiesKilled();

    // Handle building the player Recycler when the game begins.
    void BuildPlayerRecycler(const Vector& position);

    // Handle building extra player vehicles depending on difficulty.
    void BuildPlayerVehicles(const Vector& position, bool is_intro = false) const;

    // Set up any players in the game.
    Handle SetupPlayer(int team);

    // Clean any player spawns that are lingering at the start.
    void CleanSpawns() const;

    // ISDF Intro Functions.
    void ISDFCleanup();
    void ISDFIntroLine1();
    void ISDFTouchdown();
    void ISDFPrepareDropship();
    void ISDFIntroLine2();
    void ISDFDispatchUnits();
    void ISDFIntroLine3();
    void ISDFCheckEnemiesAreDead();
    void ISDFSpawnCarriers();
    void ISDFCarrierLine2();
    void ISDFEndIntro();

    // Scion Intro Functions.


protected:
    bool* b_array_;
    int b_count_;

    float* f_array_;
    int f_count_;

    int* h_array_;
    int h_count_;

    int* i_array_;
    int i_count_;

public:
    // Constructor
    Instant();

    // Destructor
    ~Instant() override;

    // Initial setup of the mission.
    void SetupMission();

    // Runs each frame.
    void Execute() override;

    // Handle delegate for PreOrdnanceHit.
    void PreOrdnanceHit(Handle shooter_handle, Handle victim_handle, int ordnance_team,
                        const char* ordnance_odf) override;
    void AddObject(const Handle new_handle) override;
};
