#pragma once

#include "../Shared/DllBase.h"
#include "../Shared/GameConfig.h"
#include <string>
#include <vector>

class CPUManager
{
    enum DispatchState : uint8_t
    {
        Idle,
        Goto,
        Attack,
        Defend,
        Retreat,
        Repair
    };
    
    enum DispatchType : uint8_t
    {
        None,
        Turret,
        Patrol,
        AntiAir,
        Service,
        Minelayer,
        Escort
    };
    
    struct DispatchUnit
    {
        Handle handle;
        Handle target;
        
        DispatchState state;
        DispatchType type;
        Vector spot;
        
        float stop_distance;
        float engage_range;
        
        int command;
    };
    
    struct Lieutenant
    {
        Handle handle;
        std::string name;
    };
    
    struct CPUTeam
    {
        std::string name;
        std::string aip_string;
        std::string spawn_path;
        
        int team;
        int taunt_cooldown;
        
        char faction;
        
        bool commander_enabled;
        bool taunt_setup_done;
        bool is_campaign;
        
        Handle commander;
        Handle recycler;
        Handle factory;
        Handle armory;
        Handle service_bay;
        
        std::vector<Handle> gun_towers;
        std::vector<Lieutenant> lieutenants;
        std::vector<std::string> lieutenants_names;
        
        std::vector<DispatchUnit> dispatch_units;
    };
    
    std::vector<CPUTeam> teams_;
    std::vector<Handle> map_pools_;
    std::vector<Handle> map_scrap_;

    static void SetCPUAIPlan(GameConfig::AIPType type, const CPUTeam* cpu_team);
    
public:
    void RegisterNewTeam(int team, char faction, const char* spawn_path, bool is_campaign);
    void Execute(int mission_turn);
    
    void AddTeamObject();
    void RemoveTeamObject(const Handle team_object);
    
    void AddMapPool(Handle map_pool);
    void AddMapScrap(Handle map_scrap);
    void RemoveMapScrap(const Handle map_scrap);
};
