/* Cosmic Oblivion - Arcade Space Shooter | gcc main.c -lraylib -lm -lpthread -ldl -lrt -lX11 */
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SW 1000
#define SH 700
#define MAX_BULLETS   80
#define MAX_METEORS   48
#define MAX_PARTICLES 600
#define MAX_STARS     200
#define HIGHSCORE_FILE "highscore.txt"

typedef enum { SCREEN_LOGO, SCREEN_MAIN_MENU, SCREEN_SHIP_SELECT, SCREEN_GAMEPLAY, SCREEN_PAUSE, SCREEN_GAME_OVER } GameScreen;
typedef enum { SHIP_INTERCEPTOR, SHIP_DESTROYER, SHIP_TITAN } ShipType;
typedef enum { METEOR_SMALL, METEOR_MEDIUM, METEOR_LARGE } MeteorSize;

typedef struct { Vector2 pos; Vector2 vel; float life, maxLife; Color color; float size; bool active; } Particle;
typedef struct { Vector2 pos; float speed, brightness, twinklePhase; int layer; } Star;
typedef struct { Vector2 pos; float speed; bool active; } Bullet;

typedef struct {
    Vector2 pos, vel; float radius, rotation, rotSpeed; int hp;
    MeteorSize size; Color color; bool active;
    float vertOffsets[12]; int sides;
} Meteor;

typedef struct {
    Vector2 pos, vel; float speed, fireRate, fireCooldown;
    int hp, maxHp; ShipType type; bool alive;
    float shieldTimer, damageFlash;
} Player;

typedef struct { Rectangle rect; const char *text; bool hovered; float hoverAnim; } Button;

typedef struct {
    GameScreen screen;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Meteor meteors[MAX_METEORS];
    Particle particles[MAX_PARTICLES];
    Star stars[MAX_STARS];
    float meteorTimer, meteorRate, meteorSpeedMul;
    float scoreTimer, gameTime;
    int score, highscore, meteorsDestroyed;
    float comboTimer, comboMultiplier;
    float shakeTimer, shakeMag;
    float logoTimer, slowMoTimer;
    ShipType selectedShip;
    int menuSel, pauseSel, goSel, shipSel;
    bool gameOver;
    Button menuBtns[3], pauseBtns[3], goBtns[2];
} GameState;

static GameState G;

/* --- Helpers --- */
static float Rf(float a, float b) { return a + (float)GetRandomValue(0,10000)/10000.0f*(b-a); }
static float Clampf(float v,float lo,float hi){return v<lo?lo:(v>hi?hi:v);}
static Color CAlpha(Color c,unsigned char a){c.a=a;return c;}
static Color CLerp(Color a,Color b,float t){return (Color){(unsigned char)(a.r+(b.r-a.r)*t),(unsigned char)(a.g+(b.g-a.g)*t),(unsigned char)(a.b+(b.b-a.b)*t),(unsigned char)(a.a+(b.a-a.a)*t)};}

static int LoadHS(void){FILE*f=fopen(HIGHSCORE_FILE,"r");if(!f)return 0;int h=0;if(fscanf(f,"%d",&h)!=1)h=0;fclose(f);return h;}
static void SaveHS(int s){FILE*f=fopen(HIGHSCORE_FILE,"w");if(!f)return;fprintf(f,"%d\n",s);fclose(f);}

/* --- Stars --- */
static void InitStars(void){
    for(int i=0;i<MAX_STARS;i++){
        Star*s=&G.stars[i];
        s->pos=(Vector2){(float)GetRandomValue(0,SW),(float)GetRandomValue(0,SH)};
        s->layer=GetRandomValue(0,2);
        s->speed=20.0f+s->layer*25.0f;
        s->brightness=Rf(0.3f,1.0f);
        s->twinklePhase=Rf(0,6.28f);
    }
}
static void UpdateStars(float dt){
    for(int i=0;i<MAX_STARS;i++){
        Star*s=&G.stars[i];
        s->pos.y+=s->speed*dt;
        if(s->pos.y>SH){s->pos.y=-2;s->pos.x=(float)GetRandomValue(0,SW);}
        s->twinklePhase+=dt*(2.0f+s->layer);
    }
}
static void DrawStars(void){
    for(int i=0;i<MAX_STARS;i++){
        Star*s=&G.stars[i];
        float tw=0.5f+0.5f*sinf(s->twinklePhase);
        float b=s->brightness*tw;
        unsigned char v=(unsigned char)(b*255);
        float sz=1.0f+s->layer*0.5f;
        DrawCircleV(s->pos,sz,(Color){v,v,(unsigned char)(v*0.9f+25),200});
    }
}
static void DrawNebula(void){
    for(int y=0;y<SH;y+=4){
        float t=(float)y/SH;
        unsigned char r=(unsigned char)(8+t*15);
        unsigned char g=(unsigned char)(5+sinf(t*3.14f)*12);
        unsigned char b=(unsigned char)(20+t*20);
        DrawRectangle(0,y,SW,4,(Color){r,g,b,255});
    }
}

