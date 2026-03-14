#include "include/subship.h"
#include "include/constants.h"
#include "include/helpers.h"
#include "include/particles.h"
#include "include/floatingtext.h"
#include "include/collision.h"
#include <math.h>

extern GameState G;

/* ---- Pod balance constants ---- */
#define POD_FABRICATOR_COOLDOWN  35.0f
#define POD_SALVO_COOLDOWN      18.0f
#define POD_SENTINEL_COOLDOWN   15.0f

#define POD_FABRICATOR_HP  5.0f
#define POD_SALVO_HP       4.0f
#define POD_SENTINEL_HP    3.0f

#define POD_REPAIR_TIME    8.0f
#define POD_SIZE           15.0f
#define POD_FOLLOW_SPEED   6.0f

#define SALVO_ROCKET_SPEED   350.0f
#define SALVO_ROCKET_TURN    3.0f
#define SALVO_ROCKET_LIFE    2.5f
#define SALVO_ROCKET_COUNT   5
#define SALVO_DETECT_RADIUS  300.0f

#define SENTINEL_DETECT_RADIUS  120.0f
#define SENTINEL_PULSE_RADIUS   150.0f

#define FABRICATOR_SHIELD_BOOST 3.0f

/* ---- Helpers ---- */
static float VDist2(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

/* ---- Init ---- */
void InitSubShip(void)
{
    SubShip *p = &G.pod;
    p->type = G.selectedPod;
    p->pos = (Vector2){G.player.pos.x + 60, G.player.pos.y + 10};
    p->vel = (Vector2){0, 0};
    p->active = true;
    p->repairing = false;
    p->repairTimer = 0;
    p->animationTimer = 0;
    p->chargeProgress = 0;
    p->formationAngle = 0;

    switch (p->type)
    {
    case POD_FABRICATOR:
        p->cooldown = POD_FABRICATOR_COOLDOWN;
        p->maxHealth = POD_FABRICATOR_HP;
        break;
    case POD_SALVO:
        p->cooldown = POD_SALVO_COOLDOWN;
        p->maxHealth = POD_SALVO_HP;
        break;
    case POD_SENTINEL:
        p->cooldown = POD_SENTINEL_COOLDOWN;
        p->maxHealth = POD_SENTINEL_HP;
        break;
    }
    p->cooldownTimer = p->cooldown;
    p->health = p->maxHealth;

    /* Clear rockets */
    for (int i = 0; i < MAX_SUBSHIP_ROCKETS; i++)
        G.podRockets[i].active = false;

    G.empPulseTimer = 0;
}

/* ---- Formation movement ---- */
static void UpdatePodMovement(SubShip *pod, float dt)
{
    /* Target: right flank, slightly behind player */
    Vector2 target;
    target.x = G.player.pos.x + 60;
    target.y = G.player.pos.y + 10;

    /* Gentle hover oscillation */
    pod->formationAngle += dt * 2.0f;
    target.y += sinf(pod->formationAngle) * 5.0f;

    if (pod->repairing)
    {
        /* Drift slowly and wobble when repairing */
        target.x = G.player.pos.x + 80;
        target.y = G.player.pos.y + 30 + sinf(pod->formationAngle * 0.5f) * 10.0f;
    }

    /* Smooth interpolation */
    float speed = pod->repairing ? 2.0f : POD_FOLLOW_SPEED;
    pod->vel.x = (target.x - pod->pos.x) * speed;
    pod->vel.y = (target.y - pod->pos.y) * speed;
    pod->pos.x += pod->vel.x * dt;
    pod->pos.y += pod->vel.y * dt;

    /* Keep within screen bounds */
    pod->pos.x = Clampf(pod->pos.x, 20, SW - 20);
    pod->pos.y = Clampf(pod->pos.y, 20, SH - 20);
}

/* ---- Fabricator Pod logic ---- */
static void UpdateFabricator(SubShip *pod, float dt)
{
    pod->cooldownTimer -= dt;
    pod->chargeProgress = 1.0f - (pod->cooldownTimer / pod->cooldown);
    if (pod->chargeProgress > 1.0f) pod->chargeProgress = 1.0f;

    if (pod->cooldownTimer <= 0)
    {
        Player *pl = &G.player;
        if (pl->hp < pl->maxHp)
        {
            /* Heal the player */
            pl->hp++;
            SpawnP(pl->pos, (Color){80, 255, 120, 255}, 15, 180, 2.5f);
            SpawnP(pod->pos, (Color){80, 255, 120, 255}, 8, 100, 2.0f);
            SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "+HP", (Color){80, 255, 120, 255});
        }
        else
        {
            /* Grant temporary shield boost */
            G.playerShieldActive = true;
            G.playerShieldDuration += FABRICATOR_SHIELD_BOOST;
            SpawnP(pl->pos, (Color){100, 200, 255, 255}, 12, 150, 2.0f);
            SpawnP(pod->pos, (Color){100, 200, 255, 255}, 6, 80, 1.5f);
            SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "+SHIELD", (Color){100, 200, 255, 255});
        }
        pod->cooldownTimer = pod->cooldown;
        pod->chargeProgress = 0;
    }
}

