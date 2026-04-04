#include "CarrierManager.h"

#include "../Shared/SPMission.h"
#include "../Utilities/Helpers.h"

CarrierManager::Carrier* CarrierManager::GetTeamCarrier(const int team)
{
    for (Carrier& carrier : carriers_)
    {
        if (carrier.team == team)
        {
            return &carrier;
        }
    }
    
    char console_log[GameConfig::MAX_CONSOLE_MSG_LENGTH] = {};
    if (sprintf_s(console_log, sizeof(console_log), "[CM]: No carrier found for team %d", team) > 0)
    {
        PrintConsoleMessage(console_log);
    }
    
    return nullptr;
}

void CarrierManager::SetupCarrier(const int team, const char faction)
{
    char spawn_path[GameConfig::MAX_NAME_LENGTH] = {};
    if (sprintf_s(spawn_path, sizeof(spawn_path), "Carrier_%d", team) < 0)
    {
        PrintConsoleMessage("[CM]: Failed to write to the spawn_path character array. Aborting process... Please report this as a bug.");
        return;
    }
    
    const Handle spawner = BuildObject("pspwn_1", team, spawn_path);
    if (!IsAround(spawner))
    {
        char console_log[GameConfig::MAX_CONSOLE_MSG_LENGTH] = {};
        if (sprintf_s(console_log, sizeof(console_log), "[CM]: Carrier spawn path not found for team %d", team) > 0)
        {
            PrintConsoleMessage(console_log);
        }
        return;
    }
    
    char odf[GameConfig::MAX_ODF_LENGTH] = {};
    if (sprintf_s(odf, sizeof(odf), "%cbcarrier_xm", faction) < 0)
    {
        PrintConsoleMessage("[CM]: Failed to write to the ODF character array. Aborting process... Please report this as a bug.");
        return;
    }
    
    Vector pos = GetPosition(spawner);
    pos.y = TerrainFindFloor(pos.x, pos.z) + 800.0f;
    RemoveObject(spawner);
    
    Carrier carrier;
    carrier.team = team;
    carrier.faction = faction;
    carrier.handle = BuildObject(odf, team, pos);
    carriers_.push_back(carrier);
}

void CarrierManager::HandleCondor(Condor* condor, const Handle landing_pad, const int mission_turn_count)
{
    if (!IsAround(landing_pad) || condor->delay_time > mission_turn_count)
    {
        return;
    }
    
    switch (condor->state)
    {
    case GameConfig::CondorState::Landing:
        {
            Vector landing_position = GetPosition(landing_pad);
            landing_position.y += 2.0f;
            condor->handle = BuildObject("ivdrop_land_x", condor->team, landing_position);
        
            SetMaxHealth(condor->handle, 0);
            StartEmitter(condor->handle, 1);
            StartEmitter(condor->handle, 2);
            SetAnimation(condor->handle, "land", 1);
        
            condor->delay_time = mission_turn_count + SecondsToTurns(17.0f);
            condor->state = GameConfig::CondorState::Replace;
            break;   
        }
    case GameConfig::CondorState::Replace:
        {
            Matrix pos;
            GetPosition(condor->handle, pos);
            RemoveObject(condor->handle);
        
            condor->handle = BuildObject("ivpdrop", condor->team, pos);        
            condor->delay_time = mission_turn_count + SecondsToTurns(2.0f);
            condor->state = GameConfig::CondorState::BuildUnits;
            break;   
        }
    case GameConfig::CondorState::BuildUnits:
        {
            MaskEmitter(condor->handle, 0);
            SetAnimation(condor->handle, "deploy", 1);

            Vector landing_pad_pos = GetPosition(landing_pad);
            landing_pad_pos.y += 5.0f;
            const Vector original_pos = landing_pad_pos;
            const Vector z_axis(0.0f, 0.0f, 1.0f);
        
            for (int i = 0; i < condor->unit_total; i++)
            {
                constexpr float pos_x_discrim = 8.0f;
                constexpr float pos_z_discrim = 5.0f;
            
                if (i == 1)
                {
                    landing_pad_pos.z -= pos_z_discrim;
                    landing_pad_pos.x += pos_x_discrim;
                }
                else if (i == 2)
                {
                    landing_pad_pos.z -= pos_z_discrim;
                    landing_pad_pos.x -= pos_x_discrim;
                }
            
                Handle unit = BuildObject(GameConfig::GetCondorUnit(condor->type), condor->team, Build_Directinal_Matrix(landing_pad_pos, z_axis));
                SetBestGroup(unit);
                condor->units.push_back(unit);
                landing_pad_pos = original_pos;
            }
        
            condor->delay_time = mission_turn_count + SecondsToTurns(2.5f);
            condor->state = GameConfig::CondorState::OffloadUnits;
            break;   
        }
    case GameConfig::CondorState::OffloadUnits:
        {
            // Ensure each unit clears the dropship.
            for (const Handle unit : condor->units)
            {
                // Calculate a dropoff vector.
                const Vector front = GetFront(condor->handle);
                Vector dropoff_pos = GetPosition(condor->handle);
                dropoff_pos.x += front.x * 75.0f;
                dropoff_pos.y += front.y * 75.0f;
                dropoff_pos.z += front.z * 75.0f;
                Goto(unit, dropoff_pos, 0);
            }
        
            StartSoundEffect("dropdoor.wav", condor->handle);
            condor->state = GameConfig::CondorState::Leave;
            break;   
        }
    case GameConfig::CondorState::Leave:
        {
            for (Handle unit : condor->units)
            {
                if (GetDistance(unit, condor->handle) < 50.0f)
                {
                    return;
                }
            }
        
            Handle p_handle = GetPlayerHandle(1);
            if (GetDistance(p_handle, condor->handle) < 50.0f)
            {
                return;
            }
        
            SetAnimation(condor->handle, "takeoff", 1);
            StartEmitter(condor->handle, 1);
            StartEmitter(condor->handle, 2);
            StartSoundEffect("dropleav.wav", condor->handle);
        
            condor->delay_time = mission_turn_count + SecondsToTurns(20.0f);
            condor->state = GameConfig::CondorState::Remove;
            break;   
        }
    case GameConfig::CondorState::Remove:
        {
            RemoveObject(condor->handle);
            condor->ready_to_delete = true;
            break;   
        }
    }
}

