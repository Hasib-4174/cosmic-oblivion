#include "include/ui.h"
#include "include/constants.h"
#include "include/helpers.h"
#include "include/stars.h"
#include "include/particles.h"
#include "include/button.h"
#include "include/ship.h"
#include "include/meteor.h"
#include "include/game.h"
#include "include/campaign.h"
#include "include/subship.h"
#include <math.h>
#include <string.h>

extern GameState G;

/* ---- Helper: play button hover sound (only on focus change) ---- */
static void PlayBtnHover(void)
{
    if (!G.audioEnabled)
        return;
    SetSoundVolume(G.sfxButtonHover, G.uiVolume);
    PlaySound(G.sfxButtonHover);
}

/* ---- Helper: play button select sound ---- */
static void PlayBtnSelect(void)
{
    if (!G.audioEnabled)
        return;
    SetSoundVolume(G.sfxButtonSelect, G.uiVolume);
    PlaySound(G.sfxButtonSelect);
}

/* ---- Draw audio toggle icon (top-right, every screen except logo) ---- */
void DrawAudioToggle(void)
{
    int x = SW - 60, y = 10;
    Color col = G.audioEnabled ? (Color){100, 255, 150, 200} : (Color){255, 80, 80, 200};
    const char *icon = G.audioEnabled ? "[ON]" : "[OFF]";
    /* Speaker body */
    DrawRectangle(x, y + 4, 10, 12, col);
    DrawTriangle((Vector2){x + 10, y + 2}, (Vector2){x + 10, y + 18}, (Vector2){x + 20, y + 22}, col);
    DrawTriangle((Vector2){x + 10, y + 2}, (Vector2){x + 20, y - 2}, (Vector2){x + 20, y + 22}, col);
    if (G.audioEnabled)
    {
        DrawCircleSectorLines((Vector2){x + 20, y + 10}, 10, -45, 45, 8, col);
        DrawCircleSectorLines((Vector2){x + 20, y + 10}, 16, -45, 45, 8, col);
    }
    else
    {
        DrawLine(x + 22, y, x + 36, y + 20, col);
        DrawLine(x + 36, y, x + 22, y + 20, col);
    }
    DrawText(icon, x - 2, y + 22, 10, col);
}

/* ---- Slider Drawing/Interaction Helper (polished) ---- */
static float DrawSlider(int x, int y, int w, float value, Color fillColor, bool selected, const char *label)
{
    /* Label on the left */
    DrawText(label, x, y + 2, 20, selected ? GOLD : WHITE);

    int sliderX = x + 250;
    int sliderW = w;
    int sliderH = 12;
    int sliderY = y + 6;

    /* Background track with rounded ends */
    DrawRectangleRounded((Rectangle){sliderX, sliderY, sliderW, sliderH}, 0.5f, 8, (Color){30, 30, 50, 220});

    /* Filled portion with gradient feel */
    int fillW = (int)(value * sliderW);
    if (fillW > 2)
    {
        Color fillDark = {(unsigned char)(fillColor.r * 0.6f), (unsigned char)(fillColor.g * 0.6f), (unsigned char)(fillColor.b * 0.6f), 255};
        (void)fillDark;
        DrawRectangleRounded((Rectangle){sliderX, sliderY, fillW, sliderH}, 0.5f, 8, fillColor);
        /* Inner highlight strip */
        if (fillW > 6)
            DrawRectangleRounded((Rectangle){sliderX + 2, sliderY + 2, fillW - 4, sliderH / 2 - 1}, 0.5f, 4,
                                 CAlpha(WHITE, 40));
    }

    /* Border — subtle glow when selected */
    if (selected)
    {
        DrawRectangleRounded((Rectangle){sliderX - 2, sliderY - 2, sliderW + 4, sliderH + 4}, 0.5f, 8,
                             CAlpha(GOLD, 40));
        DrawRectangleRoundedLinesEx((Rectangle){sliderX, sliderY, sliderW, sliderH}, 0.5f, 8, 1.5f, GOLD);
    }
    else
    {
        DrawRectangleRoundedLinesEx((Rectangle){sliderX, sliderY, sliderW, sliderH}, 0.5f, 8, 1,
                                   (Color){70, 80, 110, 180});
    }

    /* Knob */
    int knobX = sliderX + fillW;
    int knobY = sliderY + sliderH / 2;
    if (selected)
    {
        DrawCircleV((Vector2){knobX, knobY}, 10, CAlpha(GOLD, 60));
        DrawCircleV((Vector2){knobX, knobY}, 7, GOLD);
        DrawCircleV((Vector2){knobX, knobY}, 4, WHITE);
    }
    else
    {
        DrawCircleV((Vector2){knobX, knobY}, 7, (Color){160, 170, 200, 255});
        DrawCircleV((Vector2){knobX, knobY}, 4, WHITE);
    }

    /* Percentage text — clean monospaced look */
    int pct = (int)(value * 100);
    const char *pctText = TextFormat("%3d%%", pct);
    DrawText(pctText, sliderX + sliderW + 16, y + 2, 18, selected ? GOLD : (Color){160, 180, 200, 255});

    /* Mouse interaction */
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if (mouse.x >= sliderX && mouse.x <= sliderX + sliderW &&
            mouse.y >= sliderY - 12 && mouse.y <= sliderY + sliderH + 12)
        {
            value = Clampf((mouse.x - sliderX) / (float)sliderW, 0, 1);
        }
    }
    return value;
}

/* ---- Styled section title (replaces DrawTitle for settings screens) ---- */
static void DrawSettingsTitle(const char *title)
{
    float t = (float)GetTime();
    int tw = MeasureText(title, 48);
    float y = 60 + sinf(t * 1.5f) * 6;
    float hue = fmodf(t * 30, 360);
    Color c1 = ColorFromHSV(hue, 0.7f, 1.0f);
    Color c2 = ColorFromHSV(hue + 40, 0.6f, 1.0f);

    /* Shadow layers */
    DrawText(title, (int)(SW / 2 - tw / 2 + 2), (int)(y + 2), 48, CAlpha(c1, 40));
    DrawText(title, (int)(SW / 2 - tw / 2 - 1), (int)(y - 1), 48, CAlpha(c2, 60));
    /* Main text */
    DrawText(title, (int)(SW / 2 - tw / 2), (int)(y), 48, WHITE);

    /* Subtle underline decoration */
    float lineAlpha = 0.4f + 0.2f * sinf(t * 2.0f);
    int lineW = tw + 40;
    int lineX = SW / 2 - lineW / 2;
    int lineY = (int)(y + 56);
    DrawRectangle(lineX, lineY, lineW, 2, CAlpha(c1, (unsigned char)(lineAlpha * 255)));
    DrawRectangle(lineX + 10, lineY + 4, lineW - 20, 1, CAlpha(c2, (unsigned char)(lineAlpha * 150)));
}

