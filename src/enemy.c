#include "include/enemy.h"
#include "include/helpers.h"
#include "include/particles.h"
#include "include/ship.h"
#include "include/game.h"
#include "include/collision.h"
#include <math.h>

extern GameState G;

static float VDist(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

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

        // Difficulty scaling based on time
        float diffTime = G.gameTime;
        float speedScale = 1.0f + diffTime * 0.005f;
        float hpBonus = floorf(diffTime / 30.0f);
        float coolBonus = diffTime * 0.003f;

        // Apply difficulty multipliers
        if (G.difficulty == DIFF_EASY)
        {
            speedScale *= 0.7f;
            hpBonus *= 0.5f;
            coolBonus *= 0.5f;
        }
        else if (G.difficulty == DIFF_HARD)
        {
            speedScale *= 1.3f;
            hpBonus *= 1.5f;
            coolBonus *= 1.5f;
        }

        // Weighted spawning
        int roll = GetRandomValue(0, 99);
        if (roll < 10) // 10% Titan
        {
            e->type = SHIP_TITAN;
            e->hp = 12 + (int)hpBonus;
            e->speed = Rf(30, 50) * speedScale;
            e->fireCooldown = fmaxf(1.0f, Rf(2.5f, 4.0f) - coolBonus);
        }
        else if (roll < 30) // 20% Destroyer
        {
            e->type = SHIP_DESTROYER;
            e->hp = 5 + (int)hpBonus;
            e->speed = Rf(60, 90) * speedScale;
            e->fireCooldown = fmaxf(0.5f, Rf(1.0f, 2.0f) - coolBonus);
        }
        else // 70% Interceptor
        {
            e->type = SHIP_INTERCEPTOR;
            e->hp = 2 + (int)hpBonus;
            e->speed = Rf(100, 160) * speedScale;
            e->fireCooldown = fmaxf(0.8f, Rf(1.5f, 3.0f) - coolBonus);
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
        Vector2 force = {0};

        if (dist > 5.0f)
        {
            dir.x /= dist;
            dir.y /= dist;
            force.x = dir.x * e->speed;
            force.y = dir.y * e->speed;
        }

        // Bullet Evasion Logic
        float evasionMul = (G.difficulty == DIFF_EASY) ? 0.6f :
                           (G.difficulty == DIFF_HARD) ? 1.4f : 1.0f;
        float dodgeMul   = (G.difficulty == DIFF_EASY) ? 0.5f :
                           (G.difficulty == DIFF_HARD) ? 1.5f : 1.0f;
        float timeEvasionBonus = G.gameTime * 0.5f;
        float timeWidthBonus = G.gameTime * 0.2f;
        float evasionRadius = (((e->type == SHIP_INTERCEPTOR) ? 150.0f : 
                               (e->type == SHIP_DESTROYER) ? 100.0f : 80.0f) + timeEvasionBonus) * evasionMul;
        float evasionWidth = (60.0f + timeWidthBonus) * evasionMul;
        float dodgeForce = ((e->type == SHIP_INTERCEPTOR) ? 500.0f : 
                            (e->type == SHIP_DESTROYER) ? 300.0f : 150.0f) * dodgeMul;

        for (int b = 0; b < MAX_BULLETS; b++)
        {
            if (!G.bullets[b].active || G.bullets[b].isEnemy)
                continue;

            // Check if bullet is approaching from below
            float dx = e->pos.x - G.bullets[b].pos.x;
            float dy = e->pos.y - G.bullets[b].pos.y;
            
            // Only care about bullets below the enemy and not too far horizontally
            if (dy < 0 && dy > -evasionRadius && fabsf(dx) < evasionWidth)
            {
                // Steer away based on horizontal position
                if (dx > 0) force.x += dodgeForce; 
                else force.x -= dodgeForce;
                break; 
            }
        }
 
        // --- Special Weapon Defense logic ---
        float specialDodgeMul = (G.difficulty == DIFF_EASY) ? 0.3f :
                                (G.difficulty == DIFF_HARD) ? 1.2f : 0.7f;

        for (int w = 0; w < MAX_BULLETS; w++)
        {
            if (!G.weaponProjs[w].active) continue;
            WeaponProj *wp = &G.weaponProjs[w];
            float dx = e->pos.x - wp->pos.x;
            float dy = e->pos.y - wp->pos.y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (wp->wtype == WEAPON_SINGULARITY)
            {
                // Resist the pull: move directly away from the center
                if (dist < 400 && dist > 1.0f)
                {
                    float forceMag = (wp->state == 1) ? 500.0f : 250.0f;
                    force.x += (dx / dist) * forceMag * specialDodgeMul;
                    force.y += (dy / dist) * forceMag * specialDodgeMul;
                }
            }
            else if (wp->wtype == WEAPON_FLAK)
            {
                // Avoid the shell or fragments
                if (dist < 150 && dist > 1.0f)
                {
                    if (dx > 0) force.x += dodgeForce * 1.5f * specialDodgeMul;
                    else force.x -= dodgeForce * 1.5f * specialDodgeMul;
                }
            }
            else if (wp->wtype == WEAPON_WAVE)
            {
                // Dodge away from the waving path
                if (dist < 120 && dist > 1.0f)
                {
                    if (dx > 0) force.x += dodgeForce * 1.2f * specialDodgeMul;
                    else force.x -= dodgeForce * 1.2f * specialDodgeMul;
                }
            }
        }

        // Proactive Railgun/Tesla defense (when player is aiming/charging)
        if (G.player.alive && G.energy >= 25.0f)
        {
            float dx = e->pos.x - G.player.pos.x;
            if (G.selectedWeapon == WEAPON_RAILGUN)
            {
                // Railgun is hitscan and wide. If in vertical sights, dash away!
                if (fabsf(dx) < 45)
                {
                    if (dx > 0) force.x += dodgeForce * 2.0f * specialDodgeMul;
                    else force.x -= dodgeForce * 2.0f * specialDodgeMul;
                }
            }
            else if (G.selectedWeapon == WEAPON_TESLA)
            {
                // Tesla chains. If too close to player or others, spread out.
                float pDist = VDist(e->pos, G.player.pos);
                if (pDist < 300)
                {
                    force.x += (dx > 0 ? 1 : -1) * dodgeForce * specialDodgeMul;
                    force.y -= dodgeForce * specialDodgeMul; // Move back
                }
            }
        }

        e->vel.x += (force.x - e->vel.x) * dt * 3.0f;
        e->vel.y += (force.y - e->vel.y) * dt * 3.0f;

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
            
            // 1. Soft distancing (flocking)
            float minDist = (e->type == SHIP_TITAN || other->type == SHIP_TITAN) ? 110.0f : 75.0f;
            if (distSq < minDist * minDist && distSq > 0.01f)
            {
                float dist = sqrtf(distSq);
                float force = (minDist - dist) / minDist;
                e->vel.x += (dx / dist) * force * dt * 300.0f;
                e->vel.y += (dy / dist) * force * dt * 300.0f;
            }

            // 2. Hard hull-to-hull separation (Precision)
            if (CheckShipShipCollision(GetShipHitbox(e->pos, e->type, false), GetShipHitbox(other->pos, other->type, false)))
            {
                if (distSq < 0.01f) { dx = Rf(-1,1); dy = Rf(-1,1); distSq = 1.0f; }
                float dist = sqrtf(distSq);
                e->vel.x += (dx / dist) * dt * 800.0f;
                e->vel.y += (dy / dist) * dt * 800.0f;
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
