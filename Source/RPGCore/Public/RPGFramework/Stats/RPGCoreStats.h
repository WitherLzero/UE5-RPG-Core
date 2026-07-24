#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("RPGCore"),STATGROUP_RPGCore,STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("PlayerTick"),      STAT_PlayerTick,         STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("CursorTrace"),     STAT_CursorTrace,        STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("TargetIndicatorTick"), STAT_TargetIndicatorTick, STATGROUP_RPGCore);

// Phase 2 - GAS core
DECLARE_CYCLE_STAT(TEXT("AbilityInputTagHeld"),    STAT_AbilityInputTagHeld,    STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("AbilityInputTagPressed"), STAT_AbilityInputTagPressed, STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("AbilityInputTagReleased"),STAT_AbilityInputTagReleased,STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("ProcessInputTag"),        STAT_ProcessInputTag,        STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("SetEffectProperties"),    STAT_SetEffectProperties,    STATGROUP_RPGCore);

// Phase 3 - Projectiles & attributes
DECLARE_CYCLE_STAT(TEXT("ProjectileOverlap"),   STAT_ProjectileOverlap,   STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("HandleOnHit"),          STAT_HandleOnHit,          STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("OnHomingTrackerTick"),  STAT_OnHomingTrackerTick,  STATGROUP_RPGCore);
DECLARE_CYCLE_STAT(TEXT("VitalPostGEExecute"),   STAT_VitalPostGEExecute,   STATGROUP_RPGCore);