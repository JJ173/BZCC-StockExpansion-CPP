#include "InstantR.h"

instant_r::instant_r()
{
}

instant_r::~instant_r()
{
}

bool instant_r::Load(const bool mission_save)
{
    return DLLBase::Load(mission_save);
}

bool instant_r::PostLoad(const bool mission_save)
{
    return DLLBase::PostLoad(mission_save);
}

bool instant_r::Save(const bool mission_save)
{
    return DLLBase::Save(mission_save);
}

void instant_r::AddObject(Handle h)
{
    DLLBase::AddObject(h);
}

void instant_r::DeleteObject(Handle h)
{
    DLLBase::DeleteObject(h);
}

void instant_r::Execute()
{
    DLLBase::Execute();
}

EjectKillRetCodes instant_r::PlayerEjected(const Handle dead_object_handle)
{
    return DLLBase::PlayerEjected(dead_object_handle);
}

EjectKillRetCodes instant_r::ObjectKilled(const int dead_object_handle, const int killers_handle)
{
    return DLLBase::ObjectKilled(dead_object_handle, killers_handle);
}

EjectKillRetCodes instant_r::ObjectSniped(const int dead_object_handle, const int killers_handle)
{
    return DLLBase::ObjectSniped(dead_object_handle, killers_handle);
}

// Factory used by DllBase.cpp
DLLBase* BuildMission()
{
    return new instant_r();
}