#pragma once

#include <vector>
#include "../Shared/DllBase.h"
#include "../Shared/GameConfig.h"

class AnimalManager
{    
    struct Animal
    {
        Handle handle;
        GameConfig::AnimalState state;
        const char* flee_path;
    };
    
    struct AnimalHerd
    {
        int team;
        const char* mother_odf;
        const char* baby_odf;
        Animal* mother;
        std::vector<Animal> babies; // Dynamically allocated array of animals
        int mother_attack_time;
        int baby_flee_distance;
        GameConfig::AnimalState state;
    };
};
