#include <cstdint>
#include "../../Shared/DLLBase.h"
#include "../../Utilities/Helpers.h"
#include "../../Utilities/Subtitles.h"

enum class Phase: uint8_t
{
    MissionSetup,
    IntroCutscene,
    FirstObjective,
};

class Scion03 : public DLLBase
{
    struct ODFs
    {
        static constexpr auto RecyclerISDF = "ivrecy_x";
        static constexpr auto RecyclerScion = "fvrecy_x";
        static constexpr auto RhinoScion = "bcrhino";
        static constexpr auto ScoutEnemy = "ivscout_x";
        static constexpr auto TankISDF = "ivtank_x";
        static constexpr auto MbikeEnemy = "ivmbike_x";
        static constexpr auto RcktEnemy = "ivrckt_x";
        static constexpr auto MansonWeapon = "gspstab_c";
        static constexpr auto TruckISDF = "ivserv_x";
    };

    struct Paths
    {
        static constexpr auto CinematicMove = "cin_move";
        static constexpr auto CinematicRcktMove = "cin_rckt_move";
        static constexpr auto CinematicCamera = "cin1_camera";
        static constexpr auto MansonPatrol = "manson_patrol";
        static constexpr auto NRPatrol1 = "tank_1_patrol";
        static constexpr auto NRPatrol2 = "tank_2_patrol";
    };

    struct AudioFiles
    {
        static constexpr auto IntroVO = "CutSc0501.wav";
        static constexpr auto FirstObjectiveVO = "scion0501.wav";

        static constexpr auto BraddockContactVO = "isdf2021.wav";
        static constexpr auto MansonAnswerBraddockVO = "isdf2022.wav";
        static constexpr auto BraddockAnswerMansonVO = "scion0301_new.wav";

        static constexpr auto MansonHelpVO = "scion0503.wav";
    };

    struct Objectives
    {
        static constexpr auto FirstObjectiveOTF = "scion0501_new.otf";
        static constexpr auto RecyLostFail = "recy_lost.txt";
    };

public:
    Scion03()
    {
        AllowRandomTracks(true);

        b_count_ = &state.last_bool_ - &state.first_bool_ - 1;
        b_array_ = &state.first_bool_ + 1;

        f_count_ = &state.last_float_ - &state.first_float_ - 1;
        f_array_ = &state.first_float_ + 1;

        h_count_ = &state.last_handle_ - &state.first_handle_ - 1;
        h_array_ = &state.first_handle_ + 1;

        i_count_ = &state.last_int_ - &state.first_int_ - 1;
        i_array_ = &state.first_int_ + 1;

        static constexpr const char* odfsToPreload[] =
        {
            ODFs::RecyclerISDF,
            ODFs::RecyclerScion,
            ODFs::RhinoScion,
        };

        for (const char* odf : odfsToPreload)
        {
            PreloadODF(odf);
        }

        Ally(state.data.enemyTeam, state.data.tempEnemyTeam);

        SetTeamNameForStat(state.data.hostTeam, "Scion");
        SetTeamNameForStat(state.data.alliedTeam, "AAN");
        SetTeamNameForStat(state.data.enemyTeam, "New Regime");
        SetTeamNameForStat(state.data.tempEnemyTeam, "Scion Rebels");

        SetTeamColor(state.data.alliedTeam, 0, 127, 255);
        SetTeamColor(state.data.tempEnemyTeam, 85, 255, 85);
    }

    struct MissionState
    {
        bool first_bool_ = false;
        Phase currentPhase = Phase::MissionSetup;

        struct Flags
        {
            bool isSetup = false;
            bool isCooperative = false;
            bool isCameraReady = false;
            bool isFirstShotOver = false;
            bool isFirstObjectiveDelaySet = false;
            bool isFirstObjectiveSet = false;

            bool hasFirstObjectiveVoicePlayed = false;
            bool hasFirstCutsceneVoicePlayed = false;

            bool hasBraddockContactedManson = false;
            bool hasMansonAnsweredBraddock = false;
            bool hasBraddockAnswerManson = false;

            bool hasBraddockWalkerMoved = false;

            bool hasMansonCalledForHelp = false;

            bool isPlayerRecyDead = false;
        } flags;

        bool last_bool_ = false;

        float first_float_ = 0.0f;
        float last_float_ = 0.0f;

