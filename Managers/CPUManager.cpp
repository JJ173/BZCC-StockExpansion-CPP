#include "CPUManager.h"

#include <algorithm>
#include <cstdio>
#include <iterator>

#include "../shared/DLLUtils.h"
#include "../Utilities/Helpers.h"

namespace
{
    template <typename T, typename Predicate>
    bool EraseIf(std::vector<T>& items, Predicate predicate)
    {
        const auto it = std::remove_if(items.begin(), items.end(), predicate);
        if (it == items.end())
        {
            return false;
        }

        items.erase(it, items.end());
        return true;
    }
}

// ==================================================
// Utilities
// ==================================================
Handle CPUManager::GetProducer(const int team) const
{
    if (team < 0 || teams_.empty())
    {
        return 0;
    }

    if (const Handle producer = GetObjectByTeamSlot(team, DLL_TEAM_SLOT_RECYCLER) != 0)
    {
        return producer;
    }

    if (const Handle producer = GetObjectByTeamSlot(team, DLL_TEAM_SLOT_FACTORY) != 0)
    {
        return producer;
    }

    return 0;
}

CPUManager::CPUTeam* CPUManager::GetCPUTeam(const int team)
{
    if (team < 0 || teams_.empty())
    {
        return nullptr;
    }

    // Find the team object that has the same team number as the one that's passed in.
    for (CPUTeam& team_info : teams_)
    {
        if (team_info.team == team)
        {
            return &team_info;
        }
    }

    return nullptr;
}

CPUManager::DispatchType CPUManager::GetDispatchType(const char* ai_unit_type)
{
    if (ai_unit_type == nullptr || ai_unit_type[0] == '\0')
    {
        return DispatchType::Unknown;
    }

    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::LIEUTENANT) == 0)
    {
        return DispatchType::Lieutenant;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::TURRET) == 0)
    {
        return DispatchType::Turret;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::ANTI_AIR) == 0)
    {
        return DispatchType::AntiAir;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::PATROL) == 0)
    {
        return DispatchType::Patrol;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::BASE_PATROL) == 0)
    {
        return DispatchType::BasePatrol;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::DEMOLISHER) == 0)
    {
        return DispatchType::Demolisher;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::ASSAULT_SERVICE) == 0)
    {
        return DispatchType::AssaultService;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::ASSAULT_DEFENDER) == 0)
    {
        return DispatchType::AssaultDefender;
    }
    if (std::strcmp(ai_unit_type, GameConfig::AIUnitType::APC_PATROL) == 0)
    {
        return DispatchType::ApcPatrol;
    }

    return DispatchType::Unknown;
}

bool CPUManager::IsCommanderType(const char* ai_unit_type)
{
    return ai_unit_type != nullptr && std::strcmp(ai_unit_type, GameConfig::AIUnitType::COMMANDER) == 0;
}

bool CPUManager::IsLieutenantType(const char* ai_unit_type)
{
    return ai_unit_type != nullptr && std::strcmp(ai_unit_type, GameConfig::AIUnitType::LIEUTENANT) == 0;
}

Handle CPUManager::FindNearestServicePod(Handle handle) const
{
    if (service_pods_.empty())
    {
        return 0;
    }

    float nearest_distance = std::numeric_limits<float>::max();
    Handle nearest_pod = 0;

    for (Handle pod : service_pods_)
    {
        const float distance = GetDistance(handle, pod);
        if (distance < nearest_distance)
        {
            nearest_pod = pod;
            nearest_distance = distance;
        }
    }

    return nearest_pod;
}

CPUManager::DispatchUnit* CPUManager::GetDispatchByHandle(CPUTeam* cpu_team, const Handle handle)
{
    if (cpu_team == nullptr)
    {
        return nullptr;
    }

    const auto it = cpu_team->dispatch_by_handle.find(handle);
    if (it != cpu_team->dispatch_by_handle.end())
    {
        return &cpu_team->dispatch_units[it->second];
    }

    return nullptr;
}

