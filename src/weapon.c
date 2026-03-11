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
        SpawnWP((Vector2){pl->pos.x, pl->pos.y - 20},
                (Vector2){0, -350}, 0.6f, pl->pos.x, 0, WEAPON_FLAK);
        if (pl->type == SHIP_TITAN || pl->type == SHIP_DESTROYER)
        {
            SpawnWP((Vector2){pl->pos.x - 15, pl->pos.y - 15},
                    (Vector2){-80, -300}, 0.6f, pl->pos.x, 0, WEAPON_FLAK);
            SpawnWP((Vector2){pl->pos.x + 15, pl->pos.y - 15},
                    (Vector2){80, -300}, 0.6f, pl->pos.x, 0, WEAPON_FLAK);
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
            float d = VDist(pl->pos, G.enemies[i].pos);
            if (d < 350 && d < tDist) { tDist = d; tIdx = i; tType = 0; }
        }
        for (int i = 0; i < MAX_METEORS; i++)
        {
            if (!G.meteors[i].active) continue;
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
                    float d = VDist(last, G.enemies[i].pos);
                    if (d < 250 && d < nDist) { nDist = d; nIdx = i; nType = 0; }
                }
                for (int i = 0; i < MAX_METEORS; i++)
                {
                    if (!G.meteors[i].active || (lastType == 1 && i == lastIdx)) continue;
                    float d = VDist(last, G.meteors[i].pos);
                    if (d < 250 && d < nDist) { nDist = d; nIdx = i; nType = 1; }
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
        SpawnWP((Vector2){pl->pos.x, pl->pos.y - 20},
                (Vector2){0, -250}, 3.0f, pl->pos.x, 0, WEAPON_SINGULARITY);
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
            if (p->state == 0 && p->life <= 0)
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
            p->pos.x += p->vel.x * dt;
            p->pos.y += p->vel.y * dt;
            p->life -= dt;
            if (p->life <= 0)
            {
                /* Gravity burst: damage everything nearby */
                p->active = false;
                SpawnP(p->pos, PURPLE, 35, 300, 4.0f);
                SpawnP(p->pos, VIOLET, 20, 200, 3.0f);
                G.shakeTimer = 0.3f;
                G.shakeMag = 8;
                for (int e = 0; e < MAX_ENEMIES; e++)
                {
                    if (!G.enemies[e].active) continue;
                    float d = VDist(p->pos, G.enemies[e].pos);
                    if (d < 160)
                    {
                        G.enemies[e].hp -= 3;
                        SpawnP(G.enemies[e].pos, VIOLET, 8, 120, 2.0f);
                        if (G.enemies[e].hp <= 0)
                            DestroyEnemy(e, p->pos);
                    }
                }
                for (int m = 0; m < MAX_METEORS; m++)
                {
                    if (!G.meteors[m].active) continue;
                    float d = VDist(p->pos, G.meteors[m].pos);
                    if (d < 160)
                    {
                        G.meteors[m].hp -= 3;
                        SpawnP(G.meteors[m].pos, VIOLET, 8, 120, 2.0f);
                        if (G.meteors[m].hp <= 0)
                            DestroyMeteor(m, p->pos);
                    }
                }
            }
            else
            {
                /* Pull enemies and meteors toward the orb */
                for (int e = 0; e < MAX_ENEMIES; e++)
                {
                    if (!G.enemies[e].active) continue;
                    float d = VDist(p->pos, G.enemies[e].pos);
                    if (d < 280 && d > 10)
                    {
                        float nx = (p->pos.x - G.enemies[e].pos.x) / d;
                        float ny = (p->pos.y - G.enemies[e].pos.y) / d;
                        float force = 200.0f / (d * 0.01f + 1.0f);
                        G.enemies[e].pos.x += nx * force * dt;
                        G.enemies[e].pos.y += ny * force * dt;
                    }
                }
                for (int m = 0; m < MAX_METEORS; m++)
                {
                    if (!G.meteors[m].active) continue;
                    float d = VDist(p->pos, G.meteors[m].pos);
                    if (d < 280 && d > 10)
                    {
                        float nx = (p->pos.x - G.meteors[m].pos.x) / d;
                        float ny = (p->pos.y - G.meteors[m].pos.y) / d;
                        float force = 150.0f / (d * 0.01f + 1.0f);
                        G.meteors[m].pos.x += nx * force * dt;
                        G.meteors[m].pos.y += ny * force * dt;
                    }
                }
                if (GetRandomValue(0, 100) < 60)
                    SpawnP((Vector2){p->pos.x + Rf(-20, 20),
                                     p->pos.y + Rf(-20, 20)},
                           VIOLET, 1, 80, 1.8f);
                /* Swirling particles */
                float ang = p->stateTime * 8.0f;
                SpawnP((Vector2){p->pos.x + cosf(ang) * 25,
                                 p->pos.y + sinf(ang) * 25},
                       PURPLE, 1, 50, 1.2f);
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

    /* Difficulty score multiplier */
    float scoreMul = (G.difficulty == DIFF_EASY) ? 0.7f :
                     (G.difficulty == DIFF_HARD) ? 1.5f : 1.0f;
    points = (int)(points * scoreMul);

    G.score += points;
    G.enemiesDestroyed++;

    int dropChance = (e->type == SHIP_TITAN) ? 40 :
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

        /* Railgun and Tesla do instant damage in FireAdvancedWeapon —
         * their WeaponProjs are visual-only. */
        if (p->wtype == WEAPON_RAILGUN || p->wtype == WEAPON_TESLA) continue;

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
            float a = p->life / 0.25f;
            unsigned char aa = (unsigned char)(255 * a);
            /* Outer glow */
            DrawRectangle((int)(p->pos.x - 20 * a), -100,
                          (int)(40 * a), (int)(p->pos.y + 100),
                          CAlpha(SKYBLUE, (unsigned char)(aa / 3)));
            /* Core beam */
            DrawRectangle((int)(p->pos.x - 14 * a), -100,
                          (int)(28 * a), (int)(p->pos.y + 100),
                          CAlpha(WHITE, aa));
            DrawRectangle((int)(p->pos.x - 5 * a), -100,
                          (int)(10 * a), (int)(p->pos.y + 100),
                          CAlpha(SKYBLUE, aa));
            /* Impact sparks */
            for (int k = 0; k < 6; k++)
                DrawCircleV((Vector2){p->pos.x + Rf(-14*a, 14*a),
                                      p->pos.y + Rf(-10, 10)},
                            4 * a, WHITE);
        }
        else if (p->wtype == WEAPON_FLAK)
        {
            if (p->state == 0)
            {
                DrawCircleV(p->pos, 8, ORANGE);
                DrawCircleV(p->pos, 4, WHITE);
                DrawCircleGradient((int)p->pos.x, (int)p->pos.y,
                                   18, CAlpha(YELLOW, 150), BLANK);
            }
            else if (p->state == 2)
            {
                DrawCircleV(p->pos, 3, YELLOW);
                DrawCircleV(p->pos, 1, WHITE);
            }
        }
        else if (p->wtype == WEAPON_TESLA)
        {
            float d = VDist(p->pos, p->vel);
            if (d > 0.1f)
            {
                Vector2 dir = {(p->vel.x - p->pos.x) / d,
                               (p->vel.y - p->pos.y) / d};
                int segs = (int)(d / 15);
                if (segs < 1) segs = 1;
                Vector2 last = p->pos;
                for (int s = 1; s <= segs; s++)
                {
                    Vector2 next = {p->pos.x + dir.x * (s * 15),
                                    p->pos.y + dir.y * (s * 15)};
                    if (s == segs) next = p->vel;
                    if (s < segs)
                    {
                        next.x += Rf(-12, 12);
                        next.y += Rf(-12, 12);
                    }
                    DrawLineEx(last, next, 4.0f, CAlpha(SKYBLUE, 180));
                    DrawLineEx(last, next, 2.0f, WHITE);
                    last = next;
                }
            }
        }
        else if (p->wtype == WEAPON_SINGULARITY)
        {
            float rot = t * 15.0f;
            DrawCircleGradient((int)p->pos.x, (int)p->pos.y,
                               (int)(30 + sinf(t * 8) * 10),
                               CAlpha(PURPLE, 220), BLANK);
            DrawCircleV(p->pos, 11, BLACK);
            for (int j = 0; j < 6; j++)
            {
                float ang = rot + j * 3.14159265f / 3.0f;
                Vector2 tip = {p->pos.x + cosf(ang) * 22,
                               p->pos.y + sinf(ang) * 22};
                DrawLineEx(p->pos, tip, 3.5f, VIOLET);
                DrawCircle((int)tip.x, (int)tip.y, 4, WHITE);
            }
        }
        else if (p->wtype == WEAPON_WAVE)
        {
            DrawCircleGradient((int)p->pos.x, (int)p->pos.y,
                               22, CAlpha(GREEN, 120), BLANK);
            DrawCircleV(p->pos, 6, LIME);
            DrawCircleV(p->pos, 3, WHITE);
        }
    }
}
