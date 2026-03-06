/* Cosmic Oblivion - Arcade Space Shooter */
#include "raylib.h"
#include "constants.h"
#include "helpers.h"
#include "stars.h"
#include "particles.h"
#include "button.h"
#include "ship.h"
#include "meteor.h"
#include "game.h"
#include "ui.h"
#include <string.h>

GameState G;

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
