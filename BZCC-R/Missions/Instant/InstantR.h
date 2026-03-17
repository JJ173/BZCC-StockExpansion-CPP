#ifndef INSTANTR_H
#define INSTANTR_H

#include <cstdint>
#include "../../Shared/DllBase.h"

class instant_r : public DLLBase
{
    enum class aip_type : uint8_t
    {
        aip_type0 = 0,
        aip_type1,
        aip_type2,
        aip_type3,
        aip_type_a,
        aip_type_l,
        aip_type_s,
        max_aip_type,
    };
    
public:
    // Constructor.
    instant_r();
    // Destructor.
    ~instant_r() override;
    
    // Housekeeping functions.
    bool Load(bool mission_save);
    bool PostLoad(bool mission_save);
    bool Save(bool mission_save);
    
    // Callback when an object has been built into the world.
    void AddObject(Handle h);
    void DeleteObject(Handle h);
    
    // Runs each turn.
    void Execute(void);
    
    EjectKillRetCodes PlayerEjected(Handle dead_object_handle);
    EjectKillRetCodes ObjectKilled(int dead_object_handle, int killers_handle);
    EjectKillRetCodes ObjectSniped(int dead_object_handle, int killers_handle);
};

#endif // INSTANTR_H
