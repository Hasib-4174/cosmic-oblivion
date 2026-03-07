#ifndef HEALTHSTAR_H
#define HEALTHSTAR_H

#include "raylib.h"
#include "constants.h"

void InitHealthStars(void);
void UpdateHealthStars(float dt);
void DrawHealthStars(void);
void SpawnHealthStar(void);
void SpawnHealthStarAt(Vector2 pos);

#endif