// ==================================================
// Dispatch handlers
// ==================================================
void CPUManager::HandleCommander(DispatchUnit* commander, const CPUTeam* cpu_team) const
{
    if (commander == nullptr || commander->handle == 0)
    {
        return;
    }

    if (commander->command != CMD_SERVICE)
    {
        if (GetHealth(commander->handle) < 0.3f || GetAmmo(commander->handle) < 0.15f)
        {
            const Handle service_bay = GetObjectByTeamSlot(cpu_team->team, DLL_TEAM_SLOT_SERVICE);
            if (service_bay != 0)
            {
                Service(commander->handle, service_bay, 0);
                commander->command = CMD_SERVICE;
                return;
            }

            const Handle nearest_pod = FindNearestServicePod(commander->handle);
            if (nearest_pod != 0)
            {
                Goto(commander->handle, nearest_pod, 0);
                commander->command = CMD_SERVICE;
                return;
            }
        }

        // Scan for a nearby enemy and engage if they are within distance of attack.
        if (commander->command != CMD_ATTACK)
        {
            const Handle nearest_enemy = GetNearestEnemy(commander->handle, true, false, 250.0f);
            if (IsAround(nearest_enemy) && !IsBuilding(nearest_enemy))
            {
                Attack(commander->handle, nearest_enemy, 0);
                commander->command = CMD_ATTACK;
                return;
            }
        }
    }

    if (!IsIdle(commander->handle))
    {
        return;
    }

    // 0: Pools
    // 1: Scrap 
    // 2: Friendly base.
    int task_choice;

    if (map_scrap_.empty())
    {
        task_choice = Helpers::GetRandomInt(1);
    }
    else
    {
        task_choice = Helpers::GetRandomInt(2);
    }

    int random_index;
    Handle target_handle = 0;

    switch (task_choice)
    {
    case 0:
        {
            if (map_pools_.empty())
            {
                return;
            }

            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(map_pools_) - 1));
            target_handle = map_pools_[random_index];
            break;
        }
    case 1:
        {
            if (map_scrap_.empty())
            {
                return;
            }

            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(map_scrap_) - 1));
            target_handle = map_scrap_[random_index];
            break;
        }
    case 2:
        {
            if (cpu_team->buildings.empty())
            {
                return;
            }

            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(cpu_team->buildings) - 1));
            target_handle = cpu_team->buildings[random_index];
            break;
        }
    default:
        {
            break;
        }
    }

    if (target_handle == 0)
    {
        return;
    }

    Goto(commander->handle, GetPositionNear(GetPosition(target_handle), 30.0f, 50.0f), 0);
    commander->command = CMD_PATROL;
}

void CPUManager::HandleLieutenant(DispatchUnit* lieutenant, const CPUTeam* cpu_team) const
{
}

