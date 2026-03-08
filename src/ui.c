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

/* ---- Draw audio toggle icon (top-left, every screen except logo) ---- */
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

/* ---- Slider Drawing/Interaction Helper ---- */
static float DrawSlider(int x, int y, int w, float value, Color fillColor, bool selected, const char *label)
{
    int labelW = MeasureText(label, 20);
    (void)labelW;
    DrawText(label, x, y + 2, 20, selected ? GOLD : WHITE);

    int sliderX = x + 250;
    int sliderW = w;
    int sliderH = 16;
    int sliderY = y + 4;

    /* Background track */
    DrawRectangleRounded((Rectangle){sliderX, sliderY, sliderW, sliderH}, 0.5f, 4, (Color){40, 40, 60, 200});
    /* Filled portion */
    int fillW = (int)(value * sliderW);
    if (fillW > 0)
        DrawRectangleRounded((Rectangle){sliderX, sliderY, fillW, sliderH}, 0.5f, 4, fillColor);
    /* Border */
    DrawRectangleRoundedLinesEx((Rectangle){sliderX, sliderY, sliderW, sliderH}, 0.5f, 4, 1, selected ? GOLD : (Color){100, 120, 160, 200});
    /* Knob */
    int knobX = sliderX + fillW;
    DrawCircleV((Vector2){knobX, sliderY + sliderH / 2}, 8, selected ? GOLD : WHITE);
    /* Value text */
    DrawText(TextFormat("%d%%", (int)(value * 100)), sliderX + sliderW + 12, y + 2, 18, (Color){180, 200, 220, 255});

    /* Mouse interaction */
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if (mouse.x >= sliderX && mouse.x <= sliderX + sliderW &&
            mouse.y >= sliderY - 10 && mouse.y <= sliderY + sliderH + 10)
        {
            value = Clampf((mouse.x - sliderX) / (float)sliderW, 0, 1);
        }
    }
    return value;
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
            PlayMusicStream(G.bgm);
        }
        G.menuBtns[0] = MkBtn(SW / 2 - 110, 280, 220, 50, "PLAY GAME");
        G.menuBtns[1] = MkBtn(SW / 2 - 110, 350, 220, 50, "SPACESHIPS");
        G.menuBtns[2] = MkBtn(SW / 2 - 110, 420, 220, 50, "OPTIONS");
        G.menuBtns[3] = MkBtn(SW / 2 - 110, 490, 220, 50, "EXIT");
        G.menuSel = 0;
    }
}
void ScreenMenu(float dt)
{
    UpdateStars(dt);
    UpdateParticles(dt);
    if (GetRandomValue(0, 100) < 8)
        SpawnP((Vector2){Rf(0, SW), Rf(0, SH)}, CAlpha((Color){60, 100, 200, 255}, 120), 1, 20, 1.5f);
    if (IsKeyPressed(KEY_DOWN))
    {
        G.menuSel = (G.menuSel + 1) % 4;
    }
    if (IsKeyPressed(KEY_UP))
    {
        G.menuSel = (G.menuSel + 3) % 4;
    }
    for (int i = 0; i < 4; i++)
    {
        if (UpdateBtn(&G.menuBtns[i], dt))
            G.menuSel = i;
    }
    bool enter = IsKeyPressed(KEY_ENTER);
    for (int i = 0; i < 4; i++)
    {
        if (G.menuBtns[i].hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            G.menuSel = i;
            enter = true;
        }
    }
    if (enter)
    {
        if (G.menuSel == 0)
        {
            G.screen = SCREEN_SHIP_SELECT;
            G.shipSel = G.selectedShip;
        }
        else if (G.menuSel == 1)
        {
            G.screen = SCREEN_SHIP_SELECT;
            G.shipSel = G.selectedShip;
        }
        else if (G.menuSel == 2)
        {
            G.screen = SCREEN_OPTIONS;
            G.optBtns[0] = MkBtn(SW / 2 - 110, 320, 220, 50, "AUDIO");
            G.optBtns[1] = MkBtn(SW / 2 - 110, 390, 220, 50, "BACK");
            G.optSel = 0;
        }
        else if (G.menuSel == 3)
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
    for (int i = 0; i < 4; i++)
        DrawBtn(G.menuBtns[i], i == G.menuSel);
    DrawAudioToggle();
    EndDrawing();
}
void ScreenShipSelect(float dt)
{
    UpdateStars(dt);
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        G.shipSel = (G.shipSel + 2) % 3;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        G.shipSel = (G.shipSel + 1) % 3;
    if (IsKeyPressed(KEY_ENTER))
    {
        G.selectedShip = G.shipSel;
        InitGame();
        G.screen = SCREEN_GAMEPLAY;
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        G.screen = SCREEN_MAIN_MENU;
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
void ScreenPause(float dt)
{
    (void)dt;
    if (IsKeyPressed(KEY_DOWN))
        G.pauseSel = (G.pauseSel + 1) % 3;
    if (IsKeyPressed(KEY_UP))
        G.pauseSel = (G.pauseSel + 2) % 3;
    for (int i = 0; i < 3; i++)
        if (UpdateBtn(&G.pauseBtns[i], dt) && G.pauseBtns[i].hovered)
            G.pauseSel = i;
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
        if (G.pauseSel == 0)
        {
            G.screen = SCREEN_GAMEPLAY;
        }
        else if (G.pauseSel == 1)
        {
            /* Return to main menu: restart BGM */
            StopMusicStream(G.bgm);
            if (G.audioEnabled)
            {
                PlayMusicStream(G.bgm);
                SetMusicVolume(G.bgm, G.bgmVolume);
            }
            G.screen = SCREEN_MAIN_MENU;
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
    if (IsKeyPressed(KEY_DOWN))
        G.goSel = (G.goSel + 1) % 2;
    if (IsKeyPressed(KEY_UP))
        G.goSel = (G.goSel + 1) % 2;
    for (int i = 0; i < 2; i++)
        if (UpdateBtn(&G.goBtns[i], dt) && G.goBtns[i].hovered)
            G.goSel = i;
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
        if (G.goSel == 0)
        {
            /* Play again: stop gameover BGM, restart gameplay BGM */
            StopMusicStream(G.bgmGameover);
            if (G.audioEnabled)
            {
                PlayMusicStream(G.bgm);
                SetMusicVolume(G.bgm, G.bgmVolume);
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
                PlayMusicStream(G.bgm);
                SetMusicVolume(G.bgm, G.bgmVolume);
            }
            G.screen = SCREEN_MAIN_MENU;
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
    for (int i = 0; i < 2; i++)
        DrawBtn(G.goBtns[i], i == G.goSel);
    DrawAudioToggle();
    EndDrawing();
}
void ScreenOptions(float dt)
{
    UpdateStars(dt);
    if (IsKeyPressed(KEY_DOWN))
        G.optSel = (G.optSel + 1) % 2;
    if (IsKeyPressed(KEY_UP))
        G.optSel = (G.optSel + 1) % 2;
    for (int i = 0; i < 2; i++)
    {
        if (UpdateBtn(&G.optBtns[i], dt))
            G.optSel = i;
    }
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
    }
    if (enter)
    {
        if (G.optSel == 0)
        {
            G.screen = SCREEN_AUDIO;
            G.audioBackBtn = MkBtn(SW / 2 - 110, 0, 220, 50, "BACK");
            G.audioSel = 0;
        }
        else if (G.optSel == 1)
        {
            G.screen = SCREEN_MAIN_MENU;
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

    /* 5 items: 0-4 = sliders (BGM, Firing, Explosion, Health Pickup, Shield Pickup), 5 = BACK */
    int numSliders = 5;
    int totalItems = numSliders + 1;

    if (IsKeyPressed(KEY_DOWN))
        G.audioSel = (G.audioSel + 1) % totalItems;
    if (IsKeyPressed(KEY_UP))
        G.audioSel = (G.audioSel + totalItems - 1) % totalItems;

    /* Keyboard left/right for slider adjustment */
    if (G.audioSel < numSliders)
    {
        float *vol = 0;
        if (G.audioSel == 0)
            vol = &G.bgmVolume;
        else if (G.audioSel == 1)
            vol = &G.firingVolume;
        else if (G.audioSel == 2)
            vol = &G.explosionVolume;
        else if (G.audioSel == 3)
            vol = &G.healthPickupVolume;
        else if (G.audioSel == 4)
            vol = &G.shieldPickupVolume;

        if (vol)
        {
            if (IsKeyPressed(KEY_RIGHT))
                *vol = Clampf(*vol + 0.1f, 0, 1);
            if (IsKeyPressed(KEY_LEFT))
                *vol = Clampf(*vol - 0.1f, 0, 1);
        }
    }

    /* BACK button interaction */
    UpdateBtn(&G.audioBackBtn, dt);
    if (G.audioBackBtn.hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        G.audioSel = numSliders;
    }

    bool enter = IsKeyPressed(KEY_ENTER);
    if (G.audioBackBtn.hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        enter = true;

    if (IsKeyPressed(KEY_ESCAPE) || (enter && G.audioSel == numSliders))
    {
        G.screen = SCREEN_OPTIONS;
    }

    /* Apply BGM volume in real-time */
    SetMusicVolume(G.bgm, G.bgmVolume);
    SetMusicVolume(G.bgmGameover, G.bgmVolume);

    /* ---- Drawing ---- */
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    DrawStars();

    /* Title at top */
    DrawTitle((float)GetTime());

    /* Section header */
    int menuStartY = 200;
    const char *sectionTitle = "AUDIO SETTINGS";
    DrawText(sectionTitle, (SW - MeasureText(sectionTitle, 32)) / 2, menuStartY - 30, 32, WHITE);
    DrawLine(SW / 2 - 200, menuStartY, SW / 2 + 200, menuStartY, (Color){60, 80, 120, 200});

    /* Slider layout */
    int sliderStartY = menuStartY + 20;
    int rowSpacing = 60;
    int sliderX = SW / 2 - 200;
    int sliderW = 200;

    const char *labels[5] = {"BGM Volume", "Firing Volume", "Explosion Volume", "Health Pickup", "Shield Pickup"};
    float *values[5] = {&G.bgmVolume, &G.firingVolume, &G.explosionVolume, &G.healthPickupVolume, &G.shieldPickupVolume};
    Color colors[5] = {
        {0, 200, 100, 255},
        {200, 160, 0, 255},
        {200, 50, 0, 255},
        {100, 255, 150, 255},
        {100, 200, 255, 255}};

    for (int i = 0; i < numSliders; i++)
    {
        int y = sliderStartY + i * rowSpacing;
        float newVal = DrawSlider(sliderX, y, sliderW, *values[i], colors[i], G.audioSel == i, labels[i]);
        *values[i] = newVal;
    }

    /* BACK button: position below sliders */
    int backY = sliderStartY + numSliders * rowSpacing + 20;
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
