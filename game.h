#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "constants.h"

void InitPlayer(void);
void InitGame(void);
void UpdateGame(float dt);
void DrawGameplay(void);
void DrawTitle(float t);

#endif
