#include "include/collision.h"

ShipHitbox GetShipHitbox(Vector2 shipPos, ShipType type, bool isPlayer)
{
    ShipHitbox h = {0};
    if (isPlayer)
    {
        if (type == SHIP_INTERCEPTOR)
        {
            h.rects[0] = (Rectangle){shipPos.x - 12, shipPos.y - 40, 24, 70}; 
            h.rects[1] = (Rectangle){shipPos.x - 30, shipPos.y - 5, 60, 35}; 
            h.count = 2;
        }
        else if (type == SHIP_DESTROYER)
        {
            h.rects[0] = (Rectangle){shipPos.x - 24, shipPos.y - 36, 48, 70}; 
            h.rects[1] = (Rectangle){shipPos.x - 32, shipPos.y - 5, 64, 20};  
            h.count = 2;
        }
        else if (type == SHIP_TITAN)
        {
            h.rects[0] = (Rectangle){shipPos.x - 30, shipPos.y - 50, 60, 110}; 
            h.rects[1] = (Rectangle){shipPos.x - 65, shipPos.y + 10, 130, 40}; 
            h.count = 2;
        }
    }
    else
    {
        if (type == SHIP_INTERCEPTOR)
        {
            h.rects[0] = (Rectangle){shipPos.x - 8, shipPos.y - 40, 16, 80}; 
            h.rects[1] = (Rectangle){shipPos.x - 35, shipPos.y - 30, 70, 40}; 
            h.count = 2;
        }
        else if (type == SHIP_DESTROYER)
        {
            h.rects[0] = (Rectangle){shipPos.x - 30, shipPos.y - 30, 60, 80}; 
            h.rects[1] = (Rectangle){shipPos.x - 45, shipPos.y - 20, 90, 40}; 
            h.count = 2;
        }
        else if (type == SHIP_TITAN)
        {
            h.rects[0] = (Rectangle){shipPos.x - 30, shipPos.y - 50, 60, 110}; 
            h.rects[1] = (Rectangle){shipPos.x - 75, shipPos.y - 45, 150, 70}; 
            h.count = 2;
        }
        else if (type == SHIP_BOSS)
        {
            h.rects[0] = (Rectangle){shipPos.x - 60, shipPos.y - 120, 120, 220}; 
            h.rects[1] = (Rectangle){shipPos.x - 140, shipPos.y - 60, 280, 100}; 
            h.count = 2;
        }
    }
    return h;
}

bool CheckCircleShipCollision(Vector2 circlePos, float radius, ShipHitbox hitbox)
{
    for (int i = 0; i < hitbox.count; i++)
    {
        if (CheckCollisionCircleRec(circlePos, radius, hitbox.rects[i]))
            return true;
    }
    return false;
}

bool CheckShipShipCollision(ShipHitbox h1, ShipHitbox h2)
{
    for (int i = 0; i < h1.count; i++)
    {
        for (int j = 0; j < h2.count; j++)
        {
            if (CheckCollisionRecs(h1.rects[i], h2.rects[j]))
                return true;
        }
    }
    return false;
}
