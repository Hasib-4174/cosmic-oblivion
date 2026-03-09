#include "include/game.h"
#include "include/constants.h"
#include "include/helpers.h"
#include "include/stars.h"
#include "include/particles.h"
#include "include/meteor.h"
#include "include/ship.h"
#include "include/button.h"
#include "include/healthstar.h"
#include "include/shieldpickup.h"
#include "include/floatingtext.h"
#include "include/enemy.h"
#include "include/collision.h"
#include <math.h>
#include <stdio.h>

extern GameState G;

/* ---- Audio helper: load explosion variants once ---- */
static void EnsureExplosionVariants(void)
{
    if (G.explosionVariantCount > 0)
        return;
    G.explosionVariants[0] = LoadSound("audio/explosion/explosion_01.ogg");
    G.explosionVariants[1] = LoadSound("audio/explosion/explosion_02.ogg");
    G.explosionVariantCount = MAX_EXPLOSION_VARIANTS;
}

/* ---- Audio helper: load engine sounds once ---- */
static void EnsureEngineSounds(void)
{
    if (G.engineSoundsLoaded)
        return;
    G.engineSounds[SHIP_INTERCEPTOR] = LoadSound("audio/ship_engine/spaceEngine_003.ogg");
    G.engineSounds[SHIP_DESTROYER] = LoadSound("audio/ship_engine/spaceEngineLow_003.ogg");
    G.engineSounds[SHIP_TITAN] = LoadSound("audio/ship_engine/engine.mp3");
    G.engineSoundsLoaded = true;
}

/* ---- Sound play helpers (respect audioEnabled) ---- */

static void PlayFiringSound(void)
{
    if (!G.audioEnabled)
        return;
    G.firingSoundIdx++;
    if (G.firingSoundIdx >= 8)
        G.firingSoundIdx = 0;
    if (G.firingSounds[G.firingSoundIdx].frameCount == 0)
        G.firingSounds[G.firingSoundIdx] = LoadSound("audio/firing_sound/firing_sound1.ogg");
    SetSoundVolume(G.firingSounds[G.firingSoundIdx], G.gameplayVolume);
    PlaySound(G.firingSounds[G.firingSoundIdx]);
}

static void PlayExplosionSound(void)
{
    if (!G.audioEnabled)
        return;
    EnsureExplosionVariants();
    /* Pick a random variant */
    int variant = GetRandomValue(0, G.explosionVariantCount - 1);
    G.explosionSoundIdx++;
    if (G.explosionSoundIdx >= 8)
        G.explosionSoundIdx = 0;
    /* Copy the variant into the round-robin slot if needed */
    if (G.explosionSounds[G.explosionSoundIdx].frameCount == 0)
        G.explosionSounds[G.explosionSoundIdx] = LoadSoundAlias(G.explosionVariants[variant]);
    SetSoundVolume(G.explosionSounds[G.explosionSoundIdx], G.gameplayVolume);
    PlaySound(G.explosionSounds[G.explosionSoundIdx]);
}

static void PlayHealthPickupSound(void)
{
    if (!G.audioEnabled)
        return;
    G.healthPickupSoundIdx++;
    if (G.healthPickupSoundIdx >= 8)
        G.healthPickupSoundIdx = 0;
    if (G.healthPickupSounds[G.healthPickupSoundIdx].frameCount == 0)
        G.healthPickupSounds[G.healthPickupSoundIdx] = LoadSound("audio/health_pickup.wav");
    SetSoundVolume(G.healthPickupSounds[G.healthPickupSoundIdx], G.gameplayVolume);
    PlaySound(G.healthPickupSounds[G.healthPickupSoundIdx]);
}

static void PlayShieldPickupSound(void)
{
    if (!G.audioEnabled)
        return;
    G.shieldPickupSoundIdx++;
    if (G.shieldPickupSoundIdx >= 8)
        G.shieldPickupSoundIdx = 0;
    if (G.shieldPickupSounds[G.shieldPickupSoundIdx].frameCount == 0)
        G.shieldPickupSounds[G.shieldPickupSoundIdx] = LoadSound("audio/shield/shield_field.ogg");
    SetSoundVolume(G.shieldPickupSounds[G.shieldPickupSoundIdx], G.gameplayVolume);
    PlaySound(G.shieldPickupSounds[G.shieldPickupSoundIdx]);
}

