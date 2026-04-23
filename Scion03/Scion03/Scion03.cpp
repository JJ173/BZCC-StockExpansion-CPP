#include "Scion03.h"

Scion03::Scion03()
{
    AllowRandomTracks(true);

    b_count_ = &state.last_bool_ - &state.first_bool_ - 1;
    b_array_ = &state.first_bool_ + 1;

    h_count_ = &state.last_handle_ - &state.first_handle_ - 1;
    h_array_ = &state.first_handle_ + 1;

    i_count_ = &state.last_int_ - &state.first_int_ - 1;
    i_array_ = &state.first_int_ + 1;

    static constexpr const char* odfsToPreload[] =
    {
        GameConfig::ODFs::RecyclerISDF,
        GameConfig::ODFs::RecyclerScion,
        GameConfig::ODFs::IceRhino,
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

void Scion03::ManageAAN()
{
    if (state.units.aanEntryTracker != 0)
    {
        // Run a quick check to see if an enemy is near by.
        Handle nearestEnemy = GetNearestEnemy(state.units.aanEntryTracker, true, false, 125.0f);

        if (nearestEnemy != 0)
        {
            // Send all units to attack if they are not already doing so.
            if (GetTarget(state.units.manson.handle) == 0)
            {
                state.units.manson.InitAttack(nearestEnemy);
            }

            for (int i = 0; i < static_cast<int>(std::size(state.units.aanTanks)); i++)
            {
                UnitTracker& tank = state.units.aanTanks[i];

                if (!tank.active || GetTarget(state.units.manson.handle) != 0)
                {
                    continue;
                }

                tank.InitAttack(nearestEnemy);
            }

            return;
        }
    }

    if (state.units.manson.active && state.units.manson.command != CMD_PATROL)
    {
        state.units.manson.InitPatrol(Paths::MansonPatrol);
    }

    for (int i = 0; i < static_cast<int>(std::size(state.units.aanTanks)); i++)
    {
        UnitTracker& tank = state.units.aanTanks[i];

        if (!tank.active || !tank.IsIdle())
        {
            continue;
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

    if (!state.flags.isBridgeObjectiveActive)
    {
        // Keep track of off-map attackers so they can engage a nearby enemy.
        for (auto& attacker : state.units.braddockOffMapAttacks)
        {
            if (attacker.active && !Helpers::IsAliveAndOnTeam(attacker.handle, state.data.enemyTeam))
            {
                attacker.Reset();
                continue;
            }   
                        
            if (attacker.IsIdle())
            {                
                attacker.InitAttack(state.units.playerRecy);
            }
        }
        
        // Start the attacks against the player.
        if (state.timers.waveTimerPlayer.IsExpired(state.data.missionTurn) && state.flags.isBraddockPlayerAttacksActive)
        {
            char build_path[GameConfig::MAX_ODF_LENGTH] = {};

            for (int i = 0; i < BRADDOCK_PLAYER_WAVE_SIZE; i++)
            {
                const char* buildODF = PlayerAttackWaveUnits[state.data.braddockAttackPlayerCounter][state.data.
                    missionDifficulty][i];

                if (buildODF == nullptr)
                {
                    continue;
                }

                // Create the unit and send it out to attack.
                if (sprintf_s(build_path, sizeof(build_path), "brad_offmap_%d", i + 1) < 0)
                {
                    build_path[0] = '\0';
                    continue;
                }

                const Handle attacker = BuildObject(buildODF, state.data.enemyTeam, build_path);
                Helpers::TryTrackUnit(attacker, buildODF, state.units.braddockOffMapAttacks, i + 1);
            }

            state.data.braddockAttackPlayerCounter++;
            state.timers.waveTimerPlayer.Start(delay1[state.data.missionDifficulty], state.data.missionTurn);
        }

        // Start the attacks against the AAN.
        if (!state.timers.waveTimerAAN.IsExpired(state.data.missionTurn) || !state.flags.isBraddockAANAttacksActive)
        {
            return;
        }

        if (!state.flags.hasBraddockFirstWaveSpawned)
        {
            state.timers.waveTimerAAN.Start(15.0f, state.data.missionTurn);

            const Handle tank1 = BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));
            const Handle tank2 = BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));
            const Handle tank3 = BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));
            const Handle tank4 = BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));

            Follow(tank2, tank1);
            Follow(tank3, tank2);
            Follow(tank4, tank3);
            Goto(tank1, "braddock_bridge_attack");

            state.flags.hasBraddockFirstWaveSpawned = true;
        }
        else
        {
            const Handle misl1 = BuildObject(GameConfig::ODFs::MissileISDF, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));
            const Handle misl2 = BuildObject(GameConfig::ODFs::MissileISDF, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));

            Follow(misl2, misl1);
            Goto(misl1, "braddock_bridge_attack");

            state.timers.waveTimerAAN.Start(120.0f, state.data.missionTurn);
            state.flags.hasBraddockFirstWaveSpawned = false;
        }

        return;
    }

    if (state.timers.waveTimerAAN.IsExpired(state.data.missionTurn))
    {
        for (int i = 0; i < BRADDOCK_MAX_BRIDGE_ATTACKS; ++i)
        {
            const char* odf = WaveUnits[state.data.waveCounter][i];
            if (odf != nullptr)
            {
                const Handle h = BuildObject(odf, state.data.enemyTeam,
                                             GetPositionNear(state.data.braddockOffMapSpawnPos, 30.0f, 60.0f));
                Goto(h, "braddock_bridge_attack");
            }
        }

        state.data.waveCounter++;
        state.timers.waveTimerAAN.Start(state.data.braddockAANAttackDelays[state.data.missionDifficulty - 1],
                                        state.data.missionTurn);
    }
}

