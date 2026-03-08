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
