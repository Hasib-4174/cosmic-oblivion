#include "include/enemy.h"
#include "include/helpers.h"
#include "include/particles.h"
#include "include/ship.h"
#include "include/game.h"
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
        e->pos = (Vector2){Rf(60, SW - 60), -60};
        e->vel = (Vector2){0, 0};
        e->rotation = 180.0f; // Facing down

        // Weighted spawning
        int roll = GetRandomValue(0, 99);
        if (roll < 10) // 10% Titan
        {
            e->type = SHIP_TITAN;
            e->hp = 12;
            e->speed = Rf(30, 50);
            e->fireCooldown = Rf(2.5f, 4.0f);
        }
        else if (roll < 30) // 20% Destroyer
        {
            e->type = SHIP_DESTROYER;
            e->hp = 5;
            e->speed = Rf(60, 90);
            e->fireCooldown = Rf(1.0f, 2.0f);
        }
        else // 70% Interceptor
        {
            e->type = SHIP_INTERCEPTOR;
            e->hp = 2;
            e->speed = Rf(100, 160);
            e->fireCooldown = Rf(1.5f, 3.0f);
        }
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
            if (e->type == SHIP_TITAN)
            {
                e->fireCooldown = Rf(2.5f, 4.0f);
                // Triple shot
                float offsetsX[] = {-20, 0, 20};
                for (int s = 0; s < 3; s++)
                {
                    for (int b = 0; b < MAX_BULLETS; b++)
                    {
                        if (!G.bullets[b].active)
                        {
                            G.bullets[b].pos = (Vector2){e->pos.x + offsetsX[s], e->pos.y + 40};
                            G.bullets[b].speed = 350.0f;
                            G.bullets[b].active = true;
                            G.bullets[b].isEnemy = true;
                            break;
                        }
                    }
                }
                PlayEnemyShootSound();
            }
            else if (e->type == SHIP_DESTROYER)
            {
                e->fireCooldown = Rf(1.2f, 2.2f);
                // Dual shot
                float offsetsX[] = {-18, 18};
                for (int s = 0; s < 2; s++)
                {
                    for (int b = 0; b < MAX_BULLETS; b++)
                    {
                        if (!G.bullets[b].active)
                        {
                            G.bullets[b].pos = (Vector2){e->pos.x + offsetsX[s], e->pos.y + 30};
                            G.bullets[b].speed = 450.0f;
                            G.bullets[b].active = true;
                            G.bullets[b].isEnemy = true;
                            break;
                        }
                    }
                }
                PlayEnemyShootSound();
            }
            else // Interceptor
            {
                e->fireCooldown = Rf(1.5f, 3.0f);
                for (int b = 0; b < MAX_BULLETS; b++)
                {
                    if (!G.bullets[b].active)
                    {
                        G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y + 20};
                        G.bullets[b].speed = 500.0f;
                        G.bullets[b].active = true;
                        G.bullets[b].isEnemy = true;
                        break;
                    }
                }
                PlayEnemyShootSound();
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
            float minDist = (e->type == SHIP_TITAN || other->type == SHIP_TITAN) ? 100.0f : 60.0f;
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
            DrawEnemyShip(G.enemies[i].pos, G.enemies[i].type, G.enemies[i].rotation);
        }
    }
}
