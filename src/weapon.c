/*
 * weapon.c — Advanced weapon system for Cosmic Oblivion.
 *
 * The existing laser (WEAPON_LASER) fires via G.bullets[] in game.c.
 * This module handles ONLY the 5 advanced weapons. Their projectiles
 * live in G.weaponProjs[] and go through the same damage pipeline.
 */
#include "include/weapon.h"
#include "include/helpers.h"
#include "include/particles.h"
#include "include/collision.h"
#include "include/meteor.h"
#include "include/healthstar.h"
#include "include/shieldpickup.h"
#include "include/floatingtext.h"
#include <math.h>
#include <stdio.h>

extern GameState G;

/* Forward declarations for static helpers used in FireAdvancedWeapon */
static void DestroyMeteor(int mi, Vector2 hitPos);
static void DestroyEnemy(int ei, Vector2 hitPos);

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

static float VDist(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

/* Audio: reuse the same round-robin explosion pool that game.c uses.
 * Since the sound arrays live in G, we can play them directly.       */
static void WeaponPlayExplosion(void)
{
    if (!G.audioEnabled) return;
    if (G.explosionVariantCount <= 0) return;
    int v = GetRandomValue(0, G.explosionVariantCount - 1);
    G.explosionSoundIdx = (G.explosionSoundIdx + 1) % 8;
    if (G.explosionSounds[G.explosionSoundIdx].frameCount == 0)
        G.explosionSounds[G.explosionSoundIdx] = LoadSoundAlias(G.explosionVariants[v]);
    SetSoundVolume(G.explosionSounds[G.explosionSoundIdx], G.gameplayVolume);
    PlaySound(G.explosionSounds[G.explosionSoundIdx]);
}

static void WeaponPlayEnemyDestroy(void)
{
    if (!G.audioEnabled) return;
    SetSoundVolume(G.sfxEnemyDestroy, G.gameplayVolume);
    PlaySound(G.sfxEnemyDestroy);
}

/* ------------------------------------------------------------------ */
/*  Spawn a weapon projectile into the first free slot                 */
/* ------------------------------------------------------------------ */

static void SpawnWP(Vector2 p, Vector2 v, float life, float bx,
                    int state, WeaponType wt)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.weaponProjs[i].active)
        {
            G.weaponProjs[i] = (WeaponProj){p, v, life, bx, 0, state, wt, true};
            break;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */

void InitWeaponProjs(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
        G.weaponProjs[i].active = false;
}

/* ------------------------------------------------------------------ */
/*  Fire — only fires for non-LASER weapons                            */
/* ------------------------------------------------------------------ */

void FireAdvancedWeapon(void)
{
    if (G.selectedWeapon == WEAPON_LASER) return;

    Player *pl = &G.player;
    WeaponType wt = G.selectedWeapon;

    if (wt == WEAPON_RAILGUN)
    {
        /* Hitscan beam — instant damage + visual effect projectile */
        SpawnWP(pl->pos, (Vector2){0, 0}, 0.25f, pl->pos.x, 0, WEAPON_RAILGUN);
        G.shakeTimer = 0.15f;
        G.shakeMag = 5;

        Rectangle beam = {pl->pos.x - 14, -100, 28, pl->pos.y + 100};
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (!G.enemies[i].active) continue;
            ShipHitbox hb = GetShipHitbox(G.enemies[i].pos, G.enemies[i].type, false);
            for (int r = 0; r < hb.count; r++)
            {
                if (CheckCollisionRecs(beam, hb.rects[r]))
                {
                    G.enemies[i].hp -= 5;
                    SpawnP(G.enemies[i].pos, WHITE, 15, 200, 2.5f);
                    SpawnP(G.enemies[i].pos, SKYBLUE, 8, 160, 2.0f);
                    if (G.enemies[i].hp <= 0)
                        DestroyEnemy(i, G.enemies[i].pos);
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_METEORS; i++)
        {
            if (!G.meteors[i].active) continue;
            if (CheckCollisionCircleRec(G.meteors[i].pos, G.meteors[i].radius, beam))
            {
                G.meteors[i].hp -= 5;
                SpawnP(G.meteors[i].pos, WHITE, 12, 180, 2.0f);
                if (G.meteors[i].hp <= 0)
                    DestroyMeteor(i, G.meteors[i].pos);
            }
        }
    }
    else if (wt == WEAPON_FLAK)
    {
        // Faster and longer range flak shell
        SpawnWP((Vector2){pl->pos.x, pl->pos.y - 20},
                (Vector2){0, -550}, 2.0f, pl->pos.x, 0, WEAPON_FLAK);
        if (pl->type == SHIP_TITAN || pl->type == SHIP_DESTROYER)
        {
            SpawnWP((Vector2){pl->pos.x - 15, pl->pos.y - 15},
                    (Vector2){-100, -500}, 2.0f, pl->pos.x, 0, WEAPON_FLAK);
            SpawnWP((Vector2){pl->pos.x + 15, pl->pos.y - 15},
                    (Vector2){100, -500}, 2.0f, pl->pos.x, 0, WEAPON_FLAK);
        }
    }
    else if (wt == WEAPON_TESLA)
    {
        /* Chain lightning — find nearest target, chain 3-5 times */
        int tIdx = -1;
        float tDist = 99999;
        int tType = 0; /* 0=enemy, 1=meteor */

        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (!G.enemies[i].active) continue;
            if (G.enemies[i].pos.y > pl->pos.y) continue; // ONLY FORWARD
            float d = VDist(pl->pos, G.enemies[i].pos);
            if (d < 350 && d < tDist) { tDist = d; tIdx = i; tType = 0; }
        }
        for (int i = 0; i < MAX_METEORS; i++)
        {
            if (!G.meteors[i].active) continue;
            if (G.meteors[i].pos.y > pl->pos.y) continue; // ONLY FORWARD
            float d = VDist(pl->pos, G.meteors[i].pos);
            if (d < 350 && d < tDist) { tDist = d; tIdx = i; tType = 1; }
        }

        if (tIdx >= 0)
        {
            Vector2 tPos = tType == 0 ? G.enemies[tIdx].pos : G.meteors[tIdx].pos;
            SpawnWP(pl->pos, tPos, 0.15f, 0, 0, WEAPON_TESLA);
            if (tType == 0)
            {
                G.enemies[tIdx].hp -= 1;
                SpawnP(tPos, SKYBLUE, 8, 140, 2.0f);
                SpawnP(tPos, WHITE, 3, 80, 1.5f);
                if (G.enemies[tIdx].hp <= 0)
                    DestroyEnemy(tIdx, tPos);
            }
            else
            {
                G.meteors[tIdx].hp -= 1;
                SpawnP(tPos, SKYBLUE, 8, 140, 2.0f);
                if (G.meteors[tIdx].hp <= 0)
                    DestroyMeteor(tIdx, tPos);
            }

            int maxChain = (pl->type == SHIP_TITAN) ? 5 : 3;
            Vector2 last = tPos;
            int lastIdx = tIdx, lastType = tType, chains = 1;

            while (chains < maxChain)
            {
                int nIdx = -1; float nDist = 99999; int nType = 0;
                for (int i = 0; i < MAX_ENEMIES; i++)
                {
                    if (!G.enemies[i].active || (lastType == 0 && i == lastIdx)) continue;
                    if (G.enemies[i].pos.y > last.y) continue; // ONLY FORWARD
                    float d = VDist(last, G.enemies[i].pos);
                    if (d < 300 && d < nDist) { nDist = d; nIdx = i; nType = 0; }
                }
                for (int i = 0; i < MAX_METEORS; i++)
                {
                    if (!G.meteors[i].active || (lastType == 1 && i == lastIdx)) continue;
                    if (G.meteors[i].pos.y > last.y) continue; // ONLY FORWARD
                    float d = VDist(last, G.meteors[i].pos);
                    if (d < 300 && d < nDist) { nDist = d; nIdx = i; nType = 1; }
                }
                if (nIdx < 0) break;

                Vector2 nPos = nType == 0 ? G.enemies[nIdx].pos : G.meteors[nIdx].pos;
                SpawnWP(last, nPos, 0.15f, 0, 0, WEAPON_TESLA);
                if (nType == 0)
                {
                    G.enemies[nIdx].hp -= 1;
                    SpawnP(nPos, SKYBLUE, 8, 140, 2.0f);
                    SpawnP(nPos, WHITE, 3, 80, 1.5f);
                    if (G.enemies[nIdx].hp <= 0)
                        DestroyEnemy(nIdx, nPos);
                }
                else
                {
                    G.meteors[nIdx].hp -= 1;
                    SpawnP(nPos, SKYBLUE, 8, 140, 2.0f);
                    if (G.meteors[nIdx].hp <= 0)
                        DestroyMeteor(nIdx, nPos);
                }
                last = nPos; lastIdx = nIdx; lastType = nType; chains++;
            }
        }
        else
        {
            /* No target — cosmetic spark forward */
            SpawnWP(pl->pos,
                    (Vector2){pl->pos.x + Rf(-40, 40), pl->pos.y - 120},
                    0.15f, 0, 0, WEAPON_TESLA);
        }
    }
    else if (wt == WEAPON_SINGULARITY)
    {
        // Slower movement, longer duration to reach top
        SpawnWP((Vector2){pl->pos.x, pl->pos.y - 20},
                (Vector2){0, -180}, 6.0f, pl->pos.x, 0, WEAPON_SINGULARITY);
    }
    else if (wt == WEAPON_WAVE)
    {
        SpawnWP((Vector2){pl->pos.x - 10, pl->pos.y - 20},
                (Vector2){0, -450}, 0.0f, pl->pos.x - 10, 0, WEAPON_WAVE);
        SpawnWP((Vector2){pl->pos.x + 10, pl->pos.y - 20},
                (Vector2){0, -450}, 0.0f, pl->pos.x + 10, 1, WEAPON_WAVE);
        if (pl->type == SHIP_TITAN)
        {
            SpawnWP((Vector2){pl->pos.x - 20, pl->pos.y - 10},
                    (Vector2){0, -450}, 0.0f, pl->pos.x - 20, 1, WEAPON_WAVE);
            SpawnWP((Vector2){pl->pos.x + 20, pl->pos.y - 10},
                    (Vector2){0, -450}, 0.0f, pl->pos.x + 20, 0, WEAPON_WAVE);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Update projectile positions / lifetimes                            */
/* ------------------------------------------------------------------ */

void UpdateWeaponProjs(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.weaponProjs[i].active) continue;
        WeaponProj *p = &G.weaponProjs[i];
        p->stateTime += dt;

        if (p->wtype == WEAPON_RAILGUN || p->wtype == WEAPON_TESLA)
        {
            /* Visual-only — just count down */
            p->life -= dt;
            if (p->life <= 0) p->active = false;
        }
        else if (p->wtype == WEAPON_FLAK)
        {
            p->pos.x += p->vel.x * dt;
            p->pos.y += p->vel.y * dt;
            p->life -= dt;
            
            bool reachedEdge = (p->pos.y < 50);
            if ((p->state == 0 && p->life <= 0) || (p->state == 0 && reachedEdge))
            {
                /* Explode into 16 fragments */
                p->active = false;
                for (int f = 0; f < 16; f++)
                {
                    float a = (f / 16.0f) * 3.14159265f * 2;
                    SpawnWP(p->pos,
                            (Vector2){cosf(a) * 380, sinf(a) * 380},
                            0.4f, p->pos.x, 2, WEAPON_FLAK);
                }
                SpawnP(p->pos, YELLOW, 30, 350, 4.0f);
                SpawnP(p->pos, ORANGE, 15, 200, 3.0f);
                G.shakeTimer = 0.12f;
                G.shakeMag = 4;
            }
            else if (p->state == 2 && p->life <= 0)
            {
                p->active = false;
            }
        }
        else if (p->wtype == WEAPON_SINGULARITY)
        {
            // Two-Phase Logic: Traveling (state 0) -> Stationary (state 1)
            if (p->state == 0)
            {
                p->pos.x += p->vel.x * dt;
                p->pos.y += p->vel.y * dt;
                // Transition to stationary near middle-top
                if (p->pos.y < SH * 0.35f) {
                    p->state = 1;
                    p->vel = (Vector2){0, 0};
                }
            }
            
            p->life -= dt;
            
            // Only explode (final collapse) if life is truly gone
            if (p->life <= 0)
            {
                /* Gravity burst: huge damage on collapse */
                p->active = false;
                SpawnP(p->pos, PURPLE, 80, 500, 7.0f);
                SpawnP(p->pos, VIOLET, 50, 400, 5.0f);
                SpawnP(p->pos, WHITE, 20, 200, 3.0f);
                G.shakeTimer = 0.7f;
                G.shakeMag = 15;
                for (int e = 0; e < MAX_ENEMIES; e++)
                {
                    if (!G.enemies[e].active) continue;
                    float d = VDist(p->pos, G.enemies[e].pos);
                    if (d < 250)
                    {
                        G.enemies[e].hp -= 15;
                        SpawnP(G.enemies[e].pos, VIOLET, 15, 150, 2.5f);
                        if (G.enemies[e].hp <= 0)
                            DestroyEnemy(e, p->pos);
                    }
                }
                for (int m = 0; m < MAX_METEORS; m++)
                {
                    if (!G.meteors[m].active) continue;
                    float d = VDist(p->pos, G.meteors[m].pos);
                    if (d < 250)
                    {
                        G.meteors[m].hp -= 15;
                        SpawnP(G.meteors[m].pos, VIOLET, 15, 150, 2.5f);
                        if (G.meteors[m].hp <= 0)
                            DestroyMeteor(m, p->pos);
                    }
                }
            }
            else
            {
                /* GLOBAL PULL and CONTACT DESTRUCTION */
                float pullRange = 1200.0f; 
                float pullForceBase = (p->state == 1) ? 550.0f : 350.0f;
                float contactRadius = 25.0f;

                for (int e = 0; e < MAX_ENEMIES; e++)
                {
                    if (!G.enemies[e].active) continue;
                    float d = VDist(p->pos, G.enemies[e].pos);
                    
                    // Contact destruction
                    if (d < contactRadius) {
                        G.enemies[e].hp = 0;
                        DestroyEnemy(e, p->pos);
                        SpawnP(p->pos, WHITE, 5, 100, 1.5f);
                        continue;
                    }

                    if (d < pullRange)
                    {
                        float nx = (p->pos.x - G.enemies[e].pos.x) / d;
                        float ny = (p->pos.y - G.enemies[e].pos.y) / d;
                        float force = pullForceBase / (d * 0.004f + 1.0f);
                        G.enemies[e].pos.x += nx * force * dt;
                        G.enemies[e].pos.y += ny * force * dt;
                    }
                }
                for (int m = 0; m < MAX_METEORS; m++)
                {
                    if (!G.meteors[m].active) continue;
                    float d = VDist(p->pos, G.meteors[m].pos);

                    // Contact destruction
                    if (d < contactRadius) {
                        G.meteors[m].hp = 0;
                        DestroyMeteor(m, p->pos);
                        SpawnP(p->pos, WHITE, 5, 100, 1.5f);
                        continue;
                    }

                    if (d < pullRange)
                    {
                        float nx = (p->pos.x - G.meteors[m].pos.x) / d;
                        float ny = (p->pos.y - G.meteors[m].pos.y) / d;
                        float force = (pullForceBase * 0.7f) / (d * 0.004f + 1.0f);
                        G.meteors[m].pos.x += nx * force * dt;
                        G.meteors[m].pos.y += ny * force * dt;
                    }
                }
                if (GetRandomValue(0, 100) < 70)
                    SpawnP((Vector2){p->pos.x + Rf(-25, 25),
                                     p->pos.y + Rf(-25, 25)},
                           VIOLET, 1, 100, 2.0f);
            }
        }
        else if (p->wtype == WEAPON_WAVE)
        {
            p->pos.y += p->vel.y * dt;
            float sign = (p->state == 0) ? 1.0f : -1.0f;
            p->pos.x = p->baseX + sinf(p->stateTime * 15.0f) * 50.0f * sign;
            /* Glowing trail */
            SpawnP(p->pos, LIME, 1, 50, 1.2f);
            if (GetRandomValue(0, 10) > 5)
                SpawnP(p->pos, GREEN, 1, 35, 0.8f);
        }

        /* Off-screen cull */
        if (p->pos.y < -100 || p->pos.y > SH + 100 ||
            p->pos.x < -100 || p->pos.x > SW + 100)
            p->active = false;
    }
}

/* ------------------------------------------------------------------ */
/*  Collision — mirrors game.c damage pipeline exactly                 */
/* ------------------------------------------------------------------ */

/* Helper: destroy a meteor using the same logic as game.c lines 398-437 */
static void DestroyMeteor(int mi, Vector2 hitPos)
{
    Meteor *m = &G.meteors[mi];
    m->active = false;
    WeaponPlayExplosion();
    SpawnP(m->pos, m->color, 18, 200, 3.5f);
    SpawnP(m->pos, (Color){255, 220, 100, 255}, 8, 160, 2);

    int bonus = (m->size == METEOR_LARGE) ? 30 :
                (m->size == METEOR_MEDIUM) ? 20 : 10;
    
    /* Difficulty score multiplier */
    float scoreMul = (G.difficulty == DIFF_EASY) ? 0.7f :
                     (G.difficulty == DIFF_HARD) ? 1.5f : 1.0f;
                     
    G.score += (int)(bonus * G.comboMultiplier * scoreMul);
    G.comboTimer = 2.0f;
    G.comboMultiplier = Clampf(G.comboMultiplier + 0.25f, 1, 5);
    G.meteorsDestroyed++;


    if (m->size == METEOR_LARGE)
    {
        SpawnMeteorAt(m->pos, METEOR_MEDIUM);
        SpawnMeteorAt(m->pos, METEOR_MEDIUM);
    }
    else if (m->size == METEOR_MEDIUM)
    {
        SpawnMeteorAt(m->pos, METEOR_SMALL);
        SpawnMeteorAt(m->pos, METEOR_SMALL);
    }
    if (m->size >= METEOR_LARGE) { G.shakeTimer = 0.25f; G.shakeMag = 6; }
    if (m->size >= METEOR_MEDIUM)
    {
        int chance = (m->size == METEOR_LARGE) ? 20 : 8;
        /* Difficulty drop multiplier */
        float dropMul = (G.difficulty == DIFF_EASY) ? 1.5f :
                        (G.difficulty == DIFF_HARD) ? 0.5f : 1.0f;
        chance = (int)(chance * dropMul);
        
        if (GetRandomValue(0, 99) < chance)
        {
            if (GetRandomValue(0, 1) == 0) SpawnHealthStarAt(m->pos);
            else                            SpawnShieldPickupAt(m->pos);
        }
    }
    (void)hitPos;
}

/* Helper: destroy an enemy using the same logic as game.c lines 458-487 */
static void DestroyEnemy(int ei, Vector2 hitPos)
{
    Enemy *e = &G.enemies[ei];
    e->active = false;
    WeaponPlayExplosion();
    WeaponPlayEnemyDestroy();
    SpawnP(e->pos, RED, 20, 200, 3.5f);

    int points = 150;
    if (e->type == SHIP_DESTROYER)  points = 500;
    else if (e->type == SHIP_TITAN) points = 1500;
    else if (e->type == SHIP_BOSS) points = 10000;

    /* Difficulty score multiplier */
    float scoreMul = (G.difficulty == DIFF_EASY) ? 0.7f :
                     (G.difficulty == DIFF_HARD) ? 1.5f : 1.0f;
    points = (int)(points * scoreMul);

    G.score += points;
    G.enemiesDestroyed++;
    
    if (G.isCampaignMode && e->type == SHIP_BOSS)
    {
        G.campaignState.bossDefeated = true;
    }


    int dropChance = (e->type == SHIP_BOSS) ? 100 :
                     (e->type == SHIP_TITAN) ? 40 :
                     (e->type == SHIP_DESTROYER) ? 15 : 5;
                     
    /* Difficulty drop multiplier */
    float dropMul = (G.difficulty == DIFF_EASY) ? 1.5f :
                    (G.difficulty == DIFF_HARD) ? 0.5f : 1.0f;
    dropChance = (int)(dropChance * dropMul);

    if (GetRandomValue(0, 99) < dropChance)
    {
        if (GetRandomValue(0, 1) == 0) SpawnHealthStarAt(e->pos);
        else                            SpawnShieldPickupAt(e->pos);
    }

    char buf[16];
    sprintf(buf, "+%d", points);
    SpawnFloatingText(e->pos, buf, GOLD);
    (void)hitPos;
}

void CheckWeaponCollisions(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.weaponProjs[i].active) continue;
        WeaponProj *p = &G.weaponProjs[i];

        /* Railgun, Tesla, and Singularity handle their own collision/logic
         * in UpdateWeaponProjs. */
        if (p->wtype == WEAPON_RAILGUN || p->wtype == WEAPON_TESLA || p->wtype == WEAPON_SINGULARITY) continue;

        bool hit = false;

        /* --- vs Meteors (same distance check as game.c line 393) --- */
        for (int mi = 0; mi < MAX_METEORS; mi++)
        {
            if (!G.meteors[mi].active) continue;
            float d = VDist(p->pos, G.meteors[mi].pos);
            if (d < G.meteors[mi].radius + 5)
            {
                if (p->wtype == WEAPON_FLAK && p->state == 0)
                {
                    p->life = 0; /* trigger explosion on next update */
                }
                else
                {
                    int dmg = (p->wtype == WEAPON_SINGULARITY) ? 3 : 1;
                    G.meteors[mi].hp -= dmg;
                    hit = true;
                    SpawnP(p->pos, (Color){255, 200, 100, 255}, 6, 120, 2);
                    if (G.meteors[mi].hp <= 0)
                        DestroyMeteor(mi, p->pos);
                }
                break;
            }
        }

        if (hit) { p->active = false; continue; }

        /* --- vs Enemies (same hitbox check as game.c line 453) --- */
        for (int ei = 0; ei < MAX_ENEMIES; ei++)
        {
            if (!G.enemies[ei].active) continue;
            if (CheckCircleShipCollision(p->pos, 6.0f,
                    GetShipHitbox(G.enemies[ei].pos, G.enemies[ei].type, false)))
            {
                if (p->wtype == WEAPON_FLAK && p->state == 0)
                {
                    p->life = 0;
                }
                else
                {
                    int dmg = (p->wtype == WEAPON_SINGULARITY) ? 3 : 1;
                    G.enemies[ei].hp -= dmg;
                    hit = true;
                    SpawnP(p->pos, RED, 8, 100, 2);
                    if (G.enemies[ei].hp <= 0)
                        DestroyEnemy(ei, p->pos);
                }
                break;
            }
        }

        if (hit) p->active = false;
    }
}