void CPUManager::DispatchTurret(DispatchUnit* turret) const
{
    if (turret == nullptr || turret->handle == 0)
    {
        return;
    }

    if (!IsIdle(turret->handle) || !Helpers::IsVectorZero(turret->dispatch_point))
    {
        // Check if the turret has stopped prematurely once it has been shot, see if it has a target, and then tell it to move again.
        if (turret->command == CMD_DEFEND)
        {
            // Try and find the target handle.
            Handle target_handle = GetTarget(turret->handle);

            // Check if we are engaging a valid target.
            if (target_handle != 0 && GetDistance(turret->handle, target_handle) < turret->engage_range)
            {
                return;
            }

            // Check the distance from the dispatch point.
            const Vector current_pos = GetPosition(turret->handle);
            const Vector diff = {
                current_pos.x - turret->dispatch_point.x,
                0.0f, // Ignore height for this check
                current_pos.z - turret->dispatch_point.z
            };
            const float dist_sq = diff.x * diff.x + diff.z * diff.z;

            if (dist_sq < turret->distance_allowance * turret->distance_allowance)
            {
                Stop(turret->handle, 0);
                turret->command = CMD_DONE;
            }
            else
            {
                Goto(turret->handle, turret->dispatch_point, 0);
                turret->command = CMD_GO;
            }
        }

        return;
    }

    // Thinking here, just to note down, that I have the turrets act similar to the Commanders, but with a different approach.
    // If they are shot, we should have them stop and engage the target that shot them which can be registered in another method.
    // What we need to do though, is check the distance of the target, and how long we are idle for. Perhaps a "Last shot" timer?
    // Give them a choice to pick between pools, and scrap, since we care for map control.

    // 0: Pools
    // 1: Scrap 
    const int task_choice = Helpers::GetRandomInt(1);

    // 0: Closest
    // 1: Random
    // 2: Furthest
    const int task_distance = Helpers::GetRandomInt(2);

    switch (task_choice)
    {
    case 0:
        {
            if (map_pools_.empty())
            {
                return;
            }

            if (map_pools_.size() < 2)
            {
                return;
            }

            std::vector<Handle> new_map_pools;
            new_map_pools.reserve(map_pools_.size() - 2);
            std::copy(map_pools_.begin() + 1, map_pools_.end() - 1, std::back_inserter(new_map_pools));

            if (new_map_pools.empty())
            {
                return;
            }

            Vector turret_dispatch_point;

            switch (task_distance)
            {
            case 0: // Closest
                {
                    float nearest_distance = std::numeric_limits<float>::max();
                    Handle nearest_pool = 0;

                    for (Handle pool : new_map_pools)
                    {
                        const float distance = GetDistance(turret->handle, pool);

                        if (distance < nearest_distance)
                        {
                            nearest_pool = pool;
                            nearest_distance = distance;
                        }
                    }

                    if (nearest_pool == 0)
                    {
                        return;
                    }

                    turret_dispatch_point = GetPositionNear(GetPosition(nearest_pool), 30.0f, 50.0f);
                    break;
                }
            case 1: // Random
                {
                    const int random_index = Helpers::GetRandomInt(static_cast<float>(new_map_pools.size() - 1));
                    turret_dispatch_point = GetPositionNear(GetPosition(new_map_pools[random_index]), 30.0f, 50.0f);
                    break;
                }
            case 2: // Furthest
                {
                    float furthest_distance = 0.0f;
                    Handle furthest_pool = 0;

                    for (Handle pool : new_map_pools)
                    {
                        const float distance = GetDistance(turret->handle, pool);

                        if (distance > furthest_distance)
                        {
                            furthest_pool = pool;
                            furthest_distance = distance;
                        }
                    }

                    if (furthest_pool == 0)
                    {
                        return;
                    }

                    turret_dispatch_point = GetPositionNear(GetPosition(furthest_pool), 30.0f, 50.0f);
                    break;
                }
            default:
                {
                    break;
                }
            }

            Goto(turret->handle, turret_dispatch_point, 0);
            turret->dispatch_point = turret_dispatch_point;
            turret->command = CMD_GO;
            break;
        }
    case 1:
        {
            if (map_scrap_.empty())
            {
                return;
            }

            Vector turret_dispatch_point;

            switch (task_distance)
            {
            case 0: // Closest
                {
                    float nearest_distance = std::numeric_limits<float>::max();
                    Handle nearest_scrap = 0;

                    for (Handle scrap : map_scrap_)
                    {
                        const float distance = GetDistance(turret->handle, scrap);

                        if (distance < nearest_distance)
                        {
                            nearest_scrap = scrap;
                            nearest_distance = distance;
                        }
                    }

                    if (nearest_scrap == 0)
                    {
                        return;
                    }

                    turret_dispatch_point = GetPositionNear(GetPosition(nearest_scrap), 25.0f, 30.0f);
                    break;
                }
            case 1: // Random
                {
                    const int random_index = Helpers::GetRandomInt(static_cast<float>(map_scrap_.size() - 1));
                    turret_dispatch_point = GetPositionNear(GetPosition(map_scrap_[random_index]), 25.0f, 30.0f);
                    break;
                }
            case 2: // Furthest
                {
                    float furthest_distance = 0.0f;
                    Handle furthest_scrap = 0;

                    for (Handle scrap : map_scrap_)
                    {
                        const float distance = GetDistance(turret->handle, scrap);

                        if (distance > furthest_distance)
                        {
                            furthest_scrap = scrap;
                            furthest_distance = distance;
                        }
                    }

                    if (furthest_scrap == 0)
                    {
                        return;
                    }

                    turret_dispatch_point = GetPositionNear(GetPosition(furthest_scrap), 25.0f, 30.0f);
                    break;
                }
            default:
                {
                    break;
                }
            }

            Goto(turret->handle, turret_dispatch_point, 0);
            turret->dispatch_point = turret_dispatch_point;
            turret->command = CMD_GO;
            break;
        }
    default:
        {
            break;
        }
    }
}

