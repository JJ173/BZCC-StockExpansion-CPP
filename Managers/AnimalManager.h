#pragma once

#include <vector>
#include "../Shared/DllBase.h"
#include "../Shared/GameConfig.h"

class AnimalManager
{
    const int MAX_HERDS = 5;

    struct Animal
    {
        Handle handle;
        GameConfig::AnimalState state;
        Vector flee_point;
    };

    struct AnimalHerd
    {
        int team;
        const char* mother_odf;
        const char* baby_odf;
        Handle mother;
        std::vector<Animal> babies; // Dynamically allocated array of animals
        int mother_attack_time;
        float baby_flee_distance;
        GameConfig::AnimalState state;
    };

    std::vector<AnimalHerd> herds_;

    bool enable_herd_logic_ = false;
    static bool IsValidHerdPath(const char* herd_path);

public:
    void SetupBaneMapHerds(const char* mother_odf, const char* baby_odf);
    void SetupMireMapHerds() const;
    void AnimalShot(int herd_index, int shot_turn, Handle threat);
    void Execute(int mission_turn);
};
