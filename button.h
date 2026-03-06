#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"
#include "constants.h"

Button MkBtn(float x, float y, float w, float h, const char *text);
bool UpdateBtn(Button *b, float dt);
void DrawBtn(Button b, bool kbSel);

#endif
