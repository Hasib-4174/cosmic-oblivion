/* Cosmic Oblivion - Arcade Space Shooter */
#include "raylib.h"
#include "include/constants.h"
#include "include/helpers.h"
#include "include/stars.h"
#include "include/particles.h"
#include "include/button.h"
#include "include/ship.h"
#include "include/meteor.h"
#include "include/game.h"
#include "include/ui.h"
#include <string.h>

GameState G;
int SW;
int SH;

int main(void)
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "Cosmic Oblivion");
    InitAudioDevice();
    SW = GetScreenWidth();
    SH = GetScreenHeight();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    memset(&G, 0, sizeof(G));

    /* Audio volume defaults */
    G.bgmVolume = 0.5f;
    G.firingVolume = 0.7f;
    G.explosionVolume = 0.7f;
    G.healthPickupVolume = 0.7f;
    G.shieldPickupVolume = 0.7f;
    G.audioEnabled = true;

    /* Load BGM tracks */
    G.bgm = LoadMusicStream("audio/bg/bgm.wav");
    G.bgmGameover = LoadMusicStream("audio/bg/bgm_gameover.wav");
    SetMusicVolume(G.bgm, G.bgmVolume);
    SetMusicVolume(G.bgmGameover, G.bgmVolume);

    G.screen = SCREEN_LOGO;
    G.highscore = LoadHS();
    InitStars();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        /* Update the currently active music stream */
        if (G.screen == SCREEN_GAME_OVER)
            UpdateMusicStream(G.bgmGameover);
        else
            UpdateMusicStream(G.bgm);

        /* Global audio toggle: M key */
        if (IsKeyPressed(KEY_M))
        {
            G.audioEnabled = !G.audioEnabled;
            if (G.audioEnabled)
            {
                SetMasterVolume(1.0f);
                /* Resume current BGM */
                if (G.screen == SCREEN_GAME_OVER)
                {
                    PlayMusicStream(G.bgmGameover);
                    SetMusicVolume(G.bgmGameover, G.bgmVolume);
                }
                else
                {
                    PlayMusicStream(G.bgm);
                    SetMusicVolume(G.bgm, G.bgmVolume);
                }
            }
            else
            {
                SetMasterVolume(0.0f);
            }
        }

        switch (G.screen)
        {
        case SCREEN_LOGO:
            ScreenLogo(dt);
            break;
        case SCREEN_MAIN_MENU:
            ScreenMenu(dt);
            break;
        case SCREEN_SHIP_SELECT:
            ScreenShipSelect(dt);
            break;
        case SCREEN_GAMEPLAY:
            if (IsKeyPressed(KEY_ESCAPE))
            {
                G.screen = SCREEN_PAUSE;
                G.pauseSel = 0;
                G.pauseBtns[0] = MkBtn(SW / 2 - 100, 300, 200, 48, "RESUME");
                G.pauseBtns[1] = MkBtn(SW / 2 - 100, 360, 200, 48, "MAIN MENU");
                G.pauseBtns[2] = MkBtn(SW / 2 - 100, 420, 200, 48, "EXIT");
            }
            UpdateStars(dt);
            if (G.screen == SCREEN_GAMEPLAY)
                UpdateGame(dt);
            DrawGameplay();
            break;
        case SCREEN_PAUSE:
            ScreenPause(dt);
            break;
        case SCREEN_GAME_OVER:
            ScreenGameOver(dt);
            break;
        case SCREEN_OPTIONS:
            ScreenOptions(dt);
            break;
        case SCREEN_AUDIO:
            ScreenAudio(dt);
            break;
        }
        if (G.screen == SCREEN_GAMEPLAY)
            HideCursor();
        else
            ShowCursor();
    }
    CloseWindow();
    return 0;
}
