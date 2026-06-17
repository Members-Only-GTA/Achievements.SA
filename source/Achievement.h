#pragma once
#include <functional>

struct Achievement {
    const char* label;
    const char* description;
    const char* icon;
    std::function<bool()> check;
};

#if defined(ACH_SET_RA)
static constexpr int kAchCount   = 301;
static constexpr int kGlobalBase = 7800;
#elif defined(ACH_SET_STEAM)
static constexpr int kAchCount   = 34;
static constexpr int kGlobalBase = 7800;
#else
#error "Define ACH_SET_RA or ACH_SET_STEAM"
#endif

extern const Achievement kAch[kAchCount];

void ResetAchievementState();
bool IsAchievementUnlocked(int i);