void CPUManager::DispatchPatrol(DispatchUnit* patrol, CPUTeam* cpu_team) const
{
    if (patrol == nullptr || patrol->handle == 0 || !IsIdle(patrol->handle))
    {
        return;
    }

    if (!map_patrol_paths_.empty())
    {
        // Use the index to get the next path in the queue
        const std::string& patrol_path = map_patrol_paths_[cpu_team->next_patrol_path_index];

        Patrol(patrol->handle, patrol_path.c_str());
        patrol->command = CMD_PATROL;

        // Increment and wrap around the index
        cpu_team->next_patrol_path_index = (cpu_team->next_patrol_path_index + 1) % map_patrol_paths_.size();
        return;
    }

    // Fallback method. If the path list is empty, we can try other patrol strats. 
    // Have this unit move between random friendly buildings and map resources.

    // Ensure the pools and buildings are populated before trying to delegate a task to this unit.
    if (map_pools_.empty() && cpu_team->buildings.empty())
    {
        // If both are empty, we'll need to do some default behaviour.
        return;
    }

    // 0: Buildings
    // 1: Map Resources
    const int task_choice = Helpers::GetRandomInt(1);

    // 0: Closest
    // 1: Random
    // 2: Furthest
    const int task_distance = Helpers::GetRandomInt(2);

    switch (task_choice)
    {
    case 0:
        {
            if (cpu_team->buildings.empty())
            {
                return;
            }

            Vector patrol_dispatch_point;

            switch (task_distance)
            {
            case 0: // Closest
                {
                    Handle nearest_building = 0;
                    float nearest_distance = std::numeric_limits<float>::max();

                    for (Handle building : cpu_team->buildings)
                    {
                        const float distance = GetDistance(patrol->handle, building);

                        if (distance < nearest_distance)
                        {
                            nearest_building = building;
                            nearest_distance = distance;
                        }
                    }

                    if (nearest_building == 0)
                    {
                        return;
                    }

                    patrol_dispatch_point = GetPositionNear(GetPosition(nearest_building), 50.0f, 75.0f);
                    break;
                }
            case 1: // Random
                {
                    const int random_index = Helpers::GetRandomInt(
                        static_cast<float>(cpu_team->buildings.size()) - 1.0f);
                    patrol_dispatch_point = GetPositionNear(GetPosition(cpu_team->buildings[random_index]), 50.0f,
                                                            75.0f);
                    break;
                }
            case 2: // Furthest
                {
                    Handle furthest_building = 0;
                    float furthest_distance = 0.0f;

                    for (Handle building : cpu_team->buildings)
                    {
                        const float distance = GetDistance(patrol->handle, building);
                        if (distance > furthest_distance)
                        {
                            furthest_building = building;
                            furthest_distance = distance;
                        }
                    }

                    patrol_dispatch_point = GetPositionNear(GetPosition(furthest_building), 50.0f, 75.0f);
                    break;
                }
            default:
                break;
            }

            Goto(patrol->handle, patrol_dispatch_point, 0);
            patrol->dispatch_point = patrol_dispatch_point;
            patrol->command = CMD_GO;

            break;
        }
    case 1:
        {
            if (map_pools_.empty())
            {
                return;
            }

            Vector patrol_dispatch_point;

            // Random pool. Find one from the list depending on the chosen distance and find a random position near it.
            switch (task_distance)
            {
            case 0: // Closest
                {
                    Handle nearest_pool = 0;
                    float nearest_distance = std::numeric_limits<float>::max();

                    for (Handle pool : map_pools_)
                    {
                        const float distance = GetDistance(patrol->handle, pool);
                        if (distance < nearest_distance)
                        {
                            nearest_pool = pool;
                            nearest_distance = distance;
                        }
                    }

                    if (nearest_pool == 0)
                    {
                        return;
                    }

                    patrol_dispatch_point = GetPositionNear(GetPosition(nearest_pool), 50.0f, 75.0f);
                    break;
                }
            case 1: // Random
                {
                    const int random_index = Helpers::GetRandomInt(static_cast<float>(map_pools_.size() - 1));
                    patrol_dispatch_point = GetPositionNear(GetPosition(map_pools_[random_index]), 50.0f, 75.0f);
                    break;
                }
            case 2: // Furthest
                {
                    Handle furthest_pool = 0;
                    float furthest_distance = 0.0f;
                    for (Handle pool : map_pools_)
                    {
                        const float distance = GetDistance(patrol->handle, pool);
                        if (distance > furthest_distance)
                        {
                            furthest_pool = pool;
                            furthest_distance = distance;
                        }
                    }

                    if (furthest_pool == 0)
                    {
                        return;
                    }

                    patrol_dispatch_point = GetPositionNear(GetPosition(furthest_pool), 50.0f, 75.0f);
                    break;
                }
            default:
                break;
            }

            Goto(patrol->handle, patrol_dispatch_point, 0);
            patrol->dispatch_point = patrol_dispatch_point;
            patrol->command = CMD_GO;
            break;
        }
    default:
        {
            break;
        }
    }
}

