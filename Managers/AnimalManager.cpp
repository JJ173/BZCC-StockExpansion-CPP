#include "AnimalManager.h"
#include <string>
#include "../Utilities/Helpers.h"

bool AnimalManager::IsValidHerdPath(const char* herd_path)
{
    const Handle scrap = BuildObject("npscrx", 0, herd_path);
    if (scrap != 0)
    {
        RemoveObject(scrap);
        return true;
    }

    return false;
}

void AnimalManager::SetupBaneMapHerds(const char* mother_odf, const char* baby_odf)
{
    for (int i = 1; i <= MAX_HERDS; i++)
    {
        char herd_path[GameConfig::MAX_NAME_LENGTH] = {};
        if (sprintf_s(herd_path, sizeof(herd_path), "AnimalHerd%d", i) < 0)
        {
            herd_path[0] = '\0';
            continue;
        }

        if (!IsValidHerdPath(herd_path))
        {
            continue;
        }

        AnimalHerd herd;
        herd.team = 0;
        herd.mother_odf = mother_odf;
        herd.baby_odf = baby_odf;
        herd.mother_attack_time = 0;
        herd.baby_flee_distance = 100.0f;
        herd.state = GameConfig::AnimalState::Grazing;
        herd.mother = BuildObject(mother_odf, 0, herd_path);

        std::string herd_indexer = std::to_string(i);
        SetLabel(herd.mother, herd_indexer.c_str());

        const int random_baby_count = Helpers::GetRandomInt(5.0f);
        for (int j = 0; j < random_baby_count; j++)
        {
            Animal baby;
            baby.handle = BuildObject(baby_odf, 0, GetPositionNear(GetPosition(herd.mother), 20.0f, 35.0f));
            baby.state = GameConfig::AnimalState::Following;
            baby.flee_point = Vector(0.0f, 0.0f, 0.0f);
            herd.babies.push_back(baby);
            SetRandomHeadingAngle(baby.handle);
            SetLabel(baby.handle, herd_indexer.c_str());
            Follow(baby.handle, herd.mother);
        }

        herds_.push_back(herd);
    }

    enable_herd_logic_ = true;
}

void AnimalManager::SetupMireMapHerds() const
{
    for (int i = 1; i <= MAX_HERDS; i++)
    {
        char herd_path[GameConfig::MAX_NAME_LENGTH] = {};
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

void AnimalManager::AnimalShot(const int herd_index, const int shot_turn, const Handle threat)
{
    const int corrected_index = herd_index - 1;

    if (corrected_index < 0 || static_cast<size_t>(corrected_index) >= herds_.size())
    {
        return;
    }

    AnimalHerd* herd = &herds_[corrected_index];

    if (IsAround(herd->mother))
    {
        if (herd->state == GameConfig::AnimalState::Attacking)
        {
            herd->mother_attack_time = shot_turn + SecondsToTurns(30.0f);
            return;
        }
        
        herd->state = GameConfig::AnimalState::Attacking;
        herd->team = 15;
        const int animal_scream = StartAudio3D("rhin08.wav", herd->mother);
        SetVolume(animal_scream, 0.3f);
        
        for (const Animal& baby : herd->babies)
        {
            if (!IsAround(baby.handle))
            {
                continue;
            }

            SetTeamNum(baby.handle, herd->team);
        }

        Attack(herd->mother, threat);
        herd->mother_attack_time = shot_turn + SecondsToTurns(30.0f);
    }
    else
    {
        for (Animal& baby : herd->babies)
        {
            if (!IsAround(baby.handle))
            {
                continue;
            }

            baby.state = GameConfig::AnimalState::Fleeing;
            baby.flee_point = GetPositionNear(GetPosition(threat), herd->baby_flee_distance + 60.0f,
                                              herd->baby_flee_distance + 100.0f);
            Goto(baby.handle, baby.flee_point);
        }
    }
}

void AnimalManager::Execute(const int mission_turn)
{
    if (!enable_herd_logic_)
    {
        return;
    }

    for (AnimalHerd& herd : herds_)
    {
        if (herd.state != GameConfig::AnimalState::Attacking)
        {
            continue;
        }

        if (mission_turn > herd.mother_attack_time)
        {
            // Return the herd back to neutral state.
            herd.state = GameConfig::AnimalState::Grazing;
            herd.team = 0;

            SetTeamNum(herd.mother, herd.team);
            Stop(herd.mother);

            // Update the babies as well. Set them to team 0 and make them follow again.
            for (Animal& baby : herd.babies)
            {
                if (!IsAround(baby.handle))
                {
                    continue;
                }

                baby.state = GameConfig::AnimalState::Following;
                SetTeamNum(baby.handle, herd.team);
                Follow(baby.handle, herd.mother);
            }

            return;
        }

        for (Animal& baby : herd.babies)
        {
            if (!IsAround(baby.handle))
            {
                continue;
            }

            switch (baby.state)
            {
            case GameConfig::AnimalState::Fleeing:
                if (GetDistance(baby.handle, herd.mother) >= herd.baby_flee_distance + 20)
                {
                    baby.state = GameConfig::AnimalState::Following;
                    Follow(baby.handle, herd.mother);
                }
                break;
            case GameConfig::AnimalState::Following:
                if (GetDistance(baby.handle, herd.mother) < herd.baby_flee_distance)
                {
                    baby.state = GameConfig::AnimalState::Fleeing;
                    baby.flee_point = GetPositionNear(GetPosition(herd.mother), herd.baby_flee_distance + 20.0f,
                                                      herd.baby_flee_distance + 50.0f);
                    Goto(baby.handle, baby.flee_point);
                }
                break;
            case GameConfig::AnimalState::Grazing:
            case GameConfig::AnimalState::Attacking:
                break;
            }
        }
    }
}
