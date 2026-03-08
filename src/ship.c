#include "include/ship.h"
#include "include/helpers.h"
#include <math.h>

void DrawShipShape(Vector2 p, ShipType t, float hover, bool engineOn)
{
    float h = sinf(hover) * 3.0f;
    p.y += h;
    if (t == SHIP_INTERCEPTOR)
    {
        DrawTriangle((Vector2){p.x, p.y - 28}, (Vector2){p.x + 14, p.y + 18}, (Vector2){p.x - 14, p.y + 18}, (Color){30, 180, 255, 255});
        DrawTriangle((Vector2){p.x, p.y - 20}, (Vector2){p.x + 6, p.y + 8}, (Vector2){p.x - 6, p.y + 8}, (Color){140, 220, 255, 255});
        DrawTriangle((Vector2){p.x - 14, p.y + 8}, (Vector2){p.x - 6, p.y + 12}, (Vector2){p.x - 18, p.y + 20}, (Color){20, 120, 220, 255});
        DrawTriangle((Vector2){p.x + 14, p.y + 8}, (Vector2){p.x + 18, p.y + 20}, (Vector2){p.x + 6, p.y + 12}, (Color){20, 120, 220, 255});
        if (engineOn)
        {
            unsigned char ea = (unsigned char)(150 + GetRandomValue(0, 105));
            DrawCircleV((Vector2){p.x, p.y + 22}, 5, CAlpha((Color){0, 255, 255, 255}, ea));
            DrawCircleV((Vector2){p.x, p.y + 22}, 8, CAlpha((Color){0, 200, 255, 255}, (unsigned char)(ea / 3)));
        }
    }
    else if (t == SHIP_DESTROYER)
    {
        DrawTriangle((Vector2){p.x, p.y - 24}, (Vector2){p.x + 18, p.y + 20}, (Vector2){p.x - 18, p.y + 20}, (Color){80, 80, 100, 255});
        DrawTriangle((Vector2){p.x, p.y - 16}, (Vector2){p.x + 10, p.y + 10}, (Vector2){p.x - 10, p.y + 10}, (Color){160, 160, 180, 255});
        DrawRectangle((int)p.x - 20, (int)p.y + 4, 8, 14, (Color){60, 60, 80, 255});
        DrawRectangle((int)p.x + 12, (int)p.y + 4, 8, 14, (Color){60, 60, 80, 255});
        DrawRectangle((int)p.x - 3, (int)p.y + 14, 6, 8, (Color){60, 60, 80, 255});
        if (engineOn)
        {
            unsigned char ea = (unsigned char)(150 + GetRandomValue(0, 105));
            DrawCircleV((Vector2){p.x - 8, p.y + 22}, 4, CAlpha((Color){255, 160, 40, 255}, ea));
            DrawCircleV((Vector2){p.x + 8, p.y + 22}, 4, CAlpha((Color){255, 160, 40, 255}, ea));
            DrawCircleV((Vector2){p.x - 8, p.y + 22}, 7, CAlpha((Color){255, 120, 20, 255}, (unsigned char)(ea / 3)));
            DrawCircleV((Vector2){p.x + 8, p.y + 22}, 7, CAlpha((Color){255, 120, 20, 255}, (unsigned char)(ea / 3)));
        }
    }
    else
    {
        DrawTriangle((Vector2){p.x, p.y - 22}, (Vector2){p.x + 24, p.y + 22}, (Vector2){p.x - 24, p.y + 22}, (Color){140, 40, 40, 255});
        DrawTriangle((Vector2){p.x, p.y - 12}, (Vector2){p.x + 14, p.y + 12}, (Vector2){p.x - 14, p.y + 12}, (Color){200, 80, 80, 255});
        DrawRectangle((int)p.x - 26, (int)p.y + 2, 10, 16, (Color){120, 30, 30, 255});
        DrawRectangle((int)p.x + 16, (int)p.y + 2, 10, 16, (Color){120, 30, 30, 255});
        DrawRectangle((int)p.x - 4, (int)p.y - 26, 8, 10, (Color){120, 30, 30, 255});
        if (engineOn)
        {
            unsigned char ea = (unsigned char)(150 + GetRandomValue(0, 105));
            DrawCircleV((Vector2){p.x - 10, p.y + 24}, 5, CAlpha((Color){255, 50, 20, 255}, ea));
            DrawCircleV((Vector2){p.x, p.y + 24}, 5, CAlpha((Color){255, 50, 20, 255}, ea));
            DrawCircleV((Vector2){p.x + 10, p.y + 24}, 5, CAlpha((Color){255, 50, 20, 255}, ea));
            DrawCircleV((Vector2){p.x, p.y + 24}, 10, CAlpha((Color){255, 30, 10, 255}, (unsigned char)(ea / 4)));
        }
    }
}

void DrawEnemyShip(Vector2 pos, float rotation)
{
    (void)rotation;
    Vector2 p = pos;
    float t = (float)GetTime();

    // Aggressive trail effect
    float trailPulse = 0.5f + 0.5f * sinf(t * 10.0f);
    DrawTriangle((Vector2){p.x - 8, p.y - 15}, (Vector2){p.x, p.y - 40 - trailPulse * 10}, (Vector2){p.x + 8, p.y - 15}, CAlpha(RED, 100));

    // Outer wings / Scythes
    DrawTriangle((Vector2){p.x - 25, p.y - 10}, (Vector2){p.x - 35, p.y - 25}, (Vector2){p.x - 10, p.y}, (Color){100, 0, 0, 255});
    DrawTriangle((Vector2){p.x + 25, p.y - 10}, (Vector2){p.x + 10, p.y}, (Vector2){p.x + 35, p.y - 25}, (Color){100, 0, 0, 255});

    // Main hull (angular red blocks)
    DrawTriangle((Vector2){p.x, p.y + 28}, (Vector2){p.x - 18, p.y - 12}, (Vector2){p.x + 18, p.y - 12}, (Color){180, 20, 20, 255});
    DrawTriangle((Vector2){p.x - 18, p.y - 12}, (Vector2){p.x - 12, p.y - 20}, (Vector2){p.x, p.y - 12}, (Color){140, 10, 10, 255});
    DrawTriangle((Vector2){p.x + 18, p.y - 12}, (Vector2){p.x, p.y - 12}, (Vector2){p.x + 12, p.y - 20}, (Color){140, 10, 10, 255});

    // Metallic trim / Inner detail
    DrawTriangle((Vector2){p.x, p.y + 15}, (Vector2){p.x - 8, p.y}, (Vector2){p.x + 8, p.y}, (Color){60, 60, 70, 255});

    // Pulsing Cockpit
    float pulse = 0.8f + 0.2f * sinf(t * 8.0f);
    Color cockpitColor = CAlpha((Color){255, 100, 100, 255}, (unsigned char)(200 * pulse));
    DrawCircleV((Vector2){p.x, p.y + 2}, 5, cockpitColor);
    DrawCircleV((Vector2){p.x, p.y + 2}, 3, WHITE);

    // Lateral engine glows
    unsigned char ea = (unsigned char)(180 + GetRandomValue(0, 75));
    DrawCircleV((Vector2){p.x - 14, p.y - 14}, 4, CAlpha(ORANGE, ea));
    DrawCircleV((Vector2){p.x + 14, p.y - 14}, 4, CAlpha(ORANGE, ea));
}