void ScreenLogo(float dt)
{
    G.logoTimer += dt;
    BeginDrawing();
    ClearBackground(BLACK);
    float a = G.logoTimer < 1 ? G.logoTimer : (G.logoTimer < 2 ? 1 : Clampf(3 - G.logoTimer, 0, 1));
    const char *t = "COSMIC OBLIVION";
    int tw = MeasureText(t, 48);
    DrawText(t, (SW - tw) / 2, SH / 2 - 24, 48, CAlpha(WHITE, (unsigned char)(a * 255)));
    EndDrawing();
    if (G.logoTimer > 3)
    {
        G.screen = SCREEN_MAIN_MENU;
        if (G.audioEnabled)
        {
            PlayMusicStream(G.bgmMenu);
            SetMusicVolume(G.bgmMenu, G.bgmVolume);
        }
        G.menuBtns[0] = MkBtn(SW / 2 - 140, 300, 280, 50, "PLAY GAME");
        G.menuBtns[1] = MkBtn(SW / 2 - 140, 380, 280, 50, "OPTIONS");
        G.menuBtns[2] = MkBtn(SW / 2 - 140, 460, 280, 50, "EXIT");
        G.menuSel = 0;
        G.prevMenuSel = -1;
    }
}
void ScreenMenu(float dt)
{
    UpdateStars(dt);
    UpdateParticles(dt);
    if (GetRandomValue(0, 100) < 8)
        SpawnP((Vector2){Rf(0, SW), Rf(0, SH)}, CAlpha((Color){60, 100, 200, 255}, 120), 1, 20, 1.5f);

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.menuSel = (G.menuSel + 1) % 3;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.menuSel = (G.menuSel + 2) % 3;

    /* Update buttons and track mouse hover → selection */
    for (int i = 0; i < 3; i++)
    {
        UpdateBtn(&G.menuBtns[i], dt);
        if (G.menuBtns[i].hovered)
            G.menuSel = i;
    }

    /* Play hover sound on selection change */
    if (G.prevMenuSel >= 0 && G.prevMenuSel != G.menuSel)
        PlayBtnHover();
    G.prevMenuSel = G.menuSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 3; i++)
    {
        if (G.menuBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            G.menuSel = i;
            enter = true;
        }
    }
    if (enter)
    {
        PlayBtnSelect();
        if (G.menuSel == 0)
        {
            G.screen = SCREEN_MODE_SELECT;
            G.modeBtns[0] = MkBtn(SW / 2 - 140, 300, 280, 50, "CAMPAIGN");
            G.modeBtns[1] = MkBtn(SW / 2 - 140, 380, 280, 50, "ENDLESS");
            G.modeBtns[2] = MkBtn(SW / 2 - 140, 460, 280, 50, "BACK");
            G.modeSel = 0;
            G.prevModeSel = -1;
        }
        else if (G.menuSel == 1)
        {
            G.prevScreen = SCREEN_MAIN_MENU;
            G.screen = SCREEN_OPTIONS;
            G.optBtns[0] = MkBtn(SW / 2 - 140, 300, 280, 50, "AUDIO");
            G.optBtns[1] = MkBtn(SW / 2 - 140, 370, 280, 50, "FIRE MODE: HOLD");
            G.optBtns[2] = MkBtn(SW / 2 - 140, 440, 280, 50, "BACK");
            G.optSel = 0;
            G.prevOptSel = -1;
        }
        else if (G.menuSel == 2)
        {
            CloseWindow();
        }
    }
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawTitle((float)GetTime());
    for (int i = 0; i < 3; i++)
        DrawBtn(G.menuBtns[i], i == G.menuSel);
    DrawAudioToggle();
    EndDrawing();
}
void ScreenShipSelect(float dt)
{
    UpdateStars(dt);
    int oldSel = G.shipSel;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        G.shipSel = (G.shipSel + 2) % 3;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        G.shipSel = (G.shipSel + 1) % 3;

    /* Mouse hover on ship cards */
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 3; i++)
    {
        float cx = SW / 2 + (i - 1) * 280.0f;
        Rectangle card = {cx - 110, 160, 220, 380};
        if (CheckCollisionPointRec(mouse, card))
        {
            int reqLevel = (i == 1) ? 3 : (i == 2) ? 10 : 0;
            bool locked = GetMaxUnlockedLevel(&G) < reqLevel;
            
            if (G.shipSel != i)
                G.shipSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !locked)
            {
                PlayBtnSelect();
                G.selectedShip = G.shipSel;
                G.screen = SCREEN_WEAPON_SELECT;
                G.weaponSel = 0;
                G.prevWeaponSel = -1;
            }
        }
    }

    /* Play hover sound on ship change */
    if (G.shipSel != oldSel)
    {
        if (G.prevShipSel >= 0)
            PlayBtnHover();
        G.prevShipSel = G.shipSel;
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        int reqLevel = (G.shipSel == 1) ? 3 : (G.shipSel == 2) ? 10 : 0;
        bool locked = GetMaxUnlockedLevel(&G) < reqLevel;
        if (!locked)
        {
            PlayBtnSelect();
            G.selectedShip = G.shipSel;
            G.screen = SCREEN_WEAPON_SELECT;
            G.weaponSel = 0;
            G.prevWeaponSel = -1;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_MAIN_MENU;
        G.prevMenuSel = -1;
    }
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawText("SELECT YOUR SHIP", (SW - MeasureText("SELECT YOUR SHIP", 36)) / 2, 40, 36, WHITE);
    const char *names[3] = {"INTERCEPTOR", "DESTROYER", "TITAN"};
    const char *descs[3] = {"Fast & Agile\nHP: 3  Fire: Fast", "Balanced Fighter\nHP: 5  Fire: Med", "Heavy Tank\nHP: 8  Fire: Slow"};
    const char *stats[3] = {"SPD: *****  FIR: *****  HP: **", "SPD: ***  FIR: ***  HP: ****", "SPD: **  FIR: **  HP: ******"};
    float t = (float)GetTime();
    for (int i = 0; i < 3; i++)
    {
        float cx = SW / 2 + (i - 1) * 280;
        bool sel = i == G.shipSel;
        bool hov = CheckCollisionPointRec(mouse, (Rectangle){cx - 110, 160, 220, 380});
        Color bc = sel ? (Color){40, 80, 180, 200} : (hov ? (Color){30, 55, 110, 200} : (Color){20, 30, 50, 180});
        Rectangle card = {cx - 110, 160, 220, 380};
        DrawRectangleRounded(card, 0.1f, 8, bc);
        if (sel)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 3, (Color){100, 180, 255, 255});
        else if (hov)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 2, (Color){80, 130, 200, 200});
        else
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 1, (Color){60, 80, 120, 200});
            
        int reqLevel = (i == 1) ? 3 : (i == 2) ? 10 : 0;
        bool locked = GetMaxUnlockedLevel(&G) < reqLevel;
        
        if (locked)
        {
            DrawText("LOCKED", (int)(cx - MeasureText("LOCKED", 22) / 2), 300, 22, RED);
            const char *msg = TextFormat("Reach Sector %d-%d", (reqLevel / 10) + 1, (reqLevel % 10) + 1);
            if (reqLevel == 10) msg = "Clear Act I";
            DrawText(msg, (int)(cx - MeasureText(msg, 16) / 2), 340, 16, GRAY);
            DrawText("???", (int)(cx - MeasureText("???", 22) / 2), 400, 22, DARKGRAY);
            if (sel)
                DrawText("[ LOCKED ]", (int)(cx - MeasureText("[ LOCKED ]", 18) / 2), 520, 18, RED);
        }
        else
        {
            DrawShipShape((Vector2){cx, 300}, (ShipType)i, t * 3 + i, true);
            int nw = MeasureText(names[i], 22);
            DrawText(names[i], (int)(cx - nw / 2), 400, 22, WHITE);
            DrawText(stats[i], (int)(cx - 100), 435, 13, LIGHTGRAY);
            DrawText(descs[i], (int)(cx - 90), 460, 14, (Color){180, 200, 220, 255});
            if (sel)
            {
                DrawText("[ SELECTED ]", (int)(cx - MeasureText("[ SELECTED ]", 18) / 2), 520, 18, GOLD);
            }
        }
    }
    DrawText("< A/D or Arrows  |  Click or ENTER to confirm  |  ESC to go back >", (SW - MeasureText("< A/D or Arrows  |  Click or ENTER to confirm  |  ESC to go back >", 16)) / 2, SH - 40, 16, (Color){140, 160, 180, 255});
    DrawAudioToggle();
    EndDrawing();
}

/* ---- Weapon preview mini-animations ---- */
static void DrawWeaponPreview(Vector2 c, int wt, float t)
{
    if (wt == 0) /* LASER */
    {
        float f = fmodf(t * 4, 1.0f);
        float y = c.y + 40 - f * 100;
        DrawRectangle((int)c.x - 2, (int)y, 4, 14, (Color){180, 230, 255, 255});
        DrawRectangle((int)c.x - 1, (int)y + 2, 2, 10, WHITE);
    }
    else if (wt == 1) /* RAILGUN */
    {
        float f = fmodf(t * 2, 1.0f);
        if (f < 0.15f)
        {
            DrawRectangle((int)c.x - 8, (int)c.y - 70, 16, 110, CAlpha(WHITE, 200));
            DrawRectangle((int)c.x - 3, (int)c.y - 70, 6, 110, SKYBLUE);
        }
    }
    else if (wt == 2) /* FLAK */
    {
        float f = fmodf(t, 1.0f);
        if (f < 0.5f)
        {
            float y = c.y + 30 - f * 100;
            DrawCircle((int)c.x, (int)y, 6, ORANGE);
            DrawCircle((int)c.x, (int)y, 3, WHITE);
        }
        else
        {
            float e = f - 0.5f;
            DrawCircle((int)c.x, (int)c.y - 20, (int)(15 - e * 20), CAlpha(YELLOW, 180));
            for (int i = 0; i < 8; i++)
            {
                float a = i * 3.14159265f / 4.0f;
                DrawCircle((int)(c.x + cosf(a)*e*80), (int)(c.y - 20 + sinf(a)*e*80), 3, YELLOW);
            }
        }
    }
    else if (wt == 3) /* TESLA */
    {
        float f = fmodf(t * 3, 1.0f);
        Vector2 p1 = {c.x - 20, c.y + 20};
        Vector2 p2 = {c.x + 25, c.y - 5};
        Vector2 p3 = {c.x - 15, c.y - 35};
        DrawCircleV(p1, 6, RED); DrawCircleV(p2, 6, RED); DrawCircleV(p3, 6, RED);
        if (f < 0.6f)
        {
            p2.x += Rf(-4, 4); p2.y += Rf(-4, 4);
            p3.x += Rf(-4, 4); p3.y += Rf(-4, 4);
            DrawLineEx(p1, p2, 4.0f, SKYBLUE);
            DrawLineEx(p1, p2, 2.0f, WHITE);
            DrawLineEx(p2, p3, 4.0f, SKYBLUE);
            DrawLineEx(p2, p3, 2.0f, WHITE);
        }
    }
    else if (wt == 4) /* SINGULARITY */
    {
        DrawCircleGradient((int)c.x, (int)c.y - 10,
                           (int)(20 + sinf(t * 10) * 5),
                           CAlpha(PURPLE, 180), BLANK);
        DrawCircle((int)c.x, (int)c.y - 10, 8, BLACK);
        for (int i = 0; i < 3; i++)
        {
            float a = t * 5 + i * 3.14159265f * 2 / 3.0f;
            float d = 25 - fmodf(t * 20 + i * 15, 25.0f);
            DrawCircle((int)(c.x + cosf(a)*d), (int)(c.y - 10 + sinf(a)*d), 3, VIOLET);
        }
    }
    else if (wt == 5) /* WAVE */
    {
        float off = fmodf(t * 80, 100.0f);
        float y = c.y + 40 - off;
        float x = c.x + sinf(t * 15) * 20;
        DrawCircle((int)x, (int)y, 6, LIME);
        for (int i = 1; i < 5; i++)
        {
            float py = c.y + 40 - fmodf(t * 80 - i * 15, 100.0f);
            float px = c.x + sinf((t - i * 0.1f) * 15) * 20;
            if (py > c.y - 60) DrawCircle((int)px, (int)py, 4 - i, CAlpha(GREEN, 150));
        }
    }
}

