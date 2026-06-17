#include <plugin.h>
#include <CFont.h>
#include <CRGBA.h>
#include <CSprite2d.h>
#include <CTxdStore.h>
#include "Achievement.h"
#include "MenuExtender.h"

using namespace plugin;

bool IsAchievementUnlocked(int i);

extern int g_achTxdSlot;

static constexpr int kPageSize = 9;

static plugin::MenuExtender s_ext;
static CMenuItem*           s_achPage = nullptr;
static int                  s_scroll  = 0;

typedef const char* (__thiscall* CTextGetFn)(void*, const char*);
static CTextGetFn s_origCTextGet = nullptr;
static char       s_achLabel[]   = "ACHIEVEMENTS";
static uint8_t    s_origBytes[5];

static void WriteJmp();

static const char* __fastcall HookCTextGet(void* self, void* /*edx*/, const char* key) {
    if (key && key[0] == 'A') {
        if (strncmp(key, "ACHIEVMT", 8) == 0)
            return s_achLabel;

        if (key[1]=='C' && key[2]=='H' && key[3]=='S' && key[4]=='L'
            && key[5] >= '0' && key[5] <= '8' && key[6] == '\0') {
            int idx = s_scroll + (key[5] - '0');
            return (idx < kAchCount) ? kAch[idx].label : "";
        }

        if (key[1]=='C' && key[2]=='H' && key[3]=='D' && key[4]=='S' && key[5]=='C' && key[6]=='\0') {
            int idx = s_scroll + plugin::MenuExtender::GetCurrentEntryIndex();
            return (idx >= 0 && idx < kAchCount) ? kAch[idx].description : "";
        }
    }
    memcpy((void*)0x6A0050, s_origBytes, 5);
    const char* r = s_origCTextGet(self, key);
    WriteJmp();
    return r;
}

static void WriteJmp() {
    *(uint8_t*)0x6A0050 = 0xE9;
    *(int32_t*)(0x6A0050 + 1) = (int32_t)(uintptr_t)HookCTextGet - (int32_t)(0x6A0050 + 5);
}

static void InstallCTextHook() {
    static bool done = false;
    if (done) return; done = true;
    s_origCTextGet = (CTextGetFn)0x6A0050;
    DWORD old;
    VirtualProtect((void*)0x6A0050, 5, PAGE_EXECUTE_READWRITE, &old);
    memcpy(s_origBytes, (void*)0x6A0050, 5);
    WriteJmp();
}

static void RebuildPage() {
    if (!s_achPage) return;
    memset(s_achPage->m_aEntries, 0, sizeof(s_achPage->m_aEntries));

    for (int i = 0; i < kPageSize; ++i) {
        CMenuEntryData& e = s_achPage->m_aEntries[i];
        if (s_scroll + i >= kAchCount) {
            e.m_nActionType = MENU_ACTION_SKIP;
            continue;
        }
        e.m_nActionType = (unsigned char)MenuExtender::dummy_action;
        snprintf(e.m_szName, sizeof(e.m_szName), "ACHSL%d", i);
        e.m_nType  = MENU_ENTRY_BUTTON;
        e.m_nAlign = 1;
        if (i == 0) { e.m_wPosnX = 30; e.m_wPosnY = 90; }
    }

    CMenuEntryData& back = s_achPage->m_aEntries[kPageSize];
    back.m_nActionType = MENU_ACTION_BACK;
    strncpy(back.m_szName, "FED_BAC", sizeof(back.m_szName));
    back.m_nType  = MENU_ENTRY_BUTTON;
    back.m_nAlign = 3;
    back.m_wPosnX = 320;
    back.m_wPosnY = 380;
}