void Scion03::RegisterVariables()
{
    // i_count determines how many integers from i_array are saved.
    // The mission constructor sets i_count based on the range from first_int_ to last_int_.
    // Since waveCounter is inside the Data struct which is between first_int_ and last_int_,
    // it will be automatically included in the save/load if the counts are correct.
}

void Scion03::SetupMission()
{
    state.flags.isCooperative = IsNetworkOn();

    if (!state.flags.isCooperative)
    {
        state.data.missionDifficulty = GetVarItemInt("network.session.ivar102") + 1;
    }
    else
    {
        state.data.missionDifficulty = GetVarItemInt("options.play.difficulty") + 1;
    }

    state.units.aanRecy = GetHandle("aan_recy");
    state.units.braddockRecy = GetHandle("braddock_recy");
    state.units.playerRecy = GetHandle("playerrecy");
    state.units.manson.Register(GetHandle("manson"));

    for (int i = 0; i < MAX_BRIDGE_SEGMENTS; i++)
    {
        char label[GameConfig::MAX_ODF_LENGTH];

        if (sprintf_s(label, sizeof(label), "bridge%d", i + 1) < 0)
        {
            label[0] = '\0';
        }

        state.units.bridgeSegements[i] = GetHandle(label);
        SetMaxHealth(state.units.bridgeSegements[i], 5000);
    }

    Helpers::TryTrackUnit(GetHandle("tank1"), GameConfig::ODFs::TankISDF, state.units.aanTanks, 1);
    Helpers::TryTrackUnit(GetHandle("tank2"), GameConfig::ODFs::TankISDF, state.units.aanTanks, 2);
    Helpers::TryTrackUnit(GetHandle("tank3"), GameConfig::ODFs::TankISDF, state.units.aanTanks, 3);

    state.units.patrol1.Register(GetHandle("patrol1"));
    state.units.patrol2.Register(GetHandle("patrol2"));
    state.units.braddockWalker.Register(GetHandle("attack_walker"));
    state.units.braddockAssaultTank.Register(GetHandle("attack_ass"));

    Helpers::TryTrackUnit(GetHandle("attack_truck"), GameConfig::ODFs::ServiceTruckISDF, state.units.braddockTrucks, 1);
    Helpers::TryTrackUnit(GetHandle("attack_truck2"), GameConfig::ODFs::ServiceTruckISDF, state.units.braddockTrucks,
                          2);

    for (auto& braddockTruck : state.units.braddockTrucks)
    {
        braddockTruck.InitFollow(state.units.braddockWalker.handle);
    }

    state.units.patrol1.InitPatrol(Paths::NRPatrol1);
    state.units.patrol2.InitPatrol(Paths::NRPatrol2);

    state.units.aanEntryTracker = GetHandle("aan_response");
    state.units.cutsceneLook = GetHandle("cin_look");

    state.units.offmap_dropship_1 = GetHandle("offmap_ship_1");
    state.units.offmap_dropship_2 = GetHandle("offmap_ship_2");
    state.units.offmap_dropship_3 = GetHandle("offmap_ship_3");

    MaskEmitter(state.units.offmap_dropship_1, 0);
    MaskEmitter(state.units.offmap_dropship_2, 0);
    MaskEmitter(state.units.offmap_dropship_3, 0);

    SetAnimation(state.units.offmap_dropship_1, "deploy", 1);
    SetAnimation(state.units.offmap_dropship_2, "deploy", 2);
    SetAnimation(state.units.offmap_dropship_3, "deploy", 3);

    // Dirty cheater!
    SetMaxHealth(state.units.manson.handle, 0);
    SetMaxAmmo(state.units.manson.handle, 0);
    SetCanSnipe(state.units.manson.handle, 0);
    SetObjectiveName(state.units.manson.handle, "Manson");
    GiveWeapon(state.units.manson.handle, GameConfig::ODFs::SPStabber);

    // Grab the Vector position of the out-of-bounds spawn path for braddock so we don't keep querying it.
    state.data.braddockOffMapSpawnPos = Helpers::GetPathPosition("braddock_oob");

    // Kick-start the wave timer so the expiry checks work.
    state.timers.waveTimerPlayer.Start(1.0f, state.data.missionTurn);
    state.timers.waveTimerAAN.Start(1.0f, state.data.missionTurn);

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
    case Phase::BridgeObjective:
        RunBridgeObjective();
        break;
    }
}

