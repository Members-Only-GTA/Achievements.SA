#include <plugin.h>
#include <CFont.h>
#include <CRGBA.h>
#include <CSprite2d.h>
#include <CTxdStore.h>
#include <CAudioEngine.h>
#include <CTheScripts.h>
#include <CHudColours.h>

#include <string>
#include <queue>

#include "Achievement.h"

using namespace plugin;

#define TOAST_HUD(x) ((x) * (SCREEN_HEIGHT / 448.0f))

static constexpr ULONGLONG kDurationMs   = 8000;
static constexpr ULONGLONG kSlideInMs    = 400;
static constexpr ULONGLONG kFadeStartMs  = 7000;
static constexpr float kIconSize     = 28.0f;
static constexpr float kToastH       = 28.0f;
static constexpr float kToastHNoDesc = 28.0f;
static constexpr float kBgPad        = 5.0f;
static constexpr float kAccentW      = 2.0f;
static constexpr float kTop          = 20.0f;
static constexpr float kTextOffX     = kIconSize + 5.0f;
static constexpr float kTextRightPad = 6.0f;
static constexpr float kMaxTextW     = 180.0f;
static constexpr float kMinTextW     = 30.0f;

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

    float slideT = elapsed < kSlideInMs
        ? static_cast<float>(elapsed) / static_cast<float>(kSlideInMs)
        : 1.0f;
    slideT = 1.0f - (1.0f - slideT) * (1.0f - slideT);

    const bool  hasDesc = !s_toast.desc.empty();
    const float toastH  = hasDesc ? kToastH : kToastHNoDesc;

    const float topY = kTop + (1.0f - slideT) * -(toastH + 2.0f * kBgPad);

    const float t = elapsed < kSlideInMs  ? slideT
                  : elapsed < kFadeStartMs ? 1.0f
                  : 1.0f - static_cast<float>(elapsed - kFadeStartMs) / static_cast<float>(kDurationMs - kFadeStartMs);
    const uint8_t alpha = static_cast<uint8_t>(255 * t);

    const float maxTextPx = TOAST_HUD(kMaxTextW);
    const float minTextPx = TOAST_HUD(kMinTextW);

    CFont::SetWrapx(SCREEN_WIDTH);
    CFont::SetProportional(true);
    CFont::SetFontStyle(FONT_PRICEDOWN);
    CFont::SetScale(TOAST_HUD(0.275f), TOAST_HUD(0.55f));
    float labelW = CFont::GetStringWidth(s_toast.label.c_str(), true);
    if (labelW > maxTextPx) labelW = maxTextPx;

    float descW = 0.0f;
    if (hasDesc) {
        CFont::SetWrapx(SCREEN_WIDTH);
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetScale(TOAST_HUD(0.175f), TOAST_HUD(0.35f));
        descW = CFont::GetStringWidth(s_toast.desc.c_str(), true);
        if (descW > maxTextPx) descW = maxTextPx;
    }

    float textW = labelW > descW ? labelW : descW;
    if (textW < minTextPx) textW = minTextPx;

    const float bgPadPx     = TOAST_HUD(kBgPad);
    const float contentW    = TOAST_HUD(kTextOffX) + textW + TOAST_HUD(kTextRightPad);
    const float bgHalfW     = bgPadPx + contentW / 2.0f;
    const float centerX     = SCREEN_WIDTH / 2.0f;
    const float bgLeft      = centerX - bgHalfW;
    const float bgRight     = centerX + bgHalfW;
    const float contentLeft = bgLeft + bgPadPx;
    const float textX       = contentLeft + TOAST_HUD(kTextOffX);
    const float wrapX       = textX + textW + 2.0f;

    CFont::SetBackground(false, false);
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetFontStyle(FONT_PRICEDOWN);
    CFont::SetScale(TOAST_HUD(0.275f), TOAST_HUD(0.55f));
    CFont::SetDropShadowPosition(2);
    CFont::SetWrapx(wrapX);
    const int   numTitleLines = CFont::GetNumberLines(textX, TOAST_HUD(topY + 5.0f), s_toast.label.c_str());
    const float titleLineH    = CFont::m_Scale->y * 18.0f;
    const float extraTitleH   = (numTitleLines - 1) * titleLineH;

    float extraDescH = 0.0f;
    if (hasDesc) {
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetScale(TOAST_HUD(0.175f), TOAST_HUD(0.35f));
        CFont::SetDropShadowPosition(1);
        CFont::SetWrapx(wrapX);
        const int   numDescLines = CFont::GetNumberLines(textX, TOAST_HUD(topY + 17.0f), s_toast.desc.c_str());
        const float descLineH    = CFont::m_Scale->y * 18.0f;
        extraDescH = (numDescLines - 1) * descLineH;
    }
    CFont::SetDropShadowPosition(0);

    const float titleY = TOAST_HUD(hasDesc ? topY + 5.0f : topY + (kToastHNoDesc - 10.0f) * 0.5f);
    const float descY  = TOAST_HUD(topY + 17.0f) + extraTitleH;
    const float bgTop  = TOAST_HUD(topY - kBgPad);
    const float bgBot  = TOAST_HUD(topY + toastH + kBgPad) + extraTitleH + extraDescH;

    const float border = TOAST_HUD(1.0f);

    CSprite2d::DrawRect(CRect(bgLeft - border, bgTop - border, bgRight + border, bgBot + border),
        CRGBA(0, 0, 0, static_cast<uint8_t>(200 * t)));

    CSprite2d::DrawRect(CRect(bgLeft, bgTop, bgRight, bgBot),
        CRGBA(55, 55, 55, static_cast<uint8_t>(220 * t)),
        CRGBA(55, 55, 55, static_cast<uint8_t>(220 * t)),
        CRGBA(22, 22, 22, static_cast<uint8_t>(220 * t)),
        CRGBA(22, 22, 22, static_cast<uint8_t>(220 * t)));

    CSprite2d::DrawRect(CRect(bgLeft, bgTop, bgLeft + TOAST_HUD(kAccentW), bgBot),
        CRGBA(HudColour.GetRGBA(HUD_COLOUR_YELLOW)));

    if (s_iconSprite.m_pTexture) {
        s_iconSprite.Draw(contentLeft, TOAST_HUD(topY + (toastH - kIconSize) * 0.5f),
                          TOAST_HUD(kIconSize), TOAST_HUD(kIconSize),
                          CRGBA(HudColour.GetRGBA(HUD_COLOUR_WHITE)));
    }

    CFont::SetAlphaFade(255.0f * t);
    CFont::SetBackground(false, false);
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetWrapx(wrapX);

    CFont::SetFontStyle(FONT_PRICEDOWN);
    CFont::SetScale(TOAST_HUD(0.275f), TOAST_HUD(0.55f));
    CFont::SetColor(HudColour.GetRGBA(HUD_COLOUR_YELLOW));
    CFont::SetEdge(2);
    CFont::SetDropColor(HudColour.GetRGBA(HUD_COLOUR_BLACK));
    CFont::PrintString(textX, titleY, s_toast.label.c_str());
    CFont::SetEdge(0);

    if (hasDesc) {
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetScale(TOAST_HUD(0.175f), TOAST_HUD(0.35f));
        CFont::SetColor(HudColour.GetRGBA(HUD_COLOUR_WHITE));
        CFont::SetDropShadowPosition(1);
        CFont::SetDropColor(HudColour.GetRGBA(HUD_COLOUR_BLACK));
        CFont::PrintString(textX, descY, s_toast.desc.c_str());
        CFont::SetDropShadowPosition(0);
    }

    CFont::SetAlphaFade(255.0f);
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
