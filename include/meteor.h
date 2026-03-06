#ifndef METEOR_H
#define METEOR_H

#include "raylib.h"
#include "constants.h"

void SpawnMeteor(GameState *g);
void SpawnMeteorAt(Vector2 pos, MeteorSize sz);
void DrawMeteor(Meteor m);

#endif