        Handle first_handle_ = 0;

        struct Units
        {
            Handle cut1VO;
            Handle generalVO;

            Handle aanRecy;
            Handle braddockRecy;
            Handle playerRecy;

            UnitTracker manson;
            UnitTracker aanTanks[3];

            UnitTracker patrol1;
            UnitTracker patrol2;

            UnitTracker braddockWalker;
            UnitTracker braddockTrucks[2];
            UnitTracker braddockAssaultTank;

            Handle cutsceneLook;

            UnitTracker cinematicUnits[7];
        } units;

        Handle last_handle_ = 0;

        int first_int_ = 0;

        struct Data
        {
            int missionTurn = 0;

            int hostTeam = 1;
            int alliedTeam = 5;
            int enemyTeam = 6;
            int tempEnemyTeam = 7;
        } data;

        struct Timers
        {
            MissionTimer firstCinematicTimer;
            MissionTimer voiceTimer;
        } timers;

        Subtitles subtitles;
        int last_int_ = 0;
    } state;

    // --- Overrides ---
    void AddObject(Handle h) override;
    void DeleteObject(Handle h) override;
    void Execute() override;

    // bool Save(bool missionSave) override;
    // bool Load(bool missionSave) override;
    bool PostLoad(bool missionSave) override;

protected:
    bool* b_array_;
    int b_count_;

    float* f_array_;
    int f_count_;

    Handle* h_array_;
    int h_count_;

    int* i_array_;
    int i_count_;

private:
    void RegisterVariables();
    void SetupMission();

    void RunIntroCutscene();
    void RunFirstObjective();

    void ManageAAN();
    void ManageBraddock();
};

DLLBase* BuildMission()
{
    return new Scion03();
}

void Scion03::ManageAAN()
{
    // Need to index this so we can send the relevant orders.
    for (int i = 0; i < 3; i++)
    {
        UnitTracker& tank = state.units.aanTanks[i];
        tank.active = Helpers::IsAliveAndOnTeam(tank.handle, state.data.alliedTeam);

        if (!tank.IsIdle())
        {
            return;
        }

        if (i == 0)
        {
            tank.InitPatrol("tank1_patrol");
        }
        else if (i == 1)
        {
            tank.InitPatrol("tank2_patrol");
        }
        else
        {
            tank.InitFollow(state.units.manson.handle);
        }
    }
}

void Scion03::ManageBraddock()
{
    // Keep track of trackers and clean them up if the units are dead.
    for (auto& braddockTruck : state.units.braddockTrucks)
    {
        if (braddockTruck.active && !Helpers::IsAliveAndOnTeam(braddockTruck.handle, state.data.enemyTeam))
        {
            braddockTruck.Reset();
        }
    }
}

void Scion03::RegisterVariables()
{
    // Initialize legacy counts
    // b_count = 0; f_count = 0; h_count = 0; i_count = 0;

    // In a real implementation, you'd point b_array, f_array etc. 
    // to the members of the 'state' struct. For this example, 
    // we demonstrate the structure.
}

void Scion03::SetupMission()
{
    state.flags.isCooperative = IsNetworkOn();

    state.units.aanRecy = GetHandle("aan_recy");
    state.units.braddockRecy = GetHandle("braddock_recy");
    state.units.playerRecy = GetHandle("playerrecy");

    state.units.manson.handle = GetHandle("manson");
    state.units.manson.active = true;
    Helpers::TryTrackUnit(GetHandle("tank1"), ODFs::TankISDF, state.units.aanTanks, 1);
    Helpers::TryTrackUnit(GetHandle("tank2"), ODFs::TankISDF, state.units.aanTanks, 2);
    Helpers::TryTrackUnit(GetHandle("tank3"), ODFs::TankISDF, state.units.aanTanks, 3);

    state.units.patrol1.handle = GetHandle("patrol1");
    state.units.patrol2.handle = GetHandle("patrol2");

    state.units.braddockWalker.handle = GetHandle("attack_walker");
    state.units.braddockAssaultTank.handle = GetHandle("attack_ass");

    Helpers::TryTrackUnit(GetHandle("attack_truck"), ODFs::TruckISDF, state.units.braddockTrucks, 1);
    Helpers::TryTrackUnit(GetHandle("attack_truck2"), ODFs::TruckISDF, state.units.braddockTrucks, 2);

    state.units.braddockAssaultTank.InitFollow(state.units.braddockWalker.handle);

    for (auto& braddockTruck : state.units.braddockTrucks)
    {
        braddockTruck.InitFollow(state.units.braddockWalker.handle);
    }

    state.units.patrol1.InitPatrol(Paths::NRPatrol1);
    state.units.patrol2.InitPatrol(Paths::NRPatrol2);

    state.units.cutsceneLook = GetHandle("cin_look");

    SetMaxHealth(state.units.manson.handle, 0);
    SetCanSnipe(state.units.manson.handle, 0);
    SetObjectiveName(state.units.manson.handle, "Manson");
    GiveWeapon(state.units.manson.handle, ODFs::MansonWeapon);
    Patrol(state.units.manson.handle, Paths::MansonPatrol);

    if (!state.flags.isCooperative)
    {
        state.currentPhase = Phase::IntroCutscene;
    }
    else
    {
        state.currentPhase = Phase::FirstObjective;
    }

    state.flags.isSetup = true;
}

