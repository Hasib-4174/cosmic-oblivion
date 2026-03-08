#include "include/shieldpickup.h"
#include "include/constants.h"
#include "include/helpers.h"
#include <math.h>

extern GameState G;

void InitShieldPickups(void)
{
    for (int i = 0; i < MAX_SHIELD_PICKUPS; i++)
    {
        G.shieldPickups[i].active = false;
        G.shieldPickups[i].pos = (Vector2){-100, -100};
    }
}

void SpawnShieldPickupAt(Vector2 pos)
{
    for (int i = 0; i < MAX_SHIELD_PICKUPS; i++)
    {
        if (!G.shieldPickups[i].active)
        {
            G.shieldPickups[i].active = true;
            G.shieldPickups[i].pos = pos;
            G.shieldPickups[i].vel = (Vector2){Rf(-40, 40), Rf(40, 90)};
            G.shieldPickups[i].rotation = (float)GetRandomValue(0, 360);
            G.shieldPickups[i].pulsePhase = Rf(0, 6.28f);
            return;
        }
    }
}

void UpdateShieldPickups(float dt)
{
    for (int i = 0; i < MAX_SHIELD_PICKUPS; i++)
    {
        ShieldPickup *sp = &G.shieldPickups[i];
        if (!sp->active)
            continue;
        sp->pos.x += sp->vel.x * dt;
        sp->pos.y += sp->vel.y * dt;
        sp->vel.x *= 0.98f;
        sp->rotation += 60.0f * dt;
        sp->pulsePhase += dt * 4.0f;
        if (sp->pos.y > SH + 30 || sp->pos.x < -30 || sp->pos.x > SW + 30)
            sp->active = false;
    }
}

void DrawShieldPickups(void)
{
    for (int i = 0; i < MAX_SHIELD_PICKUPS; i++)
    {
        ShieldPickup *sp = &G.shieldPickups[i];
        if (!sp->active)
            continue;
        float pulse = 0.7f + 0.3f * sinf(sp->pulsePhase);
        float size = 14.0f * pulse;
        Vector2 p = sp->pos;
        /* Blue/cyan glow rings */
        DrawCircleV(p, size + 8, CAlpha((Color){50, 150, 255, 100}, (unsigned char)(80 * pulse)));
        DrawCircleV(p, size + 4, CAlpha((Color){100, 200, 255, 150}, (unsigned char)(150 * pulse)));
        /* Shield hexagon shape */
        DrawPoly((Vector2){(int)p.x, (int)p.y}, 6, size, sp->rotation, (Color){50, 160, 255, 255});
        DrawPoly((Vector2){(int)p.x, (int)p.y}, 6, size * 0.6f, sp->rotation + 30, (Color){150, 220, 255, 255});
        DrawCircleV(p, 4, WHITE);
    }
}