void CPUManager::DispatchBasePatrol(DispatchUnit* base_patrol, CPUTeam* cpu_team) const
{
    if (base_patrol == nullptr || base_patrol->handle == 0 || !IsIdle(base_patrol->handle))
    {
        return;
    }

    if (!base_patrol_paths_.empty())
    {
        const std::string& patrol_path = base_patrol_paths_[cpu_team->next_base_patrol_path_index];

        Patrol(base_patrol->handle, patrol_path.c_str());
        base_patrol->command = CMD_PATROL;

        cpu_team->next_base_patrol_path_index = (cpu_team->next_base_patrol_path_index + 1) % std::size(
            map_patrol_paths_);
        return;
    }

    // Fallback: Find a random building within the base and patrol around it.
    // Add a dispatch timer to it too, so it's considered a "slow" patrol.
    // Start patrolling between different points around the base.
    if (cpu_team->buildings.empty())
    {
        return;
    }

    // Pick the next target and grab a distance that's near it.
    const Handle random_building = cpu_team->buildings[Helpers::GetRandomInt(
        static_cast<float>(std::size(cpu_team->buildings)) - 1)];
    const Vector dispatch_point = GetPositionNear(GetPosition(random_building), 50.0f, 75.0f);

    Goto(base_patrol->handle, dispatch_point, 0);
    base_patrol->dispatch_point = dispatch_point;
    base_patrol->command = CMD_GO;
    base_patrol->dispatch_delay = mission_turn_ + SecondsToTurns(45.0f);
}

void CPUManager::DispatchDemolisher(const DispatchUnit* demolisher, const CPUTeam* cpu_team) const
{
    if (demolisher == nullptr || demolisher->handle == 0)
    {
        return;
    }

    if (!IsIdle(demolisher->handle) || player_buildings_.empty())
    {
        return;
    }

    const int index = Helpers::GetRandomInt(static_cast<float>(std::size(player_buildings_) - 1));
    const Handle demolish_target = player_buildings_[index];
    if (demolish_target == 0)
    {
        return;
    }

    SetCommand(demolisher->handle, CMD_DEMOLISH, 0, demolish_target);
}

void CPUManager::DispatchAntiAir(DispatchUnit* anti_air, CPUTeam* cpu_team) const
{
    if (anti_air == nullptr || anti_air->handle == 0 || !IsIdle(anti_air->handle))
    {
        return;
    }

    if (!anti_air_paths_.empty())
    {
        const std::string& anti_air_path = anti_air_paths_[cpu_team->next_anti_air_path_index];

        Goto(anti_air->handle, anti_air_path.c_str());
        anti_air->command = CMD_GO;

        cpu_team->next_anti_air_path_index = (cpu_team->next_anti_air_path_index + 1) % std::size(
            anti_air_paths_);
        return;
    }

    // Fallback: If there are no anti-air paths, assign the anti-air near the Recycler and have it wait for targets.
    const Handle producer = GetProducer(cpu_team->team);
    if (producer == 0)
    {
        return;
    }

    const Vector anti_air_dispatch_point = GetPositionNear(GetPosition(producer), anti_air_distance_ - 50.0f,
                                                           anti_air_distance_);
    Goto(anti_air->handle, anti_air_dispatch_point, 0);
    anti_air->dispatch_point = anti_air_dispatch_point;
    anti_air->command = CMD_GO;
}

void CPUManager::DispatchSupport(DispatchUnit* support, const CPUTeam* cpu_team) const
{
}

void CPUManager::DispatchDefender(DispatchUnit* defender, const CPUTeam* cpu_team) const
{
}

void CPUManager::DispatchAPCPatrol(DispatchUnit* apc_patrol, const CPUTeam* cpu_team) const
{
}

