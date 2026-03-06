#ifndef HELPERS_H
#define HELPERS_H

#include "raylib.h"

float Rf(float a, float b);
float Clampf(float v, float lo, float hi);
Color CAlpha(Color c, unsigned char a);
Color CLerp(Color a, Color b, float t);
int LoadHS(void);
void SaveHS(int s);
Vector2 ShakeOff(void);

#endif
