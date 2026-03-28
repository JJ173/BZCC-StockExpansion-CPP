#ifndef GAMECONFIG_H
#define GAMECONFIG_H

namespace GameConfig
{
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
    
    constexpr auto MPI_DIFFICULTY = "network.session.ivar127";
    constexpr auto MPI_INTRO_SCENE_ENABLED = "network.session.ivar126";
    constexpr auto MPI_WILDLIFE_ENABLED = "network.session.ivar125";
    constexpr auto MPI_COMMANDER_ENABLED = "network.session.ivar124";
    constexpr auto MPI_SNIPEABLE_ENEMIES = "network.session.ivar123";
    constexpr auto MPI_PLAYER_COUNT = "network.session.ivar64";
    constexpr auto MPI_CPU_TEAM_RACE = "network.session.ivar13";
    
    constexpr auto OPTIONS_AUDIO_MUSIC = "options.audio.music";
    constexpr auto OPTIONS_PLAY_SUBTITLES = "options.play.subtitles";
    
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
}
#endif // GAMECONFIG_H