/* --- Particles --- */
static void SpawnP(Vector2 pos,Color c,int n,float spread,float sz){
    for(int i=0;i<MAX_PARTICLES&&n>0;i++){
        if(G.particles[i].active)continue;
        Particle*p=&G.particles[i];
        p->active=true; p->pos=pos;
        float a=Rf(0,6.28f),sp=Rf(spread*0.2f,spread);
        p->vel=(Vector2){cosf(a)*sp,sinf(a)*sp};
        p->maxLife=Rf(0.3f,0.9f); p->life=p->maxLife;
        p->color=c; p->size=Rf(sz*0.5f,sz);
        n--;
    }
}
static void UpdateParticles(float dt){
    for(int i=0;i<MAX_PARTICLES;i++){
        if(!G.particles[i].active)continue;
        Particle*p=&G.particles[i];
        p->pos.x+=p->vel.x*dt; p->pos.y+=p->vel.y*dt;
        p->vel.x*=0.97f; p->vel.y*=0.97f;
        p->life-=dt;
        if(p->life<=0)p->active=false;
    }
}
static void DrawParticles(void){
    for(int i=0;i<MAX_PARTICLES;i++){
        if(!G.particles[i].active)continue;
        Particle*p=&G.particles[i];
        float t=p->life/p->maxLife;
        Color c=p->color; c.a=(unsigned char)(t*255);
        float s=p->size*t;
        DrawCircleV(p->pos,s+1,CAlpha(c,(unsigned char)(t*80)));
        DrawCircleV(p->pos,s,c);
    }
}

