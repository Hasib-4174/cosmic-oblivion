#include "include/enemy.h"
#include "include/helpers.h"
#include "include/particles.h"
#include "include/ship.h"
#include "include/game.h"
#include "include/collision.h"
#include "include/campaign.h"
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
        int titanWeight = 10;
        int destroyerWeight = 20;
        
        if (G.isCampaignMode) 
        {
            int act = G.campaignState.currentLevel / 10;
            int levelWithinAct = G.campaignState.currentLevel % 10;
            
            // Scaled based on act (0=Fringe, 1=Core, 2=Oblivion)
            if (act == 0) {
                titanWeight = 2;
                destroyerWeight = 15;
            } else if (act == 1) {
                titanWeight = 8;
                destroyerWeight = 25;
            } else {
                titanWeight = 20;
                destroyerWeight = 35;
            }
            
            
            // Boss levels are all heavy ships
            if (levelWithinAct == 9) {
                /* On boss levels, check if we should spawn the boss */
                /* Boss spawns after clearing the target wave count */
                LevelData bld = GetLevelData(G.campaignState.currentLevel);
                if (G.enemiesDestroyed >= bld.targetEnemies)
                {
                    /* Spawn boss if not already present */
                    bool bossExists = false;
                    for (int j = 0; j < MAX_ENEMIES; j++)
                    {
                        if (G.enemies[j].active && G.enemies[j].type == SHIP_BOSS)
                        {
                            bossExists = true;
                            break;
                        }
                    }
                        if (!bossExists)
                    {
                        e->type = SHIP_BOSS;
                        e->state = 0;
                        e->specialTimer = 0.0f;
                        if (act == 0) {
                            e->hp = 500;
                            e->speed = 40.0f;
                            e->fireCooldown = 2.0f;
                        } else if (act == 1) {
                            e->hp = 1000;
                            e->speed = 50.0f;
                            e->fireCooldown = 1.0f;
                        } else {
                            e->hp = 2000;
                            e->speed = 60.0f;
                            e->fireCooldown = 0.5f;
                        }
                        return; // Boss spawned
                    }
                }
                titanWeight = 60;
                destroyerWeight = 40;
            }
        }
        
        if (roll < titanWeight) // Titan

        {
            e->type = SHIP_TITAN;
            e->hp = 12 + (int)hpBonus;
            e->speed = Rf(30, 50) * speedScale;
            e->fireCooldown = fmaxf(1.0f, Rf(2.5f, 4.0f) - coolBonus);
        }
        else if (roll < titanWeight + destroyerWeight) // Destroyer
        {
            e->type = SHIP_DESTROYER;
            e->hp = 5 + (int)hpBonus;
            e->speed = Rf(60, 90) * speedScale;
            e->fireCooldown = fmaxf(0.5f, Rf(1.0f, 2.0f) - coolBonus);
        }
        else // Interceptor
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

        if (dist > 5.0f && e->type != SHIP_BOSS)
        {
            dir.x /= dist;
            dir.y /= dist;
            force.x = dir.x * e->speed;
            force.y = dir.y * e->speed;
        }
        else if (e->type == SHIP_BOSS)
        {
            int act = G.campaignState.currentLevel / 10;
            e->specialTimer += dt;
            
            if (act == 0) // Act 0: Fringe Commander (Scythe Dash + Targeted Bursts)
            {
                if (e->state == 0) // Sweep side to side
                {
                    target.y = 120.0f;
                    dir.y = target.y - e->pos.y;
                    force.y = (dir.y > 0 ? 1 : -1) * e->speed * 0.5f;
                    
                    float sweep = sinf(G.gameTime * 1.5f);
                    force.x = sweep * e->speed * 3.0f;
                    
                    if (e->specialTimer >= 4.0f) {
                        e->state = 1; // Targeted Burst
                        e->specialTimer = 0.0f;
                    }
                }
                else if (e->state == 1) // Targeted Burst Pause
                {
                    force.x = 0;
                    force.y = 0;
                    e->vel.x *= 0.9f;
                    e->vel.y *= 0.9f;
                    if (e->specialTimer >= 2.0f) {
                        e->state = 2; // Scythe Dash
                        e->specialTimer = 0.0f;
                    }
                }
                else if (e->state == 2) // Scythe Dash down
                {
                    force.x = 0;
                    force.y = e->speed * 4.5f; // Very Fast charge
                    if (e->pos.y > SH - 80.0f) {
                        e->state = 3; // Retreat
                    }
                }
                else if (e->state == 3) // Retreating up
                {
                    force.x = 0;
                    force.y = -e->speed * 2.0f; // Retreat
                    if (e->pos.y <= 120.0f) {
                        e->state = 0;
                        e->specialTimer = 0.0f;
                    }
                }
            }
            else if (act == 1) // Act 1: Core Worlds Commander (Nova Ring + Orbital Barrage)
            {
                if (e->state == 0) // Normal tracking + Twin Streams
                {
                    float dx = G.player.pos.x - e->pos.x;
                    force.x = (dx > 0 ? 1 : -1) * e->speed * 1.5f;
                    if (fabsf(dx) < 30) force.x = 0;
                    
                    target.y = 140.0f + sinf(G.gameTime * 2.0f) * 40.0f;
                    force.y = (target.y - e->pos.y) * 2.0f;
                    
                    if (e->specialTimer >= 4.0f) {
                        e->state = 1; // Prepare Nova
                        e->specialTimer = 0.0f;
                        e->vel = (Vector2){0, 0};
                    }
                }
                else if (e->state == 1) // Charging Nova
                {
                    force.x = 0; force.y = 0;
                    if (e->specialTimer >= 1.5f) {
                        e->state = 2; // Transition to Orbital Barrage after Nova
                        e->specialTimer = 0.0f;
                    }
                }
                else if (e->state == 2) // Orbital Barrage (Fast Circle)
                {
                    float a = G.gameTime * 3.0f;
                    Vector2 center = {SW/2.0f, 180.0f};
                    target = (Vector2){center.x + cosf(a)*250, center.y + sinf(a)*80};
                    force.x = (target.x - e->pos.x) * 4.0f;
                    force.y = (target.y - e->pos.y) * 4.0f;
                    
                    if (e->specialTimer >= 6.0f) {
                        e->state = 0;
                        e->specialTimer = 0.0f;
                    }
                }
            }
            else // Act 2: Oblivion Commander (The Final Terror)
            {
                if (e->state == 0) // Chaotic Erratic Tracking
                {
                    float dx = G.player.pos.x - e->pos.x;
                    force.x = (dx > 0 ? 1 : -1) * e->speed * 2.5f;
                    if (fabsf(dx) < 20) force.x = 0;
                    
                    target.y = 120.0f + cosf(G.gameTime * 3.0f) * 60.0f;
                    force.y = (target.y - e->pos.y) * 3.0f;
                    
                    if (e->specialTimer >= 6.0f) {
                        e->state = 1; // Wall of Death!
                        e->specialTimer = 0.0f;
                    }
                }
                else if (e->state == 1) // Wall of Death Positioning
                {
                    // Move to top center
                    target = (Vector2){SW/2.0f, 100.0f};
                    force.x = (target.x - e->pos.x) * 2.0f;
                    force.y = (target.y - e->pos.y) * 2.0f;
                    
                    if (e->specialTimer >= 4.0f) { // After 4 seconds of walls, go Vortex
                        e->state = 2; 
                        e->specialTimer = 0.0f;
                    }
                }
                else if (e->state == 2) // Void Vortex (Spinning in center)
                {
                    target = (Vector2){SW/2.0f, SH/3.0f};
                    force.x = (target.x - e->pos.x) * 5.0f;
                    force.y = (target.y - e->pos.y) * 5.0f;
                    
                    if (e->specialTimer >= 5.0f) {
                        e->state = 0;
                        e->specialTimer = 0.0f;
                    }
                }
            }
        }

        // Bullet Evasion Logic (Bosses do not evade)
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
 
        // --- Special Weapon Defense logic --- (bosses do not evade)
        if (e->type != SHIP_BOSS)
        {
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
        } /* end if (e->type != SHIP_BOSS) for special weapon evasion */

        e->vel.x += (force.x - e->vel.x) * dt * 3.0f;
        e->vel.y += (force.y - e->vel.y) * dt * 3.0f;

        e->pos.x += e->vel.x * dt;
        e->pos.y += e->vel.y * dt;

        // Shooting logic
        e->fireCooldown -= dt;
        if (e->fireCooldown <= 0)
        {
            if (e->type == SHIP_BOSS)
            {
                int act = G.campaignState.currentLevel / 10;
                
                if (act == 0)
                {
                    if (e->state == 0) // Sweep firing
                    {
                        e->fireCooldown = 0.2f; // Fast continuous rain
                        float spreadX[] = {-40, 40};
                        for (int s = 0; s < 2; s++)
                        {
                            for (int b = 0; b < MAX_BULLETS; b++)
                            {
                                if (!G.bullets[b].active)
                                {
                                    G.bullets[b].pos = (Vector2){e->pos.x + spreadX[s], e->pos.y + 40};
                                    G.bullets[b].vel = (Vector2){0, 300.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                    else if (e->state == 1) // Targeted Burst
                    {
                        e->fireCooldown = 0.4f; // Slower burst but aimed
                        float dx = G.player.pos.x - e->pos.x;
                        float dy = G.player.pos.y - e->pos.y;
                        float dist = sqrtf(dx*dx + dy*dy);
                        if (dist > 1.0f) { dx /= dist; dy /= dist; }
                        else { dx = 0; dy = 1; }
                        
                        float angles[] = {-0.15f, 0, 0.15f}; // 3-way aimed
                        float baseAngle = atan2f(dy, dx);
                        
                        for (int s = 0; s < 3; s++)
                        {
                            for (int b = 0; b < MAX_BULLETS; b++)
                            {
                                if (!G.bullets[b].active)
                                {
                                    float a = baseAngle + angles[s];
                                    G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y + 40};
                                    G.bullets[b].vel = (Vector2){cosf(a)*400.0f, sinf(a)*400.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                    else if (e->state == 2) // Dashing - fire ring
                    {
                        e->fireCooldown = 0.5f;
                        for (int s = 0; s < 8; s++)
                        {
                            for (int b = 0; b < MAX_BULLETS; b++)
                            {
                                if (!G.bullets[b].active)
                                {
                                    float a = s * (3.14159f * 2.0f / 8.0f);
                                    G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y + 20};
                                    G.bullets[b].vel = (Vector2){cosf(a)*200.0f, sinf(a)*200.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                }
                else if (act == 1)
                {
                    if (e->state == 1 && e->specialTimer >= 1.45f) // Nova Ring
                    {
                        e->fireCooldown = 1.5f;
                        // Massive 16-way explosion
                        for (int s = 0; s < 16; s++) {
                            for (int b = 0; b < MAX_BULLETS; b++) {
                                if (!G.bullets[b].active) {
                                    float a = s * (3.14159f * 2.0f / 16.0f);
                                    G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y};
                                    G.bullets[b].vel = (Vector2){cosf(a)*400.0f, sinf(a)*400.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                    else if (e->state == 0) // Rapid Twin Targeted Streams
                    {
                        e->fireCooldown = 0.25f;
                        float offsetsX[] = {-40, 40};
                        
                        float dx = G.player.pos.x - e->pos.x;
                        float dy = G.player.pos.y - e->pos.y;
                        float dist = sqrtf(dx*dx + dy*dy);
                        if (dist > 1.0f) { dx /= dist; dy /= dist; }
                        else { dx = 0; dy = 1; }
                        
                        for (int s = 0; s < 2; s++)
                        {
                            for (int b = 0; b < MAX_BULLETS; b++)
                            {
                                if (!G.bullets[b].active)
                                {
                                    G.bullets[b].pos = (Vector2){e->pos.x + offsetsX[s], e->pos.y + 30};
                                    G.bullets[b].vel = (Vector2){dx * 500.0f, dy * 500.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                    else if (e->state == 2) // Orbital Barrage - Continuous fire
                    {
                        e->fireCooldown = 0.15f;
                        for (int b = 0; b < MAX_BULLETS; b++)
                        {
                            if (!G.bullets[b].active)
                            {
                                float a = G.gameTime * 5.0f; // Spiralling effect
                                G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y};
                                G.bullets[b].vel = (Vector2){cosf(a)*350.0f, sinf(a)*350.0f};
                                G.bullets[b].active = true;
                                G.bullets[b].isEnemy = true;
                                break;
                            }
                        }
                        PlayEnemyShootSound();
                    }
                }
                else // Act 2
                {
                    if (e->state == 1) // Wall of Death
                    {
                        e->fireCooldown = 0.8f;
                        for (int s = 0; s < 12; s++) {
                            for (int b = 0; b < MAX_BULLETS; b++) {
                                if (!G.bullets[b].active) {
                                    float xOff = -500 + s * 90;
                                    G.bullets[b].pos = (Vector2){e->pos.x + xOff * 0.1f, e->pos.y + 20};
                                    G.bullets[b].vel = (Vector2){xOff * 0.1f, 250.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                    else if (e->state == 0) // Chaotic Pulse Barrage
                    {
                        e->fireCooldown = 0.5f;
                        for (int s = 0; s < 5; s++)
                        {
                            for (int b = 0; b < MAX_BULLETS; b++)
                            {
                                if (!G.bullets[b].active)
                                {
                                    float a = (G.gameTime * 2.0f) + s * (3.14159f * 2.0f / 5.0f);
                                    G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y};
                                    G.bullets[b].vel = (Vector2){cosf(a)*450.0f, sinf(a)*450.0f};
                                    G.bullets[b].active = true;
                                    G.bullets[b].isEnemy = true;
                                    break;
                                }
                            }
                        }
                        PlayEnemyShootSound();
                    }
                    else if (e->state == 2) // Void Vortex (Insane spinning fire)
                    {
                        e->fireCooldown = 0.06f; // Extremely rapid
                        for (int b = 0; b < MAX_BULLETS; b++)
                        {
                            if (!G.bullets[b].active)
                            {
                                float a = G.gameTime * 20.0f; 
                                G.bullets[b].pos = (Vector2){e->pos.x, e->pos.y};
                                G.bullets[b].vel = (Vector2){cosf(a)*500.0f, sinf(a)*500.0f};
                                G.bullets[b].active = true;
                                G.bullets[b].isEnemy = true;
                                break;
                            }
                        }
                        // Sound handled by fast fire
                    }
                }
            }
            else if (e->type == SHIP_TITAN)
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
                            G.bullets[b].vel = (Vector2){0, 350.0f};
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
                            G.bullets[b].vel = (Vector2){0, 450.0f};
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
                        G.bullets[b].vel = (Vector2){0, 500.0f};
                        G.bullets[b].active = true;
                        G.bullets[b].isEnemy = true;
                        break;
                    }
                }
                PlayEnemyShootSound();
            }
        }

        // Screen bounds (stay in top 20% of screen)
        if (e->type == SHIP_BOSS)
        {
            int act = G.campaignState.currentLevel / 10;
            e->pos.x = Clampf(e->pos.x, 100, SW - 100);
            if (act == 0 && e->state == 1) { // Dashing down
                e->pos.y = Clampf(e->pos.y, -150, SH - 50);
            } else {
                e->pos.y = Clampf(e->pos.y, -150, SH * 0.3f);
            }
        }
        else
        {
            e->pos.x = Clampf(e->pos.x, 20, SW - 20);
            e->pos.y = Clampf(e->pos.y, -100, SH * 0.2f);
        }
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
            float minDist = (e->type == SHIP_TITAN || other->type == SHIP_TITAN) ? 110.0f : 
                            (e->type == SHIP_BOSS || other->type == SHIP_BOSS) ? 200.0f : 75.0f;
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
