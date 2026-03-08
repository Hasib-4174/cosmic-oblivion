#include "include/floatingtext.h"
#include "include/constants.h"
#include "include/helpers.h"
#include <string.h>

extern GameState G;

void InitFloatingTexts(void)
{
    for (int i = 0; i < MAX_FLOATING_TEXT; i++)
    {
        G.floatingTexts[i].active = false;
    }
}

void SpawnFloatingText(Vector2 pos, const char *text, Color color)
{
    for (int i = 0; i < MAX_FLOATING_TEXT; i++)
    {
        if (!G.floatingTexts[i].active)
        {
            G.floatingTexts[i].active = true;
            G.floatingTexts[i].pos = pos;
            G.floatingTexts[i].vel = (Vector2){Rf(-20, 20), Rf(-60, -40)};
            G.floatingTexts[i].life = 1.0f;
            strncpy(G.floatingTexts[i].text, text, 31);
            G.floatingTexts[i].text[31] = '\0';
            G.floatingTexts[i].color = color;
            return;
        }
    }
}

void UpdateFloatingTexts(float dt)
{
    for (int i = 0; i < MAX_FLOATING_TEXT; i++)
    {
        FloatingText *ft = &G.floatingTexts[i];
        if (!ft->active)
            continue;
        ft->pos.x += ft->vel.x * dt;
        ft->pos.y += ft->vel.y * dt;
        ft->vel.y += 80.0f * dt;
        ft->life -= dt * 1.0f;
        if (ft->life <= 0)
            ft->active = false;
    }
}

void DrawFloatingTexts(void)
{
    for (int i = 0; i < MAX_FLOATING_TEXT; i++)
    {
        FloatingText *ft = &G.floatingTexts[i];
        if (!ft->active)
            continue;
        unsigned char alpha = (unsigned char)(ft->life * 255);
        Color c = ft->color;
        c.a = alpha;
        DrawText(ft->text, (int)ft->pos.x - MeasureText(ft->text, 20) / 2, (int)ft->pos.y, 20, c);
    }
}