/* --- Button helpers --- */
static Button MkBtn(float x,float y,float w,float h,const char*text){
    return (Button){{x,y,w,h},text,false,0.0f};
}
static bool UpdateBtn(Button*b,float dt){
    Vector2 m=GetMousePosition();
    b->hovered=CheckCollisionPointRec(m,b->rect);
    b->hoverAnim+=(b->hovered?1:-1)*dt*6.0f;
    b->hoverAnim=Clampf(b->hoverAnim,0,1);
    return b->hovered&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static void DrawBtn(Button b,bool kbSel){
    float sc=1.0f+b.hoverAnim*0.06f;
    float glow=b.hoverAnim;
    if(kbSel)glow=1.0f;
    Rectangle r={b.rect.x+b.rect.width/2*(1-sc),b.rect.y+b.rect.height/2*(1-sc),b.rect.width*sc,b.rect.height*sc};
    if(glow>0.01f) DrawRectangleRounded((Rectangle){r.x-3,r.y-3,r.width+6,r.height+6},0.3f,8,CAlpha((Color){100,180,255,255},(unsigned char)(glow*120)));
    Color bg=CLerp((Color){20,30,60,220},(Color){40,70,140,240},glow);
    DrawRectangleRounded(r,0.3f,8,bg);
    DrawRectangleRoundedLinesEx(r,0.3f,8,2.0f,CAlpha((Color){120,180,255,255},(unsigned char)(150+glow*105)));
    int tw=MeasureText(b.text,22);
    DrawText(b.text,(int)(r.x+(r.width-tw)/2),(int)(r.y+(r.height-22)/2),22,WHITE);
}

/* --- Ship drawing --- */
static void DrawShipShape(Vector2 p,ShipType t,float hover,bool engineOn){
    float h=sinf(hover)*3.0f;
    p.y+=h;
    if(t==SHIP_INTERCEPTOR){
        DrawTriangle((Vector2){p.x,p.y-28},(Vector2){p.x+14,p.y+18},(Vector2){p.x-14,p.y+18},(Color){30,180,255,255});
        DrawTriangle((Vector2){p.x,p.y-20},(Vector2){p.x+6,p.y+8},(Vector2){p.x-6,p.y+8},(Color){140,220,255,255});
        DrawTriangle((Vector2){p.x-14,p.y+8},(Vector2){p.x-6,p.y+12},(Vector2){p.x-18,p.y+20},(Color){20,120,220,255});
        DrawTriangle((Vector2){p.x+14,p.y+8},(Vector2){p.x+18,p.y+20},(Vector2){p.x+6,p.y+12},(Color){20,120,220,255});
        if(engineOn){
            unsigned char ea=(unsigned char)(150+GetRandomValue(0,105));
            DrawCircleV((Vector2){p.x,p.y+22},5,CAlpha((Color){0,255,255,255},ea));
            DrawCircleV((Vector2){p.x,p.y+22},8,CAlpha((Color){0,200,255,255},(unsigned char)(ea/3)));
        }
    } else if(t==SHIP_DESTROYER){
        DrawTriangle((Vector2){p.x,p.y-24},(Vector2){p.x+18,p.y+20},(Vector2){p.x-18,p.y+20},(Color){80,80,100,255});
        DrawTriangle((Vector2){p.x,p.y-16},(Vector2){p.x+10,p.y+10},(Vector2){p.x-10,p.y+10},(Color){160,160,180,255});
        DrawRectangle((int)p.x-20,(int)p.y+4,8,14,(Color){60,60,80,255});
        DrawRectangle((int)p.x+12,(int)p.y+4,8,14,(Color){60,60,80,255});
        DrawRectangle((int)p.x-3,(int)p.y+14,6,8,(Color){60,60,80,255});
        if(engineOn){
            unsigned char ea=(unsigned char)(150+GetRandomValue(0,105));
            DrawCircleV((Vector2){p.x-8,p.y+22},4,CAlpha((Color){255,160,40,255},ea));
            DrawCircleV((Vector2){p.x+8,p.y+22},4,CAlpha((Color){255,160,40,255},ea));
            DrawCircleV((Vector2){p.x-8,p.y+22},7,CAlpha((Color){255,120,20,255},(unsigned char)(ea/3)));
            DrawCircleV((Vector2){p.x+8,p.y+22},7,CAlpha((Color){255,120,20,255},(unsigned char)(ea/3)));
        }
    } else {
        DrawTriangle((Vector2){p.x,p.y-22},(Vector2){p.x+24,p.y+22},(Vector2){p.x-24,p.y+22},(Color){140,40,40,255});
        DrawTriangle((Vector2){p.x,p.y-12},(Vector2){p.x+14,p.y+12},(Vector2){p.x-14,p.y+12},(Color){200,80,80,255});
        DrawRectangle((int)p.x-26,(int)p.y+2,10,16,(Color){120,30,30,255});
        DrawRectangle((int)p.x+16,(int)p.y+2,10,16,(Color){120,30,30,255});
        DrawRectangle((int)p.x-4,(int)p.y-26,8,10,(Color){120,30,30,255});
        if(engineOn){
            unsigned char ea=(unsigned char)(150+GetRandomValue(0,105));
            DrawCircleV((Vector2){p.x-10,p.y+24},5,CAlpha((Color){255,50,20,255},ea));
            DrawCircleV((Vector2){p.x,p.y+24},5,CAlpha((Color){255,50,20,255},ea));
            DrawCircleV((Vector2){p.x+10,p.y+24},5,CAlpha((Color){255,50,20,255},ea));
            DrawCircleV((Vector2){p.x,p.y+24},10,CAlpha((Color){255,30,10,255},(unsigned char)(ea/4)));
        }
    }
}

/* --- Init --- */
static void InitPlayer(void){
    Player*pl=&G.player;
    pl->pos=(Vector2){SW/2.0f,SH-80.0f};
    pl->vel=(Vector2){0,0};
    pl->alive=true;
    pl->shieldTimer=0; pl->damageFlash=0;
    pl->type=G.selectedShip;
    if(pl->type==SHIP_INTERCEPTOR){pl->speed=420;pl->fireRate=0.12f;pl->maxHp=3;}
    else if(pl->type==SHIP_DESTROYER){pl->speed=320;pl->fireRate=0.2f;pl->maxHp=5;}
    else{pl->speed=220;pl->fireRate=0.28f;pl->maxHp=8;}
    pl->hp=pl->maxHp; pl->fireCooldown=0;
}
static void InitGame(void){
    for(int i=0;i<MAX_BULLETS;i++)G.bullets[i].active=false;
    for(int i=0;i<MAX_METEORS;i++)G.meteors[i].active=false;
    for(int i=0;i<MAX_PARTICLES;i++)G.particles[i].active=false;
    G.meteorTimer=0; G.meteorRate=1.2f; G.meteorSpeedMul=1.0f;
    G.scoreTimer=0; G.gameTime=0; G.score=0; G.meteorsDestroyed=0;
    G.comboTimer=0; G.comboMultiplier=1.0f;
    G.shakeTimer=0; G.shakeMag=0; G.slowMoTimer=0;
    G.gameOver=false;
    InitPlayer();
}

/* --- Meteor spawn --- */
static void SpawnMeteor(GameState*g){
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
static void SpawnMeteorAt(Vector2 pos,MeteorSize sz){
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
static void DrawMeteor(Meteor m){
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

/* --- Screen shake offset --- */
static Vector2 ShakeOff(void){
    if(G.shakeTimer<=0)return (Vector2){0,0};
    return (Vector2){Rf(-G.shakeMag,G.shakeMag),Rf(-G.shakeMag,G.shakeMag)};
}

/* --- UpdateGame --- */
static void UpdateGame(float dt){
    if(G.slowMoTimer>0){G.slowMoTimer-=GetFrameTime();dt*=0.3f;}
    Player*pl=&G.player;
    G.gameTime+=dt;
    /* difficulty ramp */
    G.meteorRate=Clampf(1.2f-G.gameTime*0.008f,0.3f,1.2f);
    G.meteorSpeedMul=1.0f+G.gameTime*0.005f;
    if(G.shakeTimer>0)G.shakeTimer-=dt;
    if(pl->damageFlash>0)pl->damageFlash-=dt;
    if(pl->shieldTimer>0)pl->shieldTimer-=dt;
    /* combo decay */
    if(G.comboTimer>0){G.comboTimer-=dt;if(G.comboTimer<=0)G.comboMultiplier=1.0f;}
    /* movement */
    if(pl->alive){
        Vector2 dir={0};
        if(IsKeyDown(KEY_W)||IsKeyDown(KEY_UP))dir.y-=1;
        if(IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN))dir.y+=1;
        if(IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT))dir.x-=1;
        if(IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT))dir.x+=1;
        float len=sqrtf(dir.x*dir.x+dir.y*dir.y);
        if(len>0){dir.x/=len;dir.y/=len; pl->vel.x+=(dir.x*pl->speed-pl->vel.x)*dt*8;pl->vel.y+=(dir.y*pl->speed-pl->vel.y)*dt*8;}
        else{pl->vel.x*=1.0f-dt*6;pl->vel.y*=1.0f-dt*6;}
        pl->pos.x+=pl->vel.x*dt; pl->pos.y+=pl->vel.y*dt;
        pl->pos.x=Clampf(pl->pos.x,24,SW-24); pl->pos.y=Clampf(pl->pos.y,28,SH-28);
        /* engine trail */
        if(fabsf(pl->vel.x)>20||fabsf(pl->vel.y)>20){
            Color ec=(pl->type==SHIP_INTERCEPTOR)?(Color){0,220,255,255}:(pl->type==SHIP_DESTROYER)?(Color){255,160,40,255}:(Color){255,50,20,255};
            SpawnP((Vector2){pl->pos.x+Rf(-4,4),pl->pos.y+22},ec,1,40,2.5f);
        }
        /* shooting */
        pl->fireCooldown-=dt;
        if(IsKeyDown(KEY_SPACE)&&pl->fireCooldown<=0){
            pl->fireCooldown=pl->fireRate;
            if(pl->type==SHIP_TITAN){
                for(int d=-1;d<=1;d++){for(int i=0;i<MAX_BULLETS;i++){if(!G.bullets[i].active){G.bullets[i]=(Bullet){{pl->pos.x+d*10,pl->pos.y-26},600,true};break;}}}
            }else if(pl->type==SHIP_DESTROYER){
                for(int d=-1;d<=1;d+=2){for(int i=0;i<MAX_BULLETS;i++){if(!G.bullets[i].active){G.bullets[i]=(Bullet){{pl->pos.x+d*12,pl->pos.y-20},550,true};break;}}}
            }else{
                for(int i=0;i<MAX_BULLETS;i++){if(!G.bullets[i].active){G.bullets[i]=(Bullet){{pl->pos.x,pl->pos.y-28},650,true};break;}}
            }
            SpawnP((Vector2){pl->pos.x,pl->pos.y-28},(Color){200,230,255,255},3,60,2);
        }
    }
    /* bullets */
    for(int i=0;i<MAX_BULLETS;i++){if(!G.bullets[i].active)continue;G.bullets[i].pos.y-=G.bullets[i].speed*dt;if(G.bullets[i].pos.y<-10)G.bullets[i].active=false;}
    /* meteor spawn */
    G.meteorTimer+=dt;
    if(G.meteorTimer>=G.meteorRate){G.meteorTimer-=G.meteorRate;SpawnMeteor(&G);}
    /* meteor update */
    for(int i=0;i<MAX_METEORS;i++){if(!G.meteors[i].active)continue;Meteor*m=&G.meteors[i];m->pos.x+=m->vel.x*dt;m->pos.y+=m->vel.y*dt;m->rotation+=m->rotSpeed*dt;if(m->pos.y>SH+m->radius+30)m->active=false;}
    /* bullet-meteor collision */
    for(int bi=0;bi<MAX_BULLETS;bi++){
        if(!G.bullets[bi].active)continue;
        for(int mi=0;mi<MAX_METEORS;mi++){
            if(!G.meteors[mi].active)continue;
            float dx=G.bullets[bi].pos.x-G.meteors[mi].pos.x,dy=G.bullets[bi].pos.y-G.meteors[mi].pos.y;
            if(dx*dx+dy*dy<G.meteors[mi].radius*G.meteors[mi].radius){
                G.bullets[bi].active=false;
                G.meteors[mi].hp--;
                SpawnP(G.bullets[bi].pos,(Color){255,200,100,255},6,120,2);
                if(G.meteors[mi].hp<=0){
                    G.meteors[mi].active=false;
                    SpawnP(G.meteors[mi].pos,G.meteors[mi].color,18,200,3.5f);
                    SpawnP(G.meteors[mi].pos,(Color){255,220,100,255},8,160,2);
                    int bonus=(G.meteors[mi].size==METEOR_LARGE)?30:(G.meteors[mi].size==METEOR_MEDIUM)?20:10;
                    G.score+=(int)(bonus*G.comboMultiplier);
                    G.comboTimer=2.0f; G.comboMultiplier=Clampf(G.comboMultiplier+0.25f,1,5);
                    G.meteorsDestroyed++;
                    if(G.meteors[mi].size==METEOR_LARGE){SpawnMeteorAt(G.meteors[mi].pos,METEOR_MEDIUM);SpawnMeteorAt(G.meteors[mi].pos,METEOR_MEDIUM);}
                    else if(G.meteors[mi].size==METEOR_MEDIUM){SpawnMeteorAt(G.meteors[mi].pos,METEOR_SMALL);SpawnMeteorAt(G.meteors[mi].pos,METEOR_SMALL);}
                    if(G.meteors[mi].size>=METEOR_LARGE){G.shakeTimer=0.25f;G.shakeMag=6;}
                }
                break;
            }
        }
    }
    /* meteor-player collision */
    if(pl->alive&&pl->shieldTimer<=0){
        for(int mi=0;mi<MAX_METEORS;mi++){
            if(!G.meteors[mi].active)continue;
            float dx=pl->pos.x-G.meteors[mi].pos.x,dy=pl->pos.y-G.meteors[mi].pos.y;
            if(dx*dx+dy*dy<(G.meteors[mi].radius+18)*(G.meteors[mi].radius+18)){
                G.meteors[mi].active=false;
                pl->hp--;pl->shieldTimer=1.0f;pl->damageFlash=0.3f;
                G.shakeTimer=0.3f;G.shakeMag=8;
                SpawnP(pl->pos,(Color){255,255,255,255},12,150,3);
                if(pl->hp<=0){
                    pl->alive=false;
                    SpawnP(pl->pos,WHITE,40,300,4);
                    SpawnP(pl->pos,(Color){255,100,30,255},30,250,3.5f);
                    SpawnP(pl->pos,SKYBLUE,20,200,3);
                    G.slowMoTimer=0.6f;G.shakeTimer=0.5f;G.shakeMag=12;
                    if(G.score>G.highscore){G.highscore=G.score;SaveHS(G.highscore);}
                    G.gameOver=true;G.screen=SCREEN_GAME_OVER;
                    G.goBtns[0]=MkBtn(SW/2-100,480,200,48,"PLAY AGAIN");
                    G.goBtns[1]=MkBtn(SW/2-100,540,200,48,"MAIN MENU");
                    G.goSel=0;
                }
                break;
            }
        }
    }
    /* survival score */
    if(pl->alive){G.scoreTimer+=dt;while(G.scoreTimer>=1){G.scoreTimer-=1;G.score++;}}
    UpdateParticles(dt);
}

/* --- DrawGame --- */
static void DrawHPBar(Player p){
    int bw=120,bh=10,x=10,y=SH-30;
    DrawRectangle(x,y,bw,bh,(Color){40,40,40,200});
    float frac=(float)p.hp/p.maxHp;
    Color hc=frac>0.5f?(Color){50,200,80,255}:frac>0.25f?(Color){255,200,40,255}:(Color){255,50,40,255};
    DrawRectangle(x,y,(int)(bw*frac),bh,hc);
    DrawRectangleLinesEx((Rectangle){x,y,bw,bh},1,WHITE);
    DrawText(TextFormat("HP %d/%d",p.hp,p.maxHp),x+bw+8,y-2,16,WHITE);
}
static void DrawGameplay(void){
    Vector2 off=ShakeOff();
    BeginDrawing();
    ClearBackground((Color){4,4,16,255});
    DrawNebula();
    Camera2D cam={0}; cam.offset=off; cam.target=(Vector2){0,0}; cam.rotation=0; cam.zoom=1;
    BeginMode2D(cam);
    DrawStars();
    DrawParticles();
    for(int i=0;i<MAX_METEORS;i++){if(G.meteors[i].active)DrawMeteor(G.meteors[i]);}
    for(int i=0;i<MAX_BULLETS;i++){
        if(!G.bullets[i].active)continue;
        Vector2 bp=G.bullets[i].pos;
        DrawRectangle((int)bp.x-2,(int)bp.y-7,4,14,(Color){180,230,255,255});
        DrawRectangle((int)bp.x-1,(int)bp.y-5,2,10,WHITE);
        DrawCircleV(bp,5,CAlpha((Color){150,200,255,255},60));
    }
    if(G.player.alive){
        if(G.player.damageFlash>0){DrawCircleV(G.player.pos,30,CAlpha(RED,(unsigned char)(G.player.damageFlash/0.3f*100)));}
        DrawShipShape(G.player.pos,G.player.type,G.gameTime*4,true);
        if(G.player.shieldTimer>0){
            unsigned char sa=(unsigned char)(G.player.shieldTimer/1.0f*120);
            DrawCircleLines((int)G.player.pos.x,(int)G.player.pos.y,28,CAlpha(SKYBLUE,sa));
            DrawCircleLines((int)G.player.pos.x,(int)G.player.pos.y,30,CAlpha(WHITE,(unsigned char)(sa/2)));
        }
    }
    EndMode2D();
    /* HUD */
    DrawText(TextFormat("SCORE: %d",G.score),10,10,26,WHITE);
    DrawText(TextFormat("HIGH: %d",G.highscore),10,40,20,LIGHTGRAY);
    if(G.comboMultiplier>1.0f){
        const char*ct=TextFormat("COMBO x%.1f",G.comboMultiplier);
        DrawText(ct,SW-MeasureText(ct,24)-14,10,24,GOLD);
    }
    DrawHPBar(G.player);
    EndDrawing();
}

/* --- Menu / Ship Select / Pause / Game Over screens --- */
static void DrawTitle(float t){
    const char*title="COSMIC OBLIVION";
    int tw=MeasureText(title,52);
    float y=100+sinf(t*1.5f)*8;
    float hue=fmodf(t*30,360);
    Color c1=ColorFromHSV(hue,0.7f,1.0f), c2=ColorFromHSV(hue+40,0.6f,1.0f);
    /* glow layers */
    DrawText(title,(int)(SW/2-tw/2+2),(int)(y+2),52,CAlpha(c1,40));
    DrawText(title,(int)(SW/2-tw/2-1),(int)(y-1),52,CAlpha(c2,60));
    DrawText(title,(int)(SW/2-tw/2),(int)(y),52,WHITE);
}

/* --- Main loop screens --- */
static void ScreenLogo(float dt){
    G.logoTimer+=dt;
    BeginDrawing();ClearBackground(BLACK);
    float a=G.logoTimer<1?G.logoTimer:(G.logoTimer<2?1:Clampf(3-G.logoTimer,0,1));
    const char*t="COSMIC OBLIVION";int tw=MeasureText(t,48);
    DrawText(t,(SW-tw)/2,SH/2-24,48,CAlpha(WHITE,(unsigned char)(a*255)));
    EndDrawing();
    if(G.logoTimer>3){G.screen=SCREEN_MAIN_MENU;G.menuBtns[0]=MkBtn(SW/2-110,320,220,50,"PLAY GAME");G.menuBtns[1]=MkBtn(SW/2-110,390,220,50,"SPACESHIPS");G.menuBtns[2]=MkBtn(SW/2-110,460,220,50,"EXIT");G.menuSel=0;}
}
static void ScreenMenu(float dt){
    UpdateStars(dt);UpdateParticles(dt);
    if(GetRandomValue(0,100)<8)SpawnP((Vector2){Rf(0,SW),Rf(0,SH)},CAlpha((Color){60,100,200,255},120),1,20,1.5f);
    /* kb nav */
    if(IsKeyPressed(KEY_DOWN)){G.menuSel=(G.menuSel+1)%3;}
    if(IsKeyPressed(KEY_UP)){G.menuSel=(G.menuSel+2)%3;}
    for(int i=0;i<3;i++){if(UpdateBtn(&G.menuBtns[i],dt))G.menuSel=i;}
    bool enter=IsKeyPressed(KEY_ENTER);
    for(int i=0;i<3;i++){if(G.menuBtns[i].hovered&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){G.menuSel=i;enter=true;}}
    if(enter){
        if(G.menuSel==0){G.screen=SCREEN_SHIP_SELECT;G.shipSel=G.selectedShip;}
        else if(G.menuSel==1){G.screen=SCREEN_SHIP_SELECT;G.shipSel=G.selectedShip;}
        else CloseWindow();
    }
    BeginDrawing();ClearBackground((Color){4,4,16,255});DrawNebula();DrawStars();DrawParticles();
    DrawTitle((float)GetTime());
    for(int i=0;i<3;i++)DrawBtn(G.menuBtns[i],i==G.menuSel);
    EndDrawing();
}
static void ScreenShipSelect(float dt){
    UpdateStars(dt);
    if(IsKeyPressed(KEY_LEFT)||IsKeyPressed(KEY_A))G.shipSel=(G.shipSel+2)%3;
    if(IsKeyPressed(KEY_RIGHT)||IsKeyPressed(KEY_D))G.shipSel=(G.shipSel+1)%3;
    if(IsKeyPressed(KEY_ENTER)){G.selectedShip=G.shipSel;InitGame();G.screen=SCREEN_GAMEPLAY;}
    if(IsKeyPressed(KEY_ESCAPE)){G.screen=SCREEN_MAIN_MENU;}
    BeginDrawing();ClearBackground((Color){4,4,16,255});DrawNebula();DrawStars();
    DrawText("SELECT YOUR SHIP",(SW-MeasureText("SELECT YOUR SHIP",36))/2,40,36,WHITE);
    const char*names[3]={"INTERCEPTOR","DESTROYER","TITAN"};
    const char*descs[3]={"Fast & Agile\nHP: 3  Fire: Fast","Balanced Fighter\nHP: 5  Fire: Med","Heavy Tank\nHP: 8  Fire: Slow"};
    const char*stats[3]={"SPD: *****  FIR: *****  HP: **","SPD: ***  FIR: ***  HP: ****","SPD: **  FIR: **  HP: ******"};
    float t=(float)GetTime();
    for(int i=0;i<3;i++){
        float cx=SW/2+(i-1)*280;
        bool sel=i==G.shipSel;
        Color bc=sel?(Color){40,80,180,200}:(Color){20,30,50,180};
        Rectangle card={cx-110,160,220,380};
        DrawRectangleRounded(card,0.1f,8,bc);
        if(sel)DrawRectangleRoundedLinesEx(card,0.1f,8,3,(Color){100,180,255,255});
        else DrawRectangleRoundedLinesEx(card,0.1f,8,1,(Color){60,80,120,200});
        DrawShipShape((Vector2){cx,300},(ShipType)i,t*3+i,true);
        int nw=MeasureText(names[i],22);
        DrawText(names[i],(int)(cx-nw/2),400,22,WHITE);
        /* stats as small text */
        DrawText(stats[i],(int)(cx-100),435,13,LIGHTGRAY);
        DrawText(descs[i],(int)(cx-90),460,14,(Color){180,200,220,255});
        if(sel){DrawText("[ SELECTED ]",(int)(cx-MeasureText("[ SELECTED ]",18)/2),520,18,GOLD);}
    }
    DrawText("< A/D or Arrows to browse  |  ENTER to confirm  |  ESC to go back >",(SW-MeasureText("< A/D or Arrows to browse  |  ENTER to confirm  |  ESC to go back >",16))/2,SH-40,16,(Color){140,160,180,255});
    EndDrawing();
}
static void ScreenPause(float dt){
    (void)dt;
    if(IsKeyPressed(KEY_DOWN))G.pauseSel=(G.pauseSel+1)%3;
    if(IsKeyPressed(KEY_UP))G.pauseSel=(G.pauseSel+2)%3;
    for(int i=0;i<3;i++)if(UpdateBtn(&G.pauseBtns[i],dt)&&G.pauseBtns[i].hovered)G.pauseSel=i;
    bool enter=IsKeyPressed(KEY_ENTER);
    for(int i=0;i<3;i++){if(G.pauseBtns[i].hovered&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){G.pauseSel=i;enter=true;}}
    if(enter||IsKeyPressed(KEY_ESCAPE)){
        if(G.pauseSel==0||IsKeyPressed(KEY_ESCAPE)){G.screen=SCREEN_GAMEPLAY;}
        else if(G.pauseSel==1){G.screen=SCREEN_MAIN_MENU;}
        else CloseWindow();
    }
    BeginDrawing();
    ClearBackground((Color){4,4,16,255});
    DrawNebula(); DrawStars();
    DrawParticles();
    for(int i=0;i<MAX_METEORS;i++){if(G.meteors[i].active)DrawMeteor(G.meteors[i]);}
    for(int i=0;i<MAX_BULLETS;i++){
        if(!G.bullets[i].active)continue;
        Vector2 bp=G.bullets[i].pos;
        DrawRectangle((int)bp.x-2,(int)bp.y-7,4,14,(Color){180,230,255,255});
    }
    if(G.player.alive)DrawShipShape(G.player.pos,G.player.type,G.gameTime*4,true);
    /* overlay */
    DrawRectangle(0,0,SW,SH,(Color){0,0,0,160});
    const char*pt="PAUSED";int pw=MeasureText(pt,50);DrawText(pt,(SW-pw)/2,180,50,WHITE);
    for(int i=0;i<3;i++)DrawBtn(G.pauseBtns[i],i==G.pauseSel);
    EndDrawing();
}
static void ScreenGameOver(float dt){
    UpdateStars(dt);UpdateParticles(dt);
    if(IsKeyPressed(KEY_DOWN))G.goSel=(G.goSel+1)%2;
    if(IsKeyPressed(KEY_UP))G.goSel=(G.goSel+1)%2;
    for(int i=0;i<2;i++)if(UpdateBtn(&G.goBtns[i],dt)&&G.goBtns[i].hovered)G.goSel=i;
    bool enter=IsKeyPressed(KEY_ENTER);
    for(int i=0;i<2;i++){if(G.goBtns[i].hovered&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){G.goSel=i;enter=true;}}
    if(enter){
        if(G.goSel==0){InitGame();G.screen=SCREEN_GAMEPLAY;}
        else{G.screen=SCREEN_MAIN_MENU;}
    }
    BeginDrawing();ClearBackground((Color){4,4,16,255});DrawNebula();DrawStars();DrawParticles();
    float t=(float)GetTime();
    Color gc=ColorFromHSV(fmodf(t*60,360),0.8f,1);
    const char*go="GAME OVER";int gw=MeasureText(go,56);
    DrawText(go,(SW-gw)/2+2,142,56,CAlpha(gc,60));
    DrawText(go,(SW-gw)/2,140,56,gc);
    DrawText(TextFormat("Score: %d",G.score),(SW-MeasureText(TextFormat("Score: %d",G.score),30))/2,240,30,WHITE);
    DrawText(TextFormat("High Score: %d",G.highscore),(SW-MeasureText(TextFormat("High Score: %d",G.highscore),24))/2,280,24,GOLD);
    DrawText(TextFormat("Meteors Destroyed: %d",G.meteorsDestroyed),(SW-MeasureText(TextFormat("Meteors Destroyed: %d",G.meteorsDestroyed),20))/2,320,20,LIGHTGRAY);
    for(int i=0;i<2;i++)DrawBtn(G.goBtns[i],i==G.goSel);
    EndDrawing();
}

/* --- Main --- */
int main(void){
    InitWindow(SW,SH,"Cosmic Oblivion");
    SetTargetFPS(60);
    memset(&G,0,sizeof(G));
    G.screen=SCREEN_LOGO; G.highscore=LoadHS();
    InitStars();
    while(!WindowShouldClose()){
        float dt=GetFrameTime();
        switch(G.screen){
            case SCREEN_LOGO: ScreenLogo(dt);break;
            case SCREEN_MAIN_MENU: ScreenMenu(dt);break;
            case SCREEN_SHIP_SELECT: ScreenShipSelect(dt);break;
            case SCREEN_GAMEPLAY:
                if(IsKeyPressed(KEY_ESCAPE)){
                    G.screen=SCREEN_PAUSE;G.pauseSel=0;
                    G.pauseBtns[0]=MkBtn(SW/2-100,300,200,48,"RESUME");
                    G.pauseBtns[1]=MkBtn(SW/2-100,360,200,48,"MAIN MENU");
                    G.pauseBtns[2]=MkBtn(SW/2-100,420,200,48,"EXIT");
                    break;
                }
                UpdateStars(dt);UpdateGame(dt);DrawGameplay();break;
            case SCREEN_PAUSE: ScreenPause(dt);break;
            case SCREEN_GAME_OVER: ScreenGameOver(dt);break;
        }
    }
    CloseWindow();
    return 0;
}
