#include "include/campaign.h"
#include "include/constants.h"
#include <stdio.h>
#include <string.h>

#define SAVE_FILE "campaign_save.dat"

/* Default levels */
static LevelData s_levels[30];
static ActData s_acts[3];

void InitCampaign(void)
{
    /* Initialize Acts */
    s_acts[0] = (ActData){0, "The invasion begins in the outer rim...", "Act I: The Fringe"};
    s_acts[1] = (ActData){1, "We push deeper into enemy territory...", "Act II: The Core Worlds"};
    s_acts[2] = (ActData){2, "The final stand against the Titan fleet...", "Act III: Oblivion"};

    /* Generate dummy data for 30 levels for now */
    for (int i = 0; i < 30; i++)
    {
        s_levels[i].levelIndex = i;
        s_levels[i].actIndex = i / 10;
        
        /* Base difficulty scales with level */
        float baseMeteorRate = 1.5f - (i * 0.03f); /* Gets faster (lower is faster) */
        float baseEnemyRate = 5.0f - (i * 0.1f);  /* Gets faster */
        if (baseMeteorRate < 0.3f) baseMeteorRate = 0.3f;
        if (baseEnemyRate < 1.0f) baseEnemyRate = 1.0f;
        
        s_levels[i].meteorSpawnRate = baseMeteorRate;
        s_levels[i].enemySpawnRate = baseEnemyRate;
        
        /* Basic escalation */
        if (i % 10 == 9)
        {
            /* Boss levels (9, 19, 29) */
            snprintf(s_levels[i].title, sizeof(s_levels[i].title), "Act %d Boss", s_levels[i].actIndex + 1);
            snprintf(s_levels[i].description, sizeof(s_levels[i].description), "Defeat the sector commander!");
            s_levels[i].targetEnemies = 10 + s_levels[i].actIndex * 5; /* waves before boss */
            s_levels[i].targetScore = 0;
            s_levels[i].targetMeteors = 0;
            s_levels[i].duration = 0;
            s_levels[i].isBossLevel = true;
            s_levels[i].enemySpawnRate *= 0.7f; /* Boss stage = lots of enemies */
            s_levels[i].meteorSpawnRate *= 1.5f; /* Less meteors */
        }
        else
        {
            s_levels[i].isBossLevel = false;
            snprintf(s_levels[i].title, sizeof(s_levels[i].title), "Sector %d-%d",
                     s_levels[i].actIndex + 1, (i % 10) + 1);
            
            /* Alternate between survival, score, and extermination */
            int type = i % 3;
            if (type == 0)
            {
                snprintf(s_levels[i].description, sizeof(s_levels[i].description), "Survive the onslaught!");
                s_levels[i].duration = 20.0f + i * 1.5f; /* Survive for X seconds */
                s_levels[i].targetScore = 0;
                s_levels[i].targetEnemies = 0;
                s_levels[i].targetMeteors = 0;
                s_levels[i].meteorSpawnRate *= 0.8f; /* More hazards */
            }
            else if (type == 1)
            {
                snprintf(s_levels[i].description, sizeof(s_levels[i].description), "Exterminate enemy forces!");
                s_levels[i].duration = 0;
                s_levels[i].targetScore = 0;
                s_levels[i].targetEnemies = 10 + i;
                s_levels[i].targetMeteors = 0;
                s_levels[i].enemySpawnRate *= 0.7f; /* Faster enemies */
            }
            else
            {
                snprintf(s_levels[i].description, sizeof(s_levels[i].description), "Clear the asteroid field!");
                s_levels[i].duration = 0;
                s_levels[i].targetScore = 0;
                s_levels[i].targetEnemies = 0;
                s_levels[i].targetMeteors = 15 + i * 2;
                s_levels[i].meteorSpawnRate *= 0.5f; /* Way more meteors */
                s_levels[i].enemySpawnRate *= 2.0f;  /* Rare enemies */
            }
        }
    }
}

LevelData GetLevelData(int levelIdx)
{
    if (levelIdx < 0) levelIdx = 0;
    if (levelIdx >= 30) levelIdx = 29;
    return s_levels[levelIdx];
}

ActData GetActData(int actIdx)
{
    if (actIdx < 0) actIdx = 0;
    if (actIdx >= 3) actIdx = 2;
    return s_acts[actIdx];
}

void LoadCampaignProgress(struct GameState *G)
{
    G->campaignState.unlockedLevels[DIFF_EASY] = 0;
    G->campaignState.unlockedLevels[DIFF_NORMAL] = 0;
    G->campaignState.unlockedLevels[DIFF_HARD] = 0;

    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return;
    
    fread(&G->campaignState.unlockedLevels[DIFF_EASY], sizeof(int), 1, f);
    fread(&G->campaignState.unlockedLevels[DIFF_NORMAL], sizeof(int), 1, f);
    fread(&G->campaignState.unlockedLevels[DIFF_HARD], sizeof(int), 1, f);
    
    fclose(f);
}

void SaveCampaignProgress(const struct GameState *G)
{
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return;
    
    fwrite(&G->campaignState.unlockedLevels[DIFF_EASY], sizeof(int), 1, f);
    fwrite(&G->campaignState.unlockedLevels[DIFF_NORMAL], sizeof(int), 1, f);
    fwrite(&G->campaignState.unlockedLevels[DIFF_HARD], sizeof(int), 1, f);
    
    fclose(f);
}

void UpdateCampaignLogic(struct GameState *G, float dt)
{
    if (!G->isCampaignMode) return;
    
    LevelData ldata = GetLevelData(G->campaignState.currentLevel);
    
    if (ldata.duration > 0)
    {
        G->campaignState.levelTimer += dt;
    }
}

bool CheckLevelComplete(const struct GameState *G)
{
    if (!G->isCampaignMode) return false;
    
    LevelData ldata = GetLevelData(G->campaignState.currentLevel);
    if (ldata.isBossLevel)
        return G->campaignState.bossDefeated;
        
    if (ldata.duration > 0 && G->campaignState.levelTimer >= ldata.duration)
        return true;
        
    if (ldata.targetScore > 0 && G->score >= ldata.targetScore)
        return true;
        
    if (ldata.targetEnemies > 0 && G->enemiesDestroyed >= ldata.targetEnemies)
        return true;
        
        
    if (ldata.targetMeteors > 0 && G->meteorsDestroyed >= ldata.targetMeteors)
        return true;
        
    return false;
}

int GetMaxUnlockedLevel(const struct GameState *G)
{
    return G->campaignState.unlockedLevels[DIFF_NORMAL];
}
