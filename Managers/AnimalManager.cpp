#include "AnimalManager.h"
#include "../Utilities/Helpers.h"

bool AnimalManager::IsValidHerdPath(const char* herd_path)
{
    const Handle scrap = BuildObject("npscrx", 0, herd_path);
    return scrap != 0;
}

void AnimalManager::SetupBaneMapHerds(const char* mother_odf, const char* baby_odf)
{
    for (int i = 1; i <= MAX_HERDS; i++)
    {
        char herd_path[64] = {};
        if (sprintf_s(herd_path, sizeof(herd_path), "AnimalHerd%d", i) < 0)
        {
            herd_path[0] = '\0';
            continue;
        }

        if (!IsValidHerdPath(herd_path))
        {
            continue;
        }

        // Create an AnimalHerd from the struct.
        AnimalHerd herd;
        herd.team = 0;
        herd.mother_odf = mother_odf;
        herd.baby_odf = baby_odf;
        herd.mother_attack_time = 0.0f;
        herd.baby_flee_distance = 100.0f;
        herd.state = GameConfig::AnimalState::Grazing;
        herd.mother = BuildObject(mother_odf, 0, herd_path);

        // Create a small random number of babies to follow the mother.
        const int random_baby_count = Helpers::GetRandomInt(5.0f);

        for (int j = 0; j < random_baby_count; j++)
        {
            Animal baby;
            baby.handle = BuildObject(baby_odf, 0, GetPositionNear(GetPosition(herd.mother), 20.0f, 35.0f));
            baby.state = GameConfig::AnimalState::Following;
            baby.flee_path = nullptr;
            herd.babies.push_back(baby);
            SetRandomHeadingAngle(baby.handle);
        }

        herds_.push_back(herd);
    }
    
    enable_herd_logic_ = true;
}

void AnimalManager::SetupMireMapHerds()
{
    for (int i = 1; i <= MAX_HERDS; i++)
    {
        char herd_path[64] = {};
        if (sprintf_s(herd_path, sizeof(herd_path), "AnimalHerd%d", i) < 0)
        {
            herd_path[0] = '\0';
            continue;
        }

        if (!IsValidHerdPath(herd_path))
        {
            continue;
        }

        const int random_jak_count = Helpers::GetRandomInt(6.0f);
        for (int j = 0; j < random_jak_count; j++)
        {
            const Handle spawner = BuildObject("pspwn_1", 0, herd_path);
            if (spawner == 0)
            {
                continue;
            }

            Vector pos = GetPositionNear(GetPosition(spawner), 100.0f, 100.0f);
            pos.y = TerrainFindFloor(pos.x, pos.z) + 2.5f;

            const Handle jak = BuildObject("mcjak01", 0, pos);
            SetRandomHeadingAngle(jak);
        }

        const int random_bird_count = Helpers::GetRandomInt(6.0f);
        SpawnBirds(i, random_bird_count, "mcwing01", 0, herd_path);
    }
}
