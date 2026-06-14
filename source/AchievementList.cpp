#include <plugin.h>
#include <CTheScripts.h>
#include <CPlayerPed.h>
#include <CPlayerData.h>
#include <CPedClothesDesc.h>
#include <CKeyGen.h>
#include <eClothesTexturePart.h>
#include <CStats.h>
#include "Achievement.h"

using namespace plugin;

static constexpr int CJ_VOMITS_FOR_MENACE       = 28;
static constexpr int FLAG_INTRO_MISSION_COUNTER  = 448;
static constexpr int FLAG_SWEET_MISSION_COUNTER  = 452;
static constexpr int FLAG_RYDER_MISSION_COUNTER  = 453;
static constexpr int FLAG_SMOKE_MISSION_COUNTER  = 454;
static constexpr int FLAG_STRAP_MISSION_COUNTER  = 455;
static constexpr int FLAG_CRASH_MISSION_COUNTER  = 456;
static constexpr int FLAG_CESAR_MISSION_COUNTER  = 457;
static constexpr int CAT_COUNTER                 = 64;
static constexpr int FLAG_CAT_MISSION1_PASSED    = 714;
static constexpr int FLAG_CAT_MISSION2_PASSED    = 715;
static constexpr int FLAG_CAT_MISSION3_PASSED    = 716;
static constexpr int FLAG_CAT_MISSION4_PASSED    = 717;
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
static constexpr int FLAG_CHANGED_HAIR_INTRO2    = 676;
static constexpr int DONE_AMBULANCE_PROGRESS     = 1487;
static constexpr int DONE_COPCAR_PROGRESS        = 1488;
static constexpr int DONE_FIRETRUCK_PROGRESS     = 1489;
static constexpr int DONE_BURGLARY_PROGRESS      = 1490;
static constexpr int DONE_TAXIODD_PROGRESS       = 1491;
static constexpr int RETURNED_OYSTERS_FLAG       = 1516;
static constexpr int FLAG_RETURNED_SHOEHORSES2   = 1517;
static constexpr int FLAG_RETURNED_SNAPSHOTS2    = 1518;
static constexpr int FLAG_RETURNED_TAGS2         = 1519;
static constexpr int DONE_TRUCK_PROGRESS         = 1492;
static constexpr int DONE_QUARRY_PROGRESS        = 1493;
static constexpr int LOWRIDER_PSCORE             = 1018;
static constexpr int CPRACE_BEST_TIMES_0         = 2240;
static constexpr int CS1_RACE_IS_GO              = 2336;

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

static int  s_hcPrev    = -1;
static bool s_hcLatched = false;

void ResetAchievementState() {
    s_hcPrev    = -1;
    s_hcLatched = false;
}

