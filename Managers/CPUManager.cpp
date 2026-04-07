#include "CPUManager.h"

#include <algorithm>
#include <cassert>
#include <cstdio>

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

// ==================================================
// Dispatch handlers
// ==================================================
void CPUManager::HandleCommander(DispatchUnit* commander, const CPUTeam* cpu_team) const
{
    // Check to see if commander is valid.
    if (commander == nullptr || commander->handle == 0)
    {
        return;
    }

    // First, do a health and ammo check. If we're below a threshold, find a pod on the map that's closest to us and move to it.
    // If there are no pods, or buildings to heal with, then just go gung-ho and attack until death. No point in wasting time.
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

    // If we're not idle, then bail.
    if (!IsIdle(commander->handle))
    {
        return;
    }

    // Add some randomization for patrol points. Could be a list of:
    // 1 - Find random pools.
    // 2 - Find random friendly base.
    // 3 - Find random scrap (only possible if scrap exists, so keep it last).
    int task_choice;

    if (map_scrap_.empty())
    {
        task_choice = Helpers::GetRandomInt(2);
    }
    else
    {
        task_choice = Helpers::GetRandomInt(3);
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
            if (cpu_team->buildings.empty())
            {
                return;
            }

            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(cpu_team->buildings) - 1));
            target_handle = cpu_team->buildings[random_index];
            break;
        }
    case 2:
        {
            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(map_scrap_) - 1));
            target_handle = map_scrap_[random_index];
            break;
        }
    default:
        break;
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

void CPUManager::DispatchTurret(DispatchUnit* turret, const CPUTeam* cpu_team) const
{
}

void CPUManager::DispatchPatrol(DispatchUnit* patrol, const CPUTeam* cpu_team) const
{
}

void CPUManager::DispatchDemolisher(DispatchUnit* demolisher, const CPUTeam* cpu_team) const
{
    if (demolisher == nullptr || demolisher->handle == 0)
    {
        return;
    }

    if (!IsIdle(demolisher->handle) || player_buildings_.empty())
    {
        return;
    }

    const Handle demolish_target = player_buildings_[Helpers::GetRandomInt(static_cast<float>(player_buildings_.size()))];
    if (demolish_target == 0)
    {
        return;
    }

    SetCommand(demolisher->handle, CMD_DEMOLISH, demolish_target, 0);

    // Just for debugging, highlight the building, and name it.
    SetObjectiveName(demolish_target, "Demolish");
    SetObjectiveOn(demolish_target);
}

void CPUManager::DispatchAntiAir(DispatchUnit* anti_air, const CPUTeam* cpu_team) const
{
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
                                 const char player_faction, const float siege_distance)
{
    const int random_index = Helpers::GetRandomInt(std::size(GameConfig::CPU_NAMES) - 1);
    const char* team_name = GameConfig::CPU_NAMES[random_index];

    CPUTeam team_info;
    team_info.name = team_name;
    team_info.team = team;
    team_info.faction = faction;
    team_info.spawn_path = spawn_path;
    team_info.is_campaign = is_campaign;
    team_info.siege_distance = siege_distance;

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
    for (CPUTeam& team_info : teams_)
    {
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
                DispatchTurret(&dispatch_unit, &team_info);
                break;
            case DispatchType::AntiAir:
                DispatchAntiAir(&dispatch_unit, &team_info);
                break;
            case DispatchType::Patrol:
                DispatchPatrol(&dispatch_unit, &team_info);
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
void CPUManager::AddTeamObject(const Handle team_object, const int mission_turn, const int team,
                               const char* ai_unit_type)
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
    dispatch_unit.birth_turn = mission_turn;
    dispatch_unit.team = team;
    dispatch_unit.type = GetDispatchType(ai_unit_type);
    dispatch_unit.dispatch_delay = mission_turn + SecondsToTurns(10.0f);
    dispatch_unit.command = CMD_NONE;
    dispatch_unit.max_health = GetMaxHealth(team_object);
    dispatch_unit.max_ammo = GetMaxAmmo(team_object);
    dispatch_unit.dispatch_point = {0, 0, 0};
    dispatch_unit.target = 0;

    if (IsCommanderType(ai_unit_type))
    {
        SetIndependence(team_object, 0);
        SetObjectiveName(team_object, cpu_team->name.c_str());
        cpu_team->commander = dispatch_unit;
    }
    else
    {
        cpu_team->dispatch_units.push_back(dispatch_unit);
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
            return;
        }
    }
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