static void PlayDamageSound(void)
{
    if (!G.audioEnabled)
        return;
    if (G.damageSound.frameCount == 0)
        G.damageSound = LoadSound("audio/damage/damage_take.mp3");
    SetSoundVolume(G.damageSound, G.gameplayVolume * 0.2f);
    PlaySound(G.damageSound);
}

void PlayEnemyShootSound(void)
{
    if (!G.audioEnabled)
        return;
    G.sfxEnemyShootIdx++;
    if (G.sfxEnemyShootIdx >= 8)
        G.sfxEnemyShootIdx = 0;
    if (G.sfxEnemyShoot[G.sfxEnemyShootIdx].frameCount == 0)
        G.sfxEnemyShoot[G.sfxEnemyShootIdx] = LoadSound("audio/enemy/shoting_sound.ogg");
    SetSoundVolume(G.sfxEnemyShoot[G.sfxEnemyShootIdx], G.gameplayVolume * 0.5f);
    PlaySound(G.sfxEnemyShoot[G.sfxEnemyShootIdx]);
}

void PlayEnemyDestroySound(void)
{
    if (!G.audioEnabled)
        return;
    SetSoundVolume(G.sfxEnemyDestroy, G.gameplayVolume);
    PlaySound(G.sfxEnemyDestroy);
}

void InitPlayer(void)
{
    Player *pl = &G.player;
    pl->pos = (Vector2){SW / 2.0f, SH - 80.0f};
    pl->vel = (Vector2){0, 0};
    pl->alive = true;
    pl->shieldTimer = 0;
    pl->damageFlash = 0;
    pl->type = G.selectedShip;
    if (pl->type == SHIP_INTERCEPTOR)
    {
        pl->speed = 420;
        pl->fireRate = 0.12f;
        pl->maxHp = 3;
    }
    else if (pl->type == SHIP_DESTROYER)
    {
        pl->speed = 320;
        pl->fireRate = 0.2f;
        pl->maxHp = 5;
    }
    else
    {
        pl->speed = 220;
        pl->fireRate = 0.28f;
        pl->maxHp = 8;
    }
    pl->hp = pl->maxHp;
    pl->fireCooldown = 0;
}
void InitGame(void)
{
    G.firingSoundIdx = -1;
    G.explosionSoundIdx = -1;
    G.healthPickupSoundIdx = -1;
    G.shieldPickupSoundIdx = -1;
    for (int i = 0; i < MAX_BULLETS; i++)
        G.bullets[i].active = false;
    for (int i = 0; i < MAX_METEORS; i++)
        G.meteors[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++)
        G.particles[i].active = false;
    InitHealthStars();
    InitShieldPickups();
    InitFloatingTexts();
    InitEnemies();
    G.meteorTimer = 0;
    G.meteorRate = 1.2f;
    G.meteorSpeedMul = 1.0f;
    G.enemyTimer = 0;
    G.enemyRate = 5.0f;
    G.scoreTimer = 0;
    G.gameTime = 0;
    G.score = 0;
    G.meteorsDestroyed = 0;
    G.enemiesDestroyed = 0;
    G.comboTimer = 0;
    G.comboMultiplier = 1.0f;
    G.shakeTimer = 0;
    G.shakeMag = 0;
    G.slowMoTimer = 0;
    G.gameOver = false;
    G.playerShieldActive = false;
    G.playerShieldDuration = 0;
    G.enginePlaying = false;
    InitPlayer();
}

