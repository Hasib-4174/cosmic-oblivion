#include "include/healthstar.h"
#include "include/constants.h"
#include "include/helpers.h"
#include <math.h>

extern GameState G;

void InitHealthStars(void)
{
    for (int i = 0; i < MAX_HEALTH_STARS; i++)
    {
        G.healthStars[i].active = false;
        G.healthStars[i].pos = (Vector2){-100, -100};
    }
}

void SpawnHealthStar(void)
{
    for (int i = 0; i < MAX_HEALTH_STARS; i++)
    {
        if (!G.healthStars[i].active)
        {
            G.healthStars[i].active = true;
            G.healthStars[i].pos = (Vector2){(float)GetRandomValue(40, SW - 40), -20};
            G.healthStars[i].vel = (Vector2){Rf(-30, 30), Rf(60, 100)};
            G.healthStars[i].rotation = (float)GetRandomValue(0, 360);
            G.healthStars[i].pulsePhase = Rf(0, 6.28f);
            return;
        }
    }
}

void SpawnHealthStarAt(Vector2 pos)
{
    for (int i = 0; i < MAX_HEALTH_STARS; i++)
    {
        if (!G.healthStars[i].active)
        {
            G.healthStars[i].active = true;
            G.healthStars[i].pos = pos;
            G.healthStars[i].vel = (Vector2){Rf(-40, 40), Rf(40, 90)};
            G.healthStars[i].rotation = (float)GetRandomValue(0, 360);
            G.healthStars[i].pulsePhase = Rf(0, 6.28f);
            return;
        }
    }
}

void UpdateHealthStars(float dt)
{
    for (int i = 0; i < MAX_HEALTH_STARS; i++)
    {
        HealthStar *hs = &G.healthStars[i];
        if (!hs->active)
            continue;
        hs->pos.x += hs->vel.x * dt;
        hs->pos.y += hs->vel.y * dt;
        hs->vel.x *= 0.98f;
        hs->rotation += 60.0f * dt;
        hs->pulsePhase += dt * 4.0f;
        if (hs->pos.y > SH + 30 || hs->pos.x < -30 || hs->pos.x > SW + 30)
            hs->active = false;
    }
}

void DrawHealthStars(void)
{
    for (int i = 0; i < MAX_HEALTH_STARS; i++)
    {
        HealthStar *hs = &G.healthStars[i];
        if (!hs->active)
            continue;
        float pulse = 0.7f + 0.3f * sinf(hs->pulsePhase);
        float size = 14.0f * pulse;
        Vector2 p = hs->pos;
        DrawCircleV(p, size + 8, CAlpha((Color){50, 255, 80, 100}, (unsigned char)(80 * pulse)));
        DrawCircleV(p, size + 4, CAlpha((Color){100, 255, 150, 150}, (unsigned char)(150 * pulse)));
        DrawPoly((Vector2){(int)p.x, (int)p.y}, 5, size, hs->rotation, (Color){50, 255, 100, 255});
        DrawPoly((Vector2){(int)p.x, (int)p.y}, 5, size * 0.6f, hs->rotation + 18, (Color){150, 255, 200, 255});
        DrawCircleV(p, 4, WHITE);
    }
}
