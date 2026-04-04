#include "CPUManager.h"

#include "../shared/DLLUtils.h"
#include "../Utilities/Helpers.h"

void CPUManager::SetCPUAIPlan(GameConfig::AIPType type, const CPUTeam* cpu_team)
{
    if (type < GameConfig::AIPType0 || type >= GameConfig::MAX_AIP_TYPE)
    {
        type = GameConfig::AIPType3;
    }

    char aip_string[GameConfig::MAX_NAME_LENGTH] = {};

    if (cpu_team->aip_string.empty())
    {
        if (sprintf_s(aip_string, sizeof(aip_string), "stock_%c_%c.aip", cpu_team->faction, type) < 0)
        {
            aip_string[0] = '\0';
        }
    }
    else
    {
        if (sprintf_s(aip_string, sizeof(aip_string), "%s%c_%c.aip", cpu_team->aip_string.c_str(), cpu_team->faction, type) < 0)
        {
            aip_string[0] = '\0';
        }
    }
    
    SetPlan(aip_string, cpu_team->team);
}

void CPUManager::Execute(int mission_turn)
{
}

void CPUManager::AddTeamObject()
{
}

void CPUManager::RemoveTeamObject(const Handle team_object)
{
}

void CPUManager::RegisterNewTeam(const int team, const char faction, const char* spawn_path, const bool is_campaign)
{
    // Pick a random name from the GameConfig list of names to use for this team.
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

        if (team_info.commander_enabled)
        {
            char commander_odf[GameConfig::MAX_ODF_LENGTH] = {};
            if (sprintf_s(commander_odf, sizeof(commander_odf), "%cvcmdr_s", faction) > 0)
            {
                const Handle spawner = BuildObject("pspwn_1", team, spawn_path);
                Vector pos = GetPositionNear(GetPosition(spawner), 30.0f, 60.0f);
                pos.y = TerrainFindFloor(pos.x, pos.z) + 2.5f;
                RemoveObject(spawner);
                team_info.commander = BuildObject(commander_odf, team, pos);
            }

            const int player_count = CountPlayers();
            if (player_count > 1)
            {
                char lieutenant_odf[GameConfig::MAX_ODF_LENGTH] = {};
                if (sprintf_s(lieutenant_odf, sizeof(lieutenant_odf), "%cvlt_c", faction) > 0)
                {
                    const Handle spawner = BuildObject("pspwn_1", team, spawn_path);
                    const Vector pos = GetPositionNear(GetPosition(spawner), 30.0f, 60.0f);
                    RemoveObject(spawner);

                    for (int i = 1; i < player_count - 1; i++)
                    {
                        BuildObject(lieutenant_odf, team, pos);
                    }
                }
            }
        }
        
        SetCPUAIPlan(GameConfig::AIPType0, &team_info);
    }

    SetScrap(team, 40);
    teams_.push_back(team_info);
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