void Scion03::AddObject(const Handle h)
{
    const int teamNum = GetTeamNum(h);

    if (teamNum == state.data.hostTeam)
    {
        SetSkill(h, 3);

        if (Helpers::IsRecycler(h))
        {
            state.units.playerRecy = h;
        }
    }

    if (teamNum == state.data.alliedTeam)
    {
        if (state.flags.isSetup)
        {
            Helpers::TryTrackUnit(h, GameConfig::ODFs::TankISDF, state.units.aanTanks, 3);
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

        for (auto& tank : state.units.aanTanks)
        {
            if (tank.active && tank.handle == h)
            {
                tank.Reset();
            }
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
        }
    }
}

bool Scion03::Save(const bool missionSave)
{
    if (missionSave)
    {
        return true;
    }

    if (b_count_ > 0) Write(b_array_, b_count_);
    if (h_count_ > 0) Write(h_array_, h_count_);
    if (i_count_ > 0) Write(i_array_, i_count_);

    return true;
}

bool Scion03::Load(const bool missionSave)
{
    if (missionSave)
    {
        return true;
    }

    if (b_count_ > 0) Read(b_array_, b_count_);
    if (h_count_ > 0) Read(h_array_, h_count_);
    if (i_count_ > 0) Read(i_array_, i_count_);

    return true;
}

bool Scion03::PostLoad(const bool missionSave)
{
    if (missionSave)
    {
        return true;
    }

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
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::ScoutISDF, state.data.enemyTeam, "cin_scout1"),
                                  GameConfig::ODFs::ScoutISDF, state.units.cinematicUnits, 1);
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::ScoutISDF, state.data.enemyTeam, "cin_scout2"),
                                  GameConfig::ODFs::ScoutISDF, state.units.cinematicUnits, 2);
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam, "cin_tank1"),
                                  GameConfig::ODFs::TankISDF, state.units.cinematicUnits, 3);
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam, "cin_tank2"),
                                  GameConfig::ODFs::TankISDF, state.units.cinematicUnits, 4);
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::TankISDF, state.data.enemyTeam, "cin_tank3"),
                                  GameConfig::ODFs::TankISDF, state.units.cinematicUnits, 5);
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::MortarBikeISDF, state.data.enemyTeam, "cin_mbike1"),
                                  GameConfig::ODFs::MortarBikeISDF, state.units.cinematicUnits, 6);
            Helpers::TryTrackUnit(BuildObject(GameConfig::ODFs::RocketTankISDF, state.data.enemyTeam, "cin_rckt1"),
                                  GameConfig::ODFs::RocketTankISDF, state.units.cinematicUnits, 7);

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

        state.currentPhase = Phase::FirstObjective;
    }
}