struct AchievementsMenuPlugin {
    AchievementsMenuPlugin() {
        Events::initGameEvent += [] {
            InstallCTextHook();
            s_ext.Initialise();

            CMenuItem* statsPage = s_ext.GetPage(0); // SCREEN_STATS
            s_achPage = s_ext.RegisterMenuPage();

            if (statsPage && s_achPage) {
                strncpy(s_achPage->m_szTitleName, "ACHIEVMT", sizeof(s_achPage->m_szTitleName));
                s_achPage->m_nPrevMenu = (char)s_ext.GetPageIndex(statsPage);

                CMenuEntryData* vanillaAch = plugin::MenuExtender::GetEntry(statsPage, "FES_ACH");
                if (vanillaAch)
                    s_ext.SetTargetMenu(vanillaAch, s_achPage, "FES_ACH");

                RebuildPage();
            }

            s_ext.RegisterUserInputListener([](plugin::MenuExtender::UserInputInfo& info) -> bool {
                if (info.page != s_achPage) return true;

                int cur = plugin::MenuExtender::GetCurrentEntryIndex();
                int vis = (kAchCount - s_scroll < kPageSize) ? (kAchCount - s_scroll) : kPageSize;

                if (info.down && cur < kPageSize && cur >= vis - 1 && s_scroll + vis < kAchCount) {
                    ++s_scroll;
                    RebuildPage();
                    return false;
                }
                if (info.up && cur < kPageSize && cur <= 0 && s_scroll > 0) {
                    --s_scroll;
                    RebuildPage();
                    return false;
                }
                return true;
            });

            s_ext.RegisterDrawListener([](plugin::MenuExtender::DrawInfo& info) -> bool {
                if (info.page != s_achPage) return true;

                float W  = SCREEN_WIDTH;
                float H  = SCREEN_HEIGHT;

                float entryH   = H * 0.080f;
                float entryGap = H * 0.007f;
                float headerH  = H * 0.055f;
                float totalH   = headerH + kPageSize * (entryH + entryGap) - entryGap;
                float originY  = (H - totalH) * 0.5f;
                float entriesY = originY + headerH;

                float listW  = W * 0.72f;
                float cardL  = (W - listW) * 0.5f;
                float cardR  = cardL + listW;

                float iconW   = entryH * 0.68f;
                float iconPad = entryH * 0.16f;
                float textX   = cardL + iconPad + iconW + entryH * 0.14f;

                int sel = plugin::MenuExtender::GetCurrentEntryIndex();

                {
                    int unlocked = 0;
                    for (int i = 0; i < kAchCount; ++i)
                        if (IsAchievementUnlocked(i)) ++unlocked;
                    static char hdr[48];
                    snprintf(hdr, sizeof(hdr), "Achievements   %d / %d", unlocked, kAchCount);
                    float hsc = SCREEN_MULTIPLIER(0.72f);
                    CFont::SetFontStyle(FONT_SUBTITLES);
                    CFont::SetOrientation(ALIGN_LEFT);
                    CFont::SetScale(hsc, hsc * 1.9f);
                    CFont::SetDropShadowPosition(0);
                    CFont::SetBackground(false, false);
                    CFont::SetColor(CRGBA(200, 200, 205, 255));
                    CFont::PrintString(cardL, originY + headerH * 0.2f, hdr);
                }

                for (int i = 0; i < kPageSize; ++i) {
                    int idx = s_scroll + i;
                    if (idx >= kAchCount) break;
                    bool unlocked = IsAchievementUnlocked(idx);
                    bool isSel    = (sel == i);
                    float rowY    = entriesY + i * (entryH + entryGap);

                    float nsc = SCREEN_MULTIPLIER(0.66f);
                    CFont::SetFontStyle(FONT_SUBTITLES);
                    CFont::SetOrientation(ALIGN_LEFT);
                    CFont::SetScale(nsc, nsc * 2.0f);
                    CFont::SetDropShadowPosition(0);
                    CFont::SetBackground(false, false);
                    CFont::SetWrapx(cardR - entryH * 0.1f);
                    CFont::SetColor(isSel    ? CRGBA(255, 255, 255, 255)
                                  : unlocked ? CRGBA(215, 215, 220, 255)
                                             : CRGBA(85,  85,  90,  255));
                    CFont::PrintString(textX, rowY + entryH * 0.12f, kAch[idx].label);

                    float dsc = SCREEN_MULTIPLIER(0.46f);
                    CFont::SetScale(dsc, dsc * 2.0f);
                    CFont::SetColor(unlocked ? CRGBA(140, 140, 150, 255) : CRGBA(58, 58, 62, 255));
                    CFont::SetWrapx(cardR - entryH * 0.1f);
                    CFont::PrintString(textX, rowY + entryH * 0.55f, kAch[idx].description);
                }

                {
                    bool backSel = (sel == kPageSize);
                    float bsc = SCREEN_MULTIPLIER(0.56f);
                    CFont::SetOrientation(ALIGN_CENTER);
                    CFont::SetScale(bsc, bsc * 2.0f);
                    CFont::SetWrapx(cardR);
                    CFont::SetColor(backSel ? CRGBA(255, 255, 255, 255) : CRGBA(155, 155, 160, 255));
                    CFont::PrintString(W * 0.5f, originY + totalH + entryGap * 2.0f, "BACK");
                }

                CSprite2d pageSprites[kPageSize] = {};
                if (g_achTxdSlot != -1) {
                    CTxdStore::PushCurrentTxd();
                    CTxdStore::SetCurrentTxd(g_achTxdSlot);
                    for (int i = 0; i < kPageSize; ++i) {
                        int idx = s_scroll + i;
                        if (idx < kAchCount)
                            pageSprites[i].SetTexture(const_cast<char*>(kAch[idx].icon));
                    }
                    CTxdStore::PopCurrentTxd();
                }

                for (int i = 0; i < kPageSize; ++i) {
                    int idx = s_scroll + i;
                    if (idx >= kAchCount) break;
                    bool unlocked = IsAchievementUnlocked(idx);
                    float rowY = entriesY + i * (entryH + entryGap);
                    float iy   = rowY + (entryH - iconW) * 0.5f;
                    CRect iconRect(cardL + iconPad, iy, cardL + iconPad + iconW, iy + iconW);
                    if (pageSprites[i].m_pTexture)
                        pageSprites[i].Draw(iconRect, unlocked ? CRGBA(255, 255, 255, 255) : CRGBA(55, 55, 60, 220));
                    else
                        CSprite2d::DrawRect(iconRect, unlocked ? CRGBA(212, 175, 55, 255) : CRGBA(42, 42, 48, 220));
                }

                return false;
            });
        };
    }
} gAchievementsMenuPlugin;
