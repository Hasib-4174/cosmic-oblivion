#ifndef SHIELDPICKUP_H
#define SHIELDPICKUP_H

#include "raylib.h"
#include "constants.h"

void InitShieldPickups(void);
void SpawnShieldPickupAt(Vector2 pos);
void UpdateShieldPickups(float dt);
void DrawShieldPickups(void);

#endif
