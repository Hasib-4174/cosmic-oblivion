#ifndef SHIP_H
#define SHIP_H

#include "raylib.h"
#include "constants.h"

void DrawShipShape(Vector2 p, ShipType t, float hover, bool engineOn);
void DrawEnemyShip(Vector2 pos, float rotation);

#endif
