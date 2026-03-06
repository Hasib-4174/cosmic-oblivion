#include "include/meteor.h"
#include "include/helpers.h"
#include <math.h>

extern GameState G;

void SpawnMeteor(GameState*g){
    for(int i=0;i<MAX_METEORS;i++){
        if(g->meteors[i].active)continue;
        Meteor*m=&g->meteors[i]; m->active=true;
        int r=GetRandomValue(0,2);
        if(r==0){m->size=METEOR_SMALL;m->radius=Rf(10,16);m->hp=1;m->sides=GetRandomValue(5,7);}
        else if(r==1){m->size=METEOR_MEDIUM;m->radius=Rf(22,32);m->hp=2;m->sides=GetRandomValue(7,9);}
        else{m->size=METEOR_LARGE;m->radius=Rf(38,52);m->hp=3;m->sides=GetRandomValue(8,11);}
        m->pos=(Vector2){Rf(m->radius,SW-m->radius),-m->radius-10};
        float sp=Rf(70,200)*g->meteorSpeedMul;
        m->vel=(Vector2){Rf(-25,25),sp};
        m->rotation=Rf(0,360); m->rotSpeed=Rf(-100,100);
        for(int j=0;j<12;j++)m->vertOffsets[j]=Rf(0.7f,1.0f);
        float t=Rf(0,1);int cp=GetRandomValue(0,2);
        Color c1,c2;
        if(cp==0){c1=(Color){139,69,19,255};c2=(Color){255,140,0,255};}
        else if(cp==1){c1=(Color){160,40,40,255};c2=(Color){255,100,60,255};}
        else{c1=(Color){100,50,140,255};c2=(Color){180,100,220,255};}
        m->color=CLerp(c1,c2,t);
        return;
    }
}
void SpawnMeteorAt(Vector2 pos,MeteorSize sz){
    for(int i=0;i<MAX_METEORS;i++){
        if(G.meteors[i].active)continue;
        Meteor*m=&G.meteors[i]; m->active=true; m->size=sz;
        if(sz==METEOR_SMALL){m->radius=Rf(10,16);m->hp=1;m->sides=GetRandomValue(5,7);}
        else{m->radius=Rf(22,32);m->hp=2;m->sides=GetRandomValue(7,9);}
        m->pos=pos;
        m->vel=(Vector2){Rf(-60,60),Rf(50,150)*G.meteorSpeedMul};
        m->rotation=Rf(0,360);m->rotSpeed=Rf(-120,120);
        for(int j=0;j<12;j++)m->vertOffsets[j]=Rf(0.7f,1.0f);
        float t=Rf(0,1);m->color=CLerp((Color){139,69,19,255},(Color){255,140,0,255},t);
        return;
    }
}
void DrawMeteor(Meteor m){
    float step=360.0f/m.sides;
    Vector2 pts[12];
    for(int i=0;i<m.sides;i++){
        float a=(m.rotation+step*i)*DEG2RAD;
        float r=m.radius*m.vertOffsets[i];
        pts[i]=(Vector2){m.pos.x+cosf(a)*r,m.pos.y+sinf(a)*r};
    }
    for(int i=1;i<m.sides-1;i++) DrawTriangle(pts[0],pts[i+1],pts[i],m.color);
    Color dk={m.color.r*0.5f,m.color.g*0.5f,m.color.b*0.4f,255};
    for(int i=1;i<m.sides-1;i++){
        Vector2 c={m.pos.x,m.pos.y};
        Vector2 a=pts[i],b=pts[(i+1)%m.sides];
        Vector2 ma={(c.x+a.x)/2,(c.y+a.y)/2},mb={(c.x+b.x)/2,(c.y+b.y)/2};
        DrawTriangle(c,mb,ma,CAlpha(dk,80));
    }
}
