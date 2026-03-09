#ifndef COLLISION_H
#define COLLISION_H

#include "raylib.h"
#include "constants.h"

#define MAX_SHIP_HULLS 4

typedef struct {
    Rectangle rects[MAX_SHIP_HULLS];
    int count;
} ShipHitbox;

// Generates a composite hitbox for any ship type
ShipHitbox GetShipHitbox(Vector2 shipPos, ShipType type, bool isPlayer);

// Checks collision between a circular object and a ship's composite hitbox
bool CheckCircleShipCollision(Vector2 circlePos, float radius, ShipHitbox hitbox);

// Checks collision between two ship composite hitboxes
bool CheckShipShipCollision(ShipHitbox h1, ShipHitbox h2);

#endif
