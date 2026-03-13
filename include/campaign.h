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

typedef struct
{
    int index;
    const char *loreText;
    const char *actName;
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
