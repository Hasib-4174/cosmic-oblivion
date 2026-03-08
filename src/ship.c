#include "include/ship.h"
#include "include/helpers.h"
#include <math.h>

void DrawShipShape(Vector2 p, ShipType t, float hover, bool engineOn)
{
    float h = sinf(hover) * 3.0f;
    p.y += h;
    float time = (float)GetTime();

    if (t == SHIP_INTERCEPTOR)
    {
        // Bloom / Halo effect
        if (engineOn) {
            float bloom = 0.6f + 0.4f * sinf(time * 25.0f);
            DrawCircleGradient((int)p.x, (int)p.y + 28, 20 + bloom * 8, CAlpha(SKYBLUE, 60), BLANK);
        }

        // Advanced Wing Scaffolding
        // Outer Wing
        DrawTriangle((Vector2){p.x - 26, p.y + 20}, (Vector2){p.x, p.y - 15}, (Vector2){p.x - 12, p.y + 25}, (Color){10, 80, 180, 255});
        DrawTriangle((Vector2){p.x + 26, p.y + 20}, (Vector2){p.x + 12, p.y + 25}, (Vector2){p.x, p.y - 15}, (Color){10, 80, 180, 255});
        
        // Inner Wing / Paneling
        DrawTriangle((Vector2){p.x - 20, p.y + 18}, (Vector2){p.x - 5, p.y - 5}, (Vector2){p.x - 10, p.y + 22}, (Color){30, 120, 220, 255});
        DrawTriangle((Vector2){p.x + 20, p.y + 18}, (Vector2){p.x + 10, p.y + 22}, (Vector2){p.x + 5, p.y - 5}, (Color){30, 120, 220, 255});

        // Wing Leading Edges (Glow)
        DrawLineEx((Vector2){p.x - 24, p.y + 19}, (Vector2){p.x - 4, p.y - 12}, 2.0f, CAlpha(SKYBLUE, 200));
        DrawLineEx((Vector2){p.x + 24, p.y + 19}, (Vector2){p.x + 4, p.y - 12}, 2.0f, CAlpha(SKYBLUE, 200));

        // Central Chassis
        DrawTriangle((Vector2){p.x, p.y - 38}, (Vector2){p.x + 18, p.y + 18}, (Vector2){p.x - 18, p.y + 18}, (Color){20, 20, 35, 255}); // Base
        DrawTriangle((Vector2){p.x, p.y - 34}, (Vector2){p.x + 14, p.y + 14}, (Vector2){p.x - 14, p.y + 14}, (Color){45, 45, 70, 255}); // Raised Panel
        
        // Bevel Details
        DrawLineV((Vector2){p.x, p.y - 34}, (Vector2){p.x - 14, p.y + 14}, (Color){80, 80, 110, 255});
        DrawLineV((Vector2){p.x, p.y - 34}, (Vector2){p.x + 14, p.y + 14}, (Color){80, 80, 110, 255});

        // High-Tech Cockpit
        DrawCircleV((Vector2){p.x, p.y - 8}, 5, (Color){10, 30, 50, 255}); // Frame
        DrawCircleV((Vector2){p.x, p.y - 8}, 3, (Color){180, 230, 255, 255}); // Glass
        DrawCircleV((Vector2){p.x - 1, p.y - 9}, 1, WHITE); // Shine

        // Nav Lights
        if (((int)(time * 2) % 2) == 0) {
            DrawCircleV((Vector2){p.x - 24, p.y + 18}, 2, RED);
            DrawCircleV((Vector2){p.x + 24, p.y + 18}, 2, GREEN);
        }

        if (engineOn)
        {
            unsigned char ea = (unsigned char)(180 + GetRandomValue(0, 75));
            // Multi-stage exhaust
            DrawCircleV((Vector2){p.x, p.y + 22}, 8, CAlpha(SKYBLUE, (unsigned char)(ea * 0.6f)));
            DrawCircleV((Vector2){p.x, p.y + 22}, 4, WHITE);
            // Core flicker
            DrawCircleV((Vector2){p.x, p.y + 20}, 2, SKYBLUE);
        }
    }
    else if (t == SHIP_DESTROYER)
    {
        // Heavy Chassis with shadow
        DrawRectangleV((Vector2){p.x - 22, p.y - 12}, (Vector2){44, 38}, (Color){30, 30, 45, 255});
        DrawRectangleV((Vector2){p.x - 20, p.y - 10}, (Vector2){40, 34}, (Color){60, 60, 80, 255});
        
        // Armor Paneling (Beveled)
        DrawRectangleV((Vector2){p.x - 18, p.y - 8}, (Vector2){16, 15}, (Color){80, 80, 100, 255});
        DrawRectangleV((Vector2){p.x + 2, p.y - 8}, (Vector2){16, 15}, (Color){80, 80, 100, 255});
        DrawRectangleLines((int)p.x - 18, (int)p.y - 8, 16, 15, (Color){100, 100, 130, 255});
        DrawRectangleLines((int)p.x + 2, (int)p.y - 8, 16, 15, (Color){100, 100, 130, 255});

        // Front Reinforcement
        DrawTriangle((Vector2){p.x - 22, p.y - 12}, (Vector2){p.x, p.y - 32}, (Vector2){p.x + 22, p.y - 12}, (Color){40, 40, 55, 255});
        DrawTriangle((Vector2){p.x - 18, p.y - 11}, (Vector2){p.x, p.y - 28}, (Vector2){p.x + 18, p.y - 11}, (Color){90, 90, 110, 255});
        
        // Animated Vents
        float ventPulse = 0.5f + 0.5f * sinf(time * 8.0f);
        Color ventColor = CLerp((Color){40, 40, 50, 255}, (Color){255, 100, 50, 255}, ventPulse * 0.4f);
        DrawRectangleV((Vector2){p.x - 28, p.y + 5}, (Vector2){6, 15}, ventColor);
        DrawRectangleV((Vector2){p.x + 22, p.y + 5}, (Vector2){6, 15}, ventColor);
        
        // Bridge Command Center
        DrawRectangleV((Vector2){p.x - 10, p.y - 18}, (Vector2){20, 10}, (Color){20, 20, 30, 255});
        DrawRectangleV((Vector2){p.x - 8, p.y - 16}, (Vector2){16, 4}, (Color){0, 50, 100, 255});
        DrawRectangleV((Vector2){p.x - 6, p.y - 15}, (Vector2){12, 2}, (Color){80, 200, 255, 180});

        if (engineOn)
        {
            unsigned char ea = (unsigned char)(160 + GetRandomValue(0, 95));
            // Twin Heavy Thrusters
            DrawRectangleGradientV((int)p.x - 18, (int)p.y + 22, 10, 15, CAlpha(ORANGE, ea), BLANK);
            DrawRectangleGradientV((int)p.x + 8, (int)p.y + 22, 10, 15, CAlpha(ORANGE, ea), BLANK);
            DrawCircleV((Vector2){p.x - 13, p.y + 24}, 4, YELLOW);
            DrawCircleV((Vector2){p.x + 13, p.y + 24}, 4, YELLOW);
        }
    }
    else // SHIP_TITAN (Player version)
    {
        // JUGGERNAUT / FORTRESS LOOK
        // Massive Engine Glow Background (Rectangle Glow)
        if (engineOn) {
            float bloom = 0.7f + 0.3f * sinf(time * 15.0f);
            DrawCircleGradient((int)p.x, (int)p.y + 30, 35 + (int)(bloom * 10), CAlpha(PURPLE, 80), BLANK);
        }

        // Heavy Base Hull (T-shape/Blocky)
        DrawRectangleV((Vector2){p.x - 30, p.y - 15}, (Vector2){60, 40}, (Color){25, 20, 35, 255});
        DrawTriangle((Vector2){p.x - 30, p.y - 15}, (Vector2){p.x, p.y - 45}, (Vector2){p.x + 30, p.y - 15}, (Color){35, 30, 45, 255});

        // Massive Sponson Armoring (Side extensions)
        DrawRectangleRounded((Rectangle){p.x - 45, p.y - 10, 20, 45}, 0.2f, 6, (Color){45, 40, 55, 255});
        DrawRectangleRounded((Rectangle){p.x + 25, p.y - 10, 20, 45}, 0.2f, 6, (Color){45, 40, 55, 255});
        
        // Weapon Mounts (Railguns)
        DrawRectangleV((Vector2){p.x - 40, p.y - 15}, (Vector2){6, 30}, (Color){60, 60, 70, 255});
        DrawRectangleV((Vector2){p.x + 34, p.y - 15}, (Vector2){6, 30}, (Color){60, 60, 70, 255});
        DrawRectangleV((Vector2){p.x - 42, p.y + 10}, (Vector2){10, 4}, (Color){100, 100, 110, 255});
        DrawRectangleV((Vector2){p.x + 32, p.y + 10}, (Vector2){10, 4}, (Color){100, 100, 110, 255});

        // Top Armor Plates (Layered)
        DrawRectangleV((Vector2){p.x - 15, p.y - 5}, (Vector2){30, 15}, (Color){70, 65, 85, 255});
        DrawRectangleV((Vector2){p.x - 10, p.y + 2}, (Vector2){20, 5}, (Color){90, 85, 110, 255});
        
        // Command Bridge (Advanced)
        DrawRectangleV((Vector2){p.x - 6, p.y - 22}, (Vector2){12, 10}, (Color){20, 15, 30, 255});
        DrawRectangleV((Vector2){p.x - 4, p.y - 20}, (Vector2){8, 4}, (Color){255, 255, 255, 150}); // White-hot sensor strip
        
        // Micro-Vent Flickers
        float vents = sinf(time * 10.0f);
        if (vents > 0.5f) {
            DrawRectangleV((Vector2){p.x - 25, p.y}, (Vector2){4, 8}, CAlpha(SKYBLUE, 180));
            DrawRectangleV((Vector2){p.x + 21, p.y}, (Vector2){4, 8}, CAlpha(SKYBLUE, 180));
        }

        if (engineOn)
        {
            unsigned char ea = (unsigned char)(200 + GetRandomValue(0, 55));
            // Massive Triple Thrusters
            DrawRectangleGradientV((int)p.x - 38, (int)p.y + 35, 12, 20, CAlpha(VIOLET, ea), BLANK);
            DrawRectangleGradientV((int)p.x + 26, (int)p.y + 35, 12, 20, CAlpha(VIOLET, ea), BLANK);
            DrawRectangleGradientV((int)p.x - 8, (int)p.y + 25, 16, 25, CAlpha(PURPLE, ea), BLANK);
            
            // Core Heat
            DrawCircleV((Vector2){p.x, p.y + 28}, 5, WHITE);
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
        // STALKER / ASSASSIN LOOK
        // Shifting Void Energy Halo
        float voidTime = time * 2.5f;
        for (int i = 0; i < 3; i++) {
            float angle = voidTime + i * (PI * 2 / 3);
            Vector2 offset = {cosf(angle) * 15, sinf(angle) * 15};
            DrawCircleV((Vector2){p.x + offset.x, p.y + offset.y}, 8, CAlpha(MAROON, 40));
        }

        // Extremely sleek, needle-like hull
        DrawTriangle((Vector2){p.x, p.y + 45}, (Vector2){p.x - 8, p.y - 10}, (Vector2){p.x + 8, p.y - 10}, (Color){30, 0, 0, 255});
        DrawTriangle((Vector2){p.x, p.y + 35}, (Vector2){p.x - 5, p.y}, (Vector2){p.x + 5, p.y}, RED); // Forward spear
        
        // Reverse-Sweep Scythe Wings (Organic feel)
        DrawTriangle((Vector2){p.x, p.y - 5}, (Vector2){p.x - 45, p.y - 45}, (Vector2){p.x - 10, p.y}, (Color){60, 0, 10, 255});
        DrawTriangle((Vector2){p.x, p.y - 5}, (Vector2){p.x + 10, p.y}, (Vector2){p.x + 45, p.y - 45}, (Color){60, 0, 10, 255});
        
        // Energy Blades on Wings
        float bladeGlow = 0.5f + 0.5f * sinf(time * 20.0f);
        DrawLineEx((Vector2){p.x - 15, p.y - 15}, (Vector2){p.x - 40, p.y - 40}, 2.0f, CAlpha(RED, (unsigned char)(150 + 100 * bladeGlow)));
        DrawLineEx((Vector2){p.x + 15, p.y - 15}, (Vector2){p.x + 40, p.y - 40}, 2.0f, CAlpha(RED, (unsigned char)(150 + 100 * bladeGlow)));

        // Cyclopean Red Eye (Cockpit)
        DrawRectangleV((Vector2){p.x - 4, p.y + 5}, (Vector2){8, 3}, (Color){10, 0, 0, 255});
        DrawRectangleV((Vector2){p.x - 3, p.y + 6}, (Vector2){6, 1}, RED);

        // Void Particles (Small flickers)
        if (GetRandomValue(0, 10) > 8) {
            DrawCircleV((Vector2){p.x + Rf(-20, 20), p.y + Rf(-20, 20)}, 1, WHITE);
        }

        // Subtle rear exhaust
        unsigned char ea = (unsigned char)(100 + GetRandomValue(0, 55));
        DrawCircleV((Vector2){p.x, p.y - 15}, 6, CAlpha(MAROON, ea));
    }
    else if (t == SHIP_DESTROYER)
    {
        // Heavy Armored Destroyer
        DrawTriangle((Vector2){p.x, p.y + 45}, (Vector2){p.x - 40, p.y - 25}, (Vector2){p.x + 40, p.y - 25}, (Color){100, 10, 10, 255});
        
        // Armor Paneling (Beveled)
        DrawTriangle((Vector2){p.x, p.y + 35}, (Vector2){p.x - 30, p.y - 20}, (Vector2){p.x + 30, p.y - 20}, (Color){160, 30, 30, 255});
        DrawTriangle((Vector2){p.x, p.y + 25}, (Vector2){p.x - 20, p.y - 15}, (Vector2){p.x + 20, p.y - 15}, (Color){50, 0, 0, 255});
        
        // Side Cannon Arrays (Glow)
        DrawRectangleV((Vector2){p.x - 45, p.y - 20}, (Vector2){14, 35}, (Color){60, 0, 0, 255});
        DrawRectangleV((Vector2){p.x + 31, p.y - 20}, (Vector2){14, 35}, (Color){60, 0, 0, 255});
        DrawRectangleV((Vector2){p.x - 42, p.y + 5}, (Vector2){10, 6}, CAlpha(ORANGE, 150));
        DrawRectangleV((Vector2){p.x + 32, p.y + 5}, (Vector2){10, 6}, CAlpha(ORANGE, 150));
        
        // Engine Vents (Horizontal)
        unsigned char ea = (unsigned char)(180 + GetRandomValue(0, 75));
        DrawRectangleGradientH((int)p.x - 25, (int)p.y - 25, 15, 8, BLANK, CAlpha(ORANGE, ea));
        DrawRectangleGradientH((int)p.x + 10, (int)p.y - 25, 15, 8, CAlpha(ORANGE, ea), BLANK);
        
        // Massive Reactor Core
        float reactorPulse = 0.5f + 0.5f * sinf(time * 10.0f);
        DrawCircleGradient((int)p.x, (int)p.y + 12, 12 + reactorPulse * 5, RED, BLANK);
        DrawCircleV((Vector2){p.x, p.y + 12}, 6, MAROON);
        DrawCircleV((Vector2){p.x, p.y + 12}, 3, WHITE);
    }
    else if (t == SHIP_TITAN)
    {
        // Dreadnought-class Titan
        // Base Hull (Massive and Dark)
        DrawTriangle((Vector2){p.x, p.y + 60}, (Vector2){p.x - 65, p.y - 40}, (Vector2){p.x + 65, p.y - 40}, (Color){60, 0, 0, 255});
        
        // Internal Structure (Glow through plates)
        float structPulse = 0.5f + 0.5f * sinf(time * 4.0f);
        DrawTriangle((Vector2){p.x, p.y + 50}, (Vector2){p.x - 50, p.y - 30}, (Vector2){p.x + 50, p.y - 30}, CAlpha(RED, (unsigned char)(100 + 50 * structPulse)));

        // Heavy Mandible Arms
        DrawTriangle((Vector2){p.x - 25, p.y + 35}, (Vector2){p.x - 45, p.y + 55}, (Vector2){p.x - 10, p.y + 25}, (Color){100, 10, 10, 255});
        DrawTriangle((Vector2){p.x + 25, p.y + 35}, (Vector2){p.x + 10, p.y + 25}, (Vector2){p.x + 45, p.y + 55}, (Color){100, 10, 10, 255});

        // Armor Plating (Beveled and Shadowed)
        DrawRectangleRounded((Rectangle){p.x - 70, p.y - 30, 30, 55}, 0.2f, 8, (Color){40, 0, 0, 255});
        DrawRectangleRounded((Rectangle){p.x + 40, p.y - 30, 30, 55}, 0.2f, 8, (Color){40, 0, 0, 255});
        DrawRectangleRoundedLines((Rectangle){p.x - 70, p.y - 30, 30, 55}, 0.2f, 8, (Color){80, 20, 20, 255});
        DrawRectangleRoundedLines((Rectangle){p.x + 40, p.y - 30, 30, 55}, 0.2f, 8, (Color){80, 20, 20, 255});
        
        // Command bridge Windows
        DrawRectangleV((Vector2){p.x - 20, p.y - 15}, (Vector2){40, 25}, (Color){20, 20, 25, 255});
        for (int i = 0; i < 3; i++) {
            DrawRectangleV((Vector2){p.x - 15 + i * 12, p.y - 10}, (Vector2){8, 12}, CAlpha(RED, (unsigned char)(100 + 100 * sinf(time * 5.0f + i))));
        }
        
        // Engine Complex
        unsigned char ea = (unsigned char)(200 + GetRandomValue(0, 55));
        DrawCircleV((Vector2){p.x - 35, p.y - 40}, 12, CAlpha(RED, ea));
        DrawCircleV((Vector2){p.x, p.y - 45}, 18, CAlpha(RED, ea));
        DrawCircleV((Vector2){p.x + 35, p.y - 40}, 12, CAlpha(RED, ea));
        
        // Super-Core Reactor
        float reactorPulse = 0.5f + 0.5f * sinf(time * 2.0f);
        DrawCircleGradient((int)p.x, (int)p.y + 20, 25 + 15 * reactorPulse, YELLOW, BLANK);
        DrawCircleV((Vector2){p.x, p.y + 20}, 10 * reactorPulse, WHITE);
    }
}