void UpdateGame(float dt)
{
    if (G.slowMoTimer > 0)
    {
        G.slowMoTimer -= GetFrameTime();
        dt *= 0.3f;
    }
    Player *pl = &G.player;
    G.gameTime += dt;
    G.meteorRate = Clampf(1.2f - G.gameTime * 0.008f, 0.3f, 1.2f);
    G.meteorSpeedMul = 1.0f + G.gameTime * 0.005f;
    if (G.shakeTimer > 0)
        G.shakeTimer -= dt;
    if (pl->damageFlash > 0)
        pl->damageFlash -= dt;
    if (pl->shieldTimer > 0)
        pl->shieldTimer -= dt;
    if (G.comboTimer > 0)
    {
        G.comboTimer -= dt;
        if (G.comboTimer <= 0)
            G.comboMultiplier = 1.0f;
    }

    /* Shield duration countdown */
    if (G.playerShieldActive)
    {
        G.playerShieldDuration -= dt;
        if (G.playerShieldDuration <= 0)
        {
            G.playerShieldActive = false;
            G.playerShieldDuration = 0;
        }
    }

    if (pl->alive)
    {
        Vector2 dir = {0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            dir.y -= 1;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            dir.y += 1;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            dir.x -= 1;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            dir.x += 1;
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        bool isMoving = (len > 0);
        if (isMoving)
        {
            dir.x /= len;
            dir.y /= len;
            pl->vel.x += (dir.x * pl->speed - pl->vel.x) * dt * 8;
            pl->vel.y += (dir.y * pl->speed - pl->vel.y) * dt * 8;
        }
        else
        {
            pl->vel.x *= 1.0f - dt * 6;
            pl->vel.y *= 1.0f - dt * 6;
        }

        /* Engine sound: play while moving, stop when idle */
        EnsureEngineSounds();
        if (isMoving && G.audioEnabled)
        {
            if (!IsSoundPlaying(G.engineSounds[pl->type]))
            {
                SetSoundVolume(G.engineSounds[pl->type], G.gameplayVolume * 0.4f);
                PlaySound(G.engineSounds[pl->type]);
            }
            G.enginePlaying = true;
        }
        else
        {
            if (G.enginePlaying)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (IsSoundPlaying(G.engineSounds[i]))
                        StopSound(G.engineSounds[i]);
                }
                G.enginePlaying = false;
            }
        }

        pl->pos.x += pl->vel.x * dt;
        pl->pos.y += pl->vel.y * dt;
        pl->pos.x = Clampf(pl->pos.x, 24, SW - 24);
        pl->pos.y = Clampf(pl->pos.y, 28, SH - 28);
        if (fabsf(pl->vel.x) > 20 || fabsf(pl->vel.y) > 20)
        {
            Color ec = (pl->type == SHIP_INTERCEPTOR) ? (Color){0, 220, 255, 255} : (pl->type == SHIP_DESTROYER) ? (Color){255, 160, 40, 255}
                                                                                                                 : (Color){255, 50, 20, 255};
            SpawnP((Vector2){pl->pos.x + Rf(-4, 4), pl->pos.y + 22}, ec, 1, 40, 2.5f);
        }
        pl->fireCooldown -= dt;
        if (IsKeyDown(KEY_SPACE) && pl->fireCooldown <= 0)
        {
            pl->fireCooldown = pl->fireRate;
            if (pl->type == SHIP_TITAN)
            {
                for (int d = -1; d <= 1; d++)
                {
                    for (int i = 0; i < MAX_BULLETS; i++)
                    {
                        if (!G.bullets[i].active)
                        {
                            G.bullets[i] = (Bullet){{pl->pos.x + d * 10, pl->pos.y - 26}, 600, true, false};
                            PlayFiringSound();
                            break;
                        }
                    }
                }
            }
            else if (pl->type == SHIP_DESTROYER)
            {
                for (int d = -1; d <= 1; d += 2)
                {
                    for (int i = 0; i < MAX_BULLETS; i++)
                    {
                        if (!G.bullets[i].active)
                        {
                            G.bullets[i] = (Bullet){{pl->pos.x + d * 12, pl->pos.y - 20}, 550, true, false};
                            PlayFiringSound();
                            break;
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < MAX_BULLETS; i++)
                {
                    if (!G.bullets[i].active)
                    {
                        G.bullets[i] = (Bullet){{pl->pos.x, pl->pos.y - 28}, 650, true, false};
                        PlayFiringSound();
                        break;
                    }
                }
            }
            SpawnP((Vector2){pl->pos.x, pl->pos.y - 28}, (Color){200, 230, 255, 255}, 3, 60, 2);
        }
    }
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.bullets[i].active)
            continue;
        
        float speed = G.bullets[i].speed;
        if (G.bullets[i].isEnemy)
            G.bullets[i].pos.y += speed * dt;
        else
            G.bullets[i].pos.y -= speed * dt;

        if (G.bullets[i].pos.y < -20 || G.bullets[i].pos.y > SH + 20)
            G.bullets[i].active = false;
    }
    G.meteorTimer += dt;
    if (G.meteorTimer >= G.meteorRate)
    {
        G.meteorTimer -= G.meteorRate;
        SpawnMeteor(&G);
    }
    UpdateHealthStars(dt);
    UpdateShieldPickups(dt);
    UpdateFloatingTexts(dt);
    UpdateEnemies(dt);

    G.enemyTimer += dt;
    float currentEnemyRate = Clampf(G.enemyRate - G.gameTime * 0.015f, 1.5f, G.enemyRate);
    if (G.enemyTimer >= currentEnemyRate)
    {
        G.enemyTimer -= currentEnemyRate;
        SpawnEnemy();
    }
    for (int i = 0; i < MAX_METEORS; i++)
    {
        if (!G.meteors[i].active)
            continue;
        Meteor *m = &G.meteors[i];
        m->pos.x += m->vel.x * dt;
        m->pos.y += m->vel.y * dt;
        m->rotation += m->rotSpeed * dt;
        if (m->pos.y > SH + m->radius + 30)
            m->active = false;
    }
    for (int bi = 0; bi < MAX_BULLETS; bi++)
    {
        if (!G.bullets[bi].active)
            continue;
        for (int mi = 0; mi < MAX_METEORS; mi++)
        {
            if (!G.meteors[mi].active)
                continue;
            float dx = G.bullets[bi].pos.x - G.meteors[mi].pos.x, dy = G.bullets[bi].pos.y - G.meteors[mi].pos.y;
            if (dx * dx + dy * dy < G.meteors[mi].radius * G.meteors[mi].radius)
            {
                G.bullets[bi].active = false;
                G.meteors[mi].hp--;
                SpawnP(G.bullets[bi].pos, (Color){255, 200, 100, 255}, 6, 120, 2);
                if (G.meteors[mi].hp <= 0)
                {
                    G.meteors[mi].active = false;
                    PlayExplosionSound();
                    SpawnP(G.meteors[mi].pos, G.meteors[mi].color, 18, 200, 3.5f);
                    SpawnP(G.meteors[mi].pos, (Color){255, 220, 100, 255}, 8, 160, 2);
                    int bonus = (G.meteors[mi].size == METEOR_LARGE) ? 30 : (G.meteors[mi].size == METEOR_MEDIUM) ? 20
                                                                                                                  : 10;
                    G.score += (int)(bonus * G.comboMultiplier);
                    G.comboTimer = 2.0f;
                    G.comboMultiplier = Clampf(G.comboMultiplier + 0.25f, 1, 5);
                    G.meteorsDestroyed++;
                    if (G.meteors[mi].size == METEOR_LARGE)
                    {
                        SpawnMeteorAt(G.meteors[mi].pos, METEOR_MEDIUM);
                        SpawnMeteorAt(G.meteors[mi].pos, METEOR_MEDIUM);
                    }
                    else if (G.meteors[mi].size == METEOR_MEDIUM)
                    {
                        SpawnMeteorAt(G.meteors[mi].pos, METEOR_SMALL);
                        SpawnMeteorAt(G.meteors[mi].pos, METEOR_SMALL);
                    }
                    if (G.meteors[mi].size >= METEOR_LARGE)
                    {
                        G.shakeTimer = 0.25f;
                        G.shakeMag = 6;
                    }
                    /* Drop health OR shield pickup from medium+ meteors */
                    if (G.meteors[mi].size >= METEOR_MEDIUM)
                    {
                        int chance = (G.meteors[mi].size == METEOR_LARGE) ? 20 : 8;
                        if (GetRandomValue(0, 99) < chance)
                        {
                            /* 50/50 health vs shield */
                            if (GetRandomValue(0, 1) == 0)
                                SpawnHealthStarAt(G.meteors[mi].pos);
                            else
                                SpawnShieldPickupAt(G.meteors[mi].pos);
                        }
                    }
                }
                break;
            }
        }
    }

    /* Player bullets hit enemies */
    for (int bi = 0; bi < MAX_BULLETS; bi++)
    {
        if (!G.bullets[bi].active || G.bullets[bi].isEnemy)
            continue;
        for (int ei = 0; ei < MAX_ENEMIES; ei++)
        {
            if (!G.enemies[ei].active)
                continue;
            if (CheckCircleShipCollision(G.bullets[bi].pos, 5.0f, GetShipHitbox(G.enemies[ei].pos, G.enemies[ei].type, false)))
            {
                G.bullets[bi].active = false;
                G.enemies[ei].hp--;
                SpawnP(G.bullets[bi].pos, RED, 8, 100, 2);
                if (G.enemies[ei].hp <= 0)
                {
                    G.enemies[ei].active = false;
                    PlayExplosionSound();
                    PlayEnemyDestroySound();
                    SpawnP(G.enemies[ei].pos, RED, 20, 200, 3.5f);
                    
                    int points = 150;
                    if (G.enemies[ei].type == SHIP_DESTROYER) points = 500;
                    else if (G.enemies[ei].type == SHIP_TITAN) points = 1500;
                    
                    G.score += points;
                    G.enemiesDestroyed++;
                    
                    // Drop health or shield pickup
                    int dropChance = (G.enemies[ei].type == SHIP_TITAN) ? 40 : 
                                     (G.enemies[ei].type == SHIP_DESTROYER) ? 15 : 5;
                    
                    if (GetRandomValue(0, 99) < dropChance)
                    {
                        if (GetRandomValue(0, 1) == 0)
                            SpawnHealthStarAt(G.enemies[ei].pos);
                        else
                            SpawnShieldPickupAt(G.enemies[ei].pos);
                    }

                    char buf[16];
                    sprintf(buf, "+%d", points);
                    SpawnFloatingText(G.enemies[ei].pos, buf, GOLD);
                }
                break;
            }
        }
    }

    /* Enemy bullets hit player */
    if (pl->alive && pl->shieldTimer <= 0 && !G.playerShieldActive)
    {
        for (int bi = 0; bi < MAX_BULLETS; bi++)
        {
            if (!G.bullets[bi].active || !G.bullets[bi].isEnemy)
                continue;
            if (CheckCircleShipCollision(G.bullets[bi].pos, 5.0f, GetShipHitbox(pl->pos, pl->type, true)))
            {
                G.bullets[bi].active = false;
                pl->hp--;
                pl->shieldTimer = 0.5f;
                pl->damageFlash = 0.2f;
                PlayDamageSound();
                SpawnP(pl->pos, WHITE, 8, 120, 2);
                if (pl->hp <= 0)
                {
                    pl->alive = false;
                    SpawnP(pl->pos, WHITE, 40, 300, 4);
                    SpawnP(pl->pos, (Color){255, 100, 30, 255}, 30, 250, 3.5f);
                    SpawnP(pl->pos, SKYBLUE, 20, 200, 3);
                    G.slowMoTimer = 0.6f;
                    G.shakeTimer = 0.5f;
                    G.shakeMag = 12;
                    if (G.score > G.highscore)
                    {
                        G.highscore = G.score;
                        SaveHS(G.highscore);
                    }
                    G.gameOver = true;
                    StopMusicStream(G.bgmGameplay);
                    if (G.audioEnabled)
                    {
                        PlayMusicStream(G.bgmGameover);
                        SetMusicVolume(G.bgmGameover, G.bgmVolume);
                    }
                    for (int i = 0; i < 3; i++)
                    {
                        if (IsSoundPlaying(G.engineSounds[i]))
                            StopSound(G.engineSounds[i]);
                    }
                    G.enginePlaying = false;
                    G.screen = SCREEN_GAME_OVER;
                    G.goBtns[0] = MkBtn(SW / 2 - 100, 480, 200, 48, "PLAY AGAIN");
                    G.goBtns[1] = MkBtn(SW / 2 - 100, 540, 200, 48, "MAIN MENU");
                    G.goSel = 0;
                }
            }
        }
    }

    /* Player-Meteor collision (respects both invincibility timer and shield) */
    if (pl->alive && pl->shieldTimer <= 0 && !G.playerShieldActive)
    {
        for (int mi = 0; mi < MAX_METEORS; mi++)
        {
            if (!G.meteors[mi].active)
                continue;
            if (CheckCircleShipCollision(G.meteors[mi].pos, G.meteors[mi].radius, GetShipHitbox(pl->pos, pl->type, true)))
            {
                G.meteors[mi].active = false;
                pl->hp--;
                pl->shieldTimer = 1.0f;
                pl->damageFlash = 0.3f;
                G.shakeTimer = 0.3f;
                G.shakeMag = 8;
                PlayDamageSound();
                SpawnP(pl->pos, (Color){255, 255, 255, 255}, 12, 150, 3);
                if (pl->hp <= 0)
                {
                    pl->alive = false;
                    SpawnP(pl->pos, WHITE, 40, 300, 4);
                    SpawnP(pl->pos, (Color){255, 100, 30, 255}, 30, 250, 3.5f);
                    SpawnP(pl->pos, SKYBLUE, 20, 200, 3);
                    G.slowMoTimer = 0.6f;
                    G.shakeTimer = 0.5f;
                    G.shakeMag = 12;
                    if (G.score > G.highscore)
                    {
                        G.highscore = G.score;
                        SaveHS(G.highscore);
                    }
                    G.gameOver = true;
                    /* Stop gameplay BGM, start gameover BGM */
                    StopMusicStream(G.bgmGameplay);
                    if (G.audioEnabled)
                    {
                        PlayMusicStream(G.bgmGameover);
                        SetMusicVolume(G.bgmGameover, G.bgmVolume);
                    }
                    /* Stop engine sounds */
                    for (int i = 0; i < 3; i++)
                    {
                        if (IsSoundPlaying(G.engineSounds[i]))
                            StopSound(G.engineSounds[i]);
                    }
                    G.enginePlaying = false;
                    G.screen = SCREEN_GAME_OVER;
                    G.goBtns[0] = MkBtn(SW / 2 - 100, 480, 200, 48, "PLAY AGAIN");
                    G.goBtns[1] = MkBtn(SW / 2 - 100, 540, 200, 48, "MAIN MENU");
                    G.goSel = 0;
                }
                break;
            }
        }
    }
    /* Shield-active: destroy meteors on contact instead */
    else if (pl->alive && G.playerShieldActive)
    {
        for (int mi = 0; mi < MAX_METEORS; mi++)
        {
            if (!G.meteors[mi].active)
                continue;
            if (CheckCircleShipCollision(G.meteors[mi].pos, G.meteors[mi].radius + 15.0f, GetShipHitbox(pl->pos, pl->type, true)))
            {
                G.meteors[mi].active = false;
                PlayExplosionSound();
                SpawnP(G.meteors[mi].pos, (Color){100, 200, 255, 255}, 12, 150, 3);
                G.shakeTimer = 0.15f;
                G.shakeMag = 4;
            }
        }
    }

    /* Health pickup collection */
    if (pl->alive)
    {
        for (int i = 0; i < MAX_HEALTH_STARS; i++)
        {
            if (!G.healthStars[i].active)
                continue;
            float dx = pl->pos.x - G.healthStars[i].pos.x, dy = pl->pos.y - G.healthStars[i].pos.y;
            float collectRad = 30.0f;
            if (pl->type == SHIP_DESTROYER) collectRad = 45.0f;
            else if (pl->type == SHIP_TITAN) collectRad = 65.0f;
            if (dx * dx + dy * dy < collectRad * collectRad)
            {
                G.healthStars[i].active = false;
                PlayHealthPickupSound();
                if (pl->hp < pl->maxHp)
                {
                    pl->hp++;
                    SpawnP(pl->pos, (Color){100, 255, 150, 255}, 15, 200, 2.5f);
                    SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "+HP", (Color){100, 255, 100, 255});
                }
                else
                {
                    SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "MAX", (Color){255, 200, 100, 255});
                }
            }
        }
    }
    /* Shield pickup collection */
    if (pl->alive)
    {
        for (int i = 0; i < MAX_SHIELD_PICKUPS; i++)
        {
            if (!G.shieldPickups[i].active)
                continue;
            float dx = pl->pos.x - G.shieldPickups[i].pos.x, dy = pl->pos.y - G.shieldPickups[i].pos.y;
            float collectRad = 30.0f;
            if (pl->type == SHIP_DESTROYER) collectRad = 45.0f;
            else if (pl->type == SHIP_TITAN) collectRad = 65.0f;
            if (dx * dx + dy * dy < collectRad * collectRad)
            {
                G.shieldPickups[i].active = false;
                PlayShieldPickupSound();
                G.playerShieldActive = true;
                G.playerShieldDuration = 5.0f;
                /* Play shield activation sound once */
                if (G.audioEnabled)
                {
                    SetSoundVolume(G.sfxShieldOn, G.gameplayVolume);
                    PlaySound(G.sfxShieldOn);
                }
                SpawnP(pl->pos, (Color){100, 200, 255, 255}, 15, 200, 2.5f);
                SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "+SHIELD", (Color){100, 200, 255, 255});
            }
        }
    }
    if (pl->alive)
    {
        G.scoreTimer += dt;
        while (G.scoreTimer >= 1)
        {
            G.scoreTimer -= 1;
            G.score++;
        }
    }
    UpdateParticles(dt);
}

