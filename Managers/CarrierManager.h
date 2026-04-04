#pragma once

#include <vector>
#include <string>

#include "../Shared/DllBase.h"
#include "../Shared/GameConfig.h"

class CarrierManager
{    
    struct DropshipRequestItem
    {
        Handle handle;
        int team;
        int time_to_delete;
        std::string type;
    };
    
    struct Condor
    {
        Handle handle;
        int team;
        std::string type;
        int unit_total;
        GameConfig::CondorState state;
        bool ready_to_delete;
        int delay_time;
        std::vector<Handle> units;
    };
    
    struct Portal
    {
        Handle handle;
        int team;
        std::string type;
        int unit_total;
        int delay_time;
        int teleported_unit_count;
        bool ready_to_delete;
    };
    
    struct Carrier
    {
        Handle handle;
        int team;
        char faction;
        Handle landing_pad;
        std::vector<DropshipRequestItem> dropship_requests;
        std::vector<Condor> condors;
        std::vector<Portal> portals;
    };
    
    std::vector<Carrier> carriers_;

    Carrier* GetTeamCarrier(int team);
    static void HandleCondor(Condor* condor, Handle landing_pad, int mission_turn_count);
    static void HandlePortal(Portal* portal, int mission_turn_count);
    
public:
    void SetupCarrier(int team, char faction);
    void RegisterLandingPad(Handle landing_pad, int team);
    void RegisterDropshipRequest(Handle request, int team, int time_to_delete);
    void Execute(int mission_turn);
};
