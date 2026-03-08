#include "include/particles.h"
#include "include/constants.h"
#include "include/helpers.h"
#include <math.h>

extern GameState G;

void SpawnP(Vector2 pos, Color c, int n, float spread, float sz)
{
    for (int i = 0; i < MAX_PARTICLES && n > 0; i++)
    {
        if (G.particles[i].active)
            continue;
        Particle *p = &G.particles[i];
        p->active = true;
        p->pos = pos;
        float a = Rf(0, 6.28f), sp = Rf(spread * 0.2f, spread);
        p->vel = (Vector2){cosf(a) * sp, sinf(a) * sp};
        p->maxLife = Rf(0.3f, 0.9f);
        p->life = p->maxLife;
        p->color = c;
        p->size = Rf(sz * 0.5f, sz);
        n--;
    }
}
void UpdateParticles(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!G.particles[i].active)
            continue;
        Particle *p = &G.particles[i];
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->vel.x *= 0.97f;
        p->vel.y *= 0.97f;
        p->life -= dt;
        if (p->life <= 0)
            p->active = false;
    }
}
void DrawParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!G.particles[i].active)
            continue;
        Particle *p = &G.particles[i];
        float t = p->life / p->maxLife;
        Color c = p->color;
        c.a = (unsigned char)(t * 255);
        float s = p->size * t;
        DrawCircleV(p->pos, s + 1, CAlpha(c, (unsigned char)(t * 80)));
        DrawCircleV(p->pos, s, c);
    }
}