const Achievement kAch[kAchCount] = {

    { "The Gang's All Here",
      R"(Clear the mission "Sweet & Kendl" [Cheats block all achievements])",
      "243339", Flag(FLAG_INTRO_MISSION_COUNTER) },

    { "Respect Plus",
      R"(During the mission "Ryder", earn Ryder's respect with your new haircut)",
      "317359",
      [] {
          if (ScmGlobal(FLAG_CHANGED_HAIR_INTRO2) < 1) return false;
          auto* ped = FindPlayerPed();
          if (!ped || !ped->m_pPlayerData || !ped->m_pPlayerData->m_pPedClothesDesc) return false;
          const unsigned int headKey = ped->m_pPlayerData->m_pPedClothesDesc->m_anTextureKeys[CLOTHES_TEXTURE_HEAD];
          static const unsigned int badCuts[] = {
              CKeyGen::GetUppercaseKey("tash"),
              CKeyGen::GetUppercaseKey("goatee"),
              CKeyGen::GetUppercaseKey("beard"),
              CKeyGen::GetUppercaseKey("player_face"),
              CKeyGen::GetUppercaseKey("highfade"),
              CKeyGen::GetUppercaseKey("flattop"),
          };
          for (auto k : badCuts)
              if (headKey == k) return false;
          return true;
      }},

    { "Sick Snatch",
      R"(During the mission "Ryder", distract the cashier for the robbery with some reverse bowel movement)",
      "321532", [] { return ScmGlobal(CJ_VOMITS_FOR_MENACE) == 1; } },

    { "Ryder, Censored!",
      R"(Clear the mission "Ryder")",
      "243340", Flag(FLAG_INTRO_MISSION_COUNTER, 2) },

    { "Leaving A Mark",
      R"(Clear the mission "Tagging Up Turf")",
      "243341", Flag(FLAG_SWEET_MISSION_COUNTER, 1) },

    { "Cleanup Crew",
      R"(Clear the mission "Cleaning The Hood")",
      "243342", Flag(FLAG_SWEET_MISSION_COUNTER, 2) },

    { "I'll Have Two Number Nine's",
      R"(Clear the mission "Drive-Thru")",
      "243343", Flag(FLAG_SWEET_MISSION_COUNTER, 3) },

    { "Tampa's Demise",
      R"(Clear the mission "Nines and AK's")",
      "243344", Flag(FLAG_SWEET_MISSION_COUNTER, 4) },

    { "Gotta Go Fast",
      R"(Clear the mission "Drive-By")",
      "243345", Flag(FLAG_SWEET_MISSION_COUNTER, 5) },

    { "Bros Before Hoes",
      R"(Clear the mission "Sweet's Girl")",
      "243346", Flag(FLAG_SWEET_MISSION_COUNTER, 6) },

    { "Hydraulic Hustle",
      R"(Clear the mission "Cesar Vialpando" by obtaining a score of at least 3.000 in the hopping minigame)",
      "317360", Flag(LOWRIDER_PSCORE, 3000) },

    { "Latin America",
      R"(Clear the mission "Cesar Vialpando")",
      "243347", Flag(FLAG_SWEET_MISSION_COUNTER, 7) },

    { "Keep Up, Holmes!",
      R"(During the mission "High Stakes, Low-Rider", finish the race in under 1:20)",
      "243348", [] { return ScmGlobal(CS1_RACE_IS_GO) >= 2 && ScmGlobal(CPRACE_BEST_TIMES_0) <= 80; } },

    { "Pink Slip",
      R"(Clear the mission "High Stakes, Low-Rider")",
      "321534", Flag(FLAG_CESAR_MISSION_COUNTER) },

    { "House Cleaning",
      R"(Steal all six crates during the mission "Home Invasion" without ever waking up Fuhrberger and complete the mission)",
      "321805",
      [] {
          if (s_hcLatched) return true;
          const int counter = ScmGlobal(FLAG_RYDER_MISSION_COUNTER);
          if (s_hcPrev != -1 && counter > s_hcPrev) {
              const int boxesLeft  = ReadAddr<int>(0x00A48A1C);
              const int ownerAwoke = ReadAddr<int>(0x00A48AE0);
              if (boxesLeft == 0 && ownerAwoke == 0)
                  s_hcLatched = true;
          }
          s_hcPrev = counter;
          return s_hcLatched;
      }},

    { "Guerrilla Warfare",
      R"(Kill Fuhrberger before collecting any crates during the mission "Home Invasion")",
      "321804", [] { return false; } },

    { "Crate Carrier",
      R"(Clear the mission "Home Invasion")",
      "243349", Flag(FLAG_RYDER_MISSION_COUNTER) },

    { "Railroad Riot",
      R"(Clear the mission "Catalyst")",
      "243350", Flag(FLAG_RYDER_MISSION_COUNTER, 2) },

    { "Military Mockery",
      R"(Clear the mission "Robbing Uncle Sam")",
      "243351", Flag(FLAG_RYDER_MISSION_COUNTER, 3) },

    { "Locked-Up Romance",
      R"(Kill Freddy before reaching the place where he says "Yoohoo! Over Here!" (At the end of the first highway portion))",
      "317361", [] { return false; } },

    { "Chase Me, Chase Me!",
      R"(Clear the mission "OG Loc")",
      "243352", Flag(FLAG_SMOKE_MISSION_COUNTER, 1) },

    { "Big Boned But Still an Athlete",
      R"(Receive weight loss advice from Big Smoke)",
      "318216", [] { return false; } },

    { "Distant Cousin",
      R"(Clear the mission "Running Dog")",
      "243353", Flag(FLAG_SMOKE_MISSION_COUNTER, 2) },

    { "Turbulent Train Heist",
      R"(Clear the mission "Wrong Side Of The Tracks" without entering Las Colinas)",
      "243354", [] { return false; } },

    { "All You Had To Do, Was Follow The Damn Train CJ!",
      R"(Clear the mission "Wrong Side Of The Tracks")",
      "321811", Flag(FLAG_SMOKE_MISSION_COUNTER, 3) },

    { "Reaching The Holy Gate",
      R"(Clear the mission "Just Business")",
      "243355", Flag(FLAG_SMOKE_MISSION_COUNTER, 4) },

    { "Dancing Queen",
      R"(Clear the mission "Life's A Beach")",
      "243356", Flag(FLAG_STRAP_MISSION_COUNTER, 1) },

    { "Into The Shadows",
      R"(Clear the mission "Madd Dogg's Rhymes" while taking no damage and killing a maximum of 3 guards in the mansion)",
      "317362", [] { return false; } },

    { "Sneaky Gangster",
      R"(Clear the mission "Madd Dogg's Rhymes")",
      "243357", Flag(FLAG_STRAP_MISSION_COUNTER, 2) },

    { "Aquaphobia",
      R"(Clear the mission "Management Issues")",
      "243358", Flag(FLAG_STRAP_MISSION_COUNTER, 3) },

    { "Hold The Line",
      R"(Clear the mission "House Party")",
      "243359", Flag(FLAG_STRAP_MISSION_COUNTER, 5) },

    { "Perfect Aim",
      R"(In "Burning Desire", destroy all the windows with only 5 molotov cocktails)",
      "317363", [] { return false; } },

    { "Molotov Master",
      R"(Clear the mission "Burning Desire")",
      "243360", Flag(FLAG_CRASH_MISSION_COUNTER, 1) },

    { "Tax Evasion",
      R"(During the mission "Gray Imports" kill the arms dealer before they reach their car)",
      "321830", [] { return false; } },

    { "Russian Mob",
      R"(Clear the mission "Gray Imports")",
      "243361", Flag(FLAG_CRASH_MISSION_COUNTER, 2) },

    { "Extended Tutorial",
      R"(Clear the mission "Doberman")",
      "243362", Flag(FLAG_SWEET_MISSION_COUNTER, 8) },

    { "Funeral Free-For-All",
      R"(Clear the mission "Los Sepulcros")",
      "243363", Flag(FLAG_SWEET_MISSION_COUNTER, 9) },

    { "A Taste of (Things to) Come",
      R"(Clear the mission "Reuniting The Families")",
      "243364", Flag(FLAG_LA1FIN1_MISSION_COUNTER, 1) },

    { "Entrepreneur",
      R"(Make your way to Doherty in San Fierro before completing "The Green Sabre")",
      "317418", [] { return false; } },

    { "Backstab",
      R"(Clear the mission "The Green Sabre")",
      "243365", Flag(FLAG_LA1FIN1_MISSION_COUNTER, 2) },

    { "Sucker Punch The Snitch",
      R"(Clear the mission "Badlands" without the snitch reaching his car and trying to escape)",
      "317365", [] { return false; } },

    { "FBI, Open Up!",
      R"(Clear the mission "Badlands")",
      "243366", Flag(FLAG_BCRASH_MISSION_COUNTER, 1) },

    { "Suicide Squad",
      R"(Clear the mission "Tanker Commander")",
      "243367", Flag(FLAG_CAT_MISSION3_PASSED) },

    { "Bet On Green",
      R"(Clear the mission "Against All Odds")",
      "243368", Flag(FLAG_CAT_MISSION4_PASSED) },

    { "Moonshine",
      R"(Clear the mission "Local Liquor Store")",
      "243369", Flag(FLAG_CAT_MISSION1_PASSED) },

    { "Alley Shooting",
      R"(Clear the mission "Small Town Bank")",
      "243370", Flag(FLAG_CAT_MISSION2_PASSED) },

    { "N64 Cameo",
      R"(Clear the mission "Body Harvest")",
      "243371", Flag(FLAG_TRUTH_MISSION_COUNTER, 1) },

    { "Buffalo Wings",
      R"(Clear the mission "Wu Zi Mu")",
      "243372", Flag(FLAG_BCESAR_MISSION_COUNTER, 5) },

    { "Mirror Mode",
      R"(Clear the mission "Farewell My Love")",
      "243373", Flag(FLAG_BCESAR_MISSION_COUNTER, 10) },

    { "Because I Got High",
      R"(During the mission "Are You Going To San Fierro?" clear the field enough to open up the Mothership within 50 seconds)",
      "317366", [] { return false; } },

    { "High As A Kite",
      R"(Clear the mission "Are You Going To San Fierro?")",
      "243374", Flag(FLAG_TRUTH_MISSION_COUNTER, 2) },

    { "Gang Gathering",
      R"(Clear the mission "Wear Flowers In Your Hair")",
      "243375", Flag(FLAG_GARAGE_MISSION_COUNTER, 1) },

    { "Spotless",
      R"(Clear the mission "555 WE TIP" without having to return to the garage due to damaging the D.A.'s car)",
      "317367", [] { return false; } },

    { "White As A Snowflake",
      R"(Clear the mission "555 WE TIP")",
      "243376", Flag(FLAG_SCRASH_MISSION_COUNTER, 1) },

    { "A Concrete Brown Mess",
      R"(Clear the mission "Deconstruction")",
      "243377", Flag(FLAG_GARAGE_MISSION_COUNTER, 2) },

    { "Up And Above",
      R"(Clear the mission "Air Raid")",
      "243378", Flag(FLAG_ZERO_MISSION_COUNTER, 1) },

    { "Infamous",
      R"(Clear the mission "Supply Lines")",
      "243379", Flag(FLAG_ZERO_MISSION_COUNTER, 2) },

    { "Berkley's Demise",
      R"(Clear the mission "New Model Army")",
      "243380", Flag(FLAG_ZERO_MISSION_COUNTER, 3) },

    { "Look Who It Is",
      R"(Clear the mission "Photo Opportunity")",
      "243381", Flag(FLAG_SYND_MISSION_COUNTER, 1) },

    { "In My Pants",
      R"(Clear the mission "Jizzy")",
      "243382", Flag(FLAG_SYND_MISSION_COUNTER, 2) },

    { "Special Delivery",
      R"(Clear the mission "T-Bone Mendez")",
      "243383", Flag(FLAG_SYND_MISSION_COUNTER, 4) },

    { "Kidnapped!",
      R"(Clear the mission "Mike Toreno")",
      "243384", Flag(FLAG_SYND_MISSION_COUNTER, 5) },

    { "Roadwork Ahead? I Sure Hope It Does",
      R"(Clear the mission "Outrider")",
      "243385", Flag(FLAG_SYND_MISSION_COUNTER, 6) },

    { "All You Had To Do... Part II",
      R"(Clear the mission "Snail Trail")",
      "243386", Flag(FLAG_SCRASH_MISSION_COUNTER, 2) },

    { "Snowball Cocktail",
      R"(Clear the mission "Ice Cold Killa")",
      "243387", Flag(FLAG_SYND_MISSION_COUNTER, 7) },

    { "Sweet Revenge",
      R"(Clear the mission "Pier 69")",
      "243388", Flag(FLAG_SYND_MISSION_COUNTER, 8) },

    { "Another One",
      R"(Clear the mission "Toreno's Last Flight")",
      "243389", Flag(FLAG_SYND_MISSION_COUNTER, 9) },

    { "Something Is Off",
      R"(Clear the mission "Mountain Cloud Boys")",
      "243390", Flag(FLAG_WUZI_MISSION_COUNTER, 1) },

    { "Mana Mana, Tudu Dududu",
      R"(Clear the mission "Ran Fa Li")",
      "243391", Flag(FLAG_WUZI_MISSION_COUNTER, 2) },

    { "Baited & Outsmarted",
      R"(Clear the mission "Lure")",
      "243392", Flag(FLAG_WUZI_MISSION_COUNTER, 3) },

    { "Air For Days",
      R"(Clear the mission "Amphibious Assault")",
      "243393", Flag(FLAG_WUZI_MISSION_COUNTER, 4) },

    { "Snakehead",
      R"(Clear the mission "The Da Nang Thang")",
      "243394", Flag(FLAG_WUZI_MISSION_COUNTER, 5) },

    { "Coughio Up El Weedo",
      R"(Knock off and grab 6 crates from the Patriot during Big Smoke's Cash)",
      "317435", [] { return false; } },

    { "Powdered Sugar",
      R"(In Loco Syndicate Drug Courier, kill the courier while he still has at least $1,800 worth of coke)",
      "317434", [] { return false; } },

    { "Hunted Down",
      R"(Complete Burger Shot's courier mission before completing "Yay Ka-Boom-Boom" without your wanted level being zero)",
      "322144", [] { return false; } },

    { "End Of An Era",
      R"(Clear the mission "Yay Ka-Boom-Boom")",
      "243395", Flag(FLAG_SYND_MISSION_COUNTER, 10) },

    { "Uranus",
      R"(Clear the mission "Zeroing In")",
      "243396", Flag(FLAG_STEAL_MISSION_COUNTER, 1) },

    { "Windy Street",
      R"(Clear the mission "Test Drive")",
      "243397", Flag(FLAG_STEAL_MISSION_COUNTER, 2) },

    { "Joking Around",
      R"(Clear the mission "Customs Fast Track")",
      "243398", Flag(FLAG_STEAL_MISSION_COUNTER, 3) },

    { "Poppin' Tires",
      R"(Clear the mission "Puncture Wounds")",
      "243399", Flag(FLAG_STEAL_MISSION_COUNTER, 4) },

    { "Blaze It",
      R"(Beat the race in "Monster" with a time of under 4:20 to be told the boss was right about you)",
      "322127", [] { return false; } },

    { "Uphill Battle",
      R"(Clear the mission "Monster")",
      "243400", Flag(FLAG_DESERT_MISSION_COUNTER, 1) },

    { "With A Little Help From My Friend",
      R"(Clear the mission "Highjack")",
      "243401", Flag(FLAG_DESERT_MISSION_COUNTER, 2) },

    { "Fire Signal",
      R"(Clear the mission "Interdiction")",
      "243402", Flag(FLAG_DESERT_MISSION_COUNTER, 3) },

    { "Sweet's Wedding Present",
      R"(Hear the details from Toreno about Sweet's new cell mate)",
      "318217", [] { return false; } },

    { "Foo Fighters \xa9",
      R"(Clear the mission "Learning to Fly")",
      "317368", [] { return false; } },

    { "Learn To Fly",
      R"(Clear the mission "N.O.E.")",
      "243403", Flag(FLAG_DESERT_MISSION_COUNTER, 6) },

    { "Andromada Demolished",
      R"(Clear the mission "Stowaway")",
      "243404", Flag(FLAG_DESERT_MISSION_COUNTER, 5) },

    { "Non Consensual 69",
      R"(Complete "Black Project" without using the vent to enter or exit the underground base in Area 69)",
      "317369", [] { return false; } },

    { "Area 69",
      R"(Clear the mission "Black Project")",
      "243405", Flag(FLAG_DESERT_MISSION_COUNTER, 8) },

    { "Slimey",
      R"(Clear the mission "Green Goo")",
      "243406", Flag(FLAG_DESERT_MISSION_COUNTER, 9) },

    { "Fear Is The Greatest Weakness",
      R"(Clear the mission "Fender Ketchup")",
      "243407", Flag(FLAG_CASINO_MISSION_COUNTER, 1) },

    { "Demolition",
      R"(Clear the mission "Explosive Situation")",
      "243408", Flag(FLAG_CASINO_MISSION_COUNTER, 2) },

    { "Factory Disturbance",
      R"(Clear the mission "You've Had Your Chips")",
      "243409", Flag(FLAG_CASINO_MISSION_COUNTER, 3) },

    { "Arco Del Oeste",
      R"(Clear the mission "Don Peyote")",
      "243410", Flag(FLAG_CASINO_MISSION_COUNTER, 4) },

    { "Blueprints",
      R"(Clear the mission "Architectural Espionage")",
      "243411", Flag(FLAG_HEIST_MISSION_COUNTER, 1) },

    { "Communicating the Bad News",
      R"(Tell Woozie about your deadly accident with Millie)",
      "318215", [] { return false; } },

    { "Dildo Dominance",
      R"(Clear the mission "Key To Her Heart")",
      "243412", Flag(FLAG_HEIST_MISSION_COUNTER, 2) },

    { "Dam Quay Destruction",
      R"(Clear the mission "Dam And Blast")",
      "243413", Flag(FLAG_HEIST_MISSION_COUNTER, 3) },

    { "Tax Collection",
      R"(Clear the mission "Cop Wheels")",
      "243414", Flag(FLAG_HEIST_MISSION_COUNTER, 4) },

    { "Securicar Secured",
      R"(Clear the mission "Up, Up And Away!")",
      "243415", Flag(FLAG_HEIST_MISSION_COUNTER, 5) },

    { "They Changed The Layout!",
      R"(Complete the mission "Breaking The Bank At Caligula's" without using thermal goggles or night vision goggles)",
      "317370", [] { return false; } },

    { "It Was All Worth It",
      R"(Clear the mission "Breaking The Bank At Caligula's")",
      "243416", Flag(FLAG_HEIST_MISSION_COUNTER, 6) },

    { "Johnny Sindacco",
      R"(Clear the mission "Intensive Care")",
      "243417", Flag(FLAG_CASINO_MISSION_COUNTER, 5) },

    { "Slow Work",
      R"(Clear the mission "Misappropriation" by only killing the target after he parks and exits his Buffalo at the farthest end of the underground garage [Icon shows up when you can kill him])",
      "322153", [] { return false; } },

    { "DEA Chasedown",
      R"(Clear the mission "Misappropriation")",
      "243418", Flag(FLAG_DESERT_MISSION_COUNTER, 7) },

    { "Veganism",
      R"(Clear the mission "The Meat Business")",
      "243419", Flag(FLAG_CASINO_MISSION_COUNTER, 6) },

    { "Tom Petty",
      R"(Clear the mission "Freefall")",
      "243420", Flag(FLAG_CASINO_MISSION_COUNTER, 8) },

    { "Punk Motherfucker!",
      R"(Clear the mission "High Noon" without entering a vehicle other than Pulaski's car)",
      "317371", [] { return false; } },

    { "Another Bastard Hits The Dust",
      R"(Clear the mission "High Noon")",
      "243421", Flag(FLAG_VCRASH_MISSION_COUNTER, 2) },

    { "I Said No, No, No!",
      R"(Clear the mission "Madd Dogg" without having Madd Dogg lose health)",
      "317372", [] { return false; } },

    { "Rehab",
      R"(Clear the mission "Madd Dogg")",
      "243422", Flag(FLAG_RIOT_MISSION_COUNTER, 1) },

    { "Taking A Little Vacation",
      R"(Clear the mission "Saint Mark's Bistro")",
      "243423", Flag(FLAG_CASINO_MISSION_COUNTER, 9) },

    { "Notorious B.I.G.",
      R"(Clear the mission "A Home In The Hills")",
      "243424", Flag(FLAG_MANSION_MISSION_COUNTER, 1) },

    { "Clipped Wings",
      R"(Sneak into the ship and enter a Hydra in "Vertical Bird" without the alarm being triggered)",
      "317373", [] { return false; } },

    { "Hyper Hydra",
      R"(Clear the mission "Vertical Bird")",
      "243425", Flag(FLAG_MANSION_MISSION_COUNTER, 2) },

    { "Reunited",
      R"(Clear the mission "Home Coming")",
      "243426", Flag(FLAG_MANSION_MISSION_COUNTER, 3) },

    { "Climbing The Charts",
      R"(Clear the mission "Cut Throat Business")",
      "243427", Flag(FLAG_MANSION_MISSION_COUNTER, 4) },

    { "Saviors",
      R"(Clear the mission "Beat Down On B Dup")",
      "243428", Flag(FLAG_GROVE_MISSION_COUNTER, 1) },

    { "Guess Who's Back",
      R"(Clear the mission "Grove 4 Life")",
      "243429", Flag(FLAG_GROVE_MISSION_COUNTER, 2) },

    { "Back To The Hood",
      R"(Clear the mission "Riot")",
      "243430", Flag(FLAG_RIOT_MISSION_COUNTER, 3) },

    { "Preparing The Ring",
      R"(Clear the mission "Los Desperados")",
      "243431", Flag(FLAG_RIOT_MISSION_COUNTER, 2) },

    { "A Crack Fortress in Return for a Slightly Better Fire Truck",
      R"(Get a S.W.A.T. in "End of the Line", destroy it, but fix it by saving it in a garage. Don't get busted or wasted.)",
      "318204", [] { return false; } },

    { "Remember The Name",
      R"(During the mission "End Of The Line", kill Big Smoke)",
      "243432", [] { return false; } },

    { "Photographic Memory",
      R"(Finish the warehouse escape sequence during "End Of The Line" without using Thermal Vision)",
      "317374", [] { return false; } },

    { "Grove Street, Home",
      R"(Clear the mission "End Of The Line")",
      "243433", Flag(FLAG_RIOT_MISSION_COUNTER, 5) },

    { "El Sonidito",
      R"(Complete 50 Taxi Fares. Taxis now have Nitro!)",
      "317425", Flag(DONE_TAXIODD_PROGRESS) },

    { "Doppler Effect",
      R"(Complete all 12 Paramedic missions to increase your maximum Health)",
      "317426", Flag(DONE_AMBULANCE_PROGRESS) },

    { "Indestructible",
      R"(Complete all 12 Firefighter missions to become resistant to fire)",
      "317427", Flag(DONE_FIRETRUCK_PROGRESS) },

    { "Good Cop, Bad Cop",
      R"(Complete all 12 Vigilante missions to increase your maximum Body Armor)",
      "317428", Flag(DONE_COPCAR_PROGRESS) },

    { "Sperm Bank Diploma",
      R"(Complete 10 Pimping missions and get a special reward)",
      "317430", Stat(STAT_PIMPING_LEVEL, 10.0f) },

    { "Usain Bolt",
      R"(Steal a total worth of $10,000 in Burglar missions to obtain Infinite Sprint)",
      "317429", Flag(DONE_BURGLARY_PROGRESS) },

    { "Upcoming Graffiti Artist",
      R"(Spray 25 tags)",
      "316719", Stat(STAT_TAGS_SPRAYED, 25.0f) },

    { "Subsidised Graffiti Artist",
      R"(Spray 50 tags)",
      "317788", Stat(STAT_TAGS_SPRAYED, 50.0f) },

    { "Professional Graffiti Artist",
      R"(Spray 75 tags)",
      "317789", Stat(STAT_TAGS_SPRAYED, 75.0f) },

    { "Banksy",
      R"(Spray 100 tags)",
      "317790", Flag(FLAG_RETURNED_TAGS2) },

    { "Early Uber Build",
      R"(Complete Roboi's Food Mart's courier mission)",
      "317469", [] { return false; } },

    { "Lance Armstrong",
      R"(Max out the Cycling Skill)",
      "317391", Stat(STAT_CYCLING_SKILL, 1000.0f) },

    { "Quads For Days",
      R"(Complete the BMX Challenge)",
      "317458", Stat(STAT_BMX_BEST_TIME, 1.0f) },

    { "Bunny Hop King",
      R"(Get a best time of 1:15 or less in the BMX Challenge)",
      "322208", [] { float t = CStats::GetStatValue(STAT_BMX_BEST_TIME); return t >= 1.0f && t <= 75.0f; } },

    { "Hotring Racer",
      R"(Get 1st place in the 8-Track Stadium Event)",
      "322209", [] { return CStats::GetStatValue(STAT_BEST_POSITION_IN_8_TRACK) == 1.0f; } },

    { "Stereo 8",
      R"(Get a best time of 5:10 or less in the 8-Track Stadium Event)",
      "317471", [] { float t = CStats::GetStatValue(STAT_BEST_TIME_IN_8_TRACK); return t >= 1.0f && t <= 310.0f; } },

    { "Speed Devil",
      R"(Get a best lap time of 0:23 or less in the 8-Track Stadium Event)",
      "322210", [] { float t = CStats::GetStatValue(STAT_BEST_LAP_TIME_IN_8_TRACK); return t >= 1.0f && t <= 23.0f; } },

    { "Mount Chiliad Climber",
      R"(Beat Scotch Bonnet Yellow Route in the Chiliad Challenge)",
      "322449", [] { return false; } },

    { "As Free As A Bird",
      R"(Beat Birdseye Winder in the Chiliad Challenge)",
      "317462", [] { return false; } },

    { "Snake's Tongue",
      R"(Beat Cobra Run in the Chiliad Challenge)",
      "322450", [] { return false; } },

    { "Local Tourist",
      R"(Collect 25 Snapshots)",
      "318220", Stat(STAT_SNAPSHOTS_TAKEN, 25.0f) },

    { "World Traveler",
      R"(Collect 50 Snapshots)",
      "318222", Flag(FLAG_RETURNED_SNAPSHOTS2) },

    { "Mystery Shopper Courier",
      R"(Complete Hippy Shopper's courier mission)",
      "317467", [] { return false; } },

    { "Dry Dry Dock",
      R"(Complete the NRG-500 Challenge)",
      "317459", Stat(STAT_NRG_500_BEST_TIME, 1.0f) },

    { "Dry Dry Dock On Steroids",
      R"(Get a best time of 2:00 or less in the NRG-500 Challenge)",
      "322207", [] { float t = CStats::GetStatValue(STAT_NRG_500_BEST_TIME); return t >= 1.0f && t <= 120.0f; } },

    { "Motor Massacre",
      R"(Complete the Blood Bowl)",
      "317473", [] { return false; } },

    { "Bat Out Of Hell",
      R"(Complete the Blood Bowl's Target Time of 2 minutes)",
      "322451", [] { return false; } },

    { "Red Collar Job",
      R"(Complete Vank Hoff Hotel's Valet Parking mission)",
      "317464", [] { return false; } },

    { "Go Kart License",
      R"(Pass Driving School)",
      "317450", [] { return false; } },

    { "Vintage Car Collector",
      R"(Get a gold medal on every Driving School test)",
      "317451", [] { return false; } },

    { "Majestic Marquis",
      R"(Pass Boat School)",
      "317455", [] { return false; } },

    { "Jetmax Pistol!",
      R"(Get a gold medal on every Boat School test)",
      "317452", [] { return false; } },

    { "Mother Trucker",
      R"(Complete all Trucking missions)",
      "317432", Flag(DONE_TRUCK_PROGRESS) },

    { "Big Enough",
      R"(Collect 25 Horseshoes)",
      "318218", Stat(STAT_HORSESHOES_COLLECTED, 25.0f) },

    { "Lucky Luke",
      R"(Collect 50 Horseshoes)",
      "318219", Flag(FLAG_RETURNED_SHOEHORSES2) },

    { "Freeway Driver",
      R"(Pass Bike School)",
      "317453", [] { return false; } },

    { "NRG-500 Specialist",
      R"(Get a gold medal on every Bike School test)",
      "317454", [] { return false; } },

    { "Paying Off The Faggio",
      R"(Complete Burger Shot's courier mission)",
      "317466", [] { return false; } },

    { "We're Going Off-Road!",
      R"(Get a best score of 26 in Kickstart)",
      "317470", Stat(STAT_KICKSTART_BEST_SCORE, 26.0f) },

    { "Acrobatic Expertise",
      R"(Get a best score of 61 in Kickstart)",
      "322154", Stat(STAT_KICKSTART_BEST_SCORE, 61.0f) },

    { "Dirty Sanchez",
      R"(Get 1st place in Dirt Track)",
      "317474", [] { return CStats::GetStatValue(STAT_BEST_POSITION_IN_DIRT_TRACK) == 1.0f; } },

    { "Crazy Train",
      R"(Complete Level 2 of the Freight Train Challenge)",
      "317433", [] { return false; } },

    { "M.U.R.I.C.A",
      R"(Complete the Shooting Range Challenge)",
      "317461", [] { return false; } },

    { "The Holy Pearl",
      R"(Collect 50 oysters)",
      "317791", Flag(RETURNED_OYSTERS_FLAG) },

    { "Crane Operator",
      R"(Finish the first list of Vehicle Exports)",
      "317460", [] { return false; } },

    { "Basin Docks Scout",
      R"(Finish the second list of Vehicle Exports)",
      "322499", [] { return false; } },

    { "Car Dealer",
      R"(Finish the third list of Vehicle Exports)",
      "322500", [] { return false; } },

    { "Jet Fuel Can't Melt Steel Beams",
      R"(Get a gold medal on every Flight School test)",
      "317449", [] { return false; } },

    { "Los Santos Pink Slip Conqueror",
      R"(Win all Los Santos Race Tournaments unlocked after "Learning to Fly")",
      "317475", [] { return false; } },

    { "San Fierro Pink Slip Conqueror",
      R"(Win all San Fierro Race Tournaments)",
      "317476", [] { return false; } },

    { "Las Venturas Pink Slip Conqueror",
      R"(Win all Las Venturas Freight Depot Race Tournaments)",
      "317477", [] { return false; } },

    { "Strutter!",
      R"(Complete all Las Venturas Airport Checkpoint Races)",
      "317478", [] { return false; } },

    { "It Ain't Much, But It's Honest Work",
      R"(Complete all Quarry missions)",
      "317431", Flag(DONE_QUARRY_PROGRESS) },

    { "MMA Fighter",
      R"(Learn all the moves from the 3 gyms)",
      "317420", [] { return false; } },

    { "Beginning Investor",
      R"(Buy all properties that are available in the beginning of the game)",
      "317436", [] { return false; } },

    { "Los Santos Magnate",
      R"(Buy all properties that become available after beating "The Green Sabre")",
      "317437", [] { return false; } },

    { "San Fierro Magnate",
      R"(Buy all properties that become available after beating "Are You Going to San Fierro?")",
      "317438", [] { return false; } },

    { "Preparing A Pension",
      R"(Buy all properties that become available after beating "Yay Ka-Boom-Boom")",
      "317439", [] { return false; } },

    { "Aero-Investment",
      R"(Buy all properties that become available after beating "Learning to Fly")",
      "317440", [] { return false; } },

    { "King Of San Andreas Jr.",
      R"(100% Achieved!)",
      "317441", [] { return false; } },

    { "Stunt Double",
      R"(Do 35 Unique Jumps)",
      "318205", Stat(STAT_UNIQUE_JUMPS_DONE, 35.0f) },

    { "Urban Legend",
      R"(Do 70 Unique Jumps)",
      "318206", Stat(STAT_UNIQUE_JUMPS_DONE, 70.0f) },

    { "Glock Glock 9000",
      R"(Reach the skill level of Hitman with the Regular Pistol)",
      "317375", Stat(STAT_PISTOL_SKILL, 1000.0f) },

    { "Silence, I Kill You!",
      R"(Reach the skill level of Hitman with the Silenced 9mm Pistol)",
      "317376", Stat(STAT_SILENCED_PISTOL_SKILL, 1000.0f) },

    { "Juan Deag",
      R"(Reach the skill level of Hitman with the Desert Eagle)",
      "317377", Stat(STAT_DESERT_EAGLE_SKILL, 1000.0f) },

    { "Preparing The Hostage Situation",
      R"(Reach the skill level of Hitman with the Regular Shotgun)",
      "317378", Stat(STAT_SHOTGUN_SKILL, 1000.0f) },

    { "Brutal Massacre",
      R"(Reach the skill level of Hitman with the Sawn-off Shotgun)",
      "317379", Stat(STAT_SAWN_OFF_SHOTGUN_SKILL, 1000.0f) },

    { "Left 4 Dead",
      R"(Reach the skill level of Hitman with the Combat Shotgun)",
      "317380", Stat(STAT_COMBAT_SHOTGUN_SKILL, 1000.0f) },

    { "Pistol Round",
      R"(Reach the skill level of Hitman with the Tec-9)",
      "317381", Stat(STAT_MACHINE_PISTOL_SKILL, 1000.0f) },

    { "Micro Compensation",
      R"(Reach the skill level of Hitman with the SMG)",
      "317382", Stat(STAT_SMG_SKILL, 1000.0f) },

    { "Al Capone",
      R"(Reach the skill level of Hitman with the AK-47)",
      "317383", Stat(STAT_AK_47_SKILL, 1000.0f) },

    { "Counter Strike",
      R"(Reach the skill level of Hitman with the M4)",
      "317384", Stat(STAT_M4_SKILL, 1000.0f) },

    { "Come To Daddy",
      R"(Max out your Sex Appeal)",
      "318198", Stat(STAT_SEX_APPEAL, 1000.0f) },

    { "Number Nine Eating Contest",
      R"(Get so fat that you're at a huge risk for a heart attack)",
      "317385", [] { return false; } },

    { "Bulkin' Up",
      R"(Max out your Muscle)",
      "317386", Stat(STAT_MUSCLE, 1000.0f) },

    { "Running Man",
      R"(Max out your Stamina)",
      "317387", Stat(STAT_STAMINA, 1000.0f) },

    { "Pack Leader",
      R"(Earn enough Respect to form a 7 man gang)",
      "317388", [] { return false; } },

    { "Natural Gasoline Talent",
      R"(Max out the Driving, Flying and Bike skills)",
      "317389", [] { return CStats::GetStatValue(STAT_DRIVING_SKILL) >= 1000.0f
                         && CStats::GetStatValue(STAT_FLYING_SKILL) >= 1000.0f
                         && CStats::GetStatValue(STAT_BIKE_SKILL) >= 1000.0f; } },

    { "That's Real Interesting",
      R"(Gift a Vibrator (the smaller of the two toys) to one of your girlfriends)",
      "321536", [] { return false; } },

    { "A Tour Around Town",
      R"(Get your car stolen by Michelle Cannes during your date)",
      "322512", [] { return false; } },

    { "Kinky Stuff",
      R"(Trigger the Extra Date with Millie Perkins)",
      "322511", [] { return false; } },

    { "Passive Income",
      R"(Unlock the Pimp Suit)",
      "317442", [] { return false; } },

    { "Livin' Off The Grid",
      R"(Unlock the Rural Clothes)",
      "317443", [] { return false; } },

    { "Formula 1 Driver",
      R"(Unlock the Racing Suit)",
      "317444", [] { return false; } },

    { "Call A Medic!",
      R"(Unlock the Medic Uniform)",
      "317445", [] { return false; } },

    { "Cop Cosplay",
      R"(Unlock the Cop Outfit)",
      "317446", [] { return false; } },

    { "Take It From El Binco",
      R"(Buy all clothes available at Binco)",
      "317392", [] { return false; } },

    { "Sub Urban Dictionary",
      R"(Buy all clothes available at Sub Urban)",
      "317393", [] { return false; } },

    { "Pro-Laps Around Uranus",
      R"(Buy all clothes available at Pro-Laps)",
      "317394", [] { return false; } },

    { "Zipper Up",
      R"(Buy all clothes available at Zip)",
      "317395", [] { return false; } },

    { "Fashion Fictim",
      R"(Buy all clothes available at Victim)",
      "317396", [] { return false; } },

    { "Empty The Sach",
      R"(Buy all clothes available at Didier Sachs)",
      "317397", [] { return false; } },

    { "Fashionista CJ",
      R"(Obtain all clothing in the game excluding the Pimp Suit)",
      "317398", [] { return false; } },

    { "Asserting Dominance",
      R"(Conquer 30 territories (A territory counts as being yours after you clear the 2nd wave))",
      "318199", [] { return false; } },

    { "Alpha Male",
      R"(Conquer 53 territories)",
      "318200", [] { return false; } },

    { "From Fat To Thin Cock",
      R"(Beat the Cock starting at Fisher's Lagoon and add the prize to your cash stack)",
      "317456", [] { return false; } },

    { "Beat Up The Cock",
      R"(Beat the Cock starting at Santa Maria Beach and add the prize to your cash stack)",
      "317457", [] { return false; } },

    { "The Legal Crime Circuit",
      R"(Max out your Gambling skill)",
      "317390", Stat(STAT_GAMBLING, 1000.0f) },

    { "A Good Sailor Sails The Red Sea",
      R"(Receive a fatal threat to your life from an angry loan shark)",
      "317447", [] { return false; } },

    { "Beerrrkkkleeeeeyyy!!!!!",
      R"(Destroy 25 vans in a run of Beefy Baron)",
      "317465", [] { return false; } },

    { "Oh Baby, A Triple!",
      R"(Pull off your first Triple Insane Stunt)",
      "318201", [] { return false; } },

    { "One Wheel Wonder",
      R"(Get a longest wheelie time of 60 seconds)",
      "318203", [] { return false; } },

    { "Vertical Velocity",
      R"(Get a longest 2 wheels time of 5 seconds)",
      "318202", [] { return false; } },

    { "It's Like This And Like That",
      R"(Clear "Nuthin' But A 'G' Thang" in the dancing minigame at a Club with at least 3.500 points)",
      "322513", [] { return false; } },

    { "And Like This And Uh",
      R"(Clear "Nuthin' But A 'G' Thang" in the dancing minigame at a Club with at least 6.500 points)",
      "317421", [] { return false; } },

    { "Say Hey Hey Hey",
      R"(Clear "Hollywood Swinging" in the dancing minigame at a Club with at least 6.000 points)",
      "322514", [] { return false; } },

    { "What You Got To Say?",
      R"(Clear "Hollywood Swinging" in the dancing minigame at a Club with at least 8.000 points)",
      "317422", [] { return false; } },

    { "We Gotta Get Over Before We Go Under",
      R"(Clear "Funky President" in the dancing minigame at a Club with at least 7.500 points)",
      "317423", [] { return false; } },

    { "P-40E Warhawk",
      R"(Get a best time of 2:00 or less in World War Ace)",
      "318223", [] { return false; } },

    { "Pitts Special S-1",
      R"(Get a best time of 5:00 or less in Barnstorming)",
      "318224", [] { return false; } },

    { "Harrier II",
      R"(Get a best time of 4:00 or less in Military Service)",
      "318225", [] { return false; } },

    { "Bell 206",
      R"(Get a best time of 2:40 or less in Chopper Checkpoint)",
      "318226", [] { return false; } },

    { "Urban Bell 206",
      R"(Get a best time of 2:45 or less in Whirly Bird Waypoint)",
      "318227", [] { return false; } },

    { "Boeing AH-64 Apache",
      R"(Get a best time of 3:45 or less in Heli Hell)",
      "318228", [] { return false; } },

    { "Chevrolet Camaro",
      R"(Save a Phoenix in a garage)",
      "318214", [] { return false; } },

    { "Glen What??",
      R"(Save a Ruined Glendale or a Ruined Sadler in a garage)",
      "318213", [] { return false; } },

    { "Mitsubishi Pajero",
      R"(Save a Sandking in a garage)",
      "318212", [] { return false; } },

    { "Chevrolet Suburban",
      R"(Save an FBI Rancher in a garage)",
      "318211", [] { return false; } },

    { "Beefy RC Van",
      R"(Save a Berkley's RC Van in a garage)",
      "318210", [] { return false; } },

    { "CH-47 Chinook",
      R"(Save a Cargobob in the Verdant Meadows hangar)",
      "318209", [] { return false; } },

    { "Too Big To Fit!",
      R"(Save a plane in any garage except the Verdant Meadows hangar)",
      "318208", [] { return false; } },

    { "Mecha-Plastic Surgery",
      R"(Save a car with 11 modifications in a garage (excluding colors and paintjobs))",
      "318207", [] { return false; } },

    { "Hoopin' It Up",
      R"(Score at least 30 points in the Basketball Challenge)",
      "317409", [] { return false; } },

    { "Small Forward Jordy",
      R"(Score at least 50 points in the Basketball Challenge)",
      "317410", [] { return false; } },

    { "DoDonPachi II: Bee Storm",
      R"(Submit a High Score of 5.000 on Let's Get Ready To Bumble)",
      "317414", [] { return false; } },

    { "The Sizzle Special",
      R"(Submit a High Score of 50.000 in Go Go Space Monkey)",
      "317413", [] { return false; } },

    { "The Big Probe",
      R"(Submit a 1st place score in They Crawled From Uranus)",
      "317415", [] { return false; } },

    { "Colorblind",
      R"(Submit a top 5 score in Duality's white high score table)",
      "317416", [] { return false; } },

    { "Top Score",
      R"(Submit a top 5 score in Duality's black high score table)",
      "317417", [] { return false; } },

    { "Master Balls",
      R"(Win a game of Pool by pocketing all your balls)",
      "317411", [] { return false; } },

    { "From Zero To Hero",
      R"(Use a Pay 'N' Spray while at a 6-star wanted rating)",
      "317424", [] { return false; } },

    { "There are no Easter Eggs up here. Go away",
      R"(Stand on the platform with the non-existent easter egg)",
      "318197", [] { return false; } },

    { "The Biggest Rock Formation in San Andreas",
      R"(Stand on top of Cock Rock)",
      "318196", [] { return false; } },

    { "Lunar Eclipse",
      R"(Make the moon reach its biggest size)",
      "318195", [] { return false; } },

    { "Grove Street, Home",
      R"(Have one of your homies take a picture of you and save it to the gallery)",
      "317787", [] { return false; } },

    // --- Subset: Master Save ---

    { "All's Fare",
      R"(Complete 50 Taxi fares before clearing the first story mission.)",
      "650443", FlagBefore(DONE_TAXIODD_PROGRESS) },

    { "Out of the Fire",
      R"(Complete all 12 Firefighter levels before clearing the first story mission.)",
      "650444", FlagBefore(DONE_FIRETRUCK_PROGRESS) },

    { "Medical Prodigy",
      R"(Complete all 12 Paramedic levels before clearing the first story mission.)",
      "650445", FlagBefore(DONE_AMBULANCE_PROGRESS) },

    { "Same Old Job",
      R"(Complete all 10 Pimping levels before clearing the first story mission.)",
      "650446", StatBefore(STAT_PIMPING_LEVEL, 10.0f) },

    { "Police Protection",
      R"(Complete 12 Vigilante levels before clearing the first story mission.)",
      "650447", FlagBefore(DONE_COPCAR_PROGRESS) },

    { "Neighborhood Courier",
      R"(Complete Roboi's Food Mart's courier mission before clearing the first story mission.)",
      "650448", [] { return false; } },

    { "Seismic Delivery",
      R"(Complete Hippy Shopper's courier mission before clearing the first story mission.)",
      "650449", [] { return false; } },

    { "Running from the Heat",
      R"(Complete Burger Shot's courier mission before clearing the first story mission.)",
      "650450", [] { return false; } },

    { "Easy Rider",
      R"(Complete the BMX Challenge before clearing the first story mission.)",
      "650451", StatBefore(STAT_BMX_BEST_TIME, 1.0f) },

    { "Dry Diving",
      R"(Complete the NRG-500 Challenge before clearing the first story mission.)",
      "650452", StatBefore(STAT_NRG_500_BEST_TIME, 1.0f) },

    { "The Hardest Climb",
      R"(Beat all 3 routes in the Chiliad Challenge before clearing the first story mission.)",
      "650453", [] { return false; } },

    { "Stock Car Driver",
      R"(Get 1st place in 8-Track before clearing the first story mission.)",
      "650454", [] { return ScmGlobal(FLAG_INTRO_MISSION_COUNTER) == 0 && CStats::GetStatValue(STAT_BEST_POSITION_IN_8_TRACK) == 1.0f; } },

    { "Murder on Wheels",
      R"(Complete the Blood Bowl before clearing the first story mission.)",
      "650455", [] { return false; } },

    { "Stunt Rider",
      R"(Get a score of 26 in Kickstart before clearing the first story mission.)",
      "650456", StatBefore(STAT_KICKSTART_BEST_SCORE, 26.0f) },

    { "Taking the Gold Medal Home",
      R"(Get 1st place in Dirt Track before clearing the first story mission.)",
      "650457", [] { return ScmGlobal(FLAG_INTRO_MISSION_COUNTER) == 0 && CStats::GetStatValue(STAT_BEST_POSITION_IN_DIRT_TRACK) == 1.0f; } },

    { "Getting Up",
      R"(Spray all 100 Tags before clearing the first story mission.)",
      "650458", FlagBefore(FLAG_RETURNED_TAGS2) },

    { "War Zone Photographer",
      R"(Take all 50 Snapshots before clearing the first story mission.)",
      "650459", FlagBefore(FLAG_RETURNED_SNAPSHOTS2) },

    { "Feeling Lucky Yet?",
      R"(Collect all 50 Horseshoes before clearing the first story mission.)",
      "650460", FlagBefore(FLAG_RETURNED_SHOEHORSES2) },

    { "Contents Under Water",
      R"(Collect all 50 Oysters before clearing the first story mission.)",
      "650461", FlagBefore(RETURNED_OYSTERS_FLAG) },

    { "1337 H4x0r",
      R"(Achieve 13.37% progress before clearing the first story mission.)",
      "650462", [] { return ScmGlobal(FLAG_INTRO_MISSION_COUNTER) == 0 && CStats::GetPercentageProgress() >= 13.37f; } },

    { "On-Call Nursing",
      R"(Get Katie as a girlfriend before clearing the first story mission.)",
      "650463", StatBefore(STAT_PROGRESS_WITH_KATIE, 1.0f) },

    { "Get Out of Jail Free Card",
      R"(Get Barbara as a girlfriend before clearing the first story mission.)",
      "650464", StatBefore(STAT_PROGRESS_WITH_BARBARA, 1.0f) },

    { "Early Millionaire",
      R"(Accumulate $1,000,000 before clearing the first story mission.)",
      "650465", [] { return false; } },

    { "Training Montage",
      R"(Max out your Muscle before clearing the first story mission.)",
      "650466", StatBefore(STAT_MUSCLE, 1000.0f) },

    { "Keeping up Appearances",
      R"(Max out your Stamina before clearing the first story mission.)",
      "650467", StatBefore(STAT_STAMINA, 1000.0f) },

    { "Off and on Again",
      R"(Max out your Sex Appeal before clearing the first story mission.)",
      "650468", StatBefore(STAT_SEX_APPEAL, 1000.0f) },

    { "Beating the Trauma",
      R"(Max out your Lung Capacity before clearing the first story mission.)",
      "650469", StatBefore(STAT_LUNG_CAPACITY, 1000.0f) },

    { "Armory Specialist",
      R"(Reach the skill level of Hitman with all weapons before clearing the first story mission.)",
      "650470", [] {
          if (ScmGlobal(FLAG_INTRO_MISSION_COUNTER) != 0) return false;
          return CStats::GetStatValue(STAT_PISTOL_SKILL)          >= 1000.0f
              && CStats::GetStatValue(STAT_SILENCED_PISTOL_SKILL) >= 1000.0f
              && CStats::GetStatValue(STAT_DESERT_EAGLE_SKILL)    >= 1000.0f
              && CStats::GetStatValue(STAT_SHOTGUN_SKILL)         >= 1000.0f
              && CStats::GetStatValue(STAT_SAWN_OFF_SHOTGUN_SKILL)>= 1000.0f
              && CStats::GetStatValue(STAT_COMBAT_SHOTGUN_SKILL)  >= 1000.0f
              && CStats::GetStatValue(STAT_MACHINE_PISTOL_SKILL)  >= 1000.0f
              && CStats::GetStatValue(STAT_SMG_SKILL)             >= 1000.0f
              && CStats::GetStatValue(STAT_AK_47_SKILL)           >= 1000.0f
              && CStats::GetStatValue(STAT_M4_SKILL)              >= 1000.0f;
      } },

    { "Performance Enhancer",
      R"(Max out your Cycling Skill before clearing the first story mission.)",
      "650471", StatBefore(STAT_CYCLING_SKILL, 1000.0f) },

    { "Reliable Rider",
      R"(Max out your Bike Skill before clearing the first story mission.)",
      "650472", StatBefore(STAT_BIKE_SKILL, 1000.0f) },

    { "Well Travelled",
      R"(Max out your Driving Skill before clearing the first story mission.)",
      "650473", StatBefore(STAT_DRIVING_SKILL, 1000.0f) },

    { "Amateur Ace",
      R"(Max out your Flying Skill before clearing the first story mission.)",
      "650474", StatBefore(STAT_FLYING_SKILL, 1000.0f) },

    { "House Regular",
      R"(Max out your Gambling Skill before clearing the first story mission.)",
      "650475", StatBefore(STAT_GAMBLING, 1000.0f) },

    { "Snatch and Run",
      R"(Steal a total worth of $10,000 in Burglary missions before clearing the first story mission.)",
      "650476", FlagBefore(DONE_BURGLARY_PROGRESS) },

    { "Real Estate Trust Fund",
      R"(Buy all properties that are available in the beginning of the game before clearing the first story mission.)",
      "650477", [] { return false; } },

    { "Nice Run",
      R"(Find all 70 Unique Jumps and do all 69 possible ones before clearing the first story mission.)",
      "650478", StatBefore(STAT_UNIQUE_JUMPS_DONE, 70.0f) },

    { "Long Shot",
      R"(Bet on the horse with the lowest odds and win at Inside Track Betting before clearing the first story mission.)",
      "650479", [] { return false; } },

    { "What Happens in Venturas...",
      R"(Win a hand of Blackjack before clearing the first story mission.)",
      "650480", [] { return false; } },

    { "Preparing for the Future",
      R"(Store a Patriot in the garage at the El Corona Safe House before clearing the first story mission.)",
      "650481", [] { return false; } },

    { "The Ultimate Law Enforcer",
      R"(Steal Barbara's All-Proof Ranger and store it in the garage at the Santa Maria Beach Safe House in one sitting before clearing the first story mission.)",
      "650482", [] { return false; } },
};

static_assert(std::size(kAch) == kAchCount, "kAchCount is out of sync with kAch");
