#pragma once
#include <functional>
#include <CTheScripts.h>
#include <CStats.h>

static constexpr int INTERIOR_CLUCKIN_BELL = 9;

static constexpr int CJ_VOMITS_FOR_MENACE        = 28;
static constexpr int FLAG_INTRO_MISSION_COUNTER   = 448;
static constexpr int FLAG_SWEET_MISSION_COUNTER   = 452;
static constexpr int FLAG_RYDER_MISSION_COUNTER   = 453;
static constexpr int FLAG_SMOKE_MISSION_COUNTER   = 454;
static constexpr int FLAG_STRAP_MISSION_COUNTER   = 455;
static constexpr int FLAG_CRASH_MISSION_COUNTER   = 456;
static constexpr int FLAG_CESAR_MISSION_COUNTER   = 457;
static constexpr int CAT_COUNTER                  = 64;
static constexpr int FLAG_CAT_MISSION1_PASSED     = 714;
static constexpr int FLAG_CAT_MISSION2_PASSED     = 715;
static constexpr int FLAG_CAT_MISSION3_PASSED     = 716;
static constexpr int FLAG_CAT_MISSION4_PASSED     = 717;
static constexpr int FLAG_LA1FIN1_MISSION_COUNTER = 458;
static constexpr int FLAG_TRUTH_MISSION_COUNTER   = 491;
static constexpr int FLAG_BCESAR_MISSION_COUNTER  = 492;
static constexpr int FLAG_BCRASH_MISSION_COUNTER  = 493;
static constexpr int FLAG_GARAGE_MISSION_COUNTER  = 541;
static constexpr int FLAG_ZERO_MISSION_COUNTER    = 542;
static constexpr int FLAG_WUZI_MISSION_COUNTER    = 543;
static constexpr int FLAG_STEAL_MISSION_COUNTER   = 544;
static constexpr int FLAG_SYND_MISSION_COUNTER    = 545;
static constexpr int FLAG_SCRASH_MISSION_COUNTER  = 546;
static constexpr int FLAG_DESERT_MISSION_COUNTER  = 593;
static constexpr int FLAG_CASINO_MISSION_COUNTER  = 597;
static constexpr int FLAG_VCRASH_MISSION_COUNTER  = 598;
static constexpr int FLAG_DOC_MISSION_COUNTER     = 599;
static constexpr int FLAG_HEIST_MISSION_COUNTER   = 600;
static constexpr int FLAG_MANSION_MISSION_COUNTER = 626;
static constexpr int FLAG_GROVE_MISSION_COUNTER   = 627;
static constexpr int FLAG_RIOT_MISSION_COUNTER    = 629;
static constexpr int FLAG_CHANGED_HAIR_INTRO2     = 676;
static constexpr int BOAT_PASSED_ONCE             = 1969;
static constexpr int FLAG_BIKESCHOOL_PASSED       = 2201;
static constexpr int F1_BRONZE_AWARD              = 7417;
static constexpr int PROP_SAVE_HOUSE_BLIP_BASE    = 1621; // prop_save_house_blip[0]; set != 0 when property is purchased
static constexpr int DONE_AMBULANCE_PROGRESS      = 1487;
static constexpr int DONE_COPCAR_PROGRESS         = 1488;
static constexpr int DONE_FIRETRUCK_PROGRESS      = 1489;
static constexpr int DONE_BURGLARY_PROGRESS       = 1490;
static constexpr int DONE_TAXIODD_PROGRESS        = 1491;
static constexpr int RETURNED_OYSTERS_FLAG        = 1516;
static constexpr int FLAG_RETURNED_SHOEHORSES2    = 1517;
static constexpr int FLAG_RETURNED_SNAPSHOTS2     = 1518;
static constexpr int FLAG_RETURNED_TAGS2          = 1519;
static constexpr int DONE_TRUCK_PROGRESS          = 1492;
static constexpr int DONE_QUARRY_PROGRESS         = 1493;
static constexpr int PLAYER_IN_OTB_FLAG           = 1789;
static constexpr int LOWRIDER_PSCORE              = 1018;
static constexpr int CPRACE_BEST_TIMES_0          = 2240;
static constexpr int CS1_RACE_IS_GO               = 2336;
static constexpr int FLAG_PHONE_DIALOG_DRIV3       = 7321;

template <typename T>
static T ReadAddr(uintptr_t addr) { return *reinterpret_cast<T*>(addr); }

static int ScmGlobal(int idx) {
    return *reinterpret_cast<int*>(CTheScripts::ScriptSpace + idx * 4);
}

static auto Flag(int var, int min = 1) -> std::function<bool()> {
    return [=] { return ScmGlobal(var) >= min; };
}

static auto FlagBefore(int var, int min = 1) -> std::function<bool()> {
    return [=] { return ScmGlobal(FLAG_INTRO_MISSION_COUNTER) == 0 && ScmGlobal(var) >= min; };
}

static auto Stat(int id, float min) -> std::function<bool()> {
    return [=] { return CStats::GetStatValue(static_cast<unsigned short>(id)) >= min; };
}

static auto StatBefore(int id, float min) -> std::function<bool()> {
    return [=] { return ScmGlobal(FLAG_INTRO_MISSION_COUNTER) == 0 && CStats::GetStatValue(static_cast<unsigned short>(id)) >= min; };
}
