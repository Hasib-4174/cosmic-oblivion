#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "raylib.h"
#include <stdbool.h>

struct GameState;

typedef struct
{
    int levelIndex; /* 0 to 29 (3 acts * 10 levels) */
    int actIndex;   /* 0 to 2 */
    
    /* Level specific constraints */
    float duration;      /* How long to survive (in seconds) */
    int targetScore;     /* Target score to reach */
    int targetEnemies;   /* Enemies to destroy */
    int targetMeteors;   /* Meteors to destroy */
    
    // Spawn params
    float meteorSpawnRate;
    float enemySpawnRate;
    
    /* Display — fixed-size arrays to avoid TextFormat dangling pointers */
    char title[48];
    char description[64];
    bool isBossLevel;

} LevelData;

typedef enum
{
    PORTRAIT_VANCE,    /* Commander Vance - Stern, tactical */
    PORTRAIT_KAEL,     /* Operator Kael - Technical, helpful */
    PORTRAIT_ENEMY,    /* Unknown Enemy - Sinister, distorted */
    PORTRAIT_NONE
} StoryPortrait;

typedef struct
{
    const char *name;
    const char *text;
    StoryPortrait portrait;
} StoryBeat;

typedef struct
{
    int index;
    const char *actName;
    int beatCount;
    StoryBeat beats[5];
    int bossBeatCount;
    StoryBeat bossBeats[5];
} ActData;

/* Global Campaign Functions */
void InitCampaign(void);
LevelData GetLevelData(int levelIdx);
ActData GetActData(int actIdx);

void LoadCampaignProgress(struct GameState *G);
void SaveCampaignProgress(const struct GameState *G);

/* Campaign Logic */
void UpdateCampaignLogic(struct GameState *G, float dt);
bool CheckLevelComplete(const struct GameState *G);
int GetMaxUnlockedLevel(const struct GameState *G);

#endif