void ScreenWeaponSelect(float dt)
{
    UpdateStars(dt);
    int oldSel = G.weaponSel;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        G.weaponSel = (G.weaponSel + 4) % 5;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        G.weaponSel = (G.weaponSel + 1) % 5;

    /* Mouse hover on weapon cards */
    int cw = 160, sp = 20;
    int tw2 = 5 * cw + 4 * sp;
    int sx = (SW - tw2) / 2;
    Vector2 mouseW = GetMousePosition();
    for (int i = 0; i < 5; i++)
    {
        int cx = sx + i * (cw + sp) + cw / 2;
        Rectangle card = {(float)(cx - cw / 2), 120, (float)cw, 310};
        if (CheckCollisionPointRec(mouseW, card))
        {
            int reqLevel = (i == 3) ? 15 : (i == 4) ? 20 : 0;
            bool locked = GetMaxUnlockedLevel(&G) < reqLevel;
            
            if (G.weaponSel != i)
                G.weaponSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !locked)
            {
                PlayBtnSelect();
                G.selectedWeapon = (WeaponType)(G.weaponSel + 1);
                G.screen = SCREEN_SUBSHIP_SELECT;
                G.podSel = 0;
                G.prevPodSel = -1;
            }
        }
    }

    if (G.weaponSel != oldSel)
    {
        if (G.prevWeaponSel >= 0) PlayBtnHover();
        G.prevWeaponSel = G.weaponSel;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        int reqLevel = (G.weaponSel == 3) ? 15 : (G.weaponSel == 4) ? 20 : 0;
        bool locked = GetMaxUnlockedLevel(&G) < reqLevel;
        if (!locked)
        {
            PlayBtnSelect();
            /* weaponSel 0..4 maps to WEAPON_RAILGUN(1)..WEAPON_WAVE(5) */
            G.selectedWeapon = (WeaponType)(G.weaponSel + 1);
            G.screen = SCREEN_SUBSHIP_SELECT;
            G.podSel = 0;
            G.prevPodSel = -1;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_SHIP_SELECT;
        G.prevShipSel = -1;
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawText("SELECT SPECIAL WEAPON",
             (SW - MeasureText("SELECT SPECIAL WEAPON", 36)) / 2, 30, 36, GOLD);
    DrawText("Laser is always available (SPACE)  |  Special weapon uses H key + Energy",
             (SW - MeasureText("Laser is always available (SPACE)  |  Special weapon uses H key + Energy", 14)) / 2,
             72, 14, (Color){160, 180, 200, 255});

    const char *names[5] = {"RAILGUN", "FLAK CANNON", "TESLA LINK",
                            "SINGULARITY", "WAVE BEAM"};
    const char *line1[5] = {"Instant piercing beam",
                            "Burst fragmentation",
                            "Chain lightning",
                            "Gravity pull bomb",
                            "Sine wave pattern"};
    const char *line2[5] = {"DMG: High  Rate: Slow",
                            "DMG: Med   Rate: Med",
                            "DMG: Low   Rate: Fast",
                            "DMG: Low   Rate: Slow",
                            "DMG: Med   Rate: Fast"};
    const int costs[5] = {40, 30, 25, 50, 20};

    float t = (float)GetTime();

    for (int i = 0; i < 5; i++)
    {
        int cx = sx + i * (cw + sp) + cw / 2;
        bool sel = (i == G.weaponSel);
        bool hov = CheckCollisionPointRec(mouseW, (Rectangle){(float)(cx - cw / 2), 120, (float)cw, 310});
        Color bc = sel ? (Color){40, 80, 180, 200} : (hov ? (Color){30, 55, 110, 200} : (Color){20, 30, 50, 180});
        Rectangle card = {(float)(cx - cw / 2), 120, (float)cw, 310};
        DrawRectangleRounded(card, 0.1f, 8, bc);
        if (sel)      DrawRectangleRoundedLinesEx(card, 0.1f, 8, 3, (Color){100, 180, 255, 255});
        else if (hov) DrawRectangleRoundedLinesEx(card, 0.1f, 8, 2, (Color){80, 130, 200, 200});
        else          DrawRectangleRoundedLinesEx(card, 0.1f, 8, 1, (Color){60, 80, 120, 200});

        int reqLevel = (i == 3) ? 15 : (i == 4) ? 20 : 0;
        bool locked = GetMaxUnlockedLevel(&G) < reqLevel;

        if (locked)
        {
            DrawText("LOCKED", cx - MeasureText("LOCKED", 18) / 2, 260, 18, RED);
            const char *msg = TextFormat("Reach Sector %d-%d", (reqLevel / 10) + 1, (reqLevel % 10) + 1);
            if (reqLevel == 20) msg = "Clear Act II";
            DrawText(msg, cx - MeasureText(msg, 12) / 2, 290, 12, GRAY);
            DrawText("???", cx - MeasureText("???", 18) / 2, 330, 18, DARKGRAY);
            if (sel)
                DrawText("[ LOCKED ]", cx - MeasureText("[ LOCKED ]", 14) / 2, 390, 14, RED);
        }
        else
        {
            /* Preview uses indices 1-5 matching WeaponType enum */
            DrawWeaponPreview((Vector2){(float)cx, 220}, i + 1, t);

            int nw = MeasureText(names[i], 18);
            DrawText(names[i], cx - nw / 2, 290, 18, WHITE);
            int l1w = MeasureText(line1[i], 12);
            DrawText(line1[i], cx - l1w / 2, 320, 12, LIGHTGRAY);
            int l2w = MeasureText(line2[i], 11);
            DrawText(line2[i], cx - l2w / 2, 340, 11, (Color){180, 200, 220, 255});
            const char *costStr = TextFormat("Energy: %d", costs[i]);
            int cfw = MeasureText(costStr, 13);
            DrawText(costStr, cx - cfw / 2, 362, 13, (Color){120, 160, 255, 255});
            if (sel) {
                int sw2 = MeasureText("[ SELECTED ]", 14);
                DrawText("[ SELECTED ]", cx - sw2 / 2, 390, 14, GOLD);
            }
        }
    }

    const char *hint = "< A/D or Arrows  |  Click or ENTER to confirm  |  ESC back >";
    DrawText(hint, (SW - MeasureText(hint, 16)) / 2, SH - 40, 16,
             (Color){140, 160, 180, 255});
    DrawAudioToggle();
    EndDrawing();
}

/* ---- Pod icon mini-drawing ---- */
static void DrawPodIcon(Vector2 c, int podType, float t)
{
    float pulse = 0.5f + 0.5f * sinf(t * 4.0f);
    if (podType == 0) /* FABRICATOR */
    {
        Color gc = (Color){80, 255, 120, 255};
        /* Green cross / heal symbol */
        DrawRectangle((int)c.x - 3, (int)c.y - 12, 6, 24, gc);
        DrawRectangle((int)c.x - 12, (int)c.y - 3, 24, 6, gc);
        DrawCircleGradient((int)c.x, (int)c.y, 18 + pulse * 6, CAlpha(gc, 40), BLANK);
        /* Orbiting glow */
        float a = t * 3.0f;
        DrawCircleV((Vector2){c.x + cosf(a) * 20, c.y + sinf(a) * 20}, 3, CAlpha(gc, 180));
        DrawCircleV((Vector2){c.x + cosf(a + 3.14f) * 20, c.y + sinf(a + 3.14f) * 20}, 3, CAlpha(gc, 180));
    }
    else if (podType == 1) /* SALVO */
    {
        Color rc = (Color){255, 160, 40, 255};
        /* Rocket shapes */
        for (int i = -1; i <= 1; i++)
        {
            float yOff = sinf(t * 5.0f + i * 1.5f) * 5.0f;
            int rx = (int)c.x + i * 12;
            int ry = (int)(c.y + yOff);
            DrawTriangle((Vector2){rx, ry - 10}, (Vector2){rx + 4, ry + 6}, (Vector2){rx - 4, ry + 6}, rc);
            DrawCircleV((Vector2){rx, ry + 8}, 3, CAlpha(ORANGE, (unsigned char)(150 + 100 * pulse)));
        }
        /* Exhaust glow */
        DrawCircleGradient((int)c.x, (int)c.y + 10, 15 + pulse * 4, CAlpha(ORANGE, 40), BLANK);
    }
    else /* SENTINEL */
    {
        Color sc = (Color){100, 180, 255, 255};
        /* EMP ring expanding */
        float ringSize = fmodf(t * 2.0f, 1.0f);
        DrawCircleLines((int)c.x, (int)c.y, 10 + ringSize * 20, CAlpha(sc, (unsigned char)(255 * (1.0f - ringSize))));
        DrawCircleLines((int)c.x, (int)c.y, 10 + fmodf(t * 2.0f + 0.5f, 1.0f) * 20,
                        CAlpha(sc, (unsigned char)(255 * (1.0f - fmodf(t * 2.0f + 0.5f, 1.0f)))));
        /* Center core */
        DrawCircleV(c, 8, (Color){30, 60, 100, 255});
        DrawCircleV(c, 5 + pulse * 2, sc);
        DrawCircleV(c, 3, WHITE);
    }
}

void ScreenSubShipSelect(float dt)
{
    UpdateStars(dt);
    int oldSel = G.podSel;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        G.podSel = (G.podSel + 2) % 3;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        G.podSel = (G.podSel + 1) % 3;

    /* Mouse hover on pod cards */
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 3; i++)
    {
        float cx = SW / 2 + (i - 1) * 280.0f;
        Rectangle card = {cx - 110, 160, 220, 380};
        if (CheckCollisionPointRec(mouse, card))
        {
            if (G.podSel != i)
                G.podSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                PlayBtnSelect();
                G.selectedPod = (PodType)G.podSel;
                if (G.isCampaignMode)
                {
                    G.screen = SCREEN_GAMEPLAY;
                    InitGame();
                    StopMusicStream(G.bgmMenu);
                    if (G.audioEnabled)
                    {
                        PlayMusicStream(G.bgmGameplay);
                        SetMusicVolume(G.bgmGameplay, G.bgmVolume);
                    }
                }
                else
                {
                    G.screen = SCREEN_DIFFICULTY_SELECT;
                    G.diffSel = 1;
                    G.prevDiffSel = -1;
                }
            }
        }
    }

    /* Play hover sound on selection change */
    if (G.podSel != oldSel)
    {
        if (G.prevPodSel >= 0)
            PlayBtnHover();
        G.prevPodSel = G.podSel;
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        PlayBtnSelect();
        G.selectedPod = (PodType)G.podSel;
        if (G.isCampaignMode)
        {
            G.screen = SCREEN_GAMEPLAY;
            InitGame();
            StopMusicStream(G.bgmMenu);
            if (G.audioEnabled)
            {
                PlayMusicStream(G.bgmGameplay);
                SetMusicVolume(G.bgmGameplay, G.bgmVolume);
            }
        }
        else
        {
            G.screen = SCREEN_DIFFICULTY_SELECT;
            G.diffSel = 1;
            G.prevDiffSel = -1;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_WEAPON_SELECT;
        G.prevWeaponSel = -1;
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawText("SELECT AUX POD",
             (SW - MeasureText("SELECT AUX POD", 36)) / 2, 40, 36, WHITE);
    DrawText("Choose a support drone to assist your ship in combat",
             (SW - MeasureText("Choose a support drone to assist your ship in combat", 14)) / 2,
             82, 14, (Color){160, 180, 200, 255});

    const char *names[3] = {"FABRICATOR", "SALVO", "SENTINEL"};
    const char *descs[3] = {
        "Auto-repair pod\nRestores HP or\nboosts shield",
        "Burst offense pod\nFires homing\nrocket salvos",
        "Defense pod\nEMP destroys\nnearby bullets"
    };
    const char *stats[3] = {
        "CD: 35s   HP: 5",
        "CD: 18s   HP: 4",
        "CD: 15s   HP: 3"
    };
    Color cardColors[3] = {
        {25, 80, 45, 200},
        {100, 50, 15, 200},
        {25, 60, 110, 200}
    };
    Color borderColors[3] = {
        {80, 255, 120, 255},
        {255, 160, 40, 255},
        {100, 180, 255, 255}
    };

    float t = (float)GetTime();
    for (int i = 0; i < 3; i++)
    {
        float cx = SW / 2 + (i - 1) * 280;
        bool sel = i == G.podSel;
        bool hov = CheckCollisionPointRec(mouse, (Rectangle){cx - 110, 160, 220, 380});
        Color bc = sel ? cardColors[i] : (hov ? (Color){30, 45, 80, 200} : (Color){20, 30, 50, 180});
        Rectangle card = {cx - 110, 160, 220, 380};
        DrawRectangleRounded(card, 0.1f, 8, bc);
        if (sel)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 3, borderColors[i]);
        else if (hov)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 2, (Color){80, 130, 200, 200});
        else
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 1, (Color){60, 80, 120, 200});

        /* Pod icon */
        DrawPodIcon((Vector2){cx, 280}, i, t);

        int nw = MeasureText(names[i], 22);
        DrawText(names[i], (int)(cx - nw / 2), 340, 22, WHITE);
        DrawText(stats[i], (int)(cx - MeasureText(stats[i], 13) / 2), 375, 13, LIGHTGRAY);
        DrawText(descs[i], (int)(cx - 90), 405, 14, (Color){180, 200, 220, 255});
        if (sel)
        {
            DrawText("[ SELECTED ]", (int)(cx - MeasureText("[ SELECTED ]", 18) / 2), 500, 18, GOLD);
        }
    }

    const char *hint = "< A/D or Arrows  |  Click or ENTER to confirm  |  ESC back >";
    DrawText(hint, (SW - MeasureText(hint, 16)) / 2, SH - 40, 16,
             (Color){140, 160, 180, 255});
    DrawAudioToggle();
    EndDrawing();
}