// ==================================================
// Spawn / Setup
// ==================================================
void CPUManager::Init(const int difficulty)
{
    // Handle general variables for the manager first.
    switch (difficulty)
    {
    case GameConfig::Difficulty::EASY:
        anti_air_distance_ = 150.0f;
        siege_distance_ = 200.0f;
        break;
    case GameConfig::Difficulty::MEDIUM:
    default:
        anti_air_distance_ = 250.0f;
        siege_distance_ = 325.0f;
        break;
    case GameConfig::Difficulty::HARD:
        anti_air_distance_ = 350.0f;
        siege_distance_ = 450.0f;
        break;
    }

    for (int i = 0; i < GameConfig::MAX_PATHS; i++)
    {
        char path_name[GameConfig::MAX_NAME_LENGTH] = {};
        if (sprintf_s(path_name, sizeof(path_name), "patrol_%d", i) >= 0)
        {
            if (Helpers::GetPathPosition(path_name) != EMPTY_VECTOR)
            {
                map_patrol_paths_.emplace_back(path_name);
            }
        }

        if (sprintf_s(path_name, sizeof(path_name), "BasePatrol%d", i) >= 0)
        {
            if (Helpers::GetPathPosition(path_name) != EMPTY_VECTOR)
            {
                base_patrol_paths_.emplace_back(path_name);
            }
        }

        if (sprintf_s(path_name, sizeof(path_name), "anti-air_%d", i) >= 0)
        {
            if (Helpers::GetPathPosition(path_name) != EMPTY_VECTOR)
            {
                anti_air_paths_.emplace_back(path_name);
            }
        }
    }
}

void CPUManager::SetCPUAIPlan(GameConfig::AIPType type, const CPUTeam* cpu_team, const char player_faction)
{
    if (type < GameConfig::AIPType0 || type >= GameConfig::MAX_AIP_TYPE)
    {
        type = GameConfig::AIPType3;
    }

    std::string aip_string = cpu_team->aip_string;
    const char aip_char = GameConfig::AIP_TYPE_STRINGS[type];
    char aip_full_string[GameConfig::MAX_NAME_LENGTH] = {};

    if (aip_string.empty())
    {
        aip_string = "stock_";
    }

    if (sprintf_s(aip_full_string, sizeof(aip_full_string), "%s%c_%c.aip", aip_string.c_str(), cpu_team->faction,
                  aip_char) < 0)
    {
        aip_full_string[0] = '\0';
    }

    if (!DoesFileExist(aip_full_string))
    {
        if (sprintf_s(aip_full_string, sizeof(aip_full_string), "%s%c%c%c.aip", aip_string.c_str(), cpu_team->faction,
                      player_faction, aip_char) < 0)
        {
            aip_full_string[0] = '\0';
        }
    }

    SetPlan(aip_full_string, cpu_team->team);
}

void CPUManager::RegisterLieutenant(CPUTeam* cpu_team, const Handle lieutenant)
{
    if (cpu_team == nullptr || lieutenant == 0)
    {
        return;
    }

    if (!cpu_team->lieutenant_name_pool.empty())
    {
        std::string name = cpu_team->lieutenant_name_pool.back();
        cpu_team->lieutenant_name_pool.pop_back();

        cpu_team->lieutenants.push_back({lieutenant, name});
        cpu_team->lieutenant_names_by_handle.emplace(lieutenant, name);

        SetObjectiveName(lieutenant, name.c_str());
    }
}

void CPUManager::RegisterNewTeam(const int team, const char faction, const char* spawn_path, const bool is_campaign,
                                 const char player_faction)
{
    const int random_index = Helpers::GetRandomInt(std::size(GameConfig::CPU_NAMES) - 1);
    const char* team_name = GameConfig::CPU_NAMES[random_index];

    CPUTeam team_info;
    team_info.name = team_name;
    team_info.team = team;
    team_info.faction = faction;
    team_info.spawn_path = spawn_path;
    team_info.is_campaign = is_campaign;

    for (const char* candidate : GameConfig::CPU_NAMES)
    {
        if (std::strcmp(candidate, team_name) != 0)
        {
            team_info.lieutenant_name_pool.emplace_back(candidate);
        }
    }

    if (!is_campaign)
    {
        if (IsNetworkOn())
        {
            team_info.aip_string = dll_utils::get_checked_network_svar(3, NETLIST_AIPs);
            team_info.commander_enabled = GetVarItemInt(GameConfig::MPI_COMMANDER_ENABLED) == 1;
        }
        else
        {
            team_info.aip_string = GetVarItemStr(GameConfig::AIP_STRING);
            team_info.commander_enabled = GetVarItemInt(GameConfig::COMMANDER_ENABLED) == 1;
        }

        SetCPUAIPlan(GameConfig::AIPType0, &team_info, player_faction);
    }

    teams_.push_back(team_info);
    SetScrap(team, 40);

    if (team_info.commander_enabled)
    {
        char commander_odf[GameConfig::MAX_ODF_LENGTH] = {};
        if (sprintf_s(commander_odf, sizeof(commander_odf), "%cvcmdr_s", faction) > 0)
        {
            Vector pos = GetPositionNear(Helpers::GetPathPosition(spawn_path), 30.0f, 60.0f);
            pos.y = TerrainFindFloor(pos.x, pos.z) + 2.5f;
            BuildObject(commander_odf, team, pos);
        }

        const int player_count = CountPlayers();
        if (player_count > 1)
        {
            char lieutenant_odf[GameConfig::MAX_ODF_LENGTH] = {};
            if (sprintf_s(lieutenant_odf, sizeof(lieutenant_odf), "%cvlt_c", faction) > 0)
            {
                const Vector pos = GetPositionNear(Helpers::GetPathPosition(spawn_path), 30.0f, 60.0f);
                for (int i = 2; i < player_count; i++)
                {
                    BuildObject(lieutenant_odf, team, pos);
                }
            }
        }
    }
}