void DrawHPBar(Player p)
{
    int bw = 120, bh = 10, x = 10, y = SH - 30;
    DrawRectangle(x, y, bw, bh, (Color){40, 40, 40, 200});
    float frac = (float)p.hp / p.maxHp;
    Color hc = frac > 0.5f ? (Color){50, 200, 80, 255} : frac > 0.25f ? (Color){255, 200, 40, 255}
                                                                      : (Color){255, 50, 40, 255};
    DrawRectangle(x, y, (int)(bw * frac), bh, hc);
    DrawRectangleLinesEx((Rectangle){x, y, bw, bh}, 1, WHITE);
    DrawText(TextFormat("HP %d/%d", p.hp, p.maxHp), x + bw + 8, y - 2, 16, WHITE);
}

void DrawGameplay(void)
{
    Vector2 off = ShakeOff();
    BeginDrawing();
    ClearBackground((Color){4, 4, 16, 255});
    DrawNebula();
    Camera2D cam = {0};
    cam.offset = off;
    cam.target = (Vector2){0, 0};
    cam.rotation = 0;
    cam.zoom = 1;
    BeginMode2D(cam);
    DrawStars();
    DrawParticles();
    DrawHealthStars();
    DrawShieldPickups();
    DrawFloatingTexts();
    DrawEnemies();
    for (int i = 0; i < MAX_METEORS; i++)
    {
        if (G.meteors[i].active)
            DrawMeteor(G.meteors[i]);
    }
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.bullets[i].active)
            continue;
        Vector2 bp = G.bullets[i].pos;
        Color bc = G.bullets[i].isEnemy ? RED : (Color){180, 230, 255, 255};
        DrawRectangle((int)bp.x - 2, (int)bp.y - 7, 4, 14, bc);
        DrawRectangle((int)bp.x - 1, (int)bp.y - 5, 2, 10, WHITE);
        DrawCircleV(bp, 5, CAlpha(bc, 60));
    }
    if (G.player.alive)
    {
        if (G.player.damageFlash > 0)
        {
            DrawCircleV(G.player.pos, 30, CAlpha(RED, (unsigned char)(G.player.damageFlash / 0.3f * 100)));
        }
        DrawShipShape(G.player.pos, G.player.type, G.gameTime * 4, true);

        /* Shield visual effects */
        if (G.playerShieldActive)
        {
            float t = (float)GetTime();
            float pulse = 0.7f + 0.3f * sinf(t * 6.0f);
            unsigned char sa = (unsigned char)(pulse * 100);
            float baseRad = 38.0f;
            if (G.player.type == SHIP_DESTROYER) baseRad = 55.0f;
            else if (G.player.type == SHIP_TITAN) baseRad = 75.0f;
            float radius = baseRad + sinf(t * 4.0f) * 3.0f;
            DrawCircleV(G.player.pos, radius + 4, CAlpha((Color){50, 150, 255, 255}, (unsigned char)(sa / 3)));
            DrawCircleV(G.player.pos, radius, CAlpha((Color){80, 200, 255, 255}, (unsigned char)(sa / 2)));
            DrawCircleLines((int)G.player.pos.x, (int)G.player.pos.y, radius, CAlpha((Color){150, 220, 255, 255}, sa));
            DrawCircleLines((int)G.player.pos.x, (int)G.player.pos.y, radius + 2, CAlpha(WHITE, (unsigned char)(sa / 3)));
        }
        /* Damage invincibility flash (original) */
        else if (G.player.shieldTimer > 0)
        {
            unsigned char sa = (unsigned char)(G.player.shieldTimer / 1.0f * 120);
            float baseRad = 36.0f;
            if (G.player.type == SHIP_DESTROYER) baseRad = 53.0f;
            else if (G.player.type == SHIP_TITAN) baseRad = 73.0f;
            DrawCircleLines((int)G.player.pos.x, (int)G.player.pos.y, baseRad, CAlpha(SKYBLUE, sa));
            DrawCircleLines((int)G.player.pos.x, (int)G.player.pos.y, baseRad + 2, CAlpha(WHITE, (unsigned char)(sa / 2)));
        }
    }
    EndMode2D();
    DrawText(TextFormat("SCORE: %d", G.score), 10, 10, 26, WHITE);
    DrawText(TextFormat("HIGH: %d", G.highscore), 10, 40, 20, LIGHTGRAY);
    if (G.comboMultiplier > 1.0f)
    {
        const char *ct = TextFormat("COMBO x%.1f", G.comboMultiplier);
        DrawText(ct, SW - MeasureText(ct, 24) - 14, 10, 24, GOLD);
    }
    /* Shield duration indicator */
    if (G.playerShieldActive)
    {
        const char *st = TextFormat("SHIELD: %.1fs", G.playerShieldDuration);
        DrawText(st, SW - MeasureText(st, 20) - 14, 40, 20, (Color){100, 200, 255, 255});
    }
    DrawHPBar(G.player);
    EndDrawing();
}

void DrawTitle(float t)
{
    const char *title = "COSMIC OBLIVION";
    int tw = MeasureText(title, 52);
    float y = 100 + sinf(t * 1.5f) * 8;
    float hue = fmodf(t * 30, 360);
    Color c1 = ColorFromHSV(hue, 0.7f, 1.0f), c2 = ColorFromHSV(hue + 40, 0.6f, 1.0f);
    DrawText(title, (int)(SW / 2 - tw / 2 + 2), (int)(y + 2), 52, CAlpha(c1, 40));
    DrawText(title, (int)(SW / 2 - tw / 2 - 1), (int)(y - 1), 52, CAlpha(c2, 60));
    DrawText(title, (int)(SW / 2 - tw / 2), (int)(y), 52, WHITE);
}