void ScreenPause(float dt)
{
    (void)dt;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.pauseSel = (G.pauseSel + 1) % 4;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.pauseSel = (G.pauseSel + 3) % 4;

    /* Update buttons and track mouse hover → selection */
    for (int i = 0; i < 4; i++)
    {
        UpdateBtn(&G.pauseBtns[i], dt);
        if (G.pauseBtns[i].hovered)
            G.pauseSel = i;
    }

    /* Play hover sound on focus change */
    if (G.prevPauseSel >= 0 && G.prevPauseSel != G.pauseSel)
        PlayBtnHover();
    G.prevPauseSel = G.pauseSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    bool escape = IsKeyPressed(KEY_ESCAPE);
    for (int i = 0; i < 4; i++)
    {
        if (G.pauseBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            G.pauseSel = i;
            enter = true;
        }
    }
    if (enter)
    {
        PlayBtnSelect();
        if (G.pauseSel == 0)
        {
            G.screen = SCREEN_GAMEPLAY;
        }
        else if (G.pauseSel == 1)
        {
            G.prevScreen = SCREEN_PAUSE;
            G.screen = SCREEN_OPTIONS;
            G.optBtns[0] = MkBtn(SW / 2 - 140, 300, 280, 50, "AUDIO");
            G.optBtns[1] = MkBtn(SW / 2 - 140, 370, 280, 50, "FIRE MODE: HOLD");
            G.optBtns[2] = MkBtn(SW / 2 - 140, 440, 280, 50, "BACK");
            G.optSel = 0;
            G.prevOptSel = -1;
        }
        else if (G.pauseSel == 2)
        {
            /* Return to main menu: stop gameplay BGM, start menu BGM */
            StopMusicStream(G.bgmGameplay);
            if (G.audioEnabled)
            {
                PlayMusicStream(G.bgmMenu);
                SetMusicVolume(G.bgmMenu, G.bgmVolume);
            }
            G.screen = SCREEN_MAIN_MENU;
            G.prevMenuSel = -1;
        }
        else if (G.pauseSel == 3)
            CloseWindow();
    }
    else if (escape)
    {
        G.screen = SCREEN_GAMEPLAY;
    }
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawParticles();
    for (int i = 0; i < MAX_METEORS; i++)
    {
        if (G.meteors[i].active)
            DrawMeteor(G.meteors[i]);
    }
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.bullets[i].active)
            continue;
        Vector2 bp = G.bullets[i].pos;
        DrawRectangle((int)bp.x - 2, (int)bp.y - 7, 4, 14, (Color){180, 230, 255, 255});
    }
    if (G.player.alive)
        DrawShipShape(G.player.pos, G.player.type, G.gameTime * 4, true);
    DrawRectangle(0, 0, SW, SH, (Color){0, 0, 0, 160});
    const char *pt = "PAUSED";
    int pw = MeasureText(pt, 50);
    DrawText(pt, (SW - pw) / 2, 180, 50, WHITE);
    for (int i = 0; i < 4; i++)
        DrawBtn(G.pauseBtns[i], i == G.pauseSel);
    DrawAudioToggle();
    EndDrawing();
}
void ScreenGameOver(float dt)
{
    UpdateStars(dt);
    UpdateParticles(dt);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.goSel = (G.goSel + 1) % 2;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.goSel = (G.goSel + 1) % 2;

    /* Update buttons and track mouse hover → selection */
    for (int i = 0; i < 2; i++)
    {
        UpdateBtn(&G.goBtns[i], dt);
        if (G.goBtns[i].hovered)
            G.goSel = i;
    }

    /* Play hover sound on focus change */
    if (G.prevGoSel >= 0 && G.prevGoSel != G.goSel)
        PlayBtnHover();
    G.prevGoSel = G.goSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 2; i++)
    {
        if (G.goBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            G.goSel = i;
            enter = true;
        }
    }
    if (enter)
    {
        PlayBtnSelect();
        if (G.goSel == 0)
        {
            /* Play again: stop gameover BGM, restart gameplay BGM */
            StopMusicStream(G.bgmGameover);
            if (G.audioEnabled)
            {
                PlayMusicStream(G.bgmGameplay);
                SetMusicVolume(G.bgmGameplay, G.bgmVolume);
            }
            InitGame();
            G.screen = SCREEN_GAMEPLAY;
        }
        else
        {
            /* Main menu: stop gameover, restart menu BGM */
            StopMusicStream(G.bgmGameover);
            if (G.audioEnabled)
            {
                PlayMusicStream(G.bgmMenu);
                SetMusicVolume(G.bgmMenu, G.bgmVolume);
            }
            G.screen = SCREEN_MAIN_MENU;
            G.prevMenuSel = -1;
        }
    }
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawParticles();
    float t = (float)GetTime();
    Color gc = ColorFromHSV(fmodf(t * 60, 360), 0.8f, 1);
    const char *go = "GAME OVER";
    int gw = MeasureText(go, 56);
    DrawText(go, (SW - gw) / 2 + 2, 142, 56, CAlpha(gc, 60));
    DrawText(go, (SW - gw) / 2, 140, 56, gc);
    DrawText(TextFormat("Score: %d", G.score), (SW - MeasureText(TextFormat("Score: %d", G.score), 30)) / 2, 240, 30, WHITE);
    DrawText(TextFormat("High Score: %d", G.highscore), (SW - MeasureText(TextFormat("High Score: %d", G.highscore), 24)) / 2, 280, 24, GOLD);
    DrawText(TextFormat("Meteors Destroyed: %d", G.meteorsDestroyed), (SW - MeasureText(TextFormat("Meteors Destroyed: %d", G.meteorsDestroyed), 20)) / 2, 320, 20, LIGHTGRAY);
    DrawText(TextFormat("Enemies Defeated: %d", G.enemiesDestroyed), (SW - MeasureText(TextFormat("Enemies Defeated: %d", G.enemiesDestroyed), 20)) / 2, 350, 20, RED);
    const char *diffNames[] = {"EASY", "NORMAL", "HARD"};
    Color diffColors[] = {{100, 220, 100, 255}, {220, 200, 80, 255}, {255, 80, 60, 255}};
    const char *diffLabel = TextFormat("Difficulty: %s", diffNames[G.difficulty]);
    DrawText(diffLabel, (SW - MeasureText(diffLabel, 20)) / 2, 380, 20, diffColors[G.difficulty]);
    for (int i = 0; i < 2; i++)
        DrawBtn(G.goBtns[i], i == G.goSel);
    DrawAudioToggle();
    EndDrawing();
}
void ScreenOptions(float dt)
{
    UpdateStars(dt);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.optSel = (G.optSel + 1) % 3;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.optSel = (G.optSel + 2) % 3;

    /* Update buttons and track mouse hover → selection */
    for (int i = 0; i < 3; i++)
    {
        /* Update text for fire mode button */
        if (i == 1)
            G.optBtns[i].text = (G.fireMode == FIRE_MODE_HOLD) ? "FIRE MODE: HOLD" : "FIRE MODE: TOGGLE";

        UpdateBtn(&G.optBtns[i], dt);
        if (G.optBtns[i].hovered)
            G.optSel = i;
    }

    /* Play hover sound on focus change */
    if (G.prevOptSel >= 0 && G.prevOptSel != G.optSel)
        PlayBtnHover();
    G.prevOptSel = G.optSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 3; i++)
    {
        if (G.optBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            G.optSel = i;
            enter = true;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = G.prevScreen;
        if (G.screen == SCREEN_MAIN_MENU) G.prevMenuSel = -1;
        else if (G.screen == SCREEN_PAUSE) G.prevPauseSel = -1;
    }
    if (enter)
    {
        PlayBtnSelect();
        if (G.optSel == 0)
        {
            G.screen = SCREEN_AUDIO;
            G.audioBackBtn = MkBtn(SW / 2 - 110, 0, 220, 50, "BACK");
            G.audioSel = 0;
            G.prevAudioSel = -1;
        }
        else if (G.optSel == 1)
        {
            G.fireMode = (G.fireMode == FIRE_MODE_HOLD) ? FIRE_MODE_TOGGLE : FIRE_MODE_HOLD;
        }
        else if (G.optSel == 2)
        {
            G.screen = G.prevScreen;
            if (G.screen == SCREEN_MAIN_MENU) G.prevMenuSel = -1;
            else if (G.screen == SCREEN_PAUSE) G.prevPauseSel = -1;
        }
    }
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawTitle((float)GetTime());
    for (int i = 0; i < 3; i++)
        DrawBtn(G.optBtns[i], i == G.optSel);
    DrawAudioToggle();
    EndDrawing();
}

