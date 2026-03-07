#ifndef FLOATINGTEXT_H
#define FLOATINGTEXT_H

#include "raylib.h"
#include "constants.h"

void InitFloatingTexts(void);
void UpdateFloatingTexts(float dt);
void DrawFloatingTexts(void);
void SpawnFloatingText(Vector2 pos, const char *text, Color color);

#endif
