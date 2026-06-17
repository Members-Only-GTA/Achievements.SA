#include <plugin.h>
#include <CFont.h>
#include <CRGBA.h>
#include <CSprite2d.h>
#include <CTxdStore.h>
#include <CAudioEngine.h>
#include <CTheScripts.h>

#include <string>
#include <queue>

#include "Achievement.h"

using namespace plugin;

static constexpr ULONGLONG kDurationMs  = 8000;
static constexpr ULONGLONG kFadeStartMs = 7000;
static constexpr float kIconSize    = 64.0f;
static constexpr float kToastH      = 80.0f;
static constexpr float kBgPad       = 10.0f;
static constexpr float kTop         = 80.0f;
static constexpr float kTextOffX    = kIconSize + 12.0f;
static constexpr float kTextRightPad = 14.0f;
static constexpr float kMaxTextW    = 360.0f;
static constexpr float kMinTextW    = 80.0f;

int g_achTxdSlot = -1;
static CSprite2d s_iconSprite;

static constexpr int kGlobalWords = (kAchCount + 31) / 32;

static bool GetSaved(int i) {
    auto& word = *reinterpret_cast<uint32_t*>(CTheScripts::ScriptSpace + (kGlobalBase + i / 32) * 4);
    return (word >> (i % 32)) & 1u;
}

static void SetSaved(int i) {
    auto& word = *reinterpret_cast<uint32_t*>(CTheScripts::ScriptSpace + (kGlobalBase + i / 32) * 4);
    word |= 1u << (i % 32);
}

static struct { bool active = false; ULONGLONG startTick = 0; std::string label, desc; } s_toast;
static std::queue<int> s_queue;

static void OnDraw() {
    if (!s_toast.active && !s_queue.empty()) {
        int i = s_queue.front(); s_queue.pop();
        s_toast.active    = true;
        s_toast.startTick = GetTickCount64();
        s_toast.label     = kAch[i].label;
        s_toast.desc      = kAch[i].description ? kAch[i].description : "";

        if (g_achTxdSlot != -1) {
            s_iconSprite.Delete();
            CTxdStore::PushCurrentTxd();
            CTxdStore::SetCurrentTxd(g_achTxdSlot);
            s_iconSprite.SetTexture(const_cast<char*>(kAch[i].icon));
            CTxdStore::PopCurrentTxd();
        }

        AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_DISPLAY_INFO, 1.0f, 1.0f);
    }
    if (!s_toast.active) return;

    const ULONGLONG elapsed = GetTickCount64() - s_toast.startTick;
    if (elapsed >= kDurationMs) {
        s_toast.active = false;
        s_iconSprite.Delete();
        return;
    }

    const float   t     = elapsed < kFadeStartMs ? 1.0f
        : 1.0f - static_cast<float>(elapsed - kFadeStartMs) / (kDurationMs - kFadeStartMs);
    const uint8_t alpha = static_cast<uint8_t>(255 * t);

    const float maxTextPx = SCREEN_COORD(kMaxTextW);
    const float minTextPx = SCREEN_COORD(kMinTextW);

    CFont::SetProportional(true);
    CFont::SetFontStyle(FONT_PRICEDOWN);
    CFont::SetScale(SCREEN_MULTIPLIER(0.75f), SCREEN_MULTIPLIER(1.5f));
    float labelW = CFont::GetStringWidth(s_toast.label.c_str(), true);
    if (labelW > maxTextPx) labelW = maxTextPx;

    float descW = 0.0f;
    if (!s_toast.desc.empty()) {
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetScale(SCREEN_MULTIPLIER(0.45f), SCREEN_MULTIPLIER(0.9f));
        descW = CFont::GetStringWidth(s_toast.desc.c_str(), true);
        if (descW > maxTextPx) descW = maxTextPx;
    }

    float textW = labelW > descW ? labelW : descW;
    if (textW < minTextPx) textW = minTextPx;

    const float bgPadPx     = SCREEN_COORD(kBgPad);
    const float contentW    = SCREEN_COORD(kTextOffX) + textW + SCREEN_COORD(kTextRightPad);
    const float bgHalfW     = bgPadPx + contentW / 2.0f;
    const float centerX     = SCREEN_COORD_CENTER_X;
    const float bgLeft      = centerX - bgHalfW;
    const float bgRight     = centerX + bgHalfW;
    const float contentLeft = bgLeft + bgPadPx;
    const float textX       = contentLeft + SCREEN_COORD(kTextOffX);
    const float wrapX       = textX + textW;

    CFont::SetBackground(false, false);
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetFontStyle(FONT_PRICEDOWN);
    CFont::SetScale(SCREEN_MULTIPLIER(0.75f), SCREEN_MULTIPLIER(1.5f));
    CFont::SetDropShadowPosition(2);
    CFont::SetWrapx(wrapX);
    const int   numTitleLines = CFont::GetNumberLines(textX, SCREEN_COORD(kTop + 10.0f), s_toast.label.c_str());
    const float titleLineH    = CFont::m_Scale->y * 18.0f;
    const float extraTitleH   = (numTitleLines - 1) * titleLineH;

    float extraDescH = 0.0f;
    if (!s_toast.desc.empty()) {
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetScale(SCREEN_MULTIPLIER(0.45f), SCREEN_MULTIPLIER(0.9f));
        CFont::SetDropShadowPosition(1);
        CFont::SetWrapx(wrapX);
        const int   numDescLines = CFont::GetNumberLines(textX, SCREEN_COORD(kTop + 46.0f), s_toast.desc.c_str());
        const float descLineH    = CFont::m_Scale->y * 18.0f;
        extraDescH = (numDescLines - 1) * descLineH;
    }
    CFont::SetDropShadowPosition(0);

    const float descY = SCREEN_COORD(kTop + 46.0f) + extraTitleH;

    CSprite2d::DrawRect(
        CRect(bgLeft,
              SCREEN_COORD(kTop - kBgPad),
              bgRight,
              SCREEN_COORD(kTop + kToastH + kBgPad) + extraTitleH + extraDescH),
        CRGBA(39, 39, 39, static_cast<uint8_t>(220 * t)));

    if (s_iconSprite.m_pTexture) {
        s_iconSprite.Draw(contentLeft, SCREEN_COORD(kTop + (kToastH - kIconSize) * 0.5f),
                          SCREEN_COORD(kIconSize), SCREEN_COORD(kIconSize),
                          CRGBA(255, 255, 255, alpha));
    }

    CFont::SetBackground(false, false);
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetWrapx(wrapX);

    CFont::SetFontStyle(FONT_PRICEDOWN);
    CFont::SetScale(SCREEN_MULTIPLIER(0.75f), SCREEN_MULTIPLIER(1.5f));
    CFont::SetColor(CRGBA(255, 215, 0, alpha));
    CFont::SetDropShadowPosition(2);
    CFont::SetDropColor(CRGBA(0, 0, 0, static_cast<uint8_t>(200 * t)));
    CFont::PrintString(textX, SCREEN_COORD(kTop + 10.0f), s_toast.label.c_str());
    CFont::SetDropShadowPosition(0);

    if (!s_toast.desc.empty()) {
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetScale(SCREEN_MULTIPLIER(0.45f), SCREEN_MULTIPLIER(0.9f));
        CFont::SetColor(CRGBA(220, 220, 220, alpha));
        CFont::SetDropShadowPosition(1);
        CFont::SetDropColor(CRGBA(0, 0, 0, static_cast<uint8_t>(180 * t)));
        CFont::PrintString(textX, descY, s_toast.desc.c_str());
        CFont::SetDropShadowPosition(0);
    }
}

