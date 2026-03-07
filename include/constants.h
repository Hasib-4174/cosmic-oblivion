#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"

extern int SW;
extern int SH;

#define MAX_BULLETS   80
#define MAX_METEORS   48
#define MAX_PARTICLES 600
#define MAX_STARS     200
#define MAX_HEALTH_STARS 8
#define MAX_FLOATING_TEXT 16
#define HIGHSCORE_FILE "highscore.txt"

typedef enum { SCREEN_LOGO, SCREEN_MAIN_MENU, SCREEN_SHIP_SELECT, SCREEN_GAMEPLAY, SCREEN_PAUSE, SCREEN_GAME_OVER } GameScreen;
typedef enum { SHIP_INTERCEPTOR, SHIP_DESTROYER, SHIP_TITAN } ShipType;
typedef enum { METEOR_SMALL, METEOR_MEDIUM, METEOR_LARGE } MeteorSize;

typedef struct { Vector2 pos; Vector2 vel; float life, maxLife; Color color; float size; bool active; } Particle;
typedef struct { Vector2 pos; float speed, brightness, twinklePhase; int layer; } Star;
typedef struct { Vector2 pos; float rotation, pulsePhase; Vector2 vel; bool active; } HealthStar;
typedef struct { Vector2 pos; Vector2 vel; float life; const char *text; Color color; bool active; } FloatingText;
typedef struct { Vector2 pos; float speed; bool active; } Bullet;

typedef struct {
    Vector2 pos, vel; float radius, rotation, rotSpeed; int hp;
    MeteorSize size; Color color; bool active;
    float vertOffsets[12]; int sides;
} Meteor;

typedef struct {
    Vector2 pos, vel; float speed, fireRate, fireCooldown;
    int hp, maxHp; ShipType type; bool alive;
    float shieldTimer, damageFlash;
} Player;

typedef struct { Rectangle rect; const char *text; bool hovered; float hoverAnim; } Button;

typedef struct {
    GameScreen screen;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Meteor meteors[MAX_METEORS];
    Particle particles[MAX_PARTICLES];
    Star stars[MAX_STARS];
    HealthStar healthStars[MAX_HEALTH_STARS];
    FloatingText floatingTexts[MAX_FLOATING_TEXT];
    float meteorTimer, meteorRate, meteorSpeedMul;
    float scoreTimer, gameTime;
    int score, highscore, meteorsDestroyed;
    float comboTimer, comboMultiplier;
    float shakeTimer, shakeMag;
    float logoTimer, slowMoTimer;
    ShipType selectedShip;
    int menuSel, pauseSel, goSel, shipSel;
    bool gameOver;
    Button menuBtns[3], pauseBtns[3], goBtns[2];
} GameState;

#endif
