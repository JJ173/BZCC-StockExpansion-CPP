#ifndef GAMECONFIG_H
#define GAMECONFIG_H

namespace GameConfig
{
    constexpr int MAX_PLAYERS = 4;
    constexpr int DEFAULT_TPS = 20;

    constexpr auto CAN_RESPAWN = "options.instant.bool0";
    constexpr auto INTRO_SCENE_ENABLED = "options.instant.introScene";
    constexpr auto WILDLIFE_ENABLED = "options.instant.wildlife";
    constexpr auto HIS_RACE = "options.instant.hisrace";
    constexpr auto MY_RACE = "options.instant.myrace";
    constexpr auto DIFFICULTY = "options.instant.difficulty";
    constexpr auto AIP_STRING = "options.instant.string0";
    constexpr auto COMMANDER_ENABLED = "options.instant.aiCommander";
    
    constexpr auto MPI_INTRO_SCENE_ENABLED = "network.session.ivar126";
    constexpr auto MPI_WILDLIFE_ENABLED = "network.session.ivar125";
    constexpr auto MPI_COMMANDER_ENABLED = "network.session.ivar124";
    constexpr auto MPI_SNIPEABLE_ENEMIES = "network.session.ivar123";
    constexpr auto MPI_CPU_TEAM_RACE = "network.session.ivar13";
    constexpr auto MPI_PLAYER_COUNT = "network.session.ivar64";
    
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
}
#endif // GAMECONFIG_H
