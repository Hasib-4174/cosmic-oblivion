#ifndef PARTICLES_H
#define PARTICLES_H

#include "raylib.h"

void SpawnP(Vector2 pos, Color c, int n, float spread, float sz);
void UpdateParticles(float dt);
void DrawParticles(void);

#endif
