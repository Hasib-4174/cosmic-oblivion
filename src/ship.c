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
        // HYPER-DETAILED INTERCEPTOR (The Needle)
        // ----------------------------------------
        // Layer 0: Engine Bloom
        if (engineOn) {
            float bloom = 0.6f + 0.4f * sinf(time * 35.0f);
            DrawCircleGradient((int)p.x, (int)p.y + 35, 25 + bloom * 12, CAlpha((Color){0, 150, 255, 255}, 60), BLANK);
        }

        // Animated Flaps
        float flapAngle = sinf(time * 5.0f) * 10.0f; // degrees of sweep
        Vector2 leftFlapTip = {p.x - 30 - flapAngle * 0.2f, p.y + 25 + flapAngle * 0.5f};
        Vector2 rightFlapTip = {p.x + 30 + flapAngle * 0.2f, p.y + 25 + flapAngle * 0.5f};

        // Layer 1: Under-Chassis / Dark Framework
        DrawTriangle((Vector2){p.x - 28, p.y + 25}, (Vector2){p.x, p.y - 10}, (Vector2){p.x - 10, p.y + 28}, (Color){15, 20, 30, 255});
        DrawTriangle((Vector2){p.x + 28, p.y + 25}, (Vector2){p.x + 10, p.y + 28}, (Vector2){p.x, p.y - 10}, (Color){15, 20, 30, 255});
        DrawTriangle((Vector2){p.x, p.y - 42}, (Vector2){p.x + 8, p.y + 10}, (Vector2){p.x - 8, p.y + 10}, (Color){10, 15, 20, 255});

        // Layer 2: Main Wing Plates (Swept)
        DrawTriangle(leftFlapTip, (Vector2){p.x, p.y - 15}, (Vector2){p.x - 12, p.y + 20}, (Color){20, 80, 180, 255});
        DrawTriangle(rightFlapTip, (Vector2){p.x + 12, p.y + 20}, (Vector2){p.x, p.y - 15}, (Color){20, 80, 180, 255});
        
        // Micro-paneling on wings
        DrawTriangle((Vector2){p.x - 20, p.y + 18}, (Vector2){p.x - 5, p.y - 5}, (Vector2){p.x - 10, p.y + 22}, (Color){40, 130, 240, 255});
        DrawTriangle((Vector2){p.x + 20, p.y + 18}, (Vector2){p.x + 10, p.y + 22}, (Vector2){p.x + 5, p.y - 5}, (Color){40, 130, 240, 255});
        
        // Dynamic leading edge
        DrawLineEx(leftFlapTip, (Vector2){p.x - 2, p.y - 16}, 2.5f, CAlpha((Color){100, 200, 255, 255}, 200));
        DrawLineEx(rightFlapTip, (Vector2){p.x + 2, p.y - 16}, 2.5f, CAlpha((Color){100, 200, 255, 255}, 200));

        // Layer 3: Central Fuselage (Layered)
        DrawTriangle((Vector2){p.x, p.y - 38}, (Vector2){p.x + 12, p.y + 15}, (Vector2){p.x - 12, p.y + 15}, (Color){30, 35, 50, 255}); 
        DrawTriangle((Vector2){p.x, p.y - 34}, (Vector2){p.x + 8, p.y + 12}, (Vector2){p.x - 8, p.y + 12}, (Color){50, 60, 80, 255}); 
        DrawTriangle((Vector2){p.x, p.y - 28}, (Vector2){p.x + 4, p.y + 8}, (Vector2){p.x - 4, p.y + 8}, (Color){100, 120, 150, 255}); // Highlight ridge

        // Layer 4: Exposed Wiring/Pipes
        DrawLineV((Vector2){p.x - 5, p.y + 10}, (Vector2){p.x - 5, p.y + 18}, (Color){200, 150, 50, 255});
        DrawLineV((Vector2){p.x + 5, p.y + 10}, (Vector2){p.x + 5, p.y + 18}, (Color){200, 150, 50, 255});

        // Layer 5: High-Tech Cockpit Canopy (Multi-glass)
        DrawCircleV((Vector2){p.x, p.y - 10}, 6, (Color){10, 20, 30, 255}); // Frame
        DrawCircleV((Vector2){p.x, p.y - 10}, 4, (Color){0, 200, 255, 200}); // Glass base
        DrawTriangle((Vector2){p.x, p.y - 14}, (Vector2){p.x + 3, p.y - 6}, (Vector2){p.x - 3, p.y - 6}, (Color){150, 240, 255, 200}); // Glass glare

        // Layer 6: Dynamic Nav Lights (Strobe effect)
        float strobe = fmodf(time, 1.0f);
        if (strobe < 0.1f) {
            DrawCircleV((Vector2){leftFlapTip.x + 2, leftFlapTip.y - 2}, 3, RED);
            DrawCircleV((Vector2){p.x, p.y - 35}, 2, WHITE);
            DrawCircleGradient((int)leftFlapTip.x + 2, (int)leftFlapTip.y - 2, 8, CAlpha(RED, 150), BLANK);
        } else if (strobe > 0.5f && strobe < 0.6f) {
            DrawCircleV((Vector2){rightFlapTip.x - 2, rightFlapTip.y - 2}, 3, GREEN);
            DrawCircleGradient((int)rightFlapTip.x - 2, (int)rightFlapTip.y - 2, 8, CAlpha(GREEN, 150), BLANK);
        }

        // Layer 7: Complex Engine Exhaust
        if (engineOn)
        {
            unsigned char ea = (unsigned char)(180 + GetRandomValue(0, 75));
            // Turbine ring
            DrawEllipse((int)p.x, (int)p.y + 18, 8, 4, (Color){20, 20, 30, 255});
            // Multi-stage exhaust
            DrawCircleV((Vector2){p.x, p.y + 24}, 9, CAlpha((Color){0, 150, 255, 255}, (unsigned char)(ea * 0.7f)));
            DrawCircleV((Vector2){p.x, p.y + 26}, 6, CAlpha(WHITE, ea));
            // Sputtering spark particles
            for(int i=0; i<3; i++) {
                if(GetRandomValue(0,10)>5) {
                    DrawRectangle((int)p.x - 2 + GetRandomValue(-4, 4), (int)p.y + 28 + GetRandomValue(0, 10), 1, 3, WHITE);
                }
            }
        }
    }
    else if (t == SHIP_DESTROYER)
    {
        // HYPER-DETAILED DESTROYER (The Gunship)
        // ----------------------------------------
        // Layer 0: Shadow/Base Outline (Thick rectangular blocks)
        DrawRectangleV((Vector2){p.x - 24, p.y - 14}, (Vector2){48, 42}, (Color){20, 20, 25, 255});
        
        // Layer 1: Multi-layered Armor Plating
        DrawRectangleV((Vector2){p.x - 22, p.y - 12}, (Vector2){44, 38}, (Color){40, 40, 50, 255}); // Main Deck
        DrawRectangleV((Vector2){p.x - 18, p.y - 8}, (Vector2){16, 18}, (Color){60, 60, 80, 255}); // Left Plate
        DrawRectangleV((Vector2){p.x + 2, p.y - 8}, (Vector2){16, 18}, (Color){60, 60, 80, 255}); // Right Plate
        
        // Bolts/Rivets on plates
        for (int i=0; i<3; i++) {
            DrawRectangleV((Vector2){p.x - 16, p.y - 6 + i*6}, (Vector2){2, 2}, (Color){30, 30, 40, 255});
            DrawRectangleV((Vector2){p.x + 14, p.y - 6 + i*6}, (Vector2){2, 2}, (Color){30, 30, 40, 255});
        }

        // Layer 2: Sloped Front Armor (Layered triangles)
        DrawTriangle((Vector2){p.x - 24, p.y - 14}, (Vector2){p.x, p.y - 36}, (Vector2){p.x + 24, p.y - 14}, (Color){30, 30, 40, 255});
        DrawTriangle((Vector2){p.x - 20, p.y - 12}, (Vector2){p.x, p.y - 32}, (Vector2){p.x + 20, p.y - 12}, (Color){70, 70, 90, 255});
        DrawTriangle((Vector2){p.x - 10, p.y - 12}, (Vector2){p.x, p.y - 28}, (Vector2){p.x + 10, p.y - 12}, (Color){100, 100, 120, 255}); // Center slope

        // Layer 3: Animated Heat Vents (Complex array)
        float ventPulse = powf(sinf(time * 6.0f), 2.0f); // Fast, sharp pulsing
        Color ventGlow = CLerp((Color){50, 20, 10, 255}, (Color){255, 120, 20, 255}, ventPulse);
        Color ventGlowBright = CLerp((Color){50, 50, 50, 255}, WHITE, ventPulse);
        for(int i=0; i<4; i++) {
            // Left Array
            DrawRectangleV((Vector2){p.x - 32, p.y - 5 + i*8}, (Vector2){8, 4}, (Color){20, 20, 20, 255});
            DrawRectangleV((Vector2){p.x - 30, p.y - 4 + i*8}, (Vector2){5, 2}, ventGlow);
            DrawRectangleV((Vector2){p.x - 28, p.y - 4 + i*8}, (Vector2){3, 2}, ventGlowBright);
            // Right Array
            DrawRectangleV((Vector2){p.x + 24, p.y - 5 + i*8}, (Vector2){8, 4}, (Color){20, 20, 20, 255});
            DrawRectangleV((Vector2){p.x + 25, p.y - 4 + i*8}, (Vector2){5, 2}, ventGlow);
            DrawRectangleV((Vector2){p.x + 25, p.y - 4 + i*8}, (Vector2){3, 2}, ventGlowBright);
        }
        
        // Layer 4: Rotating Radar/Sensor Dish
        float radarRot = time * 2.0f;
        Vector2 radarCenter = {p.x, p.y + 12};
        DrawCircleV(radarCenter, 7, (Color){30, 30, 40, 255});
        DrawLineEx(radarCenter, (Vector2){radarCenter.x + cosf(radarRot)*6, radarCenter.y + sinf(radarRot)*6}, 2.0f, CAlpha(GREEN, 150));
        DrawCircleV(radarCenter, 2, (Color){50, 150, 50, 255});

        // Layer 5: Elevated Command Bridge
        DrawRectangleV((Vector2){p.x - 12, p.y - 20}, (Vector2){24, 12}, (Color){15, 15, 20, 255}); // Base
        DrawRectangleV((Vector2){p.x - 10, p.y - 18}, (Vector2){20, 8}, (Color){30, 40, 60, 255});   // Tier 2
        // Glowing bridge windows
        for(int i=0; i<3; i++) {
            DrawRectangleV((Vector2){p.x - 7 + i*6, p.y - 16}, (Vector2){4, 4}, (Color){0, 150, 255, 200});
        }

        // Layer 6: Twin Overcharged Thrusters
        if (engineOn)
        {
            unsigned char ea = (unsigned char)(140 + GetRandomValue(0, 115));
            // Engine cowlings
            DrawRectangleV((Vector2){p.x - 20, p.y + 24}, (Vector2){14, 6}, (Color){20, 20, 20, 255});
            DrawRectangleV((Vector2){p.x + 6, p.y + 24}, (Vector2){14, 6}, (Color){20, 20, 20, 255});
            
            // Thrust core
            DrawRectangleGradientV((int)p.x - 18, (int)p.y + 30, 10, 20, CAlpha(ORANGE, ea), BLANK);
            DrawRectangleGradientV((int)p.x + 8, (int)p.y + 30, 10, 20, CAlpha(ORANGE, ea), BLANK);
            
            // Intense inner flame
            DrawRectangleV((Vector2){p.x - 15, p.y + 30}, (Vector2){4, 10}, CAlpha(YELLOW, ea));
            DrawRectangleV((Vector2){p.x + 11, p.y + 30}, (Vector2){4, 10}, CAlpha(YELLOW, ea));
        }
    }
    else // SHIP_TITAN (Player version)
    {
        // HYPER-DETAILED TITAN (The Moving Fortress)
        // ----------------------------------------
        // Layer 0: Giant Reactor Bloom
        if (engineOn) {
            float bloom = 0.5f + 0.5f * sinf(time * 12.0f);
            DrawCircleGradient((int)p.x, (int)p.y + 30, 45 + (int)(bloom * 20), CAlpha(PURPLE, 70), BLANK);
        }

        // Layer 1: Massive Segmented Under-chassis
        DrawRectangleV((Vector2){p.x - 30, p.y - 15}, (Vector2){60, 45}, (Color){15, 10, 20, 255}); // Main block
        DrawTriangle((Vector2){p.x - 35, p.y - 15}, (Vector2){p.x, p.y - 50}, (Vector2){p.x + 35, p.y - 15}, (Color){20, 15, 25, 255}); // Nose block
        
        // Internal glow strips (ribs)
        for(int i=0; i<4; i++) {
            DrawRectangleV((Vector2){p.x - 12, p.y - 30 + i*10}, (Vector2){24, 2}, CAlpha((Color){100, 50, 255, 255}, 100));
        }

        // Layer 2: Heavy Sponson Armor (Layered with deep bevels & moving parts)
        Color sponsonBase = (Color){25, 20, 35, 255};
        Color sponsonHigh = (Color){45, 40, 60, 255};
        float sponsonShift = sinf(time * 2.0f) * 2.0f; // Sponsons slowly breathe/shift
        
        // Left Sponson
        DrawRectangleV((Vector2){p.x - 50 - sponsonShift, p.y - 10}, (Vector2){24, 52}, sponsonBase);
        DrawRectangleV((Vector2){p.x - 47 - sponsonShift, p.y - 7}, (Vector2){18, 46}, sponsonHigh);
        DrawRectangleLines((int)(p.x - 47 - sponsonShift), (int)p.y - 7, 18, 46, (Color){60, 50, 80, 255});
        
        // Right Sponson
        DrawRectangleV((Vector2){p.x + 26 + sponsonShift, p.y - 10}, (Vector2){24, 52}, sponsonBase);
        DrawRectangleV((Vector2){p.x + 29 + sponsonShift, p.y - 7}, (Vector2){18, 46}, sponsonHigh);
        DrawRectangleLines((int)(p.x + 29 + sponsonShift), (int)p.y - 7, 18, 46, (Color){60, 50, 80, 255});

        // Layer 3: Active Weapon Systems & Railguns
        // Left Railgun (Recoil animation based on time)
        float recoilL = (fmodf(time * 3.0f, 1.0f) < 0.1f) ? -5.0f : 0.0f;
        DrawRectangleV((Vector2){p.x - 44 - sponsonShift, p.y - 25 + recoilL}, (Vector2){12, 20}, (Color){40, 40, 40, 255}); // Barrel
        DrawTriangle((Vector2){p.x - 44 - sponsonShift, p.y - 25 + recoilL}, (Vector2){p.x - 38 - sponsonShift, p.y - 42 + recoilL}, (Vector2){p.x - 32 - sponsonShift, p.y - 25 + recoilL}, (Color){80, 80, 90, 255}); // Muzzle
        DrawRectangleV((Vector2){p.x - 40 - sponsonShift, p.y - 35 + recoilL}, (Vector2){4, 12}, CAlpha(SKYBLUE, (unsigned char)(150 + 100*sinf(time*12.0f)))); // Energy charge
        
        // Right Railgun (Offset recoil)
        float recoilR = (fmodf(time * 3.0f + 0.5f, 1.0f) < 0.1f) ? -5.0f : 0.0f;
        DrawRectangleV((Vector2){p.x + 32 + sponsonShift, p.y - 25 + recoilR}, (Vector2){12, 20}, (Color){40, 40, 40, 255}); // Barrel
        DrawTriangle((Vector2){p.x + 32 + sponsonShift, p.y - 25 + recoilR}, (Vector2){p.x + 38 + sponsonShift, p.y - 42 + recoilR}, (Vector2){p.x + 44 + sponsonShift, p.y - 25 + recoilR}, (Color){80, 80, 90, 255}); // Muzzle
        DrawRectangleV((Vector2){p.x + 36 + sponsonShift, p.y - 35 + recoilR}, (Vector2){4, 12}, CAlpha(SKYBLUE, (unsigned char)(150 + 100*sinf(time*12.0f)))); // Energy charge

        // Layer 4: Central Reactor / Bridge Complex
        // Central thick plating (overlapping layers)
        DrawRectangleV((Vector2){p.x - 18, p.y - 8}, (Vector2){36, 28}, (Color){45, 40, 60, 255});
        DrawRectangleV((Vector2){p.x - 14, p.y - 4}, (Vector2){28, 20}, (Color){60, 55, 80, 255});
        DrawRectangleV((Vector2){p.x - 10, p.y}, (Vector2){20, 12}, (Color){80, 75, 100, 255});
        
        // Animated Power Core (Rotating & Pulsing)
        float coreAngle = time * 3.0f;
        float coreScale = 0.8f + 0.2f * sinf(time * 8.0f);
        Vector2 coreCenter = {p.x, p.y + 6};
        DrawCircleGradient((int)coreCenter.x, (int)coreCenter.y, 10 * coreScale, CAlpha(WHITE, 255), CAlpha(PURPLE, 0));
        // Rotating inner ring
        for(int i=0; i<3; i++) {
            float ang = coreAngle + i*(PI*2/3);
            DrawCircleV((Vector2){coreCenter.x + cosf(ang)*6, coreCenter.y + sinf(ang)*6}, 2, WHITE);
        }

        // The Bridge Tower
        DrawRectangleV((Vector2){p.x - 10, p.y - 28}, (Vector2){20, 18}, (Color){20, 15, 25, 255});
        DrawTriangle((Vector2){p.x - 12, p.y - 12}, (Vector2){p.x, p.y - 34}, (Vector2){p.x + 12, p.y - 12}, (Color){10, 5, 15, 255});
        // Sensor Array (Scanning line)
        float scanY = p.y - 26 + fmodf(time * 10.0f, 14.0f);
        DrawRectangleV((Vector2){p.x - 8, p.y - 26}, (Vector2){16, 14}, (Color){10, 20, 30, 255}); // Window
        DrawLineV((Vector2){p.x - 8, scanY}, (Vector2){p.x + 8, scanY}, CAlpha(GREEN, 200));

        // Layer 5: Gigantic Articulated Thruster Array
        if (engineOn)
        {
            unsigned char ea = (unsigned char)(200 + GetRandomValue(0, 55));
            // Mechanical housing & thrust vectoring flaps (animated)
            DrawRectangleV((Vector2){p.x - 22, p.y + 30}, (Vector2){44, 10}, (Color){30, 25, 40, 255});
            DrawRectangleV((Vector2){p.x - 16, p.y + 40}, (Vector2){32, 6}, (Color){15, 10, 20, 255});
            
            // Outer Sponson Thrusters (Small, attached to moving sponsons)
            DrawRectangleGradientV((int)(p.x - 42 - sponsonShift), (int)p.y + 42, 14, 20, CAlpha(VIOLET, ea), BLANK);
            DrawRectangleGradientV((int)(p.x + 28 + sponsonShift), (int)p.y + 42, 14, 20, CAlpha(VIOLET, ea), BLANK);
            DrawCircleV((Vector2){p.x - 35 - sponsonShift, p.y + 42}, 3, WHITE);
            DrawCircleV((Vector2){p.x + 35 + sponsonShift, p.y + 42}, 3, WHITE);
            
            // Main Central Plasma Drive (Massive & Violent)
            float driveFlicker = sinf(time * 50.0f) * 8.0f;
            DrawRectangleGradientV((int)p.x - 20, (int)p.y + 46, 40, 35 + (int)driveFlicker, CAlpha((Color){200, 50, 255, 255}, ea), BLANK);
            DrawCircleV((Vector2){p.x - 8, p.y + 47}, 5, WHITE);
            DrawCircleV((Vector2){p.x + 8, p.y + 47}, 5, WHITE);
            
            // Core shock diamonds (Supersonic exhaust rings)
            for(int i=0; i<3; i++) {
                float yOffset = 55 + i * 12 + fmodf(time * 40.0f, 12.0f);
                if (yOffset - 55 < 35 + driveFlicker) { // Only draw within flame
                    DrawEllipseLines((int)p.x, (int)(p.y + yOffset), 12 - i*2, 3, CAlpha(WHITE, 150 - i*40));
                }
            }
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
        // HYPER-DETAILED ENEMY INTERCEPTOR (The Stalker)
        // ----------------------------------------
        // Layer 0: Shifting Void Energy Halos (Multiple intersecting rings)
        float voidTime = time * 3.5f;
        for (int i = 0; i < 4; i++) {
            float angle = voidTime + i * (PI / 2.0f);
            float radiusOffset = sinf(time * 5.0f + i) * 5.0f;
            Vector2 offset = {cosf(angle) * (18 + radiusOffset), sinf(angle) * (18 + radiusOffset)};
            DrawCircleGradient((int)(p.x + offset.x), (int)(p.y + offset.y), 10, CAlpha((Color){150, 0, 50, 255}, 50), BLANK);
        }

        // Layer 1: Skeletal Inner Frame (Dark and jagged)
        DrawTriangle((Vector2){p.x, p.y + 45}, (Vector2){p.x - 6, p.y - 15}, (Vector2){p.x + 6, p.y - 15}, (Color){15, 0, 0, 255});
        DrawTriangle((Vector2){p.x, p.y - 10}, (Vector2){p.x - 15, p.y + 10}, (Vector2){p.x + 15, p.y + 10}, (Color){20, 5, 5, 255});
        
        // Erratic Twitching (Simulating instability)
        float twitchX = (GetRandomValue(0, 10) > 8) ? Rf(-1.5f, 1.5f) : 0.0f;
        float twitchY = (GetRandomValue(0, 10) > 8) ? Rf(-1.5f, 1.5f) : 0.0f;
        p.x += twitchX;
        p.y += twitchY;

        // Layer 2: Segmented Articulated Scythe Wings
        float wingBreathe = sinf(time * 8.0f) * 4.0f; // Wings open and close slightly
        
        // Left Scythe Complex
        Vector2 lWingBase = {p.x - 10, p.y};
        Vector2 lWingTipOuter = {p.x - 45 - wingBreathe, p.y - 45 - wingBreathe};
        Vector2 lWingTipInner = {p.x - 20 - wingBreathe*0.5f, p.y - 30};
        DrawTriangle((Vector2){p.x, p.y - 5}, lWingTipOuter, lWingBase, (Color){40, 0, 10, 255}); // Main blade
        DrawTriangle((Vector2){p.x - 5, p.y - 10}, lWingTipInner, (Vector2){p.x - 12, p.y}, (Color){70, 0, 15, 255}); // Inner panel
        
        // Right Scythe Complex
        Vector2 rWingBase = {p.x + 10, p.y};
        Vector2 rWingTipOuter = {p.x + 45 + wingBreathe, p.y - 45 - wingBreathe};
        Vector2 rWingTipInner = {p.x + 20 + wingBreathe*0.5f, p.y - 30};
        DrawTriangle((Vector2){p.x, p.y - 5}, rWingBase, rWingTipOuter, (Color){40, 0, 10, 255}); // Main blade
        DrawTriangle((Vector2){p.x + 5, p.y - 10}, (Vector2){p.x + 12, p.y}, rWingTipInner, (Color){70, 0, 15, 255}); // Inner panel

        // Layer 3: Searing Energy Blade Edges
        float bladePulse = 0.5f + 0.5f * sinf(time * 25.0f); // Rapid, dangerous strobing
        Color bladeColor = CAlpha(RED, (unsigned char)(150 + 100 * bladePulse));
        // Main outer edges
        DrawLineEx(lWingBase, lWingTipOuter, 2.5f, bladeColor);
        DrawLineEx(rWingBase, rWingTipOuter, 2.5f, bladeColor);
        // Inner fracture lines
        DrawLineEx((Vector2){p.x - 15, p.y - 10}, lWingTipInner, 1.5f, CAlpha(ORANGE, 150));
        DrawLineEx((Vector2){p.x + 15, p.y - 10}, rWingTipInner, 1.5f, CAlpha(ORANGE, 150));

        // Layer 4: Forward Piercing Spear (Multi-layered)
        DrawTriangle((Vector2){p.x, p.y + 40}, (Vector2){p.x - 4, p.y + 10}, (Vector2){p.x + 4, p.y + 10}, (Color){60, 0, 0, 255});
        DrawTriangle((Vector2){p.x, p.y + 36}, (Vector2){p.x - 2, p.y + 15}, (Vector2){p.x + 2, p.y + 15}, RED); // Core heat
        DrawLineV((Vector2){p.x, p.y + 15}, (Vector2){p.x, p.y + 45}, CAlpha(WHITE, 150)); // Center highlight

        // Layer 5: The "Eye" (Unstable Sensor Cluster)
        float eyeLook = sinf(time * 2.0f) * 3.0f; // Eye slowly scans left/right
        DrawRectangleV((Vector2){p.x - 6, p.y + 5}, (Vector2){12, 5}, (Color){20, 0, 0, 255}); // Socket
        DrawRectangleV((Vector2){p.x - 4 + eyeLook, p.y + 6}, (Vector2){8, 3}, RED); // Main optic
        DrawRectangleV((Vector2){p.x - 2 + eyeLook, p.y + 7}, (Vector2){4, 1}, WHITE); // Staring point
        
        // Occasional sensor flare
        if (fmodf(time, 2.0f) < 0.1f) {
            DrawLineEx((Vector2){p.x - 15, p.y + 7}, (Vector2){p.x + 15, p.y + 7}, 1.0f, CAlpha(WHITE, 200));
        }

        // Layer 6: Leaking Void Particles & Exhaust
        for(int i=0; i<3; i++) {
            if (GetRandomValue(0, 10) > 6) {
                Vector2 leakPos = {p.x + Rf(-15, 15), p.y + Rf(-15, 20)};
                DrawCircleV(leakPos, Rf(1.0f, 2.5f), CAlpha(MAROON, 150));
            }
        }
        
        // Thrust Trail
        unsigned char ea = (unsigned char)(100 + GetRandomValue(0, 55));
        float thrustLen = 15.0f + sinf(time * 40.0f) * 5.0f;
        DrawTriangle((Vector2){p.x - 4, p.y - 10}, (Vector2){p.x, p.y - 10 - thrustLen}, (Vector2){p.x + 4, p.y - 10}, CAlpha(RED, ea));
    }
    else if (t == SHIP_DESTROYER)
    {
        // HYPER-DETAILED ENEMY DESTROYER (The Gunship)
        // ----------------------------------------
        // Layer 0: Shadow/Base Outline
        DrawTriangle((Vector2){p.x, p.y + 48}, (Vector2){p.x - 42, p.y - 28}, (Vector2){p.x + 42, p.y - 28}, (Color){40, 0, 5, 255});
        
        // Layer 1: Heavy Armor Plating (Segmented)
        DrawTriangle((Vector2){p.x, p.y + 35}, (Vector2){p.x - 30, p.y - 20}, (Vector2){p.x + 30, p.y - 20}, (Color){100, 10, 20, 255}); // Main plate
        DrawTriangle((Vector2){p.x, p.y + 25}, (Vector2){p.x - 20, p.y - 15}, (Vector2){p.x + 20, p.y - 15}, (Color){160, 30, 40, 255}); // Inner raised plate
        
        // Micro-panel details
        DrawLineV((Vector2){p.x, p.y + 10}, (Vector2){p.x, p.y + 30}, (Color){60, 0, 10, 255}); // Center seam
        for(int i=0; i<3; i++) {
            DrawLineV((Vector2){p.x - 10 - i*5, p.y - 5}, (Vector2){p.x - 15 - i*5, p.y - 15}, (Color){80, 10, 20, 255});
            DrawLineV((Vector2){p.x + 10 + i*5, p.y - 5}, (Vector2){p.x + 15 + i*5, p.y - 15}, (Color){80, 10, 20, 255});
        }

        // Layer 2: Animated Side Cannon Arrays
        float recoil = (fmodf(time * 2.0f, 1.0f) < 0.2f) ? 4.0f : 0.0f; // Rapid recoil
        // Left pods
        DrawRectangleV((Vector2){p.x - 45, p.y - 20 + recoil}, (Vector2){14, 35}, (Color){30, 0, 0, 255}); // Base
        DrawRectangleV((Vector2){p.x - 43, p.y - 15 + recoil}, (Vector2){10, 30}, (Color){80, 10, 10, 255});  // Plate
        DrawRectangleV((Vector2){p.x - 42, p.y + 8 + recoil}, (Vector2){8, 12}, CAlpha(ORANGE, 200));       // Heat venting
        // Right pods
        DrawRectangleV((Vector2){p.x + 31, p.y - 20 + recoil}, (Vector2){14, 35}, (Color){30, 0, 0, 255}); // Base
        DrawRectangleV((Vector2){p.x + 33, p.y - 15 + recoil}, (Vector2){10, 30}, (Color){80, 10, 10, 255});  // Plate
        DrawRectangleV((Vector2){p.x + 34, p.y + 8 + recoil}, (Vector2){8, 12}, CAlpha(ORANGE, 200));       // Heat venting

        // Layer 3: Engine Vents (Horizontal pulse)
        unsigned char ea = (unsigned char)(160 + GetRandomValue(0, 95));
        float ventWidth = 15.0f + 5.0f * sinf(time * 15.0f);
        DrawRectangleGradientH((int)p.x - 10 - (int)ventWidth, (int)p.y - 25, (int)ventWidth, 8, BLANK, CAlpha(ORANGE, ea));
        DrawRectangleGradientH((int)p.x + 10, (int)p.y - 25, (int)ventWidth, 8, CAlpha(ORANGE, ea), BLANK);
        
        // Vent grilles
        for(int i=0; i<4; i++) {
            DrawRectangle((int)p.x - 22 + i*3, (int)p.y - 25, 1, 8, (Color){20, 0, 0, 255});
            DrawRectangle((int)p.x + 10 + i*3, (int)p.y - 25, 1, 8, (Color){20, 0, 0, 255});
        }

        // Layer 4: Massive Exposed Reactor Core (Pulsing)
        float reactorPulse = 0.5f + 0.5f * sinf(time * 12.0f);
        DrawCircleGradient((int)p.x, (int)p.y + 4, 15 + reactorPulse * 8, CAlpha(RED, 200), BLANK);
        
        // Core housing
        DrawCircleV((Vector2){p.x, p.y + 4}, 10, (Color){30, 0, 0, 255});
        DrawCircleLines((int)p.x, (int)p.y + 4, 10, (Color){100, 20, 20, 255});
        
        // Inner core
        DrawCircleV((Vector2){p.x, p.y + 4}, 6, MAROON);
        DrawCircleV((Vector2){p.x, p.y + 4}, 3 + reactorPulse*2, WHITE);
        
        // Energy arcs off core
        if (reactorPulse > 0.8f) {
            DrawLineEx((Vector2){p.x, p.y + 4}, (Vector2){p.x + Rf(-12, 12), p.y + Rf(-12, 12)}, 1.5f, YELLOW);
        }
    }
    else if (t == SHIP_TITAN)
    {
        // HYPER-DETAILED ENEMY TITAN (The Dreadnought)
        // ----------------------------------------
        // Layer 0: Super-Core Bloom
        float corePulse = 0.5f + 0.5f * sinf(time * 5.0f);
        DrawCircleGradient((int)p.x, (int)p.y, 60 + corePulse * 20, CAlpha(RED, 50), BLANK);

        // Layer 1: Base Hull (Massive and Dark)
        DrawTriangle((Vector2){p.x, p.y + 60}, (Vector2){p.x - 70, p.y - 40}, (Vector2){p.x + 70, p.y - 40}, (Color){25, 5, 10, 255});
        DrawRectangleV((Vector2){p.x - 65, p.y - 45}, (Vector2){130, 20}, (Color){20, 0, 5, 255});
        
        // Layer 2: Internal Structure (Glow deeply embedded through plates)
        float structPulse = 0.5f + 0.5f * sinf(time * 3.0f);
        DrawTriangle((Vector2){p.x, p.y + 50}, (Vector2){p.x - 55, p.y - 30}, (Vector2){p.x + 55, p.y - 30}, CAlpha(MAROON, (unsigned char)(100 + 80 * structPulse)));
        // Horizontal glowing ribs
        for(int i=0; i<6; i++) {
            float widthOff = i * 8.0f;
            DrawRectangleV((Vector2){p.x - 40 + widthOff/2, p.y - 20 + i*10}, (Vector2){80 - widthOff, 3}, CAlpha(RED, 150));
        }

        // Layer 3: Heavy Mandible Arms (Articulated and layered)
        Color mandBase = (Color){50, 5, 15, 255};
        Color mandHigh = (Color){80, 10, 25, 255};
        
        // Left Mandible
        DrawTriangle((Vector2){p.x - 20, p.y + 30}, (Vector2){p.x - 50, p.y + 60}, (Vector2){p.x - 10, p.y + 20}, mandBase);
        DrawTriangle((Vector2){p.x - 22, p.y + 28}, (Vector2){p.x - 45, p.y + 52}, (Vector2){p.x - 12, p.y + 20}, mandHigh);
        DrawLineEx((Vector2){p.x - 50, p.y + 60}, (Vector2){p.x - 20, p.y + 30}, 2.0f, (Color){120, 20, 30, 255}); // Edge highlight
        
        // Right Mandible
        DrawTriangle((Vector2){p.x + 20, p.y + 30}, (Vector2){p.x + 10, p.y + 20}, (Vector2){p.x + 50, p.y + 60}, mandBase);
        DrawTriangle((Vector2){p.x + 22, p.y + 28}, (Vector2){p.x + 12, p.y + 20}, (Vector2){p.x + 45, p.y + 52}, mandHigh);
        DrawLineEx((Vector2){p.x + 50, p.y + 60}, (Vector2){p.x + 20, p.y + 30}, 2.0f, (Color){120, 20, 30, 255}); // Edge highlight

        // Layer 4: Floating/Shifting Armor Plating
        float plateShift = sinf(time * 1.5f) * 3.0f;
        // Left Plate Array
        DrawRectangleRounded((Rectangle){p.x - 75 + plateShift, p.y - 35, 30, 60}, 0.2f, 8, (Color){40, 5, 10, 255});
        DrawRectangleRounded((Rectangle){p.x - 70 + plateShift, p.y - 30, 20, 50}, 0.2f, 4, (Color){60, 10, 15, 255});
        DrawRectangleLines((int)(p.x - 75 + plateShift), (int)p.y - 35, 30, 60, (Color){100, 20, 20, 255});
        // Right Plate Array
        DrawRectangleRounded((Rectangle){p.x + 45 - plateShift, p.y - 35, 30, 60}, 0.2f, 8, (Color){40, 5, 10, 255});
        DrawRectangleRounded((Rectangle){p.x + 50 - plateShift, p.y - 30, 20, 50}, 0.2f, 4, (Color){60, 10, 15, 255});
        DrawRectangleLines((int)(p.x + 45 - plateShift), (int)p.y - 35, 30, 60, (Color){100, 20, 20, 255});
        
        // Layer 5: Command Bridge (Towering structure)
        DrawRectangleV((Vector2){p.x - 24, p.y - 25}, (Vector2){48, 30}, (Color){15, 5, 10, 255}); // Base
        DrawRectangleV((Vector2){p.x - 18, p.y - 20}, (Vector2){36, 20}, (Color){30, 10, 15, 255}); // Mid tier
        DrawRectangleV((Vector2){p.x - 10, p.y - 15}, (Vector2){20, 10}, (Color){45, 15, 25, 255}); // Top tier
        
        // Sinister scanning windows
        for (int i = 0; i < 4; i++) {
            float windowPulse = sinf(time * 3.0f + i) > 0.0f ? 1.0f : 0.3f;
            DrawRectangleV((Vector2){p.x - 16 + i * 9, p.y - 12}, (Vector2){6, 4}, CAlpha(RED, (unsigned char)(255 * windowPulse)));
        }
        
        // Layer 6: Multi-Stage Engine Complex
        unsigned char ea = (unsigned char)(200 + GetRandomValue(0, 55));
        // Outer thruster pods
        DrawRectangleV((Vector2){p.x - 45, p.y - 45}, (Vector2){24, 15}, (Color){20, 0, 5, 255});
        DrawRectangleV((Vector2){p.x + 21, p.y - 45}, (Vector2){24, 15}, (Color){20, 0, 5, 255});
        DrawCircleV((Vector2){p.x - 33, p.y - 40}, 10, CAlpha(RED, ea));
        DrawCircleV((Vector2){p.x + 33, p.y - 40}, 10, CAlpha(RED, ea));
        
        // Super-Core Reactor (Central)
        DrawCircleGradient((int)p.x, (int)p.y + 15, 30 + 10 * corePulse, CAlpha(YELLOW, 180), BLANK);
        DrawCircleV((Vector2){p.x, p.y + 15}, 12 * corePulse, WHITE);
        
        // Sputtering core plasma
        for(int i=0; i<5; i++) {
            if (GetRandomValue(0,10)>5) {
               DrawRectangle((int)p.x - 4 + GetRandomValue(-10, 10), (int)p.y + 15 + GetRandomValue(-10, 10), 2, 4, WHITE);
            }
        }
    }
}
