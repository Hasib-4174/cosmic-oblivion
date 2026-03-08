#include "include/enemy.h"
#include "include/helpers.h"
#include "include/particles.h"
#include "include/ship.h"
#include <math.h>

extern GameState G;

void InitEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        G.enemies[i].active = false;
    }
}

void SpawnEnemy(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (G.enemies[i].active)
            continue;

        Enemy *e = &G.enemies[i];
        e->active = true;
        e->pos = (Vector2){Rf(40, SW - 40), -40};
        e->vel = (Vector2){0, 0};
        e->speed = Rf(80, 150);
        e->hp = 2;
        e->fireCooldown = Rf(1.0f, 2.5f);
        e->rotation = 180.0f; // Facing down
        return;
    }
}

void UpdateEnemies(float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!G.enemies[i].active)
            continue;

        Enemy *e = &G.enemies[i];

        // Simple AI: Follow player X, stay above player Y
        Vector2 target = G.player.pos;
        target.y -= 250.0f; // Keep distance
        if (target.y < 50)
            target.y = 50;

        Vector2 dir = {target.x - e->pos.x, target.y - e->pos.y};
        float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (dist > 5.0f)
        {
            dir.x /= dist;
            dir.y /= dist;
            e->vel.x += (dir.x * e->speed - e->vel.x) * dt * 3.0f;
            e->vel.y += (dir.y * e->speed - e->vel.y) * dt * 3.0f;
        }

        e->pos.x += e->vel.x * dt;
        e->pos.y += e->vel.y * dt;

        // Shooting logic
        e->fireCooldown -= dt;
        if (e->fireCooldown <= 0)
        {
            e->fireCooldown = Rf(1.5f, 3.0f);
            // Spawn enemy bullet
            for (int b = 0; b < MAX_BULLETS; b++)
            {
                if (!G.bullets[b].active)
                {
                    G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y + 20};
                    G.bullets[b].speed = 400.0f;
                    G.bullets[b].active = true;
                    G.bullets[b].isEnemy = true;
                    // Note: In a real implementation we'd probably call a Sound play function here
                    // but we'll stick to logic for now and integrate sound later if needed.
                    break;
                }
            }
        }

        // Screen bounds (stay in top 20% of screen)
        e->pos.x = Clampf(e->pos.x, 20, SW - 20);
        e->pos.y = Clampf(e->pos.y, -100, SH * 0.2f);
        if (e->pos.y > SH + 100)
            e->active = false; // Just in case, though clamped

        // Overlap prevention (Separation force)
        for (int j = 0; j < MAX_ENEMIES; j++)
        {
            if (i == j || !G.enemies[j].active)
                continue;

            Enemy *other = &G.enemies[j];
            float dx = e->pos.x - other->pos.x;
            float dy = e->pos.y - other->pos.y;
            float distSq = dx * dx + dy * dy;
            float minDist = 50.0f;
            if (distSq < minDist * minDist && distSq > 0.01f)
            {
                float dist = sqrtf(distSq);
                float force = (minDist - dist) / minDist;
                e->vel.x += (dx / dist) * force * dt * 400.0f;
                e->vel.y += (dy / dist) * force * dt * 400.0f;
            }
        }
    }
}

void DrawEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (G.enemies[i].active)
        {
            DrawEnemyShip(G.enemies[i].pos, G.enemies[i].rotation);
        }
    }
}
