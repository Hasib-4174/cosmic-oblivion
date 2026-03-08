#include "include/helpers.h"
#include "include/constants.h"
#include <stdio.h>

extern GameState G;

float Rf(float a, float b) { return a + (float)GetRandomValue(0, 10000) / 10000.0f * (b - a); }
float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
Color CAlpha(Color c, unsigned char a)
{
    c.a = a;
    return c;
}
Color CLerp(Color a, Color b, float t)
{
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)};
}

int LoadHS(void)
{
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    if (!f)
        return 0;
    int h = 0;
    if (fscanf(f, "%d", &h) != 1)
        h = 0;
    fclose(f);
    return h;
}
void SaveHS(int s)
{
    FILE *f = fopen(HIGHSCORE_FILE, "w");
    if (!f)
        return;
    fprintf(f, "%d\n", s);
    fclose(f);
}

Vector2 ShakeOff(void)
{
    if (G.shakeTimer <= 0)
        return (Vector2){0, 0};
    return (Vector2){Rf(-G.shakeMag, G.shakeMag), Rf(-G.shakeMag, G.shakeMag)};
}
