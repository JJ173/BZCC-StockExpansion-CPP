#pragma once

#include "../Shared/DllBase.h"
#include "../Shared/GameConfig.h"
#include <vector>
#include <unordered_map>

class CPUManager
{
    enum class DispatchType : std::uint8_t
    {
        Unknown,
        Lieutenant,
        Turret,
        AntiAir,
        Patrol,
        Demolisher,
        AssaultService,
        AssaultDefender,
        ApcPatrol
    };
    
    struct DispatchUnit
    {
        Handle handle;
        Handle target;
        
        AiCommand command;
        int birth_turn;
        int team;
        int dispatch_delay;
        int max_health;
        int max_ammo;
        
        Vector dispatch_point;
        DispatchType type = DispatchType::Unknown;
    };
    
    struct Lieutenant
    {
        Handle handle;
        std::string name;
    };
    
    struct CPUTeam
    {
        bool commander_enabled;
        bool taunt_setup_done;
        bool is_campaign;
        bool in_siege_mode;
        
        float siege_distance;
        
        int team;
        int taunt_cooldown;
        DispatchUnit commander;
        
        char faction;
        std::string name;
        std::string aip_string;
        std::string spawn_path;
        
        std::vector<Lieutenant> lieutenants;
        std::vector<std::string> lieutenant_name_pool;
        std::unordered_map<Handle, std::string> lieutenant_names_by_handle;
        std::vector<Handle> gun_towers;
        std::vector<Handle> buildings;
        
        std::vector<DispatchUnit> dispatch_units;
    };
    
    std::vector<CPUTeam> teams_;
    std::vector<Handle> map_pools_;
    std::vector<Handle> map_scrap_;
    std::vector<Handle> service_pods_;
    std::vector<Handle> player_buildings_;
    std::vector<Handle> player_aircraft_;
    
    static DispatchType GetDispatchType(const char* ai_unit_type);
    static bool IsCommanderType(const char* ai_unit_type);
    static bool IsLieutenantType(const char* ai_unit_type);
    
    Handle FindNearestServicePod(Handle handle) const;
    CPUTeam* GetCPUTeam(int team);
    
    static void SetCPUAIPlan(GameConfig::AIPType type, const CPUTeam* cpu_team, char player_faction);
    static void RegisterLieutenant(CPUTeam* cpu_team, Handle lieutenant);
    
    void HandleCommander(DispatchUnit* commander, const CPUTeam* cpu_team) const;
    void HandleLieutenant(DispatchUnit* lieutenant, const CPUTeam* cpu_team) const;
    void DispatchTurret(DispatchUnit* turret, const CPUTeam* cpu_team) const;
    void DispatchPatrol(DispatchUnit* patrol, const CPUTeam* cpu_team) const;
    void DispatchDemolisher(DispatchUnit* demolisher, const CPUTeam* cpu_team) const;
    void DispatchAntiAir(DispatchUnit* anti_air, const CPUTeam* cpu_team) const;
    void DispatchSupport(DispatchUnit* support, const CPUTeam* cpu_team) const;
    void DispatchDefender(DispatchUnit* defender, const CPUTeam* cpu_team) const;
    void DispatchAPCPatrol(DispatchUnit* apc_patrol, const CPUTeam* cpu_team) const;
    
public:
    void RegisterNewTeam(int team, char faction, const char* spawn_path, bool is_campaign, char player_faction, float siege_distance);
    void Execute(int mission_turn);
    
    void AddTeamObject(Handle team_object, int mission_turn, int team, const char* ai_unit_type);
    void RemoveTeamObject(Handle team_object);
    
    void AddMapPool(Handle map_pool);
    void AddMapScrap(Handle map_scrap);
    void RemoveMapScrap(Handle map_scrap);
    void AddServicePod(Handle service_pod);
    void RemoveServicePod(Handle service_pod);
    void AddPlayerBuilding(Handle building);
    void RemovePlayerBuilding(Handle building);
};