void ScreenAudio(float dt)
{
    UpdateStars(dt);

    /* 3 sliders: BGM Volume, UI Volume, Gameplay Sound; item 3 = BACK */
    int numSliders = 3;
    int totalItems = numSliders + 1;

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.audioSel = (G.audioSel + 1) % totalItems;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.audioSel = (G.audioSel + totalItems - 1) % totalItems;

    /* Mouse hover on BACK button tracks selection */
    UpdateBtn(&G.audioBackBtn, dt);
    if (G.audioBackBtn.hovered)
        G.audioSel = numSliders;

    /* Play hover sound on focus change */
    if (G.prevAudioSel >= 0 && G.prevAudioSel != G.audioSel)
        PlayBtnHover();
    G.prevAudioSel = G.audioSel;

    /* Keyboard left/right for slider adjustment */
    if (G.audioSel < numSliders)
    {
        float *vol = 0;
        if (G.audioSel == 0)
            vol = &G.bgmVolume;
        else if (G.audioSel == 1)
            vol = &G.uiVolume;
        else if (G.audioSel == 2)
            vol = &G.gameplayVolume;

        if (vol)
        {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
                *vol = Clampf(*vol + 0.1f, 0, 1);
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
                *vol = Clampf(*vol - 0.1f, 0, 1);
        }
    }

    /* BACK button click */
    bool enter = IsKeyPressed(KEY_ENTER);
    if (G.audioBackBtn.hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        enter = true;

    if (IsKeyPressed(KEY_ESCAPE) || (enter && G.audioSel == numSliders))
    {
        if (enter)
            PlayBtnSelect();
        G.screen = SCREEN_OPTIONS;
        G.prevOptSel = -1;
    }

    /* Apply BGM volume in real-time */
    SetMusicVolume(G.bgmMenu, G.bgmVolume);
    SetMusicVolume(G.bgmGameplay, G.bgmVolume);
    SetMusicVolume(G.bgmGameover, G.bgmVolume);

    /* ---- Drawing ---- */
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();

    /* Animated "AUDIO SETTINGS" title (replaces COSMIC OBLIVION + section header) */
    DrawSettingsTitle("AUDIO SETTINGS");

    /* Slider layout — positioned below the animated title */
    int sliderStartY = 160;
    int rowSpacing = 70;
    int sliderX = SW / 2 - 230;
    int sliderW = 220;

    const char *labels[3] = {"BGM Volume", "UI Volume", "Gameplay Sound"};
    float *values[3] = {&G.bgmVolume, &G.uiVolume, &G.gameplayVolume};
    Color colors[3] = {
        {0, 200, 100, 255},
        {220, 180, 40, 255},
        {80, 160, 255, 255}};

    for (int i = 0; i < numSliders; i++)
    {
        int y = sliderStartY + i * rowSpacing;
        float newVal = DrawSlider(sliderX, y, sliderW, *values[i], colors[i], G.audioSel == i, labels[i]);
        *values[i] = newVal;
    }

    /* BACK button: position below sliders */
    int backY = sliderStartY + numSliders * rowSpacing + 30;
    G.audioBackBtn.rect.x = SW / 2 - 110;
    G.audioBackBtn.rect.y = backY;
    DrawBtn(G.audioBackBtn, G.audioSel == numSliders);

    /* Mouse hover on slider rows to select them */
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < numSliders; i++)
    {
        int y = sliderStartY + i * rowSpacing;
        if (mouse.y >= y - 5 && mouse.y <= y + 30 && mouse.x >= sliderX && mouse.x <= sliderX + 250 + sliderW + 60)
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                G.audioSel = i;
        }
    }

    DrawAudioToggle();
    EndDrawing();
}

