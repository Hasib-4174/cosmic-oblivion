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
#include "include/campaign.h"
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
    G.uiVolume = 0.7f;
    G.gameplayVolume = 0.7f;
    G.audioEnabled = true;

    /* Load BGM tracks (3 separate) */
    G.bgmMenu = LoadMusicStream("audio/bg/bgm_menu.wav");
    G.bgmGameplay = LoadMusicStream("audio/bg/bgm_gameplay.wav");
    G.bgmGameover = LoadMusicStream("audio/bg/bgm_gameover.wav");
    SetMusicVolume(G.bgmMenu, G.bgmVolume);
    SetMusicVolume(G.bgmGameplay, G.bgmVolume);
    SetMusicVolume(G.bgmGameover, G.bgmVolume);

    /* Load UI sounds */
    G.sfxButtonHover = LoadSound("audio/menu/button_hover-switch.ogg");
    G.sfxButtonSelect = LoadSound("audio/menu/button_selected.ogg");

    /* Load enemy sounds */
    G.sfxEnemyDestroy = LoadSound("audio/enemy/destroyed.ogg");
    G.sfxEnemyEngine = LoadSound("audio/enemy/engine.wav");
    G.sfxEnemyShootIdx = -1;

    /* Load shield activation sound */
    G.sfxShieldOn = LoadSound("audio/shield/shield_on.ogg");

    /* Initialize prev selection trackers to -1 */
    G.prevMenuSel = -1;
    G.prevPauseSel = -1;
    G.prevGoSel = -1;
    G.prevOptSel = -1;
    G.prevAudioSel = -1;
    G.prevShipSel = -1;
    G.prevWeaponSel = -1;

    G.screen = SCREEN_LOGO;
    G.highscore = LoadHS();
    InitStars();
    InitCampaign();
    LoadCampaignProgress(&G);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        /* Update all music streams every frame */
        UpdateMusicStream(G.bgmMenu);
        UpdateMusicStream(G.bgmGameplay);
        UpdateMusicStream(G.bgmGameover);

        /* Global audio toggle: M key */
        if (IsKeyPressed(KEY_M))
        {
            G.audioEnabled = !G.audioEnabled;
            if (G.audioEnabled)
            {
                SetMasterVolume(1.0f);
                /* Resume current BGM based on screen */
                if (G.screen == SCREEN_GAME_OVER)
                {
                    PlayMusicStream(G.bgmGameover);
                    SetMusicVolume(G.bgmGameover, G.bgmVolume);
                }
                else if (G.screen == SCREEN_GAMEPLAY || G.screen == SCREEN_PAUSE)
                {
                    PlayMusicStream(G.bgmGameplay);
                    SetMusicVolume(G.bgmGameplay, G.bgmVolume);
                }
                else
                {
                    PlayMusicStream(G.bgmMenu);
                    SetMusicVolume(G.bgmMenu, G.bgmVolume);
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
        case SCREEN_MODE_SELECT:
            ScreenModeSelect(dt);
            break;
        case SCREEN_LEVEL_SELECT:
            ScreenLevelSelect(dt);
            break;
        case SCREEN_NARRATIVE:
            ScreenNarrative(dt);
            break;
        case SCREEN_LEVEL_COMPLETE:
            ScreenLevelComplete(dt);
            break;
        case SCREEN_SHIP_SELECT:
            ScreenShipSelect(dt);
            break;
        case SCREEN_WEAPON_SELECT:
            ScreenWeaponSelect(dt);
            break;
        case SCREEN_GAMEPLAY:
            if (IsKeyPressed(KEY_ESCAPE))
            {
                G.screen = SCREEN_PAUSE;
                G.pauseSel = 0;
                G.prevPauseSel = -1;
                G.pauseBtns[0] = MkBtn(SW / 2 - 140, 300, 280, 50, "RESUME");
                G.pauseBtns[1] = MkBtn(SW / 2 - 140, 380, 280, 50, "OPTIONS");
                G.pauseBtns[2] = MkBtn(SW / 2 - 140, 460, 280, 50, "MAIN MENU");
                G.pauseBtns[3] = MkBtn(SW / 2 - 140, 540, 280, 50, "EXIT");
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
        case SCREEN_DIFFICULTY_SELECT:
            ScreenDifficultySelect(dt);
            break;
        }
        if (G.screen == SCREEN_GAMEPLAY)
            HideCursor();
        else
            ShowCursor();
    }

    /* ---- Unload all audio resources ---- */
    /* BGM */
    UnloadMusicStream(G.bgmMenu);
    UnloadMusicStream(G.bgmGameplay);
    UnloadMusicStream(G.bgmGameover);

    /* UI sounds */
    UnloadSound(G.sfxButtonHover);
    UnloadSound(G.sfxButtonSelect);

    /* Enemy sounds */
    for (int i = 0; i < 8; i++)
    {
        if (G.sfxEnemyShoot[i].stream.buffer != NULL)
            UnloadSound(G.sfxEnemyShoot[i]);
    }
    UnloadSound(G.sfxEnemyDestroy);
    UnloadSound(G.sfxEnemyEngine);

    /* Shield activation */
    UnloadSound(G.sfxShieldOn);

    /* Firing sounds */
    for (int i = 0; i < 8; i++)
    {
        if (G.firingSounds[i].stream.buffer != NULL)
            UnloadSound(G.firingSounds[i]);
    }

    /* Explosion sounds */
    for (int i = 0; i < 8; i++)
    {
        if (G.explosionSounds[i].stream.buffer != NULL)
            UnloadSound(G.explosionSounds[i]);
    }
    for (int i = 0; i < G.explosionVariantCount; i++)
    {
        if (G.explosionVariants[i].stream.buffer != NULL)
            UnloadSound(G.explosionVariants[i]);
    }

    /* Health pickup sounds */
    for (int i = 0; i < 8; i++)
    {
        if (G.healthPickupSounds[i].stream.buffer != NULL)
            UnloadSound(G.healthPickupSounds[i]);
    }

    /* Shield pickup sounds */
    for (int i = 0; i < 8; i++)
    {
        if (G.shieldPickupSounds[i].stream.buffer != NULL)
            UnloadSound(G.shieldPickupSounds[i]);
    }

    /* Damage sound */
    if (G.damageSound.stream.buffer != NULL)
        UnloadSound(G.damageSound);

    /* Engine sounds */
    if (G.engineSoundsLoaded)
    {
        for (int i = 0; i < 3; i++)
        {
            if (G.engineSounds[i].stream.buffer != NULL)
                UnloadSound(G.engineSounds[i]);
        }
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
