#include "include/button.h"
#include "include/helpers.h"
#include "include/constants.h"

Button MkBtn(float x, float y, float w, float h, const char *text)
{
    return (Button){{x, y, w, h}, text, false, 0.0f};
}
bool UpdateBtn(Button *b, float dt)
{
    Vector2 m = GetMousePosition();
    b->hovered = CheckCollisionPointRec(m, b->rect);
    b->hoverAnim += (b->hovered ? 1 : -1) * dt * 6.0f;
    b->hoverAnim = Clampf(b->hoverAnim, 0, 1);
    return b->hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
void DrawBtn(Button b, bool kbSel)
{
    float sc = 1.0f + b.hoverAnim * 0.06f;
    float glow = b.hoverAnim;
    if (kbSel)
        glow = 1.0f;
    Rectangle r = {b.rect.x + b.rect.width / 2 * (1 - sc), b.rect.y + b.rect.height / 2 * (1 - sc), b.rect.width * sc, b.rect.height * sc};
    if (glow > 0.01f)
        DrawRectangleRounded((Rectangle){r.x - 3, r.y - 3, r.width + 6, r.height + 6}, 0.3f, 8, CAlpha((Color){100, 180, 255, 255}, (unsigned char)(glow * 120)));
    Color bg = CLerp((Color){20, 30, 60, 220}, (Color){40, 70, 140, 240}, glow);
    DrawRectangleRounded(r, 0.3f, 8, bg);
    DrawRectangleRoundedLinesEx(r, 0.3f, 8, 2.0f, CAlpha((Color){120, 180, 255, 255}, (unsigned char)(150 + glow * 105)));
    int tw = MeasureText(b.text, 22);
    DrawText(b.text, (int)(r.x + (r.width - tw) / 2), (int)(r.y + (r.height - 22) / 2), 22, WHITE);
}
