#include "Helpers.h"

#include <cmath>
#include <functional>

#include "../Shared/GameConfig.h"
#include "../Shared/SPMission.h"

void Helpers::AddObjectiveOverride(const char* name, const long color, const float show_time, const bool clear_existing,
                                   const bool is_coop)
{
    if (clear_existing)
    {
        ClearObjectives();
    }

    if (!is_coop)
    {
        AddObjective(name, color, show_time);
    }
    else
    {
        AddToMessagesBox2(name, color);
    }
}

bool Helpers::IsPlayerWithinDistance(const char* path, const float distance, const int total_players)
{
    for (int i = 1; i < total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);

        if (IsAlive(player) && GetDistance(player, path) <= distance)
        {
            return GetDistance(player, path) <= distance;
        }
    }

    return false;
}

bool Helpers::IsPlayerWithinDistance(Handle handle, const float distance, const int total_players)
{
    for (int i = 1; i <= total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);

        if (IsAlive(player) && GetDistance(player, handle) <= distance)
        {
            return true;
        }
    }

    return false;
}

bool Helpers::IsPlayerInBuilding(const int total_players)
{
    for (int i = 1; i <= total_players; ++i)
    {
        Handle player = GetPlayerHandle(i);

        if (IsAlive(player) && InBuilding(player))
        {
            return true;
        }
    }

    return false;
}

bool Helpers::IsAliveAndOnTeam(Handle handle, const int team)
{
    return IsAlive(handle) && GetTeamNum(handle) == team;
}

bool Helpers::IsAudioMessageFinished(const int audio_handle, const int audio_delay_time, const int mission_time,
                                     const bool is_coop)
{
    if (audio_handle == 0)
    {
        return true;
    }

    if (is_coop)
    {
        return audio_delay_time < mission_time;
    }

    return IsAudioMessageDone(audio_handle);
}

bool Helpers::IsVectorZero(const Vector& vec)
{
    return vec.x == 0.0f && vec.y == 0.0f && vec.z == 0.0f;
}

int Helpers::GetRandomInt(const float max)
{
    return static_cast<int>(std::lround(GetRandomFloat(max)));
}

bool Helpers::Teleport(const Handle handle, const Vector& destination, const float offset)
{
    if (!IsAround(handle))
    {
        return false;
    }
    
    BuildObject("teleportin", 0, GetPosition(handle));
    
    Matrix pos = Build_Directinal_Matrix(destination, Vector(0,0,0));
    const Vector front = pos.front;
    pos.posit.x += front.x * offset;
    pos.posit.y += front.y * offset + 5.0f;
    pos.posit.z += front.z * offset;
    
    BuildObject("teleportout", 0, pos);
    
    SetVectorPosition(handle, pos.posit);
    SetVelocity(handle, Vector(front.x, front.y, front.z));
    
    if (IsPlayer(handle))
    {
        SetColorFade(1.0f, 1.0f, 32767);
        StartSoundEffect("teleport.wav", handle);
    }
    
    return true;
}

Handle Helpers::TeleportIn(const char* odf, const int team, const Vector& position)
{
    Vector rand_pos = GetPositionNear(position, 3.0f, 5.0f);
    rand_pos.y += 10.0f;
    
    BuildObject("teleportin", team, rand_pos);
    return BuildObject(odf, team, rand_pos);   
}

void Helpers::TeleportOut(const Handle handle)
{
    BuildObject("teleportout", GetTeamNum(handle), GetPosition(handle));
    RemoveObject(handle);
}

bool Helpers::IsRecycler(const Handle handle)
{
    char obj_class[GameConfig::MAX_ODF_LENGTH];
    GetObjInfo(handle, Get_GOClass, obj_class);
    
    constexpr const char* recycler_classes[] = 
    {
        "CLASS_RECYCLERVEHICLE",
        "CLASS_RECYCLER",
        "CLASS_RECYCLERVEHICLEH"
    };
    
    for (const char* recycler_class : recycler_classes)
    {
        if (strcmp(obj_class, recycler_class) == 0)
        {
            return true;
        }
    }
    
    return false;
}

Vector Helpers::GetPathPosition(const char* path)
{
    if (path == nullptr)
    {
        return EMPTY_VECTOR;
    }
    
    const Handle spawner = BuildObject("pspwn_1", 0 , path);
    const Vector pos = GetPosition(spawner);
    RemoveObject(spawner);
    return pos;   
}

const char* Helpers::GetODFStringFromChain(const char* odf_path, const char* section, const char* key)
{
    static char result[GameConfig::MAX_ODF_LENGTH] = {};
    result[0] = '\0';

    // Helper lambda for recursive search
    static std::function<const char*(const char*, const char*, const char*, int)> SearchODFChain = 
        [](const char* current_odf, const char* sec, const char* k, const int depth) -> const char*
        {
            if (!current_odf || current_odf[0] == '\0' || depth <= 0)
            {
                return result;
            }

            // Try to open and read the ODF
            if (!OpenODF(current_odf))
            {
                return result;
            }

            // Check if this ODF has the key we're looking for
            char value[GameConfig::MAX_ODF_LENGTH] = {};
            GetODFString(current_odf, sec, k, GameConfig::MAX_ODF_LENGTH, value);

            if (value[0] != '\0')
            {
                strcpy_s(result, sizeof(result), value);
                CloseODF(current_odf);
                return result;
            }

            // Check if there's a parent ODF via classLabel
            char class_label[GameConfig::MAX_ODF_LENGTH] = {};
            GetODFString(current_odf, "GameObjectClass", "classLabel", GameConfig::MAX_ODF_LENGTH, class_label);

            CloseODF(current_odf);

            // If we found a classLabel, recursively search the parent
            if (class_label[0] != '\0')
            {
                strcat_s(class_label, ".odf");
                return SearchODFChain(class_label, sec, k, depth - 1);
            }

            return result;
        };

    return SearchODFChain(odf_path, section, key, 10);
}

float Helpers::GetODFFloatFromChain(const char* odf_path, const char* section, const char* key)
{
    static char result[GameConfig::MAX_ODF_LENGTH] = {};
    result[0] = '\0';

    // Helper lambda for recursive search
    static std::function<float(const char*, const char*, const char*, int)> SearchODFChain = 
        [](const char* current_odf, const char* sec, const char* k, const int depth) -> float
        {
            if (!current_odf || current_odf[0] == '\0' || depth <= 0)
            {
                return 0.0f;
            }

            // Try to open and read the ODF
            if (!OpenODF(current_odf))
            {
                return 0.0f;
            }

            // Check if this ODF has the key we're looking for
            char value[GameConfig::MAX_ODF_LENGTH] = {};
            GetODFString(current_odf, sec, k, GameConfig::MAX_ODF_LENGTH, value);

            if (value[0] != '\0')
            {
                const float float_value = std::strtof(value, nullptr);
                CloseODF(current_odf);
                return float_value;
            }

            // Check if there's a parent ODF via classLabel
            char class_label[GameConfig::MAX_ODF_LENGTH] = {};
            GetODFString(current_odf, "GameObjectClass", "classLabel", GameConfig::MAX_ODF_LENGTH, class_label);

            CloseODF(current_odf);

            // If we found a classLabel, recursively search the parent
            if (class_label[0] != '\0')
            {
                strcat_s(class_label, ".odf");
                return SearchODFChain(class_label, sec, k, depth - 1);
            }

            return 0.0f;
        };

    return SearchODFChain(odf_path, section, key, 10);
}