void CarrierManager::HandlePortal(Portal* portal, const int mission_turn_count)
{
    if (!IsAround(portal->handle))
    {
        return;
    }
    
    if (!IsPowered(portal->handle))
    {
        return;
    }
    
    if (portal->delay_time > mission_turn_count)
    {
        return;   
    }
    
    portal->teleported_unit_count++;

    const Vector pos = GetPosition(portal->handle);
    const Handle teleported_unit = Helpers::TeleportIn(GameConfig::GetPortalUnit(portal->type), portal->team, pos);
    
    const Vector front = GetFront(portal->handle);
    Vector dropoff_pos = GetPosition(portal->handle);
    dropoff_pos.x += front.x * 75.0f;
    dropoff_pos.y += front.y * 75.0f;
    dropoff_pos.z += front.z * 75.0f;
    
    SetBestGroup(teleported_unit);
    Goto(teleported_unit, dropoff_pos, 0);
    
    if (portal->teleported_unit_count >= portal->unit_total)
    {
        portal->ready_to_delete = true;
    }
    
    portal->delay_time = mission_turn_count + SecondsToTurns(1.0f);
}

void CarrierManager::RegisterLandingPad(const Handle landing_pad, const int team)
{
    Carrier* carrier = GetTeamCarrier(team);
    
    if (carrier == nullptr)
    {
        return;
    }
    
    carrier->landing_pad = landing_pad;
}

void CarrierManager::RegisterDropshipRequest(const Handle request, const int team, const int time_to_delete)
{
    Carrier* carrier = GetTeamCarrier(team);
    
    if (carrier == nullptr)
    {
        return;
    }
    
    char obj_base[GameConfig::MAX_ODF_LENGTH] = {};
    if (!GetObjInfo(request, Get_GOClass_gCfg, obj_base))
    {
        return;
    }
    
    // Create a new request object.
    const DropshipRequestItem new_request = { request, team, time_to_delete, obj_base };
    carrier->dropship_requests.push_back(new_request);
    
    char console_log[GameConfig::MAX_CONSOLE_MSG_LENGTH] = {};
    if (sprintf_s(console_log, sizeof(console_log), "[CM]: Registering dropship request for team: %d. Time To Delete: %d. Type: %s", team, time_to_delete, obj_base) > 0)
    {
        PrintConsoleMessage(console_log);   
    }
    
    int total_units = 3;
    if (strcmp(obj_base, "ScrapDropship") == 0 || strcmp(obj_base, "ScavengerDropship") == 0)
    {
        total_units = 2;
    }
    
    if (carrier->faction == GameConfig::FACTION_ISDF)
    {
        Condor condor;
        condor.team = team;
        condor.type = obj_base;
        condor.unit_total = total_units;
        condor.state = GameConfig::CondorState::Landing;
        condor.ready_to_delete = false;
        carrier->condors.push_back(condor);
    }
    else if (carrier->faction == GameConfig::FACTION_SCION)
    {
        Portal portal;
        portal.handle = carrier->landing_pad;
        portal.team = team;
        portal.type = obj_base;
        portal.unit_total = total_units;
        portal.teleported_unit_count = 0;
        portal.delay_time = 0;
        portal.ready_to_delete = false;
        carrier->portals.push_back(portal);
    }
}

void CarrierManager::Execute(const int mission_turn)
{
    if (carriers_.empty())
    {
        return;
    }

    for (Carrier& carrier : carriers_)
    {
        for (auto it = carrier.dropship_requests.begin(); it != carrier.dropship_requests.end();)
        {
            if (it->time_to_delete <= mission_turn)
            {
                RemoveObject(it->handle);
                it = carrier.dropship_requests.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (!IsAround(carrier.landing_pad))
        {
            continue;
        }

        if (carrier.faction == GameConfig::FACTION_ISDF)
        {
            if (carrier.condors.empty())
            {
                continue;
            }

            Condor& condor = carrier.condors.front();
            HandleCondor(&condor, carrier.landing_pad, mission_turn);

            if (condor.ready_to_delete)
            {
                carrier.condors.erase(carrier.condors.begin());
            }
        }
        else if (carrier.faction == GameConfig::FACTION_SCION)
        {
            if (carrier.portals.empty())
            {
                continue;
            }
            
            Portal& portal = carrier.portals.front();
            HandlePortal(&portal, mission_turn);
            
            if (portal.ready_to_delete)
            {
                carrier.portals.erase(carrier.portals.begin());
            }
        }
    }
}