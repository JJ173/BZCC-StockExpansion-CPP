#pragma once

#include "../Shared/DllBase.h"
#include "../Shared/GameConfig.h"
#include <string>
#include <vector>

class CPUManager
{
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
        std::string type;
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
        bool in_siege_mode;

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
    std::vector<Handle> service_pods_;

    Handle FindNearestServicePod(Handle handle);
    CPUTeam* GetCPUTeam(int team);
    static void SetCPUAIPlan(GameConfig::AIPType type, const CPUTeam* cpu_team, char player_faction);
    static void RegisterLieutenant(const CPUTeam* cpu_team, Handle lieutenant);
    
public:
    void RegisterNewTeam(int team, char faction, const char* spawn_path, bool is_campaign, char player_faction);
    void Execute(int mission_turn);
    
    void AddTeamObject(Handle team_object, int mission_turn, int team, const char* ai_unit_type);
    void RemoveTeamObject(Handle team_object);
    
    void AddMapPool(Handle map_pool);
    void AddMapScrap(Handle map_scrap);
    void RemoveMapScrap(Handle map_scrap);
    void AddServicePod(Handle service_pod);
    void RemoveServicePod(Handle service_pod);
};
