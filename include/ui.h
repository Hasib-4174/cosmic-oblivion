#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "constants.h"

void ScreenLogo(float dt);
void ScreenMenu(float dt);
void ScreenShipSelect(float dt);
void ScreenWeaponSelect(float dt);
void ScreenPause(float dt);
void ScreenGameOver(float dt);
void ScreenOptions(float dt);
void ScreenAudio(float dt);
void ScreenDifficultySelect(float dt);
void DrawAudioToggle(void);

#endif
