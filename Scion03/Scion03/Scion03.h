#pragma once

#include "../../Shared/DLLBase.h"
#include "../../Utilities/Helpers.h"
#include "../../Utilities/Subtitles.h"

enum class Phase: uint8_t
{
    MissionSetup,
    IntroCutscene,
    FirstObjective,
    BridgeObjective,
};

class Scion03 : public DLLBase
{
    static constexpr int BRADDOCK_MAX_WAVES = 11;
    
    static constexpr int BRADDOCK_MAX_BRIDGE_ATTACKS = 10;
    static constexpr int BRADDOCK_MAX_PLAYER_WAVES = 12;
    static constexpr int BRADDOCK_PLAYER_WAVE_SIZE = 3;
    static constexpr int MAX_DIFFICULTY = 3;
    static constexpr int MAX_BRIDGE_SEGMENTS = 4;
    
    static constexpr float delay1[3] = { 105.f, 90.f, 75.f };
    static constexpr float delay2[3] = { 160.f, 140.f, 120.f };
    static constexpr float braddock_first_delay[3] = { 120.0f, 90.0f, 60.0f };
    
    static constexpr const char* WaveUnits[BRADDOCK_MAX_WAVES][BRADDOCK_MAX_BRIDGE_ATTACKS] =
    {
        {
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::MortarBikeISDF,
            GameConfig::ODFs::MortarBikeISDF,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
        },
        {
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::MortarBikeISDF,
            GameConfig::ODFs::MortarBikeISDF,
            GameConfig::ODFs::MissileISDF,
            GameConfig::ODFs::MissileISDF,
            nullptr,
            nullptr
        },
        {
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::MortarBikeISDF,
            GameConfig::ODFs::MortarBikeISDF,
            GameConfig::ODFs::MissileISDF,
            GameConfig::ODFs::MissileISDF,
            nullptr,
            nullptr
        },
        {
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            nullptr,
            nullptr,
            nullptr
        },
        {
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            nullptr
        },
        {
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            nullptr
        },
        {
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::TankISDF,
            GameConfig::ODFs::TankISDF,
            nullptr
        },
        {
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
        },
        {
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
            GameConfig::ODFs::RocketTankISDF,
        },
        {
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
            GameConfig::ODFs::AssaultTankISDF,
        },
        {
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
            GameConfig::ODFs::WalkerISDF,
        }
    };

    // Just to help explain the 3D elements:
    // Size BRADDOCK_MAX_PLAYER_WAVES = 12 Waves.
    // Size BRADDOCK_PLAYER_WAVE_SIZE (2nd) = Max units per attack wave.
    // Size MAX_DIFFICULTY (3rd) = Difficulty.
    static constexpr const char* PlayerAttackWaveUnits[BRADDOCK_MAX_PLAYER_WAVES][BRADDOCK_PLAYER_WAVE_SIZE][MAX_DIFFICULTY]
    {
        {
            {GameConfig::ODFs::ScoutISDF, GameConfig::ODFs::ScoutISDF, nullptr},
            {GameConfig::ODFs::ScoutISDF, GameConfig::ODFs::MissileISDF, nullptr},
            {GameConfig::ODFs::ScoutISDF, GameConfig::ODFs::TankISDF, nullptr}
        },
        {
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::ScoutISDF, nullptr},
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::MissileISDF, nullptr},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::MissileISDF, nullptr}
        },
        {
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::ScoutISDF, nullptr},
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::MissileISDF, nullptr},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::MortarBikeISDF}
        },
        {
            {GameConfig::ODFs::ScoutISDF, GameConfig::ODFs::MissileISDF, nullptr},
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::MortarBikeISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF}
        },
        {
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::MissileISDF, nullptr},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::MortarBikeISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF}
        },
        {
            {GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::MortarBikeISDF, nullptr},
            {GameConfig::ODFs::MissileISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::MortarBikeISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF}
        },
        {
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::ScoutISDF, GameConfig::ODFs::ScoutISDF},
            {GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::TankISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::ScoutISDF}
        },
        {
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::ScoutISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::MissileISDF, GameConfig::ODFs::MortarBikeISDF},
            {GameConfig::ODFs::RocketTankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::ScoutISDF}
        },
        {
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::ScoutISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF, GameConfig::ODFs::RocketTankISDF},
            {GameConfig::ODFs::TankISDF, GameConfig::ODFs::RocketTankISDF, GameConfig::ODFs::AssaultTankISDF}
        },
        {
            {GameConfig::ODFs::RocketTankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::ScoutISDF},
            {GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::TankISDF, GameConfig::ODFs::MortarBikeISDF},
            {GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::TankISDF}
        },
        {
            {GameConfig::ODFs::RocketTankISDF, GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::ScoutISDF},
            {GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::RocketTankISDF},
            {GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::WalkerISDF, GameConfig::ODFs::AssaultTankISDF}
        },
        {
            {GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::RocketTankISDF, GameConfig::ODFs::RocketTankISDF},
            {GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::AssaultTankISDF, GameConfig::ODFs::WalkerISDF},
            {GameConfig::ODFs::WalkerISDF, GameConfig::ODFs::WalkerISDF, GameConfig::ODFs::AssaultTankISDF}
        }
    };

    struct Paths
    {
        static constexpr auto CinematicMove = "cin_move";
        static constexpr auto CinematicRcktMove = "cin_rckt_move";
        static constexpr auto CinematicCamera = "cin1_camera";
        static constexpr auto MansonPatrol = "manson_patrol";
        static constexpr auto NRPatrol1 = "tank_1_patrol";
        static constexpr auto NRPatrol2 = "tank_2_patrol";
    };

    struct AudioFiles
    {
        static constexpr auto IntroVO = "CutSc0501.wav";
        static constexpr auto FirstObjectiveVO = "scion0501.wav";
        static constexpr auto BridgeObjectiveVO = "scion0502.wav";

        static constexpr auto BraddockContactVO = "isdf2021.wav";
        static constexpr auto MansonAnswerBraddockVO = "isdf2022.wav";
        static constexpr auto BraddockAnswerMansonVO = "scion0301_new.wav";

        static constexpr auto MansonHelpVO = "scion0503.wav";
    };

    struct Objectives
    {
        static constexpr auto FirstObjectiveOTF = "scion0501_new.otf";
        static constexpr auto BridgeObjectiveOTF = "scion0502_new.otf";
    };

    struct AIPs
    {
        static constexpr auto MansonFirstAIP = "scion0301_x.aip";
        static constexpr auto BraddockFirstAIP = "scion0302_x.aip";
        static constexpr auto MansonSecondAIP = "scion0303_x.aip";
        static constexpr auto BraddockSecondAIP = "scion0304_x.aip";
    };
    