/* ------------------------------------------------------------------ */
/*  DrawWeaponEffects                                                  */
/* ------------------------------------------------------------------ */

void DrawWeaponEffects(void)
{
    float t = (float)GetTime();
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.weaponProjs[i].active) continue;
        WeaponProj *p = &G.weaponProjs[i];

        if (p->wtype == WEAPON_RAILGUN)
        {
            float lifeFrac = p->life / 0.25f;
            unsigned char alpha = (unsigned char)(255 * lifeFrac);
            
            // 1. Massive Primary Beam (Layered)
            // Outer glow
            DrawRectangle((int)(p->pos.x - 30 * lifeFrac), -100,
                          (int)(60 * lifeFrac), (int)(p->pos.y + 100),
                          CAlpha(SKYBLUE, (unsigned char)(alpha / 4)));
            // Inner glow
            DrawRectangle((int)(p->pos.x - 18 * lifeFrac), -100,
                          (int)(36 * lifeFrac), (int)(p->pos.y + 100),
                          CAlpha(WHITE, (unsigned char)(alpha / 2)));
            // Core
            DrawRectangle((int)(p->pos.x - 6 * lifeFrac), -100,
                          (int)(12 * lifeFrac), (int)(p->pos.y + 100),
                          CAlpha(WHITE, alpha));

            // 2. Shockwave Rings (Expanding outward from impact)
            float ringTimer = (0.25f - p->life) * 4.0f; // 0 to 1
            for (int r = 0; r < 3; r++)
            {
                float rt = fmodf(ringTimer + r * 0.33f, 1.0f);
                float radius = rt * 80.0f;
                unsigned char ra = (unsigned char)((1.0f - rt) * 150);
                DrawCircleLines((int)p->pos.x, (int)p->pos.y, radius, CAlpha(SKYBLUE, ra));
                DrawCircleLines((int)p->pos.x, (int)p->pos.y, radius + 2, CAlpha(WHITE, (unsigned char)(ra/2)));
            }

            // 3. Lingering Plasma "Afterimage" (Jagged fragments along the beam)
            for (int k = 0; k < 12; k++)
            {
                float py = p->pos.y - (k * 60) - fmodf(t * 200, 60);
                if (py < -100) continue;
                float off = sinf(t * 20 + k) * 15 * lifeFrac;
                DrawLineEx((Vector2){p->pos.x + off, py}, (Vector2){p->pos.x - off, py + 30}, 
                           3 * lifeFrac, CAlpha(SKYBLUE, (unsigned char)(alpha/2)));
            }
        }
        else if (p->wtype == WEAPON_FLAK)
        {
            if (p->state == 0) // Primary shell
            {
                // Rotating shell body
                float rot = t * 20.0f;
                // Draw shell fins
                for(int j=0; j<3; j++) {
                    float ang = rot + j * (PI*2/3);
                    Vector2 tip = {p->pos.x + cosf(ang)*12, p->pos.y + sinf(ang)*12};
                    DrawLineEx(p->pos, tip, 3.0f, DARKGRAY);
                    DrawCircleV(tip, 2, ORANGE);
                }
                
                // Glowing core & Heat trail
                DrawCircleGradient((int)p->pos.x, (int)p->pos.y, 22, CAlpha(ORANGE, 130), BLANK);
                DrawCircleV(p->pos, 7, ORANGE);
                DrawCircleV(p->pos, 4, WHITE);
                
                // Smoke trail
                if (GetRandomValue(0, 10) > 4)
                    SpawnP(p->pos, GRAY, 1, 15, 3.0f);
            }
            else if (p->state == 2) // Fragments
            {
                float fv = p->life / 0.4f;
                DrawLineEx(p->pos, (Vector2){p->pos.x - p->vel.x * 0.05f, p->pos.y - p->vel.y * 0.05f}, 
                           2.5f * fv, CAlpha(YELLOW, (unsigned char)(fv * 255)));
                DrawCircleV(p->pos, 3 * fv, WHITE);
            }
        }
        else if (p->wtype == WEAPON_TESLA)
        {
            float d = VDist(p->pos, p->vel);
            if (d > 0.1f)
            {
                float lifeFrac = p->life / 0.15f;
                int segs = (int)(d / 12);
                if (segs < 1) segs = 1;
                
                Vector2 last = p->pos;
                Vector2 dir = {(p->vel.x - p->pos.x) / d, (p->vel.y - p->pos.y) / d};
                
                for (int s = 1; s <= segs; s++)
                {
                    Vector2 next = {p->pos.x + dir.x * (s * 12), p->pos.y + dir.y * (s * 12)};
                    if (s == segs) next = p->vel;
                    else {
                        float jitter = 14.0f * lifeFrac;
                        next.x += Rf(-jitter, jitter);
                        next.y += Rf(-jitter, jitter);
                    }
                    
                    // Main bolt
                    DrawLineEx(last, next, 5.0f * lifeFrac, CAlpha(SKYBLUE, (unsigned char)(180 * lifeFrac)));
                    DrawLineEx(last, next, 2.0f * lifeFrac, WHITE);
                    
                    // Sub-branches (randomly sprout)
                    if (GetRandomValue(0, 100) < 20 && s < segs) {
                        Vector2 branchDir = {dir.y, -dir.x}; // perpendicular
                        if (GetRandomValue(0, 1) == 0) branchDir = (Vector2){-dir.y, dir.x};
                        Vector2 branchEnd = {next.x + branchDir.x * 25 * lifeFrac, next.y + branchDir.y * 25 * lifeFrac};
                        DrawLineEx(next, branchEnd, 2.0f * lifeFrac, CAlpha(SKYBLUE, (unsigned char)(120 * lifeFrac)));
                    }
                    
                    last = next;
                }
                
                // Static impact pop
                DrawCircleGradient((int)p->vel.x, (int)p->vel.y, (int)(25 * lifeFrac), CAlpha(SKYBLUE, 100), BLANK);
                DrawCircleV(p->vel, 5 * lifeFrac, WHITE);
            }
        }
        else if (p->wtype == WEAPON_SINGULARITY)
        {
            float rot = t * 12.0f;
            float pulse = sinf(t * 10.0f) * 0.15f;
            float sizeBase = 35.0f * (1.15f + pulse);
            
            // 1. Spacetime Distortion (Expanding/Contracting rings)
            for (int r = 0; r < 2; r++) {
                float ringRad = sizeBase * (1.0f + r * 0.5f + sinf(t * 5 + r) * 0.2f);
                DrawCircleLines((int)p->pos.x, (int)p->pos.y, ringRad, CAlpha(PURPLE, 100));
            }

            // --- Range Glow (Subtle indicator for the 250 radius) ---
            float rangePulse = 0.5f + 0.5f * sinf(t * 3.0f);
            unsigned char rangeAlpha = (unsigned char)(20 + 20 * rangePulse);
            DrawCircleLines((int)p->pos.x, (int)p->pos.y, 250, CAlpha(VIOLET, rangeAlpha));
            DrawCircleGradient((int)p->pos.x, (int)p->pos.y, 250, CAlpha(PURPLE, (unsigned char)(rangeAlpha / 2)), BLANK);
            
            // Event Horizon shadow with Void distortion
            Color ringC = (p->state == 1) ? VIOLET : PURPLE;
            DrawCircleGradient((int)p->pos.x, (int)p->pos.y, (int)sizeBase, CAlpha(ringC, 180), BLANK);
            DrawCircleGradient((int)p->pos.x, (int)p->pos.y, (int)sizeBase * 0.7f, CAlpha(BLACK, 255), CAlpha(BLACK, 0));
            DrawCircleV(p->pos, 15, BLACK);
            
            // Stationary intense glow
            if (p->state == 1) {
                float intensity = 0.5f + 0.5f * sinf(t * 15.0f);
                DrawCircleLines((int)p->pos.x, (int)p->pos.y, 18, CAlpha(WHITE, (unsigned char)(100 * intensity)));
            }

            // Pulse effect
            float orbPulse = sinf(t * 12.0f) * 0.1f + 1.0f;
            DrawCircleLines((int)p->pos.x, (int)p->pos.y, (int)(16 * orbPulse), CAlpha(VIOLET, 150));
            
            // Accretion particles - denser
            for(int k=0; k<12; k++) {
                float ang = t * 6.0f + k * (PI/6);
                float dist = sizeBase * (0.6f + 0.3f * sinf(t * 8.0f + k));
                DrawCircleV((Vector2){p->pos.x + cosf(ang)*dist, p->pos.y + sinf(ang)*dist}, 2, VIOLET);
            }
            
            // 3. Accretion Disk "Ribbons" (Swirling arcs)
            for (int j = 0; j < 4; j++)
            {
                float ang = rot + j * (PI / 2.0f);
                float arcStart = ang;
                float arcEnd = ang + PI / 4.0f;
                DrawRing(p->pos, 16, 22, arcStart * RAD2DEG, arcEnd * RAD2DEG, 8, VIOLET);
                
                // Light motes being "sucked in"
                float moteDist = 50.0f * (1.0f - fmodf(t + j * 0.25f, 1.0f));
                Vector2 motePos = {p->pos.x + cosf(ang) * moteDist, p->pos.y + sinf(ang) * moteDist};
                DrawCircleV(motePos, 2, WHITE);
            }
            
            // 4. Polar Glow
            DrawRectangleGradientV((int)p->pos.x - 2, (int)p->pos.y - 60, 4, 120, BLANK, CAlpha(VIOLET, 180));
        }
        else if (p->wtype == WEAPON_WAVE)
        {
            // 1. Dual-Chord Visual (Emerald and Lime)
            float waveT = p->stateTime * 15.0f;
            float waveA = 55.0f;
            float sign = (p->state == 0) ? 1.0f : -1.0f;
            
            for(int layer=0; layer<2; layer++) {
                Color wc = (layer == 0) ? LIME : GREEN;
                float layerOff = layer * 0.5f;
                float px = p->baseX + sinf(waveT + layerOff) * waveA * sign;
                Vector2 currentPos = {px, p->pos.y};
                
                // Draw core
                DrawCircleGradient((int)currentPos.x, (int)currentPos.y, 25, CAlpha(wc, 100), BLANK);
                DrawCircleV(currentPos, 7, wc);
                DrawCircleV(currentPos, 3, WHITE);
                
                // 2. Sonic Ripples (Trailing waves)
                for(int r=0; r<3; r++) {
                    float rDist = (r + 1) * 25.0f;
                    float rY = p->pos.y + rDist;
                    float rX = p->baseX + sinf(waveT - r * 0.4f + layerOff) * waveA * sign;
                    unsigned char ra = (unsigned char)(150 / (r + 1));
                    DrawCircleLines((int)rX, (int)rY, 10 - r*2, CAlpha(wc, ra));
                }
            }
        }
    }
}
