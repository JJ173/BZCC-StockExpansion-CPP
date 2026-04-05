#include "CPUManager.h"

#include "../shared/DLLUtils.h"
#include "../Utilities/Helpers.h"

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

Handle CPUManager::FindNearestServicePod(Handle handle)
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
void CPUManager::HandleCommander(DispatchUnit* commander, const CPUTeam* cpu_team)
{
    // If we're a nullptr for whatever reason then return.
    if (commander == nullptr)
    {
        return;
    }

    // First, do a health and ammo check. If we're below a threshold, find a pod on the map that's closest to us and move to it.
    // If there are no pods, or buildings to heal with, then just go gung-ho and attack until death. No point in wasting time.
    if (commander->command != CMD_SERVICE)
    {
        if (GetHealth(commander->handle) < 0.3f || GetAmmo(commander->handle) < 0.15f)
        {
            if (const Handle service_bay = GetObjectByTeamSlot(cpu_team->team, DLL_TEAM_SLOT_SERVICE) != 0)
            {
                Service(commander->handle, service_bay, 0);
                commander->command = CMD_SERVICE;
                return;
            }

            if (const Handle nearest_pod = FindNearestServicePod(commander->handle) != 0)
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
            if (IsAround(nearest_enemy))
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
    // 3 - Find random enemy base.
    // 4 - Find random scrap (only possible if scrap exists, so keep it last).
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
            if (player_buildings_.empty())
            {
                return;
            }

            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(player_buildings_) - 1));
            target_handle = player_buildings_[random_index];
            break;
        }
    case 3:
        {
            random_index = Helpers::GetRandomInt(static_cast<float>(std::size(map_scrap_) - 1));
            target_handle = map_scrap_[random_index];
            break;
        }
    default:
        break;
    }

    Goto(commander->handle, GetPositionNear(GetPosition(target_handle), 30.0f, 50.0f), 0);
    commander->command = CMD_PATROL;
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

void CPUManager::RegisterLieutenant(const CPUTeam* cpu_team, const Handle lieutenant)
{
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
void CPUManager::Execute(int mission_turn)
{
    for (CPUTeam& team_info : teams_)
    {
        if (team_info.commander_enabled)
        {
            HandleCommander(&team_info.commander, &team_info);
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

    if (ai_unit_type == nullptr)
    {
        return;
    }

    if (strcmp(ai_unit_type, GameConfig::AIUnitType::LIEUTENANT) == 0)
    {
        RegisterLieutenant(cpu_team, team_object);
        return;
    }

    // Create a new dispatch object and get it ready for processing.
    DispatchUnit dispatch_unit;
    dispatch_unit.handle = team_object;
    dispatch_unit.birth_turn = mission_turn;
    dispatch_unit.team = team;
    dispatch_unit.type = ai_unit_type;
    dispatch_unit.dispatch_delay = mission_turn + SecondsToTurns(10.0f);
    dispatch_unit.command = CMD_NONE;
    dispatch_unit.max_health = GetMaxHealth(team_object);
    dispatch_unit.max_ammo = GetMaxAmmo(team_object);
    dispatch_unit.dispatch_point = {0, 0, 0};
    dispatch_unit.target = 0;

    if (strcmp(ai_unit_type, GameConfig::AIUnitType::COMMANDER) == 0)
    {
        SetObjectiveName(team_object, cpu_team->name);
        cpu_team->commander = dispatch_unit;
    }
    else
    {
        cpu_team->dispatch_units.push_back(dispatch_unit);
    }
}

void CPUManager::RemoveTeamObject(const Handle team_object)
{
}

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