void Scion03::Execute()
{
    state.data.missionTurn++;
    state.subtitles.Execute();

    // Stop Manson from being a target for the enemy since he is unkillable.
    SetPerceivedTeam(state.units.manson.handle, state.data.enemyTeam);

    if (state.currentPhase > Phase::IntroCutscene)
    {
        ManageAAN();
        ManageBraddock();
    }

    switch (state.currentPhase)
    {
    case Phase::MissionSetup:
        SetupMission();
        break;
    case Phase::IntroCutscene:
        RunIntroCutscene();
        break;
    case Phase::FirstObjective:
        RunFirstObjective();
        break;
    }
}

void Scion03::AddObject(Handle h)
{
    const int teamNum = GetTeamNum(h);

    if (teamNum == state.data.hostTeam)
    {
        SetSkill(h, 3);
    }

    if (teamNum == state.data.alliedTeam)
    {
        if (state.flags.isSetup)
        {
            Helpers::TryTrackUnit(h, ODFs::TankISDF, state.units.aanTanks, 3);
        }
    }
}

void Scion03::DeleteObject(const Handle h)
{
    // Grab the teams to ensure we only process relevant handles for each team.
    const int teamNum = GetTeamNum(h);

    if (teamNum == state.data.hostTeam)
    {
        if (h == state.units.playerRecy)
        {
            state.flags.isPlayerRecyDead = true;
            state.units.playerRecy = 0;
            return;
        }

        return;
    }

    if (teamNum == state.data.alliedTeam)
    {
        if (h == state.units.aanRecy)
        {
            state.units.aanRecy = 0;
            return;
        }

        if (h == state.units.manson.handle)
        {
            state.units.manson.Reset();
            return;
        }

        return;
    }

    if (teamNum == state.data.enemyTeam)
    {
        if (h == state.units.braddockRecy)
        {
            state.units.braddockRecy = 0;
            return;
        }

        if (h == state.units.patrol1.handle)
        {
            state.units.patrol1.Reset();
            return;
        }

        if (h == state.units.patrol2.handle)
        {
            state.units.patrol2.Reset();
            return;
        }

        if (h == state.units.braddockAssaultTank.handle)
        {
            for (auto& braddockTruck : state.units.braddockTrucks)
            {
                if (state.units.braddockWalker.active)
                {
                    braddockTruck.InitFollow(state.units.braddockWalker.handle);
                }
                else
                {
                    const Vector pos = GetPositionNear(GetPosition(state.units.braddockRecy), 30.0f, 60.0f);
                    Goto(braddockTruck.handle, pos);
                }
            }

            state.units.braddockAssaultTank.Reset();
            return;
        }

        if (h == state.units.braddockWalker.handle)
        {
            for (auto& braddockTruck : state.units.braddockTrucks)
            {
                if (state.units.braddockAssaultTank.active)
                {
                    braddockTruck.InitFollow(state.units.braddockAssaultTank.handle);
                }
                else
                {
                    const Vector pos = GetPositionNear(GetPosition(state.units.braddockRecy), 30.0f, 60.0f);
                    Goto(braddockTruck.handle, pos);
                }
            }

            state.units.braddockWalker.Reset();
            return;
        }
    }
}