void ScreenDifficultySelect(float dt)
{
    UpdateStars(dt);
    int oldSel = G.diffSel;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        G.diffSel = (G.diffSel + 2) % 3;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        G.diffSel = (G.diffSel + 1) % 3;

    /* Mouse hover on difficulty cards */
    Vector2 mouseD = GetMousePosition();
    for (int i = 0; i < 3; i++)
    {
        float cx = SW / 2 + (i - 1) * 280.0f;
        Rectangle card = {cx - 110, 140, 220, 380};
        if (CheckCollisionPointRec(mouseD, card))
        {
            if (G.diffSel != i)
                G.diffSel = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                PlayBtnSelect();
                G.difficulty = (DifficultyLevel)G.diffSel;
                StopMusicStream(G.bgmMenu);
                if (G.audioEnabled)
                {
                    PlayMusicStream(G.bgmGameplay);
                    SetMusicVolume(G.bgmGameplay, G.bgmVolume);
                }
                InitGame();
                G.screen = SCREEN_GAMEPLAY;
            }
        }
    }

    /* Play hover sound on selection change */
    if (G.diffSel != oldSel)
    {
        if (G.prevDiffSel >= 0)
            PlayBtnHover();
        G.prevDiffSel = G.diffSel;
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        PlayBtnSelect();
        G.difficulty = (DifficultyLevel)G.diffSel;
        StopMusicStream(G.bgmMenu);
        if (G.audioEnabled)
        {
            PlayMusicStream(G.bgmGameplay);
            SetMusicVolume(G.bgmGameplay, G.bgmVolume);
        }
        InitGame();
        G.screen = SCREEN_GAMEPLAY;
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_SUBSHIP_SELECT;
        G.prevPodSel = -1;
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawText("SELECT DIFFICULTY",
             (SW - MeasureText("SELECT DIFFICULTY", 36)) / 2, 40, 36, WHITE);
    DrawText("Affects enemy stats, spawn rates, and difficulty scaling",
             (SW - MeasureText("Affects enemy stats, spawn rates, and difficulty scaling", 14)) / 2,
             82, 14, (Color){160, 180, 200, 255});

    const char *names[3] = {"EASY", "NORMAL", "HARD"};
    const char *descs[3] = {
        "Relaxed pace\nSlower enemies\nFewer spawns",
        "Balanced challenge\nStandard speeds\nNormal spawns",
        "Intense action\nFaster enemies\nMore spawns"
    };
    const char *stats[3] = {
        "Meteors: Slow  Enemies: Few",
        "Meteors: Med   Enemies: Med",
        "Meteors: Fast  Enemies: Many"
    };
    Color cardColors[3] = {
        {30, 100, 50, 200},
        {50, 60, 120, 200},
        {120, 30, 30, 200}
    };
    Color borderColors[3] = {
        {100, 220, 100, 255},
        {100, 150, 255, 255},
        {255, 80, 60, 255}
    };

    float t = (float)GetTime();
    for (int i = 0; i < 3; i++)
    {
        float cx = SW / 2 + (i - 1) * 280;
        bool sel = i == G.diffSel;
        bool hov = CheckCollisionPointRec(mouseD, (Rectangle){cx - 110, 140, 220, 380});
        Color bc = sel ? cardColors[i] : (hov ? (Color){30, 45, 80, 200} : (Color){20, 30, 50, 180});
        Rectangle card = {cx - 110, 140, 220, 380};
        DrawRectangleRounded(card, 0.1f, 8, bc);
        if (sel)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 3, borderColors[i]);
        else if (hov)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 2, (Color){80, 130, 200, 200});
        else
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 1, (Color){60, 80, 120, 200});

        /* Difficulty icon: circles indicating intensity */
        int dots = i + 1;
        int dotY = 220;
        int dotSpacing = 24;
        int dotStartX = (int)cx - (dots - 1) * dotSpacing / 2;
        for (int d = 0; d < dots; d++)
        {
            float pulse = 0.7f + 0.3f * sinf(t * 3.0f + d * 1.5f);
            int dotX = dotStartX + d * dotSpacing;
            DrawCircle(dotX, dotY, (int)(10 * pulse), borderColors[i]);
            DrawCircle(dotX, dotY, 5, WHITE);
        }

        int nw = MeasureText(names[i], 26);
        DrawText(names[i], (int)(cx - nw / 2), 270, 26, WHITE);
        DrawText(stats[i], (int)(cx - 100), 310, 13, LIGHTGRAY);
        DrawText(descs[i], (int)(cx - 90), 345, 14, (Color){180, 200, 220, 255});
        if (sel)
        {
            DrawText("[ SELECTED ]", (int)(cx - MeasureText("[ SELECTED ]", 18) / 2), 480, 18, GOLD);
        }
    }
    DrawText("< A/D or Arrows  |  Click or ENTER to start  |  ESC back >",
             (SW - MeasureText("< A/D or Arrows  |  Click or ENTER to start  |  ESC back >", 16)) / 2,
             SH - 40, 16, (Color){140, 160, 180, 255});
    DrawAudioToggle();
    EndDrawing();
}

void ScreenModeSelect(float dt)
{
    UpdateStars(dt);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.modeSel = (G.modeSel + 1) % 3;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.modeSel = (G.modeSel + 2) % 3;

    for (int i = 0; i < 3; i++)
    {
        UpdateBtn(&G.modeBtns[i], dt);
        if (G.modeBtns[i].hovered) G.modeSel = i;
    }

    if (G.prevModeSel >= 0 && G.prevModeSel != G.modeSel)
        if (G.audioEnabled) { SetSoundVolume(G.sfxButtonHover, G.uiVolume); PlaySound(G.sfxButtonHover); }
    G.prevModeSel = G.modeSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 3; i++)
        if (G.modeBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { G.modeSel = i; enter = true; }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_MAIN_MENU;
        G.prevMenuSel = -1;
    }

    if (enter)
    {
        if (G.audioEnabled) { SetSoundVolume(G.sfxButtonSelect, G.uiVolume); PlaySound(G.sfxButtonSelect); }
        if (G.modeSel == 0)
        {
            G.isCampaignMode = true;
            /* Bypass difficulty select */
            G.screen = SCREEN_LEVEL_SELECT;
            G.actSel = 0;
            G.levelSel = 0;
            G.prevLevelSel = -1;
            /* Layout buttons: 10 levels + Back + Next Act */
            for (int i=0; i<10; i++) {
                int col = i % 5;
                int row = i / 5;
                G.levelBtns[i] = MkBtn(SW/2 - 260 + col * 110, 200 + row * 110, 80, 80, TextFormat("%d", i+1));
            }
            G.levelBtns[10] = MkBtn(SW/2 - 260, 450, 200, 50, "BACK");
            G.levelBtns[11] = MkBtn(SW/2 + 60, 450, 200, 50, "NEXT ACT");
        }
        else if (G.modeSel == 1)
        {
            G.isCampaignMode = false;
            G.screen = SCREEN_SHIP_SELECT;
            G.shipSel = G.selectedShip;
            G.prevShipSel = -1;
        }
        else if (G.modeSel == 2)
        {
            G.screen = SCREEN_MAIN_MENU;
            G.prevMenuSel = -1;
        }
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawTitle((float)GetTime());
    DrawText("SELECT GAME MODE", (SW - MeasureText("SELECT GAME MODE", 30)) / 2, 220, 30, GOLD);
    
    for (int i = 0; i < 3; i++)
        DrawBtn(G.modeBtns[i], i == G.modeSel);
        
    DrawAudioToggle();
    EndDrawing();
}

