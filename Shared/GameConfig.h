#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <cstdint>
#include <cstring>
#include <string>

namespace GameConfig
{
    namespace AIUnitType
    {
        constexpr auto ANTI_AIR = "AntiAir";
        constexpr auto APC_PATROL = "APCPatrol";
        constexpr auto ASSAULT = "Assault";
        constexpr auto ASSAULT_DEFENDER = "AssaultDefender";
        constexpr auto ASSAULT_SERVICE = "AssaultService";
        constexpr auto BASE_PATROL = "BasePatrol";
        constexpr auto CARRIER = "Carrier";
        constexpr auto COMMANDER = "Commander";
        constexpr auto DEMOLISHER = "Demolisher";
        constexpr auto DROPSHIP_REQUEST = "DropshipRequest";
        constexpr auto LANDING_PAD = "LandingPad";
        constexpr auto LIEUTENANT = "Lieutenant";
        constexpr auto PATROL = "Patrol";
        constexpr auto TURRET = "Turret";
    }
    
    struct KeyValuePair
    {
        const char* key;
        const char* value;
    };

    enum : std::uint16_t
    {
        MAX_ODF_LENGTH = 64,
        MAX_NAME_LENGTH = 256,
        MAX_CONSOLE_MSG_LENGTH = 512,
        MAX_MESSAGE_LENGTH = 2048,
        MAX_SUBTITLE_LENGTH = 8192
    };

    enum Difficulty : std::uint8_t
    {
        EASY = 1,
        MEDIUM = 2,
        HARD = 3
    };

    enum SubtitlePanelSize : std::uint8_t
    {
        Small,
        Medium,
        Large
    };

    enum AnimalState : std::uint8_t
    {
        Grazing,
        Attacking,
        Fleeing,
        Following
    };

    enum CondorState : std::uint8_t
    {
        Landing,
        Replace,
        BuildUnits,
        OffloadUnits,
        Leave,
        Remove
    };
    
    enum AIPType : std::uint8_t
    {
        AIPType0, 
        AIPType1, 
        AIPType2, 
        AIPType3, 
        AIPTypeA, 
        AIPTypeL, 
        AIPTypeS, 
        MAX_AIP_TYPE, 
    };

    constexpr int MAX_PLAYERS = 4;
    constexpr int DEFAULT_TPS = 20;

    constexpr char FACTION_ISDF = 'i';
    constexpr char FACTION_SCION = 'f';

    constexpr auto CAN_RESPAWN = "options.instant.bool0";
    constexpr auto INTRO_SCENE_ENABLED = "options.instant.introScene";
    constexpr auto WILDLIFE_ENABLED = "options.instant.wildlife";
    constexpr auto HIS_RACE = "options.instant.hisrace";
    constexpr auto MY_RACE = "options.instant.myrace";
    constexpr auto DIFFICULTY = "options.instant.difficulty";
    constexpr auto AIP_STRING = "options.instant.string0";
    constexpr auto COMMANDER_ENABLED = "options.instant.aiCommander";
    constexpr auto RECYCLER_ODF = "options.instant.string1";
    constexpr auto CPU_RECYCLER_ODF = "options.instant.string2";
    constexpr auto PILOT_MODE_ENABLED = "options.instant.pilotMode";
    constexpr auto INFANTRY_ENABLED = "options.instant.infantry";

    constexpr auto MPI_DIFFICULTY = "network.session.ivar127";
    constexpr auto MPI_INTRO_SCENE_ENABLED = "network.session.ivar126";
    constexpr auto MPI_WILDLIFE_ENABLED = "network.session.ivar125";
    constexpr auto MPI_COMMANDER_ENABLED = "network.session.ivar124";
    constexpr auto MPI_SNIPEABLE_ENEMIES = "network.session.ivar123";
    constexpr auto MPI_PILOT_MODE_ENABLED = "network.session.ivar122";
    constexpr auto MPI_INFANTRY_ENABLED = "network.session.ivar121";
    constexpr auto MPI_PLAYER_COUNT = "network.session.ivar64";
    constexpr auto MPI_CPU_TEAM_RACE = "network.session.ivar13";
    constexpr auto MPI_CPU_RECYCLER_ODF = "network.session.ivar12";

    constexpr auto OPTIONS_AUDIO_MUSIC = "options.audio.music";
    constexpr auto OPTIONS_PLAY_SUBTITLES = "options.play.subtitles";

    constexpr char AIP_TYPE_STRINGS[7] =
    {
        '0',
        '1',
        '2',
        '3',
        'a',
        'l',
        's'
    };
    
    constexpr const char* BANE_MAPS[6] =
    {
        "dunesi.trn",
        "chill.trn",
        "ground4.trn",
        "ground0.trn",
        "MPIIsland.trn",
        "sea_battle.trn"
    };

    constexpr const char* CPU_NAMES[16] =
    {
        "SIR BRAMBLEY",
        "GrizzlyOne95",
        "BlackDragon",
        "Spymaster",
        "Autarch Katherlyn",
        "blue_banana",
        "Zorn",
        "Gravey",
        "VTrider",
        "Ultraken",
        "Darkvale",
        "Econchump",
        "Sev",
        "TheBonelord",
        "Lithium",
        "Uncle Kunckles"
    };

    constexpr const char* MIRE_MAPS[8] =
    {
        "bridges.trn",
        "mpicanyons.trn",
        "iacirclebzcc.trn",
        "iadustbzcc.trn",
        "iaentrapbzcc.trn",
        "iafirebzcc.trn",
        "iafortbzcc.trn",
        "iaghzonebzcc.trn"
    };

    static constexpr KeyValuePair CondorUnits[] =
    {
        {"ScavengerDropship", "ivscav_x"},
        {"ScrapDropship", "ivscrap_gh"},
        {"LightDropship", "ivmisl_x"},
        {"TurretDropship", "ivturr_x"}
    };

    static constexpr KeyValuePair PortalUnits[] =
    {
        {"ScavengerDropship", "fvscav_x"},
        {"ScrapDropship", "fvscrap_gh"},
        {"LightDropship", "fvsent_x"},
        {"TurretDropship", "fvturr_x"}
    };
        
    inline const char* GetPortalUnit(const std::string& name)
    {
        for (const auto& entry : PortalUnits)
        {
            if (std::strcmp(entry.key, name.c_str()) == 0)
            {
                return entry.value;
            }
        }
        return nullptr;
    }

    inline const char* GetCondorUnit(const std::string& name)
    {
        for (const auto& entry : CondorUnits)
        {
            if (std::strcmp(entry.key, name.c_str()) == 0)
            {
                return entry.value;
            }
        }
        return nullptr;
    }

    inline float GetDropshipCooldownRequestTime(const int difficulty)
    {
        switch (difficulty)
        {
        case EASY:
            return DEFAULT_TPS * 60 * 5.0f;
        case MEDIUM:
            return DEFAULT_TPS * 60 * 7.5f;
        case HARD:
            return DEFAULT_TPS * 60 * 10.0f;
        default:
            return 0.0f;
        }
    }
}
#endif // GAMECONFIG_H