void Scion03::RunFirstObjective()
{
    if (!state.flags.isFirstObjectiveDelaySet)
    {
        // Done here to prevent betty spam of allied units being attacked during the initial cutscene.
        for (int i = 2; i <= 5; i++)
        {
            Ally(i, state.data.hostTeam);
        }

        SetScrap(state.data.hostTeam, 40);
        SetScrap(state.data.alliedTeam, 40);
        SetScrap(state.data.enemyTeam, 40);
        SetPlan(AIPs::MansonFirstAIP, state.data.alliedTeam);

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
            state.flags.isBraddockPlayerAttacksActive = true;
            state.flags.isBraddockAANAttacksActive = true;

            Goto(state.units.braddockWalker.handle, "braddock_walker_attack");
            Goto(state.units.braddockAssaultTank.handle, "braddock_walker_attack");

            // Setup AIP here for extra spice against the player so we're not completely relying on off-map spawns.
            SetPlan(AIPs::BraddockFirstAIP, state.data.enemyTeam);

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
        state.timers.voiceTimer.Start(braddock_first_delay[state.data.missionDifficulty], state.data.missionTurn);
    }

    if (state.data.braddockAttackPlayerCounter >= 13)
    {
        state.flags.isBridgeObjectiveActive = true;
        state.timers.missionDelayTimer.Start(70.0f, state.data.missionTurn);
        state.currentPhase = Phase::BridgeObjective;
    }
}

void Scion03::RunBridgeObjective()
{
    if (!state.timers.missionDelayTimer.IsExpired(state.data.missionTurn))
    {
        return;
    }

    if (!state.flags.hasBridgeObjectiveVoicePlayed)
    {
        state.units.generalVO = state.subtitles.AudioWithSubtitles(AudioFiles::BridgeObjectiveVO,
                                                                   GameConfig::SubtitlePanelSize::Small);
        state.flags.isBridgeObjectiveActive = true;
        return;
    }


    if (Helpers::IsAudioMessageFinished(state.units.generalVO, 17, state.data.missionTurn,
                                        state.flags.isCooperative) && !state.flags.isBridgeObjectiveSet)
    {
        Helpers::AddObjectiveOverride(Objectives::BridgeObjectiveOTF, WHITE, 10.0f, true, state.flags.isCooperative);

        const Handle bridge_interest = state.units.bridgeSegements[1];

        if (bridge_interest != 0)
        {
            SetObjectiveName(bridge_interest, "ISDF Bridge");
            SetObjectiveOn(bridge_interest);
        }

        state.flags.isBridgeObjectiveSet = true;
    }
}

DLLBase* BuildMission()
{
    return new Scion03();
}