// ==================================================
// Main Logic
// ==================================================
void CPUManager::Execute(const int mission_turn)
{
    // Update the mission turn in the CPU manager class rather than passing it back and forth between DLLs.
    mission_turn_ = mission_turn;

    for (CPUTeam& team_info : teams_)
    {
        Handle nearest_enemy = 0;

        // Check to see if there are any hostiles near the main base. If so, engage siege mode.
        if (const Handle factory = GetObjectByTeamSlot(team_info.team, DLL_TEAM_SLOT_FACTORY) != 0)
        {
            nearest_enemy = GetNearestEnemy(factory, true, true, siege_distance_);
        }
        else if (const Handle recycler = GetObjectByTeamSlot(team_info.team, DLL_TEAM_SLOT_RECYCLER) != 0)
        {
            nearest_enemy = GetNearestEnemy(recycler, true, true, siege_distance_);
        }

        if (nearest_enemy != 0)
        {
            team_info.in_siege_mode = true;
            team_info.siege_mode_cooldown = mission_turn_ + SecondsToTurns(20.0f);
        }

        if (team_info.commander_enabled)
        {
            HandleCommander(&team_info.commander, &team_info);
        }

        for (auto it = team_info.dispatch_units.begin(); it != team_info.dispatch_units.end();)
        {
            DispatchUnit& dispatch_unit = *it;

            if (dispatch_unit.handle == 0)
            {
                it = team_info.dispatch_units.erase(it);
                continue;
            }

            if (!IsAround(dispatch_unit.handle))
            {
                it = team_info.dispatch_units.erase(it);
                continue;
            }

            if (dispatch_unit.dispatch_delay > mission_turn)
            {
                ++it;
                continue;
            }

            switch (dispatch_unit.type)
            {
            case DispatchType::Lieutenant:
                HandleLieutenant(&dispatch_unit, &team_info);
                break;
            case DispatchType::Turret:
                DispatchTurret(&dispatch_unit);
                break;
            case DispatchType::AntiAir:
                DispatchAntiAir(&dispatch_unit, &team_info);
                break;
            case DispatchType::Patrol:
                DispatchPatrol(&dispatch_unit, &team_info);
                break;
            case DispatchType::BasePatrol:
                DispatchBasePatrol(&dispatch_unit, &team_info);
                break;
            case DispatchType::Demolisher:
                DispatchDemolisher(&dispatch_unit, &team_info);
                break;
            case DispatchType::AssaultService:
                DispatchSupport(&dispatch_unit, &team_info);
                break;
            case DispatchType::AssaultDefender:
                DispatchDefender(&dispatch_unit, &team_info);
                break;
            case DispatchType::ApcPatrol:
                DispatchAPCPatrol(&dispatch_unit, &team_info);
                break;
            case DispatchType::Unknown:
                break;
            }

            ++it;
        }
    }
}

