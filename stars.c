#include "stars.h"
#include "constants.h"
#include "helpers.h"
#include <math.h>

extern GameState G;

void InitStars(void){
    for(int i=0;i<MAX_STARS;i++){
        Star*s=&G.stars[i];
        s->pos=(Vector2){(float)GetRandomValue(0,SW),(float)GetRandomValue(0,SH)};
        s->layer=GetRandomValue(0,2);
        s->speed=20.0f+s->layer*25.0f;
        s->brightness=Rf(0.3f,1.0f);
        s->twinklePhase=Rf(0,6.28f);
    }
}
void UpdateStars(float dt){
    for(int i=0;i<MAX_STARS;i++){
        Star*s=&G.stars[i];
        s->pos.y+=s->speed*dt;
        if(s->pos.y>SH){s->pos.y=-2;s->pos.x=(float)GetRandomValue(0,SW);}
        s->twinklePhase+=dt*(2.0f+s->layer);
    }
}
void DrawStars(void){
    for(int i=0;i<MAX_STARS;i++){
        Star*s=&G.stars[i];
        float tw=0.5f+0.5f*sinf(s->twinklePhase);
        float b=s->brightness*tw;
        unsigned char v=(unsigned char)(b*255);
        float sz=1.0f+s->layer*0.5f;
        DrawCircleV(s->pos,sz,(Color){v,v,(unsigned char)(v*0.9f+25),200});
    }
}
void DrawNebula(void){
    for(int y=0;y<SH;y+=4){
        float t=(float)y/SH;
        unsigned char r=(unsigned char)(8+t*15);
        unsigned char g=(unsigned char)(5+sinf(t*3.14f)*12);
        unsigned char b=(unsigned char)(20+t*20);
        DrawRectangle(0,y,SW,4,(Color){r,g,b,255});
    }
}