/* ---- Salvo Pod logic ---- */
static int FindNearestTarget(Vector2 from, float radius)
{
    /* Returns index; sets a file-scope flag for target type */
    float bestDist = radius * radius;
    int bestIdx = -1;

    /* Check enemies first */
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!G.enemies[i].active) continue;
        float d = VDist2(from, G.enemies[i].pos);
        if (d < bestDist)
        {
            bestDist = d;
            bestIdx = i;
        }
    }
    if (bestIdx >= 0) return bestIdx;

    /* Check meteors */
    for (int i = 0; i < MAX_METEORS; i++)
    {
        if (!G.meteors[i].active) continue;
        float d = VDist2(from, G.meteors[i].pos);
        if (d < bestDist)
        {
            bestDist = d;
            bestIdx = 100 + i; /* offset to distinguish from enemies */
        }
    }
    return bestIdx;
}

static void FireSalvoRockets(SubShip *pod)
{
    int targetIdx = FindNearestTarget(pod->pos, SALVO_DETECT_RADIUS);
    if (targetIdx < 0) return; /* No target in range */

    bool isEnemy = (targetIdx < 100);
    int realIdx = isEnemy ? targetIdx : (targetIdx - 100);

    int fired = 0;
    for (int i = 0; i < MAX_SUBSHIP_ROCKETS && fired < SALVO_ROCKET_COUNT; i++)
    {
        if (G.podRockets[i].active) continue;
        SubShipRocket *r = &G.podRockets[i];
        r->active = true;
        r->life = SALVO_ROCKET_LIFE;
        r->targetIdx = realIdx;
        r->targetIsEnemy = isEnemy;
        r->pos = pod->pos;

        /* Spread initial velocity in a fan pattern */
        float angle = -1.57f + (fired - SALVO_ROCKET_COUNT / 2.0f) * 0.3f; /* upward fan */
        r->vel = (Vector2){cosf(angle) * SALVO_ROCKET_SPEED * 0.5f,
                           sinf(angle) * SALVO_ROCKET_SPEED * 0.5f};
        fired++;
    }

    if (fired > 0)
    {
        /* Heat vent particles */
        SpawnP(pod->pos, (Color){255, 160, 40, 255}, 10, 120, 2.0f);
        SpawnP(pod->pos, (Color){255, 100, 20, 200}, 6, 80, 1.5f);
    }
}

static void UpdateSalvo(SubShip *pod, float dt)
{
    pod->cooldownTimer -= dt;
    pod->chargeProgress = 1.0f - (pod->cooldownTimer / pod->cooldown);
    if (pod->chargeProgress > 1.0f) pod->chargeProgress = 1.0f;

    if (pod->cooldownTimer <= 0)
    {
        /* Check if a target is in range */
        int target = FindNearestTarget(pod->pos, SALVO_DETECT_RADIUS);
        if (target >= 0)
        {
            FireSalvoRockets(pod);
            pod->cooldownTimer = pod->cooldown;
            pod->chargeProgress = 0;
        }
        else
        {
            /* Stay charged, ready to fire when target appears */
            pod->cooldownTimer = 0;
            pod->chargeProgress = 1.0f;
        }
    }
}

static void UpdateSalvoRockets(float dt)
{
    for (int i = 0; i < MAX_SUBSHIP_ROCKETS; i++)
    {
        if (!G.podRockets[i].active) continue;
        SubShipRocket *r = &G.podRockets[i];
        r->life -= dt;
        if (r->life <= 0)
        {
            r->active = false;
            SpawnP(r->pos, (Color){255, 200, 100, 255}, 4, 60, 1.5f);
            continue;
        }

        /* Homing: steer toward target */
        Vector2 targetPos = {0, 0};
        bool hasTarget = false;

        if (r->targetIsEnemy && r->targetIdx >= 0 && r->targetIdx < MAX_ENEMIES &&
            G.enemies[r->targetIdx].active)
        {
            targetPos = G.enemies[r->targetIdx].pos;
            hasTarget = true;
        }
        else if (!r->targetIsEnemy && r->targetIdx >= 0 && r->targetIdx < MAX_METEORS &&
                 G.meteors[r->targetIdx].active)
        {
            targetPos = G.meteors[r->targetIdx].pos;
            hasTarget = true;
        }

        if (hasTarget)
        {
            /* Calculate desired direction */
            float dx = targetPos.x - r->pos.x;
            float dy = targetPos.y - r->pos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 1.0f)
            {
                dx /= dist;
                dy /= dist;
            }

            /* Steer velocity toward target (weak homing) */
            float currentSpeed = sqrtf(r->vel.x * r->vel.x + r->vel.y * r->vel.y);
            if (currentSpeed < 10.0f) currentSpeed = SALVO_ROCKET_SPEED;

            float curDx = r->vel.x / currentSpeed;
            float curDy = r->vel.y / currentSpeed;
            curDx += (dx - curDx) * SALVO_ROCKET_TURN * dt;
            curDy += (dy - curDy) * SALVO_ROCKET_TURN * dt;

            /* Renormalize */
            float len = sqrtf(curDx * curDx + curDy * curDy);
            if (len > 0.001f)
            {
                curDx /= len;
                curDy /= len;
            }
            r->vel.x = curDx * SALVO_ROCKET_SPEED;
            r->vel.y = curDy * SALVO_ROCKET_SPEED;
        }

        r->pos.x += r->vel.x * dt;
        r->pos.y += r->vel.y * dt;

        /* Smoke trail */
        if (GetRandomValue(0, 3) == 0)
            SpawnP(r->pos, (Color){200, 200, 200, 150}, 1, 30, 1.2f);

        /* Collision with enemies */
        for (int e = 0; e < MAX_ENEMIES; e++)
        {
            if (!G.enemies[e].active) continue;
            if (CheckCircleShipCollision(r->pos, 6.0f,
                    GetShipHitbox(G.enemies[e].pos, G.enemies[e].type, false)))
            {
                r->active = false;
                G.enemies[e].hp -= 3;
                SpawnP(r->pos, (Color){255, 180, 60, 255}, 8, 100, 2.0f);
                if (G.enemies[e].hp <= 0)
                {
                    G.enemies[e].active = false;
                    SpawnP(G.enemies[e].pos, RED, 20, 200, 3.5f);
                }
                break;
            }
        }
        if (!G.podRockets[i].active) continue;

        /* Collision with meteors */
        for (int m = 0; m < MAX_METEORS; m++)
        {
            if (!G.meteors[m].active) continue;
            float dx = r->pos.x - G.meteors[m].pos.x;
            float dy = r->pos.y - G.meteors[m].pos.y;
            if (dx * dx + dy * dy < G.meteors[m].radius * G.meteors[m].radius)
            {
                r->active = false;
                G.meteors[m].hp -= 3;
                SpawnP(r->pos, (Color){255, 200, 100, 255}, 6, 80, 1.5f);
                if (G.meteors[m].hp <= 0)
                {
                    G.meteors[m].active = false;
                    SpawnP(G.meteors[m].pos, G.meteors[m].color, 18, 200, 3.5f);
                }
                break;
            }
        }

        /* Off-screen cleanup */
        if (r->pos.y < -50 || r->pos.y > SH + 50 ||
            r->pos.x < -50 || r->pos.x > SW + 50)
            r->active = false;
    }
}

/* ---- Sentinel Pod logic ---- */
static void UpdateSentinel(SubShip *pod, float dt)
{
    pod->cooldownTimer -= dt;
    pod->chargeProgress = 1.0f - (pod->cooldownTimer / pod->cooldown);
    if (pod->chargeProgress > 1.0f) pod->chargeProgress = 1.0f;

    if (pod->cooldownTimer <= 0)
    {
        /* Scan for enemy bullets near the player */
        bool threat = false;
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (!G.bullets[i].active || !G.bullets[i].isEnemy) continue;
            float d = VDist2(G.bullets[i].pos, G.player.pos);
            if (d < SENTINEL_DETECT_RADIUS * SENTINEL_DETECT_RADIUS)
            {
                threat = true;
                break;
            }
        }

        if (threat)
        {
            /* Trigger EMP pulse: clear enemy bullets within radius */
            int cleared = 0;
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (!G.bullets[i].active || !G.bullets[i].isEnemy) continue;
                float d = VDist2(G.bullets[i].pos, G.player.pos);
                if (d < SENTINEL_PULSE_RADIUS * SENTINEL_PULSE_RADIUS)
                {
                    G.bullets[i].active = false;
                    SpawnP(G.bullets[i].pos, (Color){100, 180, 255, 255}, 3, 50, 1.5f);
                    cleared++;
                }
            }

            if (cleared > 0)
            {
                /* EMP visual */
                G.empPulseTimer = 0.4f;
                G.empPulsePos = G.player.pos;
                SpawnP(pod->pos, (Color){80, 160, 255, 255}, 12, 150, 2.5f);
                SpawnFloatingText((Vector2){pod->pos.x, pod->pos.y - 20}, "EMP", (Color){100, 200, 255, 255});
            }

            pod->cooldownTimer = pod->cooldown;
            pod->chargeProgress = 0;
        }
        else
        {
            /* Stay charged */
            pod->cooldownTimer = 0;
            pod->chargeProgress = 1.0f;
        }
    }
}

/* ---- Pod damage system ---- */
static void UpdatePodDamage(SubShip *pod, float dt)
{
    (void)dt;
    if (!pod->active || pod->repairing) return;

    /* Enemy bullet hits */
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!G.bullets[i].active || !G.bullets[i].isEnemy) continue;
        float d = VDist2(G.bullets[i].pos, pod->pos);
        if (d < POD_SIZE * POD_SIZE)
        {
            G.bullets[i].active = false;
            pod->health -= 1.0f;
            SpawnP(pod->pos, (Color){255, 200, 100, 255}, 5, 80, 1.5f);
        }
    }

    /* Meteor hits */
    for (int i = 0; i < MAX_METEORS; i++)
    {
        if (!G.meteors[i].active) continue;
        float dx = pod->pos.x - G.meteors[i].pos.x;
        float dy = pod->pos.y - G.meteors[i].pos.y;
        float combinedR = POD_SIZE + G.meteors[i].radius;
        if (dx * dx + dy * dy < combinedR * combinedR)
        {
            pod->health -= 1.0f;
            SpawnP(pod->pos, (Color){255, 180, 80, 255}, 6, 100, 2.0f);
        }
    }

    /* Enter repair mode */
    if (pod->health <= 0)
    {
        pod->health = 0;
        pod->repairing = true;
        pod->repairTimer = POD_REPAIR_TIME;
        pod->cooldownTimer = pod->cooldown; /* Reset ability cooldown */
        pod->chargeProgress = 0;
        SpawnP(pod->pos, (Color){255, 100, 40, 255}, 15, 150, 2.5f);
        SpawnFloatingText((Vector2){pod->pos.x, pod->pos.y - 20}, "POD DOWN", (Color){255, 100, 40, 255});
    }
}

/* ---- Pod repair system ---- */
static void UpdatePodRepair(SubShip *pod, float dt)
{
    if (!pod->repairing) return;

    pod->repairTimer -= dt;

    /* Repair sparks */
    if (GetRandomValue(0, 10) > 7)
        SpawnP(pod->pos, (Color){255, 220, 80, 200}, 1, 40, 1.0f);

    if (pod->repairTimer <= 0)
    {
        pod->repairing = false;
        pod->health = pod->maxHealth;
        pod->cooldownTimer = pod->cooldown;
        pod->chargeProgress = 0;
        SpawnP(pod->pos, (Color){80, 255, 180, 255}, 12, 120, 2.0f);
        SpawnFloatingText((Vector2){pod->pos.x, pod->pos.y - 20}, "REPAIRED", (Color){80, 255, 180, 255});
    }
}

/* ---- Main update ---- */
void UpdateSubShips(float dt)
{
    if (!G.player.alive) return;

    SubShip *pod = &G.pod;
    if (!pod->active) return;

    pod->animationTimer += dt;

    UpdatePodMovement(pod, dt);
    UpdatePodDamage(pod, dt);
    UpdatePodRepair(pod, dt);

    /* Only run ability logic when not repairing */
    if (!pod->repairing)
    {
        switch (pod->type)
        {
        case POD_FABRICATOR: UpdateFabricator(pod, dt); break;
        case POD_SALVO:      UpdateSalvo(pod, dt);      break;
        case POD_SENTINEL:   UpdateSentinel(pod, dt);    break;
        }
    }

    /* Update rockets regardless */
    UpdateSalvoRockets(dt);

    /* EMP pulse visual timer */
    if (G.empPulseTimer > 0)
        G.empPulseTimer -= dt;
}

/* ---- Drawing ---- */
static void DrawPodBody(SubShip *pod)
{
    Vector2 p = pod->pos;
    float t = pod->animationTimer;
    float hover = sinf(t * 3.0f) * 2.0f;
    p.y += hover;

    Color bodyColor, glowColor;
    switch (pod->type)
    {
    case POD_FABRICATOR:
        bodyColor = (Color){40, 180, 80, 255};
        glowColor = (Color){80, 255, 120, 255};
        break;
    case POD_SALVO:
        bodyColor = (Color){200, 100, 30, 255};
        glowColor = (Color){255, 160, 40, 255};
        break;
    case POD_SENTINEL:
        bodyColor = (Color){60, 120, 200, 255};
        glowColor = (Color){100, 180, 255, 255};
        break;
    }

    if (pod->repairing)
    {
        /* Blinking when repairing */
        float blink = sinf(t * 8.0f);
        if (blink < 0) return; /* Flash off */
        bodyColor = CAlpha(bodyColor, 150);
    }

    /* Pod body — small diamond shape */
    Vector2 top    = {p.x, p.y - POD_SIZE * 0.8f};
    Vector2 right  = {p.x + POD_SIZE * 0.6f, p.y};
    Vector2 bottom = {p.x, p.y + POD_SIZE * 0.5f};
    Vector2 left   = {p.x - POD_SIZE * 0.6f, p.y};

    /* Under-chassis */
    DrawTriangle(top, right, bottom, (Color){20, 25, 35, 255});
    DrawTriangle(top, bottom, left, (Color){20, 25, 35, 255});

    /* Main body plates */
    Vector2 top2    = {p.x, p.y - POD_SIZE * 0.65f};
    Vector2 right2  = {p.x + POD_SIZE * 0.45f, p.y};
    Vector2 bottom2 = {p.x, p.y + POD_SIZE * 0.35f};
    Vector2 left2   = {p.x - POD_SIZE * 0.45f, p.y};
    DrawTriangle(top2, right2, bottom2, bodyColor);
    DrawTriangle(top2, bottom2, left2, bodyColor);

    /* Center core glow */
    float pulse = 0.5f + 0.5f * sinf(t * 5.0f);
    DrawCircleV(p, 4, CAlpha(glowColor, (unsigned char)(100 + 80 * pulse)));
    DrawCircleV(p, 2, CAlpha(WHITE, (unsigned char)(150 + 60 * pulse)));

    /* Charge ring (when not repairing) */
    if (!pod->repairing && pod->chargeProgress > 0.01f)
    {
        float ringRadius = POD_SIZE + 4.0f;
        /* Draw partial ring based on charge progress */
        int segments = (int)(pod->chargeProgress * 16);
        for (int i = 0; i < segments; i++)
        {
            float a1 = (float)i / 16.0f * 6.28f - 1.57f;
            float a2 = (float)(i + 1) / 16.0f * 6.28f - 1.57f;
            Vector2 p1 = {p.x + cosf(a1) * ringRadius, p.y + sinf(a1) * ringRadius};
            Vector2 p2 = {p.x + cosf(a2) * ringRadius, p.y + sinf(a2) * ringRadius};
            DrawLineEx(p1, p2, 1.5f, CAlpha(glowColor, (unsigned char)(100 + 100 * pod->chargeProgress)));
        }

        /* Full charge glow */
        if (pod->chargeProgress > 0.99f)
        {
            float gPulse = 0.5f + 0.5f * sinf(t * 8.0f);
            DrawCircleV(p, ringRadius + 2, CAlpha(glowColor, (unsigned char)(20 + 30 * gPulse)));
        }
    }

    /* Pod-specific details */
    if (pod->type == POD_SALVO && !pod->repairing)
    {
        /* Exhaust vents during reload */
        if (pod->chargeProgress < 0.5f && pod->chargeProgress > 0.01f)
        {
            if (GetRandomValue(0, 5) == 0)
                SpawnP((Vector2){p.x, p.y + POD_SIZE * 0.4f}, (Color){200, 200, 200, 120}, 1, 20, 1.0f);
        }
    }

    /* Edge lines for detail */
    DrawLineEx(top, right, 1.0f, CAlpha(glowColor, 100));
    DrawLineEx(top, left, 1.0f, CAlpha(glowColor, 100));
}

static void DrawSalvoRockets(void)
{
    for (int i = 0; i < MAX_SUBSHIP_ROCKETS; i++)
    {
        if (!G.podRockets[i].active) continue;
        SubShipRocket *r = &G.podRockets[i];

        /* Rocket body */
        float angle = atan2f(r->vel.y, r->vel.x);
        Vector2 tip = {r->pos.x + cosf(angle) * 6, r->pos.y + sinf(angle) * 6};
        Vector2 lw  = {r->pos.x + cosf(angle + 2.5f) * 4, r->pos.y + sinf(angle + 2.5f) * 4};
        Vector2 rw  = {r->pos.x + cosf(angle - 2.5f) * 4, r->pos.y + sinf(angle - 2.5f) * 4};

        DrawTriangle(tip, rw, lw, (Color){255, 160, 40, 255});
        DrawCircleV(r->pos, 2, (Color){255, 220, 100, 255});

        /* Exhaust glow */
        Vector2 exhaust = {r->pos.x - cosf(angle) * 5, r->pos.y - sinf(angle) * 5};
        DrawCircleV(exhaust, 3, CAlpha((Color){255, 100, 20, 255}, 180));
    }
}

static void DrawEMPPulse(void)
{
    if (G.empPulseTimer <= 0) return;

    float t = G.empPulseTimer / 0.4f; /* 1.0 to 0.0 */
    float radius = SENTINEL_PULSE_RADIUS * (1.0f - t * 0.3f);
    unsigned char alpha = (unsigned char)(255 * t * t);

    DrawCircleLines((int)G.empPulsePos.x, (int)G.empPulsePos.y,
                    radius, CAlpha((Color){100, 180, 255, 255}, alpha));
    DrawCircleLines((int)G.empPulsePos.x, (int)G.empPulsePos.y,
                    radius * 0.7f, CAlpha((Color){150, 220, 255, 255}, (unsigned char)(alpha * 0.6f)));
    DrawCircleV(G.empPulsePos, radius * 0.3f * t,
                CAlpha((Color){200, 240, 255, 255}, (unsigned char)(alpha * 0.3f)));
}

static void DrawFabricatorBeam(SubShip *pod)
{
    /* Draw beam effect when healing (during the brief activation window) */
    if (pod->repairing || pod->chargeProgress < 0.95f) return;

    float t = pod->animationTimer;
    float pulse = 0.5f + 0.5f * sinf(t * 12.0f);

    /* Beam from pod to player */
    DrawLineEx(pod->pos, G.player.pos, 2.0f + pulse,
               CAlpha((Color){80, 255, 120, 255}, (unsigned char)(60 + 40 * pulse)));
    DrawLineEx(pod->pos, G.player.pos, 1.0f,
               CAlpha(WHITE, (unsigned char)(40 + 30 * pulse)));
}

void DrawSubShips(void)
{
    if (!G.player.alive) return;
    SubShip *pod = &G.pod;
    if (!pod->active) return;

    /* Draw beam first (under pod) */
    if (pod->type == POD_FABRICATOR)
        DrawFabricatorBeam(pod);

    DrawPodBody(pod);
    DrawSalvoRockets();
    DrawEMPPulse();
}

/* ---- HUD ---- */
void DrawSubShipHUD(void)
{
    if (!G.player.alive) return;
    SubShip *pod = &G.pod;
    if (!pod->active) return;

    int x = 10, y = SH - 65;

    /* Pod name */
    const char *podName = "???";
    Color podColor = WHITE;
    switch (pod->type)
    {
    case POD_FABRICATOR:
        podName = "FAB";
        podColor = (Color){80, 255, 120, 255};
        break;
    case POD_SALVO:
        podName = "SAL";
        podColor = (Color){255, 160, 40, 255};
        break;
    case POD_SENTINEL:
        podName = "SEN";
        podColor = (Color){100, 180, 255, 255};
        break;
    }

    DrawText(podName, x, y, 10, podColor);

    /* Cooldown bar */
    int bw = 80, bh = 5;
    int bx = x + 30, by = y + 1;
    DrawRectangle(bx, by, bw, bh, (Color){30, 30, 40, 200});

    if (pod->repairing)
    {
        /* Repair progress */
        float repairFrac = 1.0f - (pod->repairTimer / POD_REPAIR_TIME);
        DrawRectangle(bx, by, (int)(bw * repairFrac), bh, (Color){255, 180, 40, 255});
    }
    else
    {
        float cdFrac = pod->chargeProgress;
        Color cdColor = cdFrac > 0.99f ? (Color){100, 255, 150, 255} : podColor;
        DrawRectangle(bx, by, (int)(bw * cdFrac), bh, cdColor);
    }
    DrawRectangleLinesEx((Rectangle){(float)bx, (float)by, (float)bw, (float)bh}, 1,
                         CAlpha(podColor, 120));

    /* Health bar */
    int hy = by + bh + 2;
    DrawRectangle(bx, hy, bw, bh, (Color){30, 30, 40, 200});
    float hpFrac = pod->health / pod->maxHealth;
    Color hpColor = hpFrac > 0.5f ? (Color){50, 200, 80, 255}
                 : hpFrac > 0.25f ? (Color){255, 200, 40, 255}
                 : (Color){255, 50, 40, 255};
    if (pod->repairing) hpColor = (Color){255, 180, 40, 255};
    DrawRectangle(bx, hy, (int)(bw * hpFrac), bh, hpColor);
    DrawRectangleLinesEx((Rectangle){(float)bx, (float)hy, (float)bw, (float)bh}, 1,
                         CAlpha(podColor, 120));

    /* Status text */
    const char *status = "CHARGING";
    Color statusColor = (Color){160, 180, 200, 255};
    if (pod->repairing)
    {
        status = "REPAIR";
        statusColor = (Color){255, 180, 40, 255};
    }
    else if (pod->chargeProgress > 0.99f)
    {
        status = "READY";
        statusColor = (Color){100, 255, 150, 255};
    }
    DrawText(status, bx + bw + 6, y, 10, statusColor);
}