// ==================================================
// Delegates
// ==================================================
void CPUManager::AddTeamObject(const Handle team_object, const int team, const char* ai_unit_type, const char* obj_odf)
{
    CPUTeam* cpu_team = GetCPUTeam(team);

    if (cpu_team == nullptr)
    {
        return;
    }

    char building_class[GameConfig::MAX_ODF_LENGTH] = {};
    GetObjInfo(team_object, Get_GOClass, building_class);

    if (strcmp(building_class, "CLASS_TURRET") == 0)
    {
        cpu_team->gun_towers.push_back(team_object);
        return;
    }

    if (IsBuilding(team_object))
    {
        cpu_team->buildings.push_back(team_object);
        return;
    }

    if (ai_unit_type == nullptr || ai_unit_type[0] == '\0')
    {
        return;
    }

    if (IsLieutenantType(ai_unit_type))
    {
        RegisterLieutenant(cpu_team, team_object);
        return;
    }

    // Create a new dispatch object and get it ready for processing.
    DispatchUnit dispatch_unit;
    dispatch_unit.handle = team_object;
    dispatch_unit.birth_turn = mission_turn_;
    dispatch_unit.team = team;
    dispatch_unit.type = GetDispatchType(ai_unit_type);
    dispatch_unit.dispatch_delay = mission_turn_ + SecondsToTurns(10.0f);
    dispatch_unit.command = CMD_NONE;
    dispatch_unit.max_health = GetMaxHealth(team_object);
    dispatch_unit.max_ammo = GetMaxAmmo(team_object);
    dispatch_unit.dispatch_point = {0, 0, 0};
    dispatch_unit.target = 0;

    // Try and grab the unit's engage range from the ODF so we can populate it.
    const float engage_range = Helpers::GetODFFloatFromChain(obj_odf, "CraftClass", "engageRange");

    if (engage_range > 0.0f)
    {
        dispatch_unit.engage_range = engage_range;
    }
    else
    {
        dispatch_unit.engage_range = 0.0f;
    }

    if (IsCommanderType(ai_unit_type))
    {
        SetIndependence(team_object, 0);
        SetObjectiveName(team_object, cpu_team->name.c_str());
        cpu_team->commander = dispatch_unit;
    }
    else
    {
        cpu_team->dispatch_units.push_back(dispatch_unit);
        cpu_team->dispatch_by_handle[team_object] = cpu_team->dispatch_units.size() - 1; // Store index
    }
}

void CPUManager::RemoveTeamObject(const Handle team_object)
{
    for (CPUTeam& cpu_team : teams_)
    {
        const auto lieutenant_it = cpu_team.lieutenant_names_by_handle.find(team_object);
        if (lieutenant_it != cpu_team.lieutenant_names_by_handle.end())
        {
            if (!lieutenant_it->second.empty())
            {
                cpu_team.lieutenant_name_pool.push_back(lieutenant_it->second);
            }

            cpu_team.lieutenant_names_by_handle.erase(lieutenant_it);

            EraseIf(cpu_team.lieutenants, [team_object](const Lieutenant& lieutenant)
            {
                return lieutenant.handle == team_object;
            });

            return;
        }

        if (EraseIf(cpu_team.dispatch_units, [team_object](const DispatchUnit& dispatch_unit)
        {
            return dispatch_unit.handle == team_object;
        }))
        {
            // Rebuild the index map to keep it in sync with the vector.
            cpu_team.dispatch_by_handle.clear();
            for (size_t i = 0; i < cpu_team.dispatch_units.size(); ++i)
            {
                cpu_team.dispatch_by_handle[cpu_team.dispatch_units[i].handle] = i;
            }
            return;
        }
    }
}

void CPUManager::TurretShot(const Handle turret, const int team)
{
    CPUTeam* cpu_team = GetCPUTeam(team);

    if (cpu_team == nullptr)
    {
        return;
    }

    DispatchUnit* turret_dispatch_unit = GetDispatchByHandle(cpu_team, turret);
    if (turret_dispatch_unit == nullptr)
    {
        return;
    }

    Defend(turret_dispatch_unit->handle, team);
    turret_dispatch_unit->command = CMD_DEFEND;
    turret_dispatch_unit->dispatch_delay = mission_turn_ + SecondsToTurns(15.0f);
}

// ==================================================
// Registration
// ==================================================
void CPUManager::AddMapPool(const Handle map_pool)
{
    map_pools_.push_back(map_pool);
}

void CPUManager::AddMapScrap(const Handle map_scrap)
{
    map_scrap_.push_back(map_scrap);
}

void CPUManager::RemoveMapScrap(const Handle map_scrap)
{
    map_scrap_.erase(std::remove(map_scrap_.begin(), map_scrap_.end(), map_scrap), map_scrap_.end());
}

void CPUManager::AddServicePod(const Handle service_pod)
{
    service_pods_.push_back(service_pod);
}

void CPUManager::RemoveServicePod(const Handle service_pod)
{
    service_pods_.erase(std::remove(service_pods_.begin(), service_pods_.end(), service_pod), service_pods_.end());
}

void CPUManager::AddPlayerBuilding(const Handle building)
{
    player_buildings_.push_back(building);
}

void CPUManager::RemovePlayerBuilding(const Handle building)
{
    player_buildings_.erase(std::remove(player_buildings_.begin(), player_buildings_.end(), building),
                            player_buildings_.end());
}