struct AchievementsPlugin {
    bool   m_initialized  = false;
    ULONGLONG m_lastPollTick = 0;
    bool   m_unlocked[kAchCount] = {};

    AchievementsPlugin() {
        Events::gameProcessEvent += [this] { OnProcess(); };
        Events::drawingEvent     += [] { OnDraw(); };
        Events::reInitGameEvent  += [this] { Reset(); };
        Events::restartGameEvent += [this] { Reset(); };

        static char s_txdPath[MAX_PATH];
        GetModuleFileNameA(GetModuleHandleA(TARGET_NAME ".asi"), s_txdPath, MAX_PATH);
        if (char* p = strrchr(s_txdPath, '\\')) strcpy_s(p + 1, MAX_PATH - (p + 1 - s_txdPath), "achievements.txd");

        Events::initRwEvent += [] {
            g_achTxdSlot = CTxdStore::AddTxdSlot("achievements");
            CTxdStore::LoadTxd(g_achTxdSlot, s_txdPath);
            CTxdStore::AddRef(g_achTxdSlot);
        };
        Events::shutdownRwEvent += [] {
            s_iconSprite.Delete();
            CTxdStore::RemoveRef(g_achTxdSlot);
            CTxdStore::RemoveTxdSlot(g_achTxdSlot);
            g_achTxdSlot = -1;
        };
    }

    void Reset() {
        m_initialized = false;
        memset(m_unlocked, 0, sizeof(m_unlocked));
        ResetAchievementState();
    }

    void OnProcess() {
        if (!m_initialized) {
            for (int i = 0; i < kAchCount; i++) {
                try { if (kAch[i].check()) m_unlocked[i] = true; } catch (...) {}
            }
            m_initialized = true;
            m_lastPollTick = GetTickCount64();
        }

        if (GetTickCount64() - m_lastPollTick < 500) return;
        m_lastPollTick = GetTickCount64();

        for (int i = 0; i < kAchCount; i++) {
            if (m_unlocked[i]) continue;
            try {
                if (kAch[i].check()) { m_unlocked[i] = true; SetSaved(i); s_queue.push(i); }
            } catch (...) {}
        }
    }
} gInstance;

bool IsAchievementUnlocked(int i) { return gInstance.m_unlocked[i] || GetSaved(i); }
