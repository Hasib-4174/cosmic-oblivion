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

void DrawEnemyShip(Vector2 pos, ShipType t, float rotation)
{
    (void)rotation;
    Vector2 p = pos;
    float time = (float)GetTime();

    if (t == SHIP_INTERCEPTOR)
    {
        // Aggressive trail effect
        float trailPulse = 0.5f + 0.5f * sinf(time * 10.0f);
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
        float pulse = 0.8f + 0.2f * sinf(time * 8.0f);
        Color cockpitColor = CAlpha((Color){255, 100, 100, 255}, (unsigned char)(200 * pulse));
        DrawCircleV((Vector2){p.x, p.y + 2}, 5, cockpitColor);
        DrawCircleV((Vector2){p.x, p.y + 2}, 3, WHITE);

        // Lateral engine glows
        unsigned char ea = (unsigned char)(180 + GetRandomValue(0, 75));
        DrawCircleV((Vector2){p.x - 14, p.y - 14}, 4, CAlpha(ORANGE, ea));
        DrawCircleV((Vector2){p.x + 14, p.y - 14}, 4, CAlpha(ORANGE, ea));
    }
    else if (t == SHIP_DESTROYER)
    {
        // Bulky red destroyer
        DrawTriangle((Vector2){p.x, p.y + 35}, (Vector2){p.x - 30, p.y - 20}, (Vector2){p.x + 30, p.y - 20}, (Color){150, 0, 0, 255});
        DrawTriangle((Vector2){p.x, p.y + 20}, (Vector2){p.x - 15, p.y}, (Vector2){p.x + 15, p.y}, (Color){200, 50, 50, 255});
        
        // Side cannons/pods
        DrawRectangle((int)p.x - 35, (int)p.y - 10, 12, 25, (Color){80, 0, 0, 255});
        DrawRectangle((int)p.x + 23, (int)p.y - 10, 12, 25, (Color){80, 0, 0, 255});
        
        // Engine glows
        unsigned char ea = (unsigned char)(180 + GetRandomValue(0, 75));
        DrawCircleV((Vector2){p.x - 15, p.y - 25}, 6, CAlpha(ORANGE, ea));
        DrawCircleV((Vector2){p.x + 15, p.y - 25}, 6, CAlpha(ORANGE, ea));
        
        // Glowing core
        float pulse = 0.7f + 0.3f * sinf(time * 6.0f);
        DrawCircleV((Vector2){p.x, p.y + 5}, 8 * pulse, CAlpha(RED, 200));
    }
    else if (t == SHIP_TITAN)
    {
        // Massive heavy titan
        DrawTriangle((Vector2){p.x, p.y + 50}, (Vector2){p.x - 50, p.y - 30}, (Vector2){p.x + 50, p.y - 30}, (Color){100, 0, 0, 255});
        DrawTriangle((Vector2){p.x, p.y + 30}, (Vector2){p.x - 30, p.y - 10}, (Vector2){p.x + 30, p.y - 10}, (Color){160, 20, 20, 255});
        
        // Heavy armor plates
        DrawRectangle((int)p.x - 55, (int)p.y - 20, 20, 40, (Color){60, 0, 0, 255});
        DrawRectangle((int)p.x + 35, (int)p.y - 20, 20, 40, (Color){60, 0, 0, 255});
        
        // Multi-engines
        unsigned char ea = (unsigned char)(200 + GetRandomValue(0, 55));
        DrawCircleV((Vector2){p.x - 25, p.y - 35}, 8, CAlpha(RED, ea));
        DrawCircleV((Vector2){p.x, p.y - 35}, 10, CAlpha(RED, ea));
        DrawCircleV((Vector2){p.x + 25, p.y - 35}, 8, CAlpha(RED, ea));
        
        // Pulsing main reactor
        float pulse = 0.6f + 0.4f * sinf(time * 4.0f);
        DrawCircleV((Vector2){p.x, p.y + 10}, 12 * pulse, CAlpha(GOLD, 180));
        DrawCircleV((Vector2){p.x, p.y + 10}, 6 * pulse, WHITE);
    }
}
