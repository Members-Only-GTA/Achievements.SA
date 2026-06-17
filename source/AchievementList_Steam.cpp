#ifdef ACH_SET_STEAM
#include <plugin.h>
#include <CWanted.h>
#include <CGangWars.h>
#include <CPlayerPed.h>
#include <CPlayerInfo.h>
#include <CGame.h>
#include "Achievement.h"
#include "AchievementUtils.h"

using namespace plugin;

bool IsAchievementUnlocked(int i);

void ResetAchievementState() {}

const Achievement kAch[kAchCount] = {
    { "Getting Started",
      R"(Complete "Big Smoke".)",
      "ACH01", Flag(FLAG_INTRO_MISSION_COUNTER) },

    { "Pay 'n' Spray",
      "Use a Pay 'n' Spray with wanted level.",
      "ACH08", [] {
          static float s_prevResprays = -1.0f;
          static unsigned int s_prevWanted = 0;
          float cur = CStats::GetStatValue(STAT_VEHICLE_RESPRAYS);
          unsigned int wanted = FindPlayerWanted()->m_nWantedLevel;
          bool hit = s_prevResprays >= 0.0f && cur > s_prevResprays && s_prevWanted > 0;
          s_prevResprays = cur;
          s_prevWanted   = wanted;
          return hit;
      } },

    { "All Dressed Up for San Fierro",
      R"(Complete "The Green Sabre".)",
      "ACH23", Flag(FLAG_LA1FIN1_MISSION_COUNTER, 2) },

    { "The American Dream",
      "Purchase any house.",
      "ACH07", Stat(STAT_PROPERTY_BUDGET, 1.0f) },

    { "School's Out",
      "Fully complete a vehicle school.",
      "ACH13", [] {
          return ScmGlobal(F1_BRONZE_AWARD)        >= 1
              || ScmGlobal(FLAG_BIKESCHOOL_PASSED) >= 1
              || ScmGlobal(BOAT_PASSED_ONCE)       >= 1;
      } },

    { "Smooth Moves",
      "Perform a perfect dance routine.",
      "ACH16", [] { return false; } },

    { "What happens in Las Venturas...",
      R"(Complete "Yay Ka-Boom-Boom".)",
      "ACH22", Flag(FLAG_SYND_MISSION_COUNTER, 10) },

    { "Today Was a Good Day",
      "Go 24 hours (in game) without breaking the law, being wanted, killing or harming anyone.",
      "ACH29", [] { return false; } },

    { "Horror of the Santa Maria",
      "Drown.",
      "ACH14", Stat(STAT_TIMES_DROWNED, 1.0f) },

    { "Original Gangster",
      "Reach maximum respect.",
      "ACH28", Stat(STAT_RESPECT, 1000.0f) },

    { "Liberty City State of Mind",
      R"(Complete the "Saint Mark's Bistro" mission.)",
      "ACH11", Flag(FLAG_CASINO_MISSION_COUNTER, 9) },

    { "With Extra Dip",
      "Buy 8 meals from Cluckin' Bell throughout the game.",
      "ACH02", [] {
          constexpr int kCountGlobal = kGlobalBase + (kAchCount + 31) / 32;
          int& count = *reinterpret_cast<int*>(CTheScripts::ScriptSpace + kCountGlobal * 4);
          if (count >= 8) return true;

          static float s_prevMeals = -1.0f;
          float curMeals = CStats::GetStatValue(STAT_NUMBER_OF_MEALS_EATEN);
          if (s_prevMeals >= 0.0f && curMeals > s_prevMeals && CGame::currArea == INTERIOR_CLUCKIN_BELL)
              ++count;
          s_prevMeals = curMeals;
          return count >= 8;
      } },

    { "The End of the Line",
      R"(Complete "End of the Line".)",
      "ACH03", Flag(FLAG_RIOT_MISSION_COUNTER, 5) },

    { "Saviour",
      "Complete 12 levels of Paramedic.",
      "ACH18", Flag(DONE_AMBULANCE_PROGRESS) },

    { "Rescue a Kitten Too?",
      "Complete 12 levels of Firefighter.",
      "ACH19", Flag(DONE_FIRETRUCK_PROGRESS) },

    { "What are the Odds",
      "Win a race in Inside Track Betting.",
      "ACH25", [] {
          static float s_prevWon = -1.0f;
          static int   s_phase   = 0; // 0=idle, 1=in OTB, 2=grace poll after OTB ends

          bool inOtb = CGame::currArea == INTERIOR_OTB;
          if (inOtb)             s_phase = 1;
          else if (s_phase == 1) s_phase = 2;

          float curWon = CStats::GetStatValue(STAT_MONEY_WON_GAMBLING);
          bool hit = s_prevWon >= 0.0f && curWon > s_prevWon && s_phase > 0;
          s_prevWon = curWon;
          if (s_phase == 2) s_phase = 0;
          return hit;
      } },

    { "I'll Have Two Number 9s",
      "Reach maximum weight.",
      "ACH06", Stat(STAT_FAT, 1000.0f) },

    { "Public Enemy No. 1",
      "Reach 6 wanted stars.",
      "ACH30", [] { return FindPlayerWanted()->m_nWantedLevel >= 6; } },

    { "Double or Nothin'",
      "Put all your money or the maximum bet on red or black and win.",
      "ACH26", [] {
          static float s_prevWon  = -1.0f;
          static int   s_prevFlag = 0;
          static bool  s_validBet = false;

          int flag  = ScmGlobal(3505);
          int red   = ScmGlobal(8536);
          int black = ScmGlobal(8537);
          int stake = ScmGlobal(8551);

          if (s_prevFlag == 0 && flag == 1 && stake > 0) {
              bool redOrBlackOnly = (red + black == stake);
              int money = FindPlayerPed()->GetPlayerInfoForThisPlayerPed()->m_nMoney;
              bool allMoneyOrMax = money == 0
                  || stake == 100    || stake == 1000   || stake == 10000
                  || stake == 100000 || stake == 1000000;
              s_validBet = redOrBlackOnly && allMoneyOrMax;
          }
          s_prevFlag = flag;

          float curWon = CStats::GetStatValue(STAT_MONEY_WON_GAMBLING);
          bool hit = s_prevWon >= 0.0f && curWon > s_prevWon && s_validBet;
          if (hit) s_validBet = false;
          s_prevWon = curWon;
          return hit;
      } },

    { "Swiss Army Strife",
      "Max all weapon skills.",
      "ACH12", [] {
          return CStats::GetStatValue(STAT_PISTOL_SKILL)           >= 1000.0f
              && CStats::GetStatValue(STAT_SILENCED_PISTOL_SKILL)  >= 1000.0f
              && CStats::GetStatValue(STAT_DESERT_EAGLE_SKILL)     >= 1000.0f
              && CStats::GetStatValue(STAT_SHOTGUN_SKILL)          >= 1000.0f
              && CStats::GetStatValue(STAT_SAWN_OFF_SHOTGUN_SKILL) >= 1000.0f
              && CStats::GetStatValue(STAT_COMBAT_SHOTGUN_SKILL)   >= 1000.0f
              && CStats::GetStatValue(STAT_MACHINE_PISTOL_SKILL)   >= 1000.0f
              && CStats::GetStatValue(STAT_SMG_SKILL)              >= 1000.0f
              && CStats::GetStatValue(STAT_AK_47_SKILL)            >= 1000.0f
              && CStats::GetStatValue(STAT_M4_SKILL)               >= 1000.0f; } },

    { "What the City Needs",
      "Complete 12 levels of Vigilante.",
      "ACH17", Flag(DONE_COPCAR_PROGRESS) },

    { "Bike or Biker",
      "Complete BMX or NRG challenge.",
      "ACH09", [] {
          return CStats::GetStatValue(STAT_BMX_BEST_TIME)     >= 1.0f
              || CStats::GetStatValue(STAT_NRG_500_BEST_TIME) >= 1.0f; } },

    { "Yes I Speak English",
      "Reach 50 fares in Taxi Mode.",
      "ACH20", Flag(DONE_TAXIODD_PROGRESS) },

    { "Who Needs Directions?",
      R"(Find Mike Toreno without any of the referenced locations during "Mike Toreno".)",
      "ACH10", [] {
          static int  s_prev            = 0;
          static bool s_locationVisited = false;

          int flag = ScmGlobal(FLAG_PHONE_DIALOG_DRIV3);

          if (s_prev != 0 && flag == 0)  // new mission attempt
              s_locationVisited = false;

          if (flag >= 8)  // player entered Doherty zone
              s_locationVisited = true;

          bool hit = !s_locationVisited && flag >= 24 && s_prev < 24;
          s_prev = flag;
          return hit;
      } },

    { "Assassin",
      R"(Stealth kill all enemies in the mission "Madd Dogg's Rhymes".)",
      "ACH27", [] { return false; } },

    { "Chick Magnet",
      "Achieve maximum sex appeal.",
      "ACH31", Stat(STAT_SEX_APPEAL, 1000.0f) },

    { "They Can't Stop All of Us",
      R"(Sneak into the underground base without setting off the alarm above ground during "Black Project".)",
      "ACH15", [] { return false; } },

    { "Ain't Nothing But a G Thing",
      "Own all gang warfare turfs, properties and have $1,000,000.",
      "ACH32", [] {
          if (CGangWars::TerritoryUnderControlPercentage < 1.0f) return false;
          if (FindPlayerPed()->GetPlayerInfoForThisPlayerPed()->m_nMoney < 1000000) return false;
          // TODO: check all 9 purchasable properties once indices are confirmed
          // for (int i : {?, ?, ?})
          //     if (ScmGlobal(PROP_SAVE_HOUSE_BLIP_BASE + i) == 0) return false;
          return true;
      } },

    { "Lucky Spinner",
      "Win at least $1,000 in a single spin of the Wheel of Fortune.",
      "ACH21", [] { return false; } },

    { "Hoopin' it Up",
      "Score at least 30 points in the basketball mini-game.",
      "ACH04", Stat(STAT_HIGHEST_BASKETBALL_SCORE, 30.0f) },

    { "A Legitimate Business",
      "Export all three car lists.",
      "ACH05", Stat(STAT_NUMBER_OF_VEHICLES_EXPORTED, 30.0f) },

    { "Not a Player",
      "Go on at least one date with every potential girlfriend.",
      "ACH24", [] {
          return CStats::GetStatValue(STAT_PROGRESS_WITH_DENISE)   >= 1.0f
              && CStats::GetStatValue(STAT_PROGRESS_WITH_MICHELLE) >= 1.0f
              && CStats::GetStatValue(STAT_PROGRESS_WITH_HELENA)   >= 1.0f
              && CStats::GetStatValue(STAT_PROGRESS_WITH_BARBARA)  >= 1.0f
              && CStats::GetStatValue(STAT_PROGRESS_WITH_KATIE)    >= 1.0f
              && CStats::GetStatValue(STAT_PROGRESS_WITH_MILLIE)   >= 1.0f; } },

    { "Remastered",
      "Earn 100% completion.",
      "ACH33", [] { return CStats::GetPercentageProgress() >= 100.0f; } },

    //{ "...Here we go again",
    //  "Start a new game after getting 100% on a save file.",
    //  "ACH34", [] { return false; } },

    { "I Ain't No Buster",
      "Unlock all achievements.",
      "ACH35", [] {
          for (int i = 0; i < kAchCount - 1; ++i)
              if (!IsAchievementUnlocked(i)) return false;
          return true; } },
};

static_assert(std::size(kAch) == kAchCount, "kAchCount mismatch");

#endif // ACH_SET_STEAM