void ScreenLevelSelect(float dt)
{
    UpdateStars(dt);
    
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) G.levelSel = (G.levelSel + 1) % 12;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) G.levelSel = (G.levelSel + 11) % 12;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) 
    {
        if (G.levelSel < 5) {
            if (G.levelSel < 3) G.levelSel = 10;
            else G.levelSel = 11;
        }
        else if (G.levelSel < 10) G.levelSel -= 5;
        else if (G.levelSel == 10) G.levelSel = 5;
        else if (G.levelSel == 11) G.levelSel = 9;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) 
    {
        if (G.levelSel < 5) G.levelSel += 5;
        else if (G.levelSel < 10) {
            if (G.levelSel < 8) G.levelSel = 10;
            else G.levelSel = 11;
        }
        else if (G.levelSel == 10) G.levelSel = 0;
        else if (G.levelSel == 11) G.levelSel = 4;
    }

    for (int i = 0; i < 12; i++)
    {
        UpdateBtn(&G.levelBtns[i], dt);
        if (G.levelBtns[i].hovered) G.levelSel = i;
    }

    if (G.prevLevelSel >= 0 && G.prevLevelSel != G.levelSel)
        if (G.audioEnabled) { SetSoundVolume(G.sfxButtonHover, G.uiVolume); PlaySound(G.sfxButtonHover); }
    G.prevLevelSel = G.levelSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 12; i++)
        if (G.levelBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { G.levelSel = i; enter = true; }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_MODE_SELECT;
        G.prevModeSel = -1;
    }

    if (enter)
    {
        if (G.audioEnabled) { SetSoundVolume(G.sfxButtonSelect, G.uiVolume); PlaySound(G.sfxButtonSelect); }
        if (G.levelSel < 10)
        {
            int displayLevel = G.actSel * 10 + G.levelSel;
            /* Check if unlocked using unified normal difficulty slot */
            if (displayLevel <= GetMaxUnlockedLevel(&G))
            {
                G.campaignState.currentLevel = displayLevel;
                /* Enforce Act-locked difficulty */
                if (G.actSel == 0) G.difficulty = DIFF_EASY;
                else if (G.actSel == 1) G.difficulty = DIFF_NORMAL;
                else G.difficulty = DIFF_HARD;

                /* Trigger narrative for act start OR boss levels */
                if (G.levelSel == 0 || G.levelSel == 9)
                {
                    G.screen = SCREEN_NARRATIVE;
                    G.campaignState.narrativeBeat = 0;
                    G.campaignState.narrativeTimer = 0;
                    G.campaignState.isBossNarrative = (G.levelSel == 9);
                    G.narBtns[0] = MkBtn(SW/2 - 100, SH - 100, 200, 50, "CONTINUE");
                }
                else
                {
                    G.screen = SCREEN_SHIP_SELECT;
                    G.shipSel = G.selectedShip;
                    G.prevShipSel = -1;
                }
            }
        }
        else if (G.levelSel == 10)
        {
            G.screen = SCREEN_MODE_SELECT;
            G.prevModeSel = -1;
        }
        else if (G.levelSel == 11)
        {
            G.actSel = (G.actSel + 1) % 3;
            /* Update level button texts for new act */
        }
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    
    /* === Title === */
    const char* actNames[3] = {"ACT I: THE FRINGE", "ACT II: CORE WORLDS", "ACT III: OBLIVION"};
    Color actColors[3] = {{100, 220, 255, 255}, {180, 100, 255, 255}, {255, 80, 80, 255}};
    const char* actTitle = actNames[G.actSel];
    DrawText(actTitle, (SW - MeasureText(actTitle, 36)) / 2, 60, 36, actColors[G.actSel]);
    DrawText("SELECT LEVEL", (SW - MeasureText("SELECT LEVEL", 20)) / 2, 108, 20, (Color){160, 170, 190, 255});

    /* === Level Buttons Grid === */
    for (int i = 0; i < 12; i++)
    {
        if (i >= 10)
        {
            /* BACK / NEXT ACT buttons */
            DrawBtn(G.levelBtns[i], i == G.levelSel);
            continue;
        }

        int displayLevel = G.actSel * 10 + i;
        bool locked = (displayLevel > GetMaxUnlockedLevel(&G));
        bool isBoss = (i == 9);

        Button b = G.levelBtns[i];
        bool sel = (i == G.levelSel);
        bool hov = b.hovered;

        if (locked)
        {
            /* Dark locked card */
            DrawRectangleRounded(b.rect, 0.2f, 8, (Color){40, 40, 55, 210});
            if (sel)
            {
                /* Draw a distinct outline if selected but locked */
                DrawRectangleRoundedLinesEx(b.rect, 0.2f, 8, 3.0f, (Color){150, 150, 150, 255});
            }
            else
            {
                DrawRectangleRoundedLinesEx(b.rect, 0.2f, 8, 1, (Color){70, 70, 90, 200});
            }
            
            int lx = (int)(b.rect.x + b.rect.width/2 - MeasureText("\xe2\x98\x93", 14)/2);
            DrawText("[X]", (int)(b.rect.x + b.rect.width/2 - MeasureText("[X]", 12)/2),
                     (int)(b.rect.y + b.rect.height/2 - 6), 12, (Color){100, 100, 120, 200});
            (void)lx;
        }
        else
        {
            /* Unlocked card color */
            Color cardCol = sel   ? (isBoss ? (Color){160, 20, 20, 220} : (Color){40, 80, 170, 220}) :
                            hov   ? (isBoss ? (Color){120, 20, 20, 200} : (Color){30, 55, 110, 200}) :
                                    (isBoss ? (Color){80, 15, 15, 190}  : (Color){20, 30, 60, 190});
            Color borderCol = sel ? (isBoss ? RED : (Color){100, 180, 255, 255}) :
                              hov ? (isBoss ? (Color){200, 60, 60, 200} : (Color){80, 130, 200, 200}) :
                                    (isBoss ? (Color){160, 40, 40, 180} : (Color){50, 70, 110, 180});

            DrawRectangleRounded(b.rect, 0.2f, 8, cardCol);
            DrawRectangleRoundedLinesEx(b.rect, 0.2f, 8, sel ? 3.0f : 1.5f, borderCol);

            /* Level number */
            const char *numStr = TextFormat("%d", i + 1);
            int nw = MeasureText(numStr, 20);
            Color numCol = isBoss ? (Color){255, 80, 80, 255} : WHITE;
            DrawText(numStr, (int)(b.rect.x + b.rect.width/2 - nw/2),
                     (int)(b.rect.y + b.rect.height/2 - (isBoss ? 14 : 10)), 20, numCol);

            /* Boss indicator */
            if (isBoss)
            {
                const char *bs = "!!";
                DrawText(bs, (int)(b.rect.x + b.rect.width/2 - MeasureText(bs, 11)/2),
                         (int)(b.rect.y + b.rect.height - 20), 11, (Color){255, 100, 100, 220});
            }
        }
    }

    /* === Selected Level Info Panel === */
    if (G.levelSel < 10)
    {
        int displayLevel = G.actSel * 10 + G.levelSel;
        LevelData ld = GetLevelData(displayLevel);
        bool locked = (displayLevel > GetMaxUnlockedLevel(&G));

        int px = SW/2 - 250, py = 530, pw = 500, ph = 85;
        DrawRectangleRounded((Rectangle){(float)px, (float)py, (float)pw, (float)ph}, 0.12f, 8,
                             (Color){15, 20, 45, 220});
        DrawRectangleRoundedLinesEx((Rectangle){(float)px, (float)py, (float)pw, (float)ph}, 0.12f, 8,
                                    1.5f, (Color){60, 80, 130, 200});

        if (locked)
        {
            DrawText("LOCKED", px + pw/2 - MeasureText("LOCKED", 22)/2, py + 12, 22, (Color){180, 60, 60, 255});
            DrawText("Complete previous levels to unlock.",
                     px + pw/2 - MeasureText("Complete previous levels to unlock.", 14)/2, py + 46, 14, GRAY);
        }
        else
        {
            DrawText(ld.title, px + 16, py + 10, 20, ld.isBossLevel ? (Color){255, 100, 80, 255} : GOLD);
            DrawText(ld.description, px + 16, py + 38, 15, (Color){180, 200, 222, 255});

            /* Objective summary line */
            const char *objStr = "";
            if (ld.isBossLevel)                   objStr = TextFormat("Defeat %d enemies, then destroy the Sector Commander!", ld.targetEnemies);
            else if (ld.duration > 0)              objStr = TextFormat("Survive for %.0f seconds", ld.duration);
            else if (ld.targetEnemies > 0)         objStr = TextFormat("Defeat %d enemies", ld.targetEnemies);
            else if (ld.targetMeteors > 0)         objStr = TextFormat("Destroy %d meteors", ld.targetMeteors);
            DrawText(objStr, px + 16, py + 60, 13, (Color){140, 175, 215, 255});
        }
    }

    /* === Act navigation hint === */
    DrawText("< ESC: Mode Select  |  WASD/Arrows: Navigate  |  ENTER/Click: Launch >",
             (SW - MeasureText("< ESC: Mode Select  |  WASD/Arrows: Navigate  |  ENTER/Click: Launch >", 14)) / 2,
             SH - 35, 14, (Color){120, 140, 170, 200});

    DrawAudioToggle();
    EndDrawing();
}