public:
    Scion03();

    struct MissionState
    {
        bool first_bool_ = false;
        Phase currentPhase = Phase::MissionSetup;

        struct Flags
        {
            bool isSetup = false;
            bool isCooperative = false;
            bool isCameraReady = false;
            bool isFirstShotOver = false;
            bool isFirstObjectiveDelaySet = false;
            bool isFirstObjectiveSet = false;

            bool isBridgeObjectiveActive = false;
            bool isBridgeAlive = true;
            bool isBridgeObjectiveSet = false;

            bool isBraddockAANAttacksActive = false;
            bool isBraddockPlayerAttacksActive = false;

            bool hasFirstObjectiveVoicePlayed = false;
            bool hasFirstCutsceneVoicePlayed = false;
            bool hasBridgeObjectiveVoicePlayed = false;

            bool hasBraddockContactedManson = false;
            bool hasMansonAnsweredBraddock = false;
            bool hasBraddockAnswerManson = false;

            bool hasBraddockWalkerMoved = false;
            bool hasBraddockFirstWaveSpawned = false;

            bool hasMansonCalledForHelp = false;
            bool hasMansonPowerDialogPlayed = false;
            bool hasMansonGunTowerDialogPlayed = false;

            bool isPlayerRecyDead = false;
        } flags;

        bool last_bool_ = false;

        Handle first_handle_ = 0;

        struct Units
        {
            Handle cut1VO;
            Handle generalVO;

            Handle aanRecy;
            Handle braddockRecy;
            Handle playerRecy;

            Handle aanEntryTracker;
            Handle cutsceneLook;

            Handle offmap_dropship_1;
            Handle offmap_dropship_2;
            Handle offmap_dropship_3;
            
            Handle bridgeSegements[4];
            
            UnitTracker manson;
            UnitTracker aanTanks[3];

            UnitTracker patrol1;
            UnitTracker patrol2;

            UnitTracker braddockWalker;
            UnitTracker braddockTrucks[2];
            UnitTracker braddockAssaultTank;
            
            UnitTracker braddockOffMapAttacks[3];
            UnitTracker cinematicUnits[7];
        } units;

        Handle last_handle_ = 0;

        int first_int_ = 0;

        struct Data
        {
            int missionDifficulty = 0;
            int missionTurn = 0;

            int hostTeam = 1;
            int alliedTeam = 5;
            int enemyTeam = 6;
            int tempEnemyTeam = 7;

            int braddockAttackAANCounter = 0;
            int braddockAttackPlayerCounter = 0;

            int waveCounter = 0;

            float braddockAANAttackDelays[3] = {160.0f, 140.0f, 120.0f};

            Vector braddockOffMapSpawnPos = Vector(0.0f, 0.0f, 0.0f);
        } data;

        struct Timers
        {
            MissionTimer firstCinematicTimer;
            MissionTimer voiceTimer;
            MissionTimer waveTimerAAN;
            MissionTimer waveTimerPlayer;
            MissionTimer missionDelayTimer;
        } timers;

        Subtitles subtitles;
        int last_int_ = 0;
    } state;

    // --- Overrides ---
    void AddObject(Handle h) override;
    void DeleteObject(Handle h) override;
    void Execute() override;

    bool Save(bool missionSave) override;
    bool Load(bool missionSave) override;
    bool PostLoad(bool missionSave) override;

protected:
    bool* b_array_;
    int b_count_;

    Handle* h_array_;
    int h_count_;

    int* i_array_;
    int i_count_;

private:
    void RegisterVariables();
    void SetupMission();

    void RunIntroCutscene();
    void RunFirstObjective();
    void RunBridgeObjective();

    void ManageAAN();
    void ManageBraddock();
};