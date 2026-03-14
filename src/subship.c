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
#define POD_ORBIT_RADIUS   55.0f
#define POD_ORBIT_SPEED    1.8f

#define SALVO_ROCKET_SPEED   350.0f
#define SALVO_ROCKET_TURN    3.0f
#define SALVO_ROCKET_LIFE    2.5f
#define SALVO_ROCKET_COUNT   5
#define SALVO_DETECT_RADIUS  300.0f

#define SENTINEL_DETECT_RADIUS  120.0f
#define SENTINEL_PULSE_RADIUS   150.0f

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

/* ---- Orbiting movement: pod flies around the player ---- */
static void UpdatePodMovement(SubShip *pod, float dt)
{
    /* Orbit around the player */
    pod->formationAngle += dt * POD_ORBIT_SPEED;
    if (pod->formationAngle > 6.2832f) pod->formationAngle -= 6.2832f;

    Vector2 target;
    float orbitR = POD_ORBIT_RADIUS;

    if (pod->repairing)
    {
        /* Widen orbit and slow wobble when repairing */
        orbitR = POD_ORBIT_RADIUS + 25.0f;
    }

    /* Elliptical orbit: wider horizontally, tighter vertically */
    target.x = G.player.pos.x + cosf(pod->formationAngle) * orbitR;
    target.y = G.player.pos.y + sinf(pod->formationAngle) * (orbitR * 0.6f);

    /* Smooth interpolation toward orbit target */
    float speed = pod->repairing ? 2.5f : POD_FOLLOW_SPEED;
    pod->vel.x = (target.x - pod->pos.x) * speed;
    pod->vel.y = (target.y - pod->pos.y) * speed;
    pod->pos.x += pod->vel.x * dt;
    pod->pos.y += pod->vel.y * dt;

    /* Keep within screen bounds */
    pod->pos.x = Clampf(pod->pos.x, 20, SW - 20);
    pod->pos.y = Clampf(pod->pos.y, 20, SH - 20);
}

/* ---- Fabricator Pod logic: shield until damaged, recharge after break ---- */
static void UpdateFabricator(SubShip *pod, float dt)
{
    pod->cooldownTimer -= dt;
    pod->chargeProgress = 1.0f - (pod->cooldownTimer / pod->cooldown);
    if (pod->chargeProgress > 1.0f) pod->chargeProgress = 1.0f;

    /* If fab shield is currently active, nothing to do (it stays on until hit) */
    if (G.fabShieldActive)
    {
        /* Keep charge bar visually full while shield is up */
        pod->chargeProgress = 1.0f;
        pod->cooldownTimer = pod->cooldown; /* Reset timer so it takes 35s to recharge after shield breaks */
        return;
    }

    /* Cooldown finished — decide: heal or re-shield */
    if (pod->cooldownTimer <= 0)
    {
        Player *pl = &G.player;
        if (pl->hp < pl->maxHp)
        {
            /* Heal the player +1 HP */
            pl->hp++;
            SpawnP(pl->pos, (Color){80, 255, 120, 255}, 15, 180, 2.5f);
            SpawnP(pod->pos, (Color){80, 255, 120, 255}, 8, 100, 2.0f);
            SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "+HP", (Color){80, 255, 120, 255});
            pod->cooldownTimer = pod->cooldown;
            pod->chargeProgress = 0;
        }
        else
        {
            /* HP full → activate permanent shield */
            G.fabShieldActive = true;
            G.playerShieldActive = true;
            G.playerShieldDuration = 999.0f;
            SpawnP(pl->pos, (Color){100, 200, 255, 255}, 16, 180, 2.5f);
            SpawnP(pod->pos, (Color){100, 200, 255, 255}, 8, 100, 2.0f);
            SpawnFloatingText((Vector2){pl->pos.x, pl->pos.y - 30}, "SHIELD UP", (Color){100, 200, 255, 255});
            pod->chargeProgress = 1.0f;
            pod->cooldownTimer = 0;
        }
    }
}

/* When fabricator pod enters repair mode, remove the permanent shield */
static void FabricatorOnDamaged(SubShip *pod)
{
    if (pod->type == POD_FABRICATOR)
    {
        /* Shield remains on the player even if the drone goes down for repairs. */
        /* It will only break if the player actually takes damage. */
    }
}