// bool Scion03::Save(const bool missionSave)
// {
//     if (missionSave)
//         return true;
//
//     if (b_count_ > 0) Write(b_array_, b_count_);
//     if (f_count_ > 0) Write(f_array_, f_count_);
//     if (h_count_ > 0) Write(h_array_, h_count_);
//     if (i_count_ > 0) Write(i_array_, i_count_);
//
//     return true;
// }
//
// bool Scion03::Load(const bool missionSave)
// {
//     if (missionSave)
//     {
//         for (int i = 0; i < b_count_; i++) b_array_[i] = false;
//         for (int i = 0; i < f_count_; i++) f_array_[i] = 0.0f;
//         for (int i = 0; i < h_count_; i++) h_array_[i] = 0;
//         for (int i = 0; i < i_count_; i++) i_array_[i] = 0;
//
//         return true;
//     }
//
//     if (b_count_ > 0) Read(b_array_, b_count_);
//     if (f_count_ > 0) Read(f_array_, f_count_);
//     if (h_count_ > 0) Read(h_array_, h_count_);
//     if (i_count_ > 0) Read(i_array_, i_count_);
//
//     return true;
// }

bool Scion03::PostLoad(const bool missionSave)
{
    if (missionSave)
        return true;

    ConvertHandles(h_array_, h_count_);

    return true;
}

void Scion03::RunIntroCutscene()
{
    if (!state.flags.isFirstShotOver)
    {
        if (!state.flags.isCameraReady)
        {
            // Prep the camera.
            CameraReady();

            // Build the right objects for the cinematic attackers.
            state.units.cinematicUnits[0].handle = BuildObject(ODFs::ScoutEnemy, state.data.enemyTeam, "cin_scout1");
            state.units.cinematicUnits[0].active = true;
            state.units.cinematicUnits[1].handle = BuildObject(ODFs::ScoutEnemy, state.data.enemyTeam, "cin_scout2");
            state.units.cinematicUnits[1].active = true;
            state.units.cinematicUnits[2].handle = BuildObject(ODFs::TankISDF, state.data.enemyTeam, "cin_tank1");
            state.units.cinematicUnits[2].active = true;
            state.units.cinematicUnits[3].handle = BuildObject(ODFs::TankISDF, state.data.enemyTeam, "cin_tank2");
            state.units.cinematicUnits[3].active = true;
            state.units.cinematicUnits[4].handle = BuildObject(ODFs::TankISDF, state.data.enemyTeam, "cin_tank3");
            state.units.cinematicUnits[4].active = true;
            state.units.cinematicUnits[5].handle = BuildObject(ODFs::MbikeEnemy, state.data.enemyTeam, "cin_mbike1");
            state.units.cinematicUnits[5].active = true;
            state.units.cinematicUnits[6].handle = BuildObject(ODFs::RcktEnemy, state.data.enemyTeam, "cin_rckt1");
            state.units.cinematicUnits[6].active = true;

            // Grab the random position path of interest.
            const Vector pos = Helpers::GetPathPosition(Paths::CinematicMove);

            // Move all the units to a random place in the base and have them attack.
            for (const auto& cinematicUnit : state.units.cinematicUnits)
            {
                if (cinematicUnit.active)
                {
                    // Give them all good health.
                    SetMaxHealth(cinematicUnit.handle, 8000);
                    SetCurHealth(cinematicUnit.handle, 8000);
                    Goto(cinematicUnit.handle, GetPositionNear(pos, 30.0f, 50.0f));
                }
            }

            // Move the rocket tank to the specific path.
            Retreat(state.units.cinematicUnits[6].handle, Paths::CinematicRcktMove);

            // Prepare timers.
            state.timers.firstCinematicTimer.Start(35.0f, state.data.missionTurn);
            state.timers.voiceTimer.Start(3.0f, state.data.missionTurn);

            // Camera is ready and done.
            state.flags.isCameraReady = true;
        }

        CameraPath(Paths::CinematicCamera, 2000, 700, state.units.cutsceneLook);
    }

    if (!state.flags.hasFirstCutsceneVoicePlayed && state.timers.voiceTimer.IsExpired(state.data.missionTurn))
    {
        state.units.cut1VO = state.subtitles.AudioWithSubtitles(AudioFiles::IntroVO,
                                                                GameConfig::SubtitlePanelSize::Small);
        state.flags.hasFirstCutsceneVoicePlayed = true;
    }

    if (CameraCancelled() || state.timers.firstCinematicTimer.IsExpired(state.data.missionTurn))
    {
        // Remove the cutscene units.
        for (auto& cinematicUnit : state.units.cinematicUnits)
        {
            if (cinematicUnit.active)
            {
                RemoveObject(cinematicUnit.handle);
                cinematicUnit.Reset();
            }
        }

        RemoveObject(state.units.cutsceneLook);
        CameraFinish();

        if (IsAudioPlaying(state.units.cut1VO))
        {
            StopAudioMessage(state.units.cut1VO);
        }

        SetScrap(state.data.hostTeam, 40);
        SetScrap(state.data.alliedTeam, 40);
        SetScrap(state.data.enemyTeam, 40);

        state.currentPhase = Phase::FirstObjective;
    }
}

