#pragma once
#include <functional>

struct Achievement {
    const char* label;
    const char* description;
    const char* icon;
    std::function<bool()> check;
};

static constexpr int kAchCount = 301;
extern const Achievement kAch[kAchCount];

void ResetAchievementState();