/* When fabricator finishes repair, start recharging (don't immediately restore shield) */
static void FabricatorOnRepaired(SubShip *pod)
{
    if (pod->type == POD_FABRICATOR)
    {
        /* Shield will be re-granted after the next cooldown cycle */
        pod->cooldownTimer = pod->cooldown;
        pod->chargeProgress = 0;
    }
}

/* ---- Salvo Pod logic ---- */
static int FindNearestTarget(Vector2 from, float radius)
{
    /* Check both enemies AND meteors, return the absolute closest.
       Returns: 0..MAX_ENEMIES-1 = enemy, 100..100+MAX_METEORS-1 = meteor, -1 = none */
    float bestDist = radius * radius;
    int bestIdx = -1;

    /* Check enemies */
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

    /* Check meteors — can still override if closer */
    for (int i = 0; i < MAX_METEORS; i++)
    {
        if (!G.meteors[i].active) continue;
        float d = VDist2(from, G.meteors[i].pos);
        if (d < bestDist)
        {
            bestDist = d;
            bestIdx = 100 + i;
        }
    }
    return bestIdx;
}

static void FireSalvoRockets(SubShip *pod)
{
    int targetIdx = FindNearestTarget(pod->pos, SALVO_DETECT_RADIUS);
    if (targetIdx < 0) return;

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

        float angle = -1.57f + (fired - SALVO_ROCKET_COUNT / 2.0f) * 0.3f;
        r->vel = (Vector2){cosf(angle) * SALVO_ROCKET_SPEED * 0.5f,
                           sinf(angle) * SALVO_ROCKET_SPEED * 0.5f};
        fired++;
    }

    if (fired > 0)
    {
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
        int target = FindNearestTarget(pod->pos, SALVO_DETECT_RADIUS);
        if (target >= 0)
        {
            FireSalvoRockets(pod);
            pod->cooldownTimer = pod->cooldown;
            pod->chargeProgress = 0;
        }
        else
        {
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

        /* Homing: steer toward target. If original target dead, retarget. */
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

        /* If original target dead, find a new one */
        if (!hasTarget)
        {
            int newTarget = FindNearestTarget(r->pos, SALVO_DETECT_RADIUS * 1.5f);
            if (newTarget >= 0)
            {
                r->targetIsEnemy = (newTarget < 100);
                r->targetIdx = r->targetIsEnemy ? newTarget : (newTarget - 100);
                if (r->targetIsEnemy)
                    targetPos = G.enemies[r->targetIdx].pos;
                else
                    targetPos = G.meteors[r->targetIdx].pos;
                hasTarget = true;
            }
        }

        if (hasTarget)
        {
            float dx = targetPos.x - r->pos.x;
            float dy = targetPos.y - r->pos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 1.0f) { dx /= dist; dy /= dist; }

            float currentSpeed = sqrtf(r->vel.x * r->vel.x + r->vel.y * r->vel.y);
            if (currentSpeed < 10.0f) currentSpeed = SALVO_ROCKET_SPEED;

            float curDx = r->vel.x / currentSpeed;
            float curDy = r->vel.y / currentSpeed;
            curDx += (dx - curDx) * SALVO_ROCKET_TURN * dt;
            curDy += (dy - curDy) * SALVO_ROCKET_TURN * dt;

            float len = sqrtf(curDx * curDx + curDy * curDy);
            if (len > 0.001f) { curDx /= len; curDy /= len; }
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

    if (pod->health <= 0)
    {
        pod->health = 0;
        pod->repairing = true;
        pod->repairTimer = POD_REPAIR_TIME;
        pod->cooldownTimer = pod->cooldown;
        pod->chargeProgress = 0;
        SpawnP(pod->pos, (Color){255, 100, 40, 255}, 15, 150, 2.5f);
        SpawnFloatingText((Vector2){pod->pos.x, pod->pos.y - 20}, "POD DOWN", (Color){255, 100, 40, 255});
        FabricatorOnDamaged(pod);
    }
}

/* ---- Pod repair system ---- */
static void UpdatePodRepair(SubShip *pod, float dt)
{
    if (!pod->repairing) return;

    pod->repairTimer -= dt;

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
        FabricatorOnRepaired(pod);
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

    if (!pod->repairing)
    {
        switch (pod->type)
        {
        case POD_FABRICATOR: UpdateFabricator(pod, dt); break;
        case POD_SALVO:      UpdateSalvo(pod, dt);      break;
        case POD_SENTINEL:   UpdateSentinel(pod, dt);    break;
        }
    }

    UpdateSalvoRockets(dt);

    if (G.empPulseTimer > 0)
        G.empPulseTimer -= dt;
}

/* ================================================================== */
/* ---- DRAWING: unique pod designs per type ---- */
/* ================================================================== */

/* Fabricator Pod: organic, rounded healer drone with rotating energy ring */
static void DrawFabricatorPod(Vector2 p, float t, bool repairing, float charge)
{
    float pulse = 0.5f + 0.5f * sinf(t * 5.0f);
    Color body1 = (Color){30, 140, 70, 255};
    Color body2 = (Color){50, 200, 100, 255};
    Color glow  = (Color){80, 255, 120, 255};

    if (repairing)
    {
        float blink = sinf(t * 8.0f);
        if (blink < 0) return;
        body1 = CAlpha(body1, 150);
        body2 = CAlpha(body2, 150);
    }

    /* Outer dome (rounded) */
    DrawCircleV(p, 11, (Color){15, 30, 20, 255});
    DrawCircleV(p, 9, body1);
    DrawCircleV((Vector2){p.x, p.y - 2}, 6, body2);

    /* Cross symbol (healer identity) */
    DrawRectangle((int)p.x - 1, (int)p.y - 5, 3, 10, glow);
    DrawRectangle((int)p.x - 5, (int)p.y - 1, 10, 3, glow);

    /* Rotating energy ring */
    float ringA = t * 4.0f;
    for (int i = 0; i < 6; i++)
    {
        float a = ringA + i * 1.047f;
        Vector2 rp = {p.x + cosf(a) * 14, p.y + sinf(a) * 14};
        DrawCircleV(rp, 1.5f + pulse, CAlpha(glow, (unsigned char)(120 + 80 * pulse)));
    }

    /* Charge ring */
    if (!repairing && charge > 0.01f)
    {
        int segs = (int)(charge * 20);
        for (int i = 0; i < segs; i++)
        {
            float a1 = (float)i / 20.0f * 6.28f - 1.57f;
            float a2 = (float)(i + 1) / 20.0f * 6.28f - 1.57f;
            Vector2 p1 = {p.x + cosf(a1) * 16, p.y + sinf(a1) * 16};
            Vector2 p2 = {p.x + cosf(a2) * 16, p.y + sinf(a2) * 16};
            DrawLineEx(p1, p2, 1.5f, CAlpha(glow, (unsigned char)(80 + 120 * charge)));
        }
    }

    /* Core glow when fully charged */
    if (charge > 0.99f)
    {
        DrawCircleV(p, 13 + pulse * 3, CAlpha(glow, (unsigned char)(20 + 20 * pulse)));
    }

    /* Tiny thrust flicker */
    if (!repairing)
    {
        unsigned char ea = (unsigned char)(100 + GetRandomValue(0, 80));
        DrawCircleV((Vector2){p.x, p.y + 10}, 2 + pulse, CAlpha(glow, ea));
    }
}

/* Salvo Pod: angular, aggressive gunship drone with missile bay doors */
static void DrawSalvoPod(Vector2 p, float t, bool repairing, float charge)
{
    float pulse = 0.5f + 0.5f * sinf(t * 6.0f);
    Color frame = (Color){120, 60, 20, 255};
    Color plate = (Color){200, 100, 30, 255};
    Color glow  = (Color){255, 160, 40, 255};

    if (repairing)
    {
        float blink = sinf(t * 8.0f);
        if (blink < 0) return;
        frame = CAlpha(frame, 150);
        plate = CAlpha(plate, 150);
    }

    /* Main angular body (aggressive wedge) */
    DrawTriangle((Vector2){p.x, p.y - 12},
                 (Vector2){p.x + 10, p.y + 8},
                 (Vector2){p.x - 10, p.y + 8}, (Color){25, 15, 8, 255});
    DrawTriangle((Vector2){p.x, p.y - 10},
                 (Vector2){p.x + 8, p.y + 6},
                 (Vector2){p.x - 8, p.y + 6}, frame);
    DrawTriangle((Vector2){p.x, p.y - 7},
                 (Vector2){p.x + 5, p.y + 4},
                 (Vector2){p.x - 5, p.y + 4}, plate);

    /* Wing pylons */
    float bayOpen = charge > 0.8f ? (charge - 0.8f) * 5.0f : 0;
    float wingSpread = 3.0f + bayOpen * 4.0f;
    DrawRectangleV((Vector2){p.x - 14 - wingSpread, p.y - 2}, (Vector2){8, 6}, frame);
    DrawRectangleV((Vector2){p.x + 6 + wingSpread, p.y - 2}, (Vector2){8, 6}, frame);

    /* Missile dots on pylons */
    Color missileColor = charge > 0.95f ? WHITE : CAlpha(glow, 150);
    DrawCircleV((Vector2){p.x - 12 - wingSpread, p.y + 1}, 2, missileColor);
    DrawCircleV((Vector2){p.x + 12 + wingSpread, p.y + 1}, 2, missileColor);

    /* Center targeting lens */
    DrawCircleV(p, 3, (Color){40, 20, 10, 255});
    DrawCircleV(p, 2, CAlpha(glow, (unsigned char)(150 + 80 * pulse)));

    /* Charge ring */
    if (!repairing && charge > 0.01f)
    {
        int segs = (int)(charge * 16);
        for (int i = 0; i < segs; i++)
        {
            float a1 = (float)i / 16.0f * 6.28f - 1.57f;
            float a2 = (float)(i + 1) / 16.0f * 6.28f - 1.57f;
            Vector2 p1 = {p.x + cosf(a1) * 18, p.y + sinf(a1) * 18};
            Vector2 p2 = {p.x + cosf(a2) * 18, p.y + sinf(a2) * 18};
            DrawLineEx(p1, p2, 1.5f, CAlpha(glow, (unsigned char)(80 + 120 * charge)));
        }
    }

    /* Engine exhaust */
    if (!repairing)
    {
        unsigned char ea = (unsigned char)(120 + GetRandomValue(0, 100));
        DrawCircleV((Vector2){p.x - 3, p.y + 9}, 2, CAlpha(glow, ea));
        DrawCircleV((Vector2){p.x + 3, p.y + 9}, 2, CAlpha(glow, ea));
    }
}

/* Sentinel Pod: sleek, tech-heavy shield drone with scanning rings */
static void DrawSentinelPod(Vector2 p, float t, bool repairing, float charge)
{
    float pulse = 0.5f + 0.5f * sinf(t * 4.0f);
    Color frame = (Color){40, 80, 160, 255};
    Color glass = (Color){80, 160, 255, 255};
    Color glow  = (Color){100, 180, 255, 255};

    if (repairing)
    {
        float blink = sinf(t * 8.0f);
        if (blink < 0) return;
        frame = CAlpha(frame, 150);
        glass = CAlpha(glass, 150);
    }

    /* Hexagonal chassis */
    for (int i = 0; i < 6; i++)
    {
        float a1 = i * 1.047f + 0.524f;
        float a2 = (i + 1) * 1.047f + 0.524f;
        Vector2 v1 = {p.x + cosf(a1) * 10, p.y + sinf(a1) * 10};
        Vector2 v2 = {p.x + cosf(a2) * 10, p.y + sinf(a2) * 10};
        DrawTriangle(p, v1, v2, (i % 2 == 0) ? frame : (Color){30, 60, 120, 255});
    }

    /* Inner sensor dome */
    DrawCircleV(p, 5, (Color){15, 30, 50, 255});
    DrawCircleV(p, 3.5f, glass);

    /* Scanning line (rotates) */
    float scanAngle = t * 5.0f;
    Vector2 scanEnd = {p.x + cosf(scanAngle) * 8, p.y + sinf(scanAngle) * 8};
    DrawLineEx(p, scanEnd, 1.5f, CAlpha(glow, 200));

    /* Hex edge outlines */
    for (int i = 0; i < 6; i++)
    {
        float a1 = i * 1.047f + 0.524f;
        float a2 = (i + 1) * 1.047f + 0.524f;
        Vector2 v1 = {p.x + cosf(a1) * 10, p.y + sinf(a1) * 10};
        Vector2 v2 = {p.x + cosf(a2) * 10, p.y + sinf(a2) * 10};
        DrawLineEx(v1, v2, 1.0f, CAlpha(glow, 80));
    }

    /* Orbiting EMP charge nodes */
    if (!repairing && charge > 0.3f)
    {
        float nodeAngle = t * 3.0f;
        int nodeCount = (int)(charge * 4) + 1;
        for (int i = 0; i < nodeCount && i < 4; i++)
        {
            float a = nodeAngle + i * 1.57f;
            Vector2 np = {p.x + cosf(a) * 15, p.y + sinf(a) * 15};
            DrawCircleV(np, 2.0f + pulse, CAlpha(glow, (unsigned char)(100 + 100 * charge)));
        }
    }

    /* Charge ring */
    if (!repairing && charge > 0.01f)
    {
        int segs = (int)(charge * 16);
        for (int i = 0; i < segs; i++)
        {
            float a1 = (float)i / 16.0f * 6.28f - 1.57f;
            float a2 = (float)(i + 1) / 16.0f * 6.28f - 1.57f;
            Vector2 p1 = {p.x + cosf(a1) * 14, p.y + sinf(a1) * 14};
            Vector2 p2 = {p.x + cosf(a2) * 14, p.y + sinf(a2) * 14};
            DrawLineEx(p1, p2, 1.5f, CAlpha(glow, (unsigned char)(80 + 120 * charge)));
        }
    }

    /* Full charge pulse */
    if (charge > 0.99f)
    {
        float gPulse = 0.5f + 0.5f * sinf(t * 8.0f);
        DrawCircleV(p, 16 + gPulse * 3, CAlpha(glow, (unsigned char)(15 + 15 * gPulse)));
    }
}

static void DrawPodBody(SubShip *pod)
{
    Vector2 p = pod->pos;
    float t = pod->animationTimer;
    float hover = sinf(t * 3.0f) * 2.0f;
    p.y += hover;

    switch (pod->type)
    {
    case POD_FABRICATOR:
        DrawFabricatorPod(p, t, pod->repairing, pod->chargeProgress);
        break;
    case POD_SALVO:
        DrawSalvoPod(p, t, pod->repairing, pod->chargeProgress);
        break;
    case POD_SENTINEL:
        DrawSentinelPod(p, t, pod->repairing, pod->chargeProgress);
        break;
    }
}

static void DrawSalvoRockets(void)
{
    for (int i = 0; i < MAX_SUBSHIP_ROCKETS; i++)
    {
        if (!G.podRockets[i].active) continue;
        SubShipRocket *r = &G.podRockets[i];

        float angle = atan2f(r->vel.y, r->vel.x);
        Vector2 tip = {r->pos.x + cosf(angle) * 6, r->pos.y + sinf(angle) * 6};
        Vector2 lw  = {r->pos.x + cosf(angle + 2.5f) * 4, r->pos.y + sinf(angle + 2.5f) * 4};
        Vector2 rw  = {r->pos.x + cosf(angle - 2.5f) * 4, r->pos.y + sinf(angle - 2.5f) * 4};

        DrawTriangle(tip, rw, lw, (Color){255, 160, 40, 255});
        DrawCircleV(r->pos, 2, (Color){255, 220, 100, 255});

        Vector2 exhaust = {r->pos.x - cosf(angle) * 5, r->pos.y - sinf(angle) * 5};
        DrawCircleV(exhaust, 3, CAlpha((Color){255, 100, 20, 255}, 180));
    }
}

static void DrawEMPPulse(void)
{
    if (G.empPulseTimer <= 0) return;

    float t = G.empPulseTimer / 0.4f;
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
    if (pod->repairing || pod->chargeProgress < 0.95f) return;

    float t = pod->animationTimer;
    float pulse = 0.5f + 0.5f * sinf(t * 12.0f);

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

    int bw = 80, bh = 5;
    int bx = x + 30, by = y + 1;
    DrawRectangle(bx, by, bw, bh, (Color){30, 30, 40, 200});

    if (pod->repairing)
    {
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
