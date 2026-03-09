#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"
#include <stdbool.h>

extern int SW;
extern int SH;

#define MAX_BULLETS 128
#define MAX_METEORS 48
#define MAX_PARTICLES 600
#define MAX_STARS 200
#define MAX_HEALTH_STARS 8
#define MAX_SHIELD_PICKUPS 8
#define MAX_ENEMIES 8
#define MAX_FLOATING_TEXT 16
#define MAX_EXPLOSION_VARIANTS 2
#define HIGHSCORE_FILE "highscore.txt"

typedef enum
{
    SCREEN_LOGO,
    SCREEN_MAIN_MENU,
    SCREEN_SHIP_SELECT,
    SCREEN_GAMEPLAY,
    SCREEN_PAUSE,
    SCREEN_GAME_OVER,
    SCREEN_OPTIONS,
    SCREEN_AUDIO
} GameScreen;
typedef enum
{
    SHIP_INTERCEPTOR,
    SHIP_DESTROYER,
    SHIP_TITAN
} ShipType;
typedef enum
{
    METEOR_SMALL,
    METEOR_MEDIUM,
    METEOR_LARGE
} MeteorSize;

typedef struct
{
    Vector2 pos;
    Vector2 vel;
    float life, maxLife;
    Color color;
    float size;
    bool active;
} Particle;
typedef struct
{
    Vector2 pos;
    float speed, brightness, twinklePhase;
    int layer;
} Star;
typedef struct
{
    Vector2 pos;
    float rotation, pulsePhase;
    Vector2 vel;
    bool active;
} HealthStar;
typedef struct
{
    Vector2 pos;
    float rotation, pulsePhase;
    Vector2 vel;
    bool active;
} ShieldPickup;
typedef struct
{
    Vector2 pos;
    Vector2 vel;
    float life;
    char text[32];
    Color color;
    bool active;
} FloatingText;
typedef struct
{
    Vector2 pos;
    float speed;
    bool active;
    bool isEnemy;
} Bullet;

typedef struct
{
    Vector2 pos, vel;
    float rotation, speed;
    float fireCooldown;
    int hp;
    ShipType type;
    bool active;
} Enemy;

typedef struct
{
    Vector2 pos, vel;
    float radius, rotation, rotSpeed;
    int hp;
    MeteorSize size;
    Color color;
    bool active;
    float vertOffsets[12];
    int sides;
} Meteor;

typedef struct
{
    Vector2 pos, vel;
    float speed, fireRate, fireCooldown;
    int hp, maxHp;
    ShipType type;
    bool alive;
    float shieldTimer, damageFlash;
} Player;

typedef struct
{
    Rectangle rect;
    const char *text;
    bool hovered;
    float hoverAnim;
} Button;

typedef struct
{
    GameScreen screen;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Meteor meteors[MAX_METEORS];
    Particle particles[MAX_PARTICLES];
    Star stars[MAX_STARS];
    HealthStar healthStars[MAX_HEALTH_STARS];
    ShieldPickup shieldPickups[MAX_SHIELD_PICKUPS];
    Enemy enemies[MAX_ENEMIES];
    FloatingText floatingTexts[MAX_FLOATING_TEXT];
    float meteorTimer, meteorRate, meteorSpeedMul;
    float enemyTimer, enemyRate;
    float scoreTimer, gameTime;
    int score, highscore, meteorsDestroyed, enemiesDestroyed;
    float comboTimer, comboMultiplier;
    float shakeTimer, shakeMag;
    float logoTimer, slowMoTimer;
    ShipType selectedShip;
    int menuSel, pauseSel, goSel, shipSel, optSel, audioSel;
    bool gameOver;

    /* Audio volumes (3 categories) */
    float bgmVolume;
    float uiVolume;
    float gameplayVolume;

    /* Audio toggle */
    bool audioEnabled;

    /* Buttons */
    Button menuBtns[3], pauseBtns[3], goBtns[2], optBtns[2];
    Button audioBackBtn;

    /* Firing sound pool */
    Sound firingSounds[8];
    int firingSoundIdx;

    /* Explosion sound pool (round-robin instances of random variants) */
    Sound explosionSounds[8];
    int explosionSoundIdx;
    Sound explosionVariants[MAX_EXPLOSION_VARIANTS];
    int explosionVariantCount;

    /* Health pickup sound pool */
    Sound healthPickupSounds[8];
    int healthPickupSoundIdx;

    /* Shield pickup sound pool */
    Sound shieldPickupSounds[8];
    int shieldPickupSoundIdx;

    /* Damage sound */
    Sound damageSound;

    /* Engine sounds (one per ship type) */
    Sound engineSounds[3];
    bool engineSoundsLoaded;
    bool enginePlaying;

    /* BGM (3 separate tracks) */
    Music bgmMenu;
    Music bgmGameplay;
    Music bgmGameover;

    /* UI sounds */
    Sound sfxButtonHover;
    Sound sfxButtonSelect;

    /* Enemy sounds */
    Sound sfxEnemyShoot[8];
    int sfxEnemyShootIdx;
    Sound sfxEnemyDestroy;
    Sound sfxEnemyEngine;

    /* Shield activation sound */
    Sound sfxShieldOn;

    /* Previous selection trackers (for button hover sound deduplication) */
    int prevMenuSel, prevPauseSel, prevGoSel, prevOptSel, prevAudioSel, prevShipSel;

    /* Shield pickup system */
    bool playerShieldActive;
    float playerShieldDuration;
} GameState;

#endif