void Scion03::RunFirstObjective()
{
    if (!state.flags.isFirstObjectiveDelaySet)
    {
        // Done here to prevent betty spam of allied units being attacked.
        for (int i = 2; i <= 5; i++)
        {
            Ally(i, state.data.hostTeam);
        }
        
        SetPlan("scion0301_x.aip", state.data.alliedTeam);
        
        state.timers.voiceTimer.Start(2.5f, state.data.missionTurn);
        state.flags.isFirstObjectiveDelaySet = true;
    }

    if (state.timers.voiceTimer.IsExpired(state.data.missionTurn) && Helpers::IsAudioMessageFinished(
        state.units.generalVO, state.timers.voiceTimer.expiry, state.data.missionTurn, state.flags.isCooperative))
    {
        if (!state.flags.hasFirstObjectiveVoicePlayed)
        {
            state.units.generalVO = state.subtitles.AudioWithSubtitles(AudioFiles::FirstObjectiveVO,
                                                                       GameConfig::SubtitlePanelSize::Small);
            state.flags.hasFirstObjectiveVoicePlayed = true;
            return;
        }

        if (state.flags.isFirstObjectiveSet && !state.flags.hasBraddockContactedManson)
        {
            state.units.generalVO = state.subtitles.AudioWithSubtitles(AudioFiles::BraddockContactVO,
                                                                       GameConfig::SubtitlePanelSize::Small);
            state.flags.hasBraddockContactedManson = true;
            return;
        }

        if (state.flags.hasBraddockContactedManson && !state.flags.hasMansonAnsweredBraddock)
        {
            state.units.generalVO = state.subtitles.AudioWithSubtitles(AudioFiles::MansonAnswerBraddockVO,
                                                                       GameConfig::SubtitlePanelSize::Small);
            state.flags.hasMansonAnsweredBraddock = true;
            return;
        }

        if (state.flags.hasMansonAnsweredBraddock && !state.flags.hasBraddockAnswerManson)
        {
            state.units.generalVO = state.subtitles.AudioWithSubtitles(AudioFiles::BraddockAnswerMansonVO,
                                                                       GameConfig::SubtitlePanelSize::Small);

            state.flags.hasBraddockAnswerManson = true;
            return;
        }

        if (state.flags.hasBraddockAnswerManson && !state.flags.hasBraddockWalkerMoved)
        {
            Goto(state.units.braddockWalker.handle, "braddock_walker_attack");
            Goto(state.units.braddockAssaultTank.handle, "braddock_walker_attack");

            state.timers.voiceTimer.Start(120.0f, state.data.missionTurn);
            state.flags.hasBraddockWalkerMoved = true;
            return;
        }

        if (state.flags.hasBraddockAnswerManson && !state.flags.hasMansonCalledForHelp)
        {
            state.units.generalVO = state.subtitles.AudioWithSubtitles(AudioFiles::MansonHelpVO,
                                                                       GameConfig::SubtitlePanelSize::Small);
            state.flags.hasMansonCalledForHelp = true;
            return;
        }
    }

    if (state.flags.hasFirstObjectiveVoicePlayed
        && !state.flags.isFirstObjectiveSet
        && Helpers::IsAudioMessageFinished(state.units.generalVO, state.timers.voiceTimer.expiry,
                                           state.data.missionTurn,
                                           state.flags.isCooperative))
    {
        AddObjective(Objectives::FirstObjectiveOTF, WHITE);
        state.flags.isFirstObjectiveSet = true;
        state.timers.voiceTimer.Start(60.0f, state.data.missionTurn);
    }
}