void ScreenNarrative(float dt)
{
    UpdateStars(dt);
    
    int actIdx = G.campaignState.currentLevel / 10;
    ActData act = GetActData(actIdx);
    int beatIdx = G.campaignState.narrativeBeat;
    int maxBeats = G.campaignState.isBossNarrative ? act.bossBeatCount : act.beatCount;
    
    if (beatIdx >= maxBeats)
    {
        /* Finished all beats */
        G.screen = SCREEN_SHIP_SELECT;
        G.shipSel = G.selectedShip;
        G.prevShipSel = -1;
        return;
    }
    
    StoryBeat beat = G.campaignState.isBossNarrative ? act.bossBeats[beatIdx] : act.beats[beatIdx];
    
    /* Update typewriter */
    G.campaignState.narrativeTimer += dt * 30.0f; /* 30 chars per second */
    int charCount = (int)G.campaignState.narrativeTimer;
    int len = (int)strlen(beat.text);
    if (charCount > len) charCount = len;
    
    UpdateBtn(&G.narBtns[0], dt);
    
    bool next = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || (G.narBtns[0].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
    
    if (next)
    {
        if (charCount < len)
        {
            /* Skip typewriter */
            G.campaignState.narrativeTimer = (float)len;
        }
        else
        {
            /* Move to next beat */
            if (G.audioEnabled) { SetSoundVolume(G.sfxButtonSelect, G.uiVolume); PlaySound(G.sfxButtonSelect); }
            G.campaignState.narrativeBeat++;
            G.campaignState.narrativeTimer = 0;
            
            if (G.campaignState.narrativeBeat >= maxBeats)
            {
                G.screen = SCREEN_SHIP_SELECT;
                G.shipSel = G.selectedShip;
                G.prevShipSel = -1;
            }
        }
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    
    /* Draw Comms UI */
    float pad = 60.0f;
    Rectangle frame = {pad, SH - 280, SW - pad * 2, 220};
    DrawRectangleRounded(frame, 0.1f, 8, CAlpha(DARKBLUE, 100));
    DrawRectangleRoundedLinesEx(frame, 0.1f, 8, 2.0f, SKYBLUE);
    
    /* Scanline effect in frame */
    for (int i = 0; i < (int)frame.height; i += 4)
    {
        DrawLine((int)frame.x, (int)(frame.y + i), (int)(frame.x + frame.width), (int)(frame.y + i), CAlpha(SKYBLUE, 20));
    }

    /* Portrait Area */
    Rectangle portRect = {frame.x + 20, frame.y + 20, 180, 180};
    DrawRectangleRec(portRect, (Color){10, 20, 40, 255});
    DrawRectangleLinesEx(portRect, 2.0f, (Color){40, 80, 120, 255});
    
    /* Draw Character Portrait */
    Vector2 portCenter = {portRect.x + portRect.width / 2, portRect.y + portRect.height / 2};
    
    /* Internal static call */
    void DrawCharacterPortrait(Vector2 center, StoryPortrait p);
    DrawCharacterPortrait(portCenter, beat.portrait);

    /* Name and Text */
    DrawText(beat.name, (int)frame.x + 220, (int)frame.y + 30, 24, GOLD);
    
    char visibleText[512];
    strncpy(visibleText, beat.text, charCount);
    visibleText[charCount] = '\0';
    
    /* Wrap text manually or use DrawTextRec if available in raylib version */
    /* For now, simple DrawText with a bit of wrapping logic if needed, but let's stick to simple for now */
    DrawText(visibleText, (int)frame.x + 220, (int)frame.y + 70, 20, WHITE);
    
    /* Prompt to continue */
    if (charCount >= len)
    {
        float pulse = 0.5f + 0.5f * sinf((float)GetTime() * 5.0f);
        DrawText("PRESS ENTER TO CONTINUE", (int)frame.x + (int)frame.width - 250, (int)frame.y + (int)frame.height - 30, 16, CAlpha(SKYBLUE, (unsigned char)(200 * pulse)));
    }
    
    /* Act Title centered at top */
    DrawText(act.actName, (SW - MeasureText(act.actName, 32)) / 2, 50, 32, CAlpha(GOLD, 180));

    EndDrawing();
}

void ScreenLevelComplete(float dt)
{
    UpdateStars(dt);
    
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) G.lcSel = (G.lcSel + 1) % 3;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) G.lcSel = (G.lcSel + 2) % 3;

    for (int i = 0; i < 3; i++)
    {
        UpdateBtn(&G.lcBtns[i], dt);
        if (G.lcBtns[i].hovered) G.lcSel = i;
    }

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 3; i++)
        if (G.lcBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { G.lcSel = i; enter = true; }

    if (enter)
    {
        if (G.audioEnabled) { SetSoundVolume(G.sfxButtonSelect, G.uiVolume); PlaySound(G.sfxButtonSelect); }
        if (G.lcSel == 0)
        {
            /* NEXT LEVEL — advance and reset boss flag */
            G.campaignState.bossDefeated = false;
            G.campaignState.currentLevel++;
            if (G.campaignState.currentLevel >= 30)
                G.campaignState.currentLevel = 29; /* cap at 29 */
            if (G.campaignState.currentLevel > G.campaignState.unlockedLevels[DIFF_NORMAL])
            {
                G.campaignState.unlockedLevels[DIFF_NORMAL] = G.campaignState.currentLevel;
                SaveCampaignProgress(&G);
            }
            G.screen = SCREEN_SHIP_SELECT;
            G.shipSel = G.selectedShip;
            G.prevShipSel = -1;
        }
        else if (G.lcSel == 1)
        {
            /* LEVEL SELECT */
            G.screen = SCREEN_LEVEL_SELECT;
            G.prevLevelSel = -1;
        }
        else
        {
            /* MAIN MENU */
            G.screen = SCREEN_MAIN_MENU;
            G.prevMenuSel = -1;
        }
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    
    LevelData lcd = GetLevelData(G.campaignState.currentLevel);
    float t = (float)GetTime();
    
    /* Animated glowing title like Game Over */
    Color titleCol = ColorFromHSV(fmodf(t * 60, 360), 0.8f, 1);
    const char *title = "LEVEL COMPLETE!";
    int tw = MeasureText(title, 56);
    DrawText(title, (SW - tw) / 2 + 2, 112, 56, CAlpha(titleCol, 60));
    DrawText(title, (SW - tw) / 2, 110, 56, titleCol);
    
    /* Level name subtitle */
    DrawText(lcd.title, (SW - MeasureText(lcd.title, 26)) / 2, 180, 26, (Color){150, 220, 255, 255});
    
    /* Stats similar to Game Over centering */
    DrawText(TextFormat("Score: %d", G.score), (SW - MeasureText(TextFormat("Score: %d", G.score), 30)) / 2, 240, 30, WHITE);
    DrawText(TextFormat("Meteors Destroyed: %d", G.meteorsDestroyed), (SW - MeasureText(TextFormat("Meteors Destroyed: %d", G.meteorsDestroyed), 20)) / 2, 280, 20, LIGHTGRAY);
    DrawText(TextFormat("Enemies Defeated: %d", G.enemiesDestroyed), (SW - MeasureText(TextFormat("Enemies Defeated: %d", G.enemiesDestroyed), 20)) / 2, 310, 20, RED);
    
    for (int i = 0; i < 3; i++)
        DrawBtn(G.lcBtns[i], i == G.lcSel);
        
    DrawAudioToggle();
    EndDrawing();
}

void DrawCharacterPortrait(Vector2 center, StoryPortrait p)
{
    float t = (float)GetTime();
    
    if (p == PORTRAIT_NONE) return;

    if (p == PORTRAIT_VANCE)
    {
        /* Commander Vance - Stern, tactical */
        /* Face Shape */
        DrawRectangleV((Vector2){center.x - 40, center.y - 50}, (Vector2){80, 100}, (Color){60, 60, 70, 255});
        DrawRectangleLinesEx((Rectangle){center.x - 40, center.y - 50, 80, 100}, 2, DARKGRAY);
        
        /* Visor/Eyes */
        DrawRectangleV((Vector2){center.x - 35, center.y - 25}, (Vector2){70, 15}, (Color){20, 20, 30, 255});
        DrawRectangleV((Vector2){center.x - 30, center.y - 22}, (Vector2){60, 8}, (Color){0, 120, 255, 200});
        
        /* Helmet details */
        DrawRectangleV((Vector2){center.x - 45, center.y - 60}, (Vector2){90, 20}, (Color){40, 40, 50, 255});
        DrawRectangleV((Vector2){center.x - 10, center.y - 70}, (Vector2){20, 15}, GOLD);
    }
    else if (p == PORTRAIT_KAEL)
    {
        /* Operator Kael - Technical, helpful */
        /* Face Shape */
        DrawCircleV(center, 50, (Color){50, 70, 90, 255});
        DrawCircleLines((int)center.x, (int)center.y, 50, SKYBLUE);
        
        /* Virtual Interface/Eyes */
        float scan = sinf(t * 3.0f) * 10.0f;
        DrawRectangleV((Vector2){center.x - 40, center.y - 15 + scan}, (Vector2){80, 2}, CAlpha(SKYBLUE, 150));
        DrawCircleV((Vector2){center.x - 20, center.y - 10}, 8, (Color){10, 20, 40, 255});
        DrawCircleV((Vector2){center.x + 20, center.y - 10}, 8, (Color){10, 20, 40, 255});
        DrawCircleV((Vector2){center.x - 20, center.y - 10}, 4, SKYBLUE);
        DrawCircleV((Vector2){center.x + 20, center.y - 10}, 4, SKYBLUE);
        
        /* Headset */
        DrawRectangleV((Vector2){center.x - 55, center.y - 20}, (Vector2){10, 40}, DARKGRAY);
        DrawLineEx((Vector2){center.x - 55, center.y}, (Vector2){center.x - 10, center.y + 30}, 2.0f, GRAY);
    }
    else if (p == PORTRAIT_ENEMY)
    {
        /* Unknown Enemy - Sinister, distorted */
        /* Glitchy silhouette */
        for (int i = 0; i < 5; i++)
        {
            float offX = Rf(-5.0f, 5.0f);
            float offY = Rf(-5.0f, 5.0f);
            DrawRectangleV((Vector2){center.x - 40 + offX, center.y - 60 + offY}, (Vector2){80, 120}, CAlpha(BLACK, 180));
        }
        
        /* Piercing red eyes */
        float flicker = 0.5f + 0.5f * sinf(t * 20.0f);
        DrawCircleV((Vector2){center.x - 25, center.y - 20}, 6, CAlpha(RED, (unsigned char)(255 * flicker)));
        DrawCircleV((Vector2){center.x + 25, center.y - 20}, 6, CAlpha(RED, (unsigned char)(255 * flicker)));
        
        /* Static effect */
        for (int i = 0; i < 20; i++)
        {
            DrawPixel((int)(center.x + Rf(-40, 40)), (int)(center.y + Rf(-60, 60)), CAlpha(PURPLE, 100));
        }
    }
}
