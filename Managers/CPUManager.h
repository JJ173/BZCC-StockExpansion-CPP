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
        BasePatrol,
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
        int bordem_check_turn;
        
        // Add an engage range for checks as it seems "GetTarget" ends up being infinite for AI? Seems odd.
        float engage_range = 0.0f;
        
        // Distance that the turret can be from its dispatch point.      
        float distance_allowance = 100.0f;
                
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
                
        int team;
        int taunt_cooldown;
        int siege_mode_cooldown;
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
        std::unordered_map<Handle, size_t> dispatch_by_handle;
        
        size_t next_patrol_path_index = 0; // Track the next path for "Patrol" units
        size_t next_base_patrol_path_index = 0; // Track the next path for "BasePatrol" units
    };
    
    int mission_turn_ = 0;
    float siege_distance_;
    
    std::vector<CPUTeam> teams_;
    std::vector<Handle> map_pools_;
    std::vector<Handle> map_scrap_;
    std::vector<Handle> service_pods_;
    std::vector<Handle> player_buildings_;
    std::vector<Handle> player_aircraft_;
    
    // TODO: Change this to a team specific path selector if we ever plan to support more than one AIP.
    std::vector<std::string> map_patrol_paths_;
    std::vector<std::string> base_patrol_paths_;
    
    static DispatchUnit* GetDispatchByHandle(CPUTeam* cpu_team, Handle handle);
    static DispatchType GetDispatchType(const char* ai_unit_type);
    
    static bool IsCommanderType(const char* ai_unit_type);
    static bool IsLieutenantType(const char* ai_unit_type);
    
    Handle FindNearestServicePod(Handle handle) const;
    CPUTeam* GetCPUTeam(int team);
    
    static void SetCPUAIPlan(GameConfig::AIPType type, const CPUTeam* cpu_team, char player_faction);
    static void RegisterLieutenant(CPUTeam* cpu_team, Handle lieutenant);
    
    void HandleCommander(DispatchUnit* commander, const CPUTeam* cpu_team) const;
    void HandleLieutenant(DispatchUnit* lieutenant, const CPUTeam* cpu_team) const;
    void DispatchTurret(DispatchUnit* turret) const;
    void DispatchPatrol(DispatchUnit* patrol, CPUTeam* cpu_team) const;
    void DispatchBasePatrol(DispatchUnit* base_patrol, CPUTeam* cpu_team) const;
    void DispatchDemolisher(const DispatchUnit* demolisher, const CPUTeam* cpu_team) const;
    void DispatchAntiAir(DispatchUnit* anti_air, const CPUTeam* cpu_team) const;
    void DispatchSupport(DispatchUnit* support, const CPUTeam* cpu_team) const;
    void DispatchDefender(DispatchUnit* defender, const CPUTeam* cpu_team) const;
    void DispatchAPCPatrol(DispatchUnit* apc_patrol, const CPUTeam* cpu_team) const;
    
public:
    void Init(int difficulty);
    void RegisterNewTeam(int team, char faction, const char* spawn_path, bool is_campaign, char player_faction);
    void Execute(int mission_turn);
    
    void AddTeamObject(Handle team_object, int team, const char* ai_unit_type, const char* obj_odf);
    void RemoveTeamObject(Handle team_object);
    
    void AddMapPool(Handle map_pool);
    void AddMapScrap(Handle map_scrap);
    void RemoveMapScrap(Handle map_scrap);
    void AddServicePod(Handle service_pod);
    void RemoveServicePod(Handle service_pod);
    void AddPlayerBuilding(Handle building);
    void RemovePlayerBuilding(Handle building);
    
    void TurretShot(Handle turret, int team);
};
