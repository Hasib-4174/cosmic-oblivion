#include "include/ui.h"
#include "include/constants.h"
#include "include/helpers.h"
#include "include/stars.h"
#include "include/particles.h"
#include "include/button.h"
#include "include/ship.h"
#include "include/meteor.h"
#include "include/game.h"
#include <math.h>

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
        G.menuBtns[0] = MkBtn(SW / 2 - 110, 300, 220, 50, "PLAY GAME");
        G.menuBtns[1] = MkBtn(SW / 2 - 110, 390, 220, 50, "OPTIONS");
        G.menuBtns[2] = MkBtn(SW / 2 - 110, 480, 220, 50, "EXIT");
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
            G.screen = SCREEN_SHIP_SELECT;
            G.shipSel = G.selectedShip;
            G.prevShipSel = -1;
        }
        else if (G.menuSel == 1)
        {
            G.screen = SCREEN_OPTIONS;
            G.optBtns[0] = MkBtn(SW / 2 - 110, 320, 220, 50, "AUDIO");
            G.optBtns[1] = MkBtn(SW / 2 - 110, 390, 220, 50, "BACK");
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
    DrawParticles();
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

    /* Play hover sound on ship change */
    if (G.shipSel != oldSel)
    {
        if (G.prevShipSel >= 0)
            PlayBtnHover();
        G.prevShipSel = G.shipSel;
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        PlayBtnSelect();
        G.selectedShip = G.shipSel;
        G.screen = SCREEN_WEAPON_SELECT;
        G.weaponSel = 0;
        G.prevWeaponSel = -1;
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
        Color bc = sel ? (Color){40, 80, 180, 200} : (Color){20, 30, 50, 180};
        Rectangle card = {cx - 110, 160, 220, 380};
        DrawRectangleRounded(card, 0.1f, 8, bc);
        if (sel)
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 3, (Color){100, 180, 255, 255});
        else
            DrawRectangleRoundedLinesEx(card, 0.1f, 8, 1, (Color){60, 80, 120, 200});
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
    DrawText("< A/D or Arrows to browse  |  ENTER to confirm  |  ESC to go back >", (SW - MeasureText("< A/D or Arrows to browse  |  ENTER to confirm  |  ESC to go back >", 16)) / 2, SH - 40, 16, (Color){140, 160, 180, 255});
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
        G.weaponSel = (G.weaponSel + 5) % 6;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        G.weaponSel = (G.weaponSel + 1) % 6;
    if (G.weaponSel != oldSel)
    {
        if (G.prevWeaponSel >= 0) PlayBtnHover();
        G.prevWeaponSel = G.weaponSel;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        PlayBtnSelect();
        G.selectedWeapon = (WeaponType)G.weaponSel;
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
        G.screen = SCREEN_SHIP_SELECT;
        G.prevShipSel = -1;
    }

    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawText("SELECT WEAPON SYSTEM",
             (SW - MeasureText("SELECT WEAPON SYSTEM", 36)) / 2, 40, 36, GOLD);

    const char *names[6] = {"STANDARD LASER", "RAILGUN", "FLAK CANNON",
                            "TESLA LINK", "SINGULARITY", "WAVE BEAM"};
    const char *line1[6] = {"Rapid-fire energy bolts",
                            "Instant piercing beam",
                            "Burst fragmentation",
                            "Chain lightning",
                            "Gravity pull bomb",
                            "Sine wave pattern"};
    const char *line2[6] = {"DMG: Med   Rate: Ship-dep.",
                            "DMG: High  Rate: Slow",
                            "DMG: Med   Rate: Med",
                            "DMG: Low   Rate: Fast",
                            "DMG: Low   Rate: Slow",
                            "DMG: Med   Rate: Fast"};

    float t = (float)GetTime();
    int cw = 140, sp = 15;
    int tw = 6 * cw + 5 * sp;
    int sx = (SW - tw) / 2;

    for (int i = 0; i < 6; i++)
    {
        int cx = sx + i * (cw + sp) + cw / 2;
        bool sel = (i == G.weaponSel);
        Color bc = sel ? (Color){40, 80, 180, 200} : (Color){20, 30, 50, 180};
        Rectangle card = {(float)(cx - cw / 2), 160, (float)cw, 260};
        DrawRectangleRounded(card, 0.1f, 8, bc);
        if (sel) DrawRectangleRoundedLinesEx(card, 0.1f, 8, 3, (Color){100, 180, 255, 255});
        else     DrawRectangleRoundedLinesEx(card, 0.1f, 8, 1, (Color){60, 80, 120, 200});

        DrawWeaponPreview((Vector2){(float)cx, 240}, i, t);

        int nw = MeasureText(names[i], 16);
        DrawText(names[i], cx - nw / 2, 320, 16, WHITE);
        int l1w = MeasureText(line1[i], 12);
        DrawText(line1[i], cx - l1w / 2, 350, 12, LIGHTGRAY);
        int l2w = MeasureText(line2[i], 11);
        DrawText(line2[i], cx - l2w / 2, 370, 11, (Color){180, 200, 220, 255});
        if (sel) {
            int sw2 = MeasureText("[ SELECTED ]", 14);
            DrawText("[ SELECTED ]", cx - sw2 / 2, 400, 14, GOLD);
        }
    }

    const char *hint = "< A/D or Arrows  |  ENTER to start  |  ESC back >";
    DrawText(hint, (SW - MeasureText(hint, 16)) / 2, SH - 40, 16,
             (Color){140, 160, 180, 255});
    DrawAudioToggle();
    EndDrawing();
}

void ScreenPause(float dt)
{
    (void)dt;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.pauseSel = (G.pauseSel + 1) % 3;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.pauseSel = (G.pauseSel + 2) % 3;

    /* Update buttons and track mouse hover → selection */
    for (int i = 0; i < 3; i++)
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
    for (int i = 0; i < 3; i++)
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
        else
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
    for (int i = 0; i < 3; i++)
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
    for (int i = 0; i < 2; i++)
        DrawBtn(G.goBtns[i], i == G.goSel);
    DrawAudioToggle();
    EndDrawing();
}
void ScreenOptions(float dt)
{
    UpdateStars(dt);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        G.optSel = (G.optSel + 1) % 2;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        G.optSel = (G.optSel + 1) % 2;

    /* Update buttons and track mouse hover → selection */
    for (int i = 0; i < 2; i++)
    {
        UpdateBtn(&G.optBtns[i], dt);
        if (G.optBtns[i].hovered)
            G.optSel = i;
    }

    /* Play hover sound on focus change */
    if (G.prevOptSel >= 0 && G.prevOptSel != G.optSel)
        PlayBtnHover();
    G.prevOptSel = G.optSel;

    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 2; i++)
    {
        if (G.optBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            G.optSel = i;
            enter = true;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_MAIN_MENU;
        G.prevMenuSel = -1;
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
            G.screen = SCREEN_MAIN_MENU;
            G.prevMenuSel = -1;
        }
    }
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();
    DrawTitle((float)GetTime());
    for (int i = 0; i < 2; i++)
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
