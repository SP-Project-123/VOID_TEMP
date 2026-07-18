#include "common.h"
#include <stdlib.h>

BossId GetBossId(EnemyType type) {
    switch (type) {
        case ENEMY_RAT_KING:      return BOSS_RAT_KING;
        case ENEMY_DOOM_SCROLLER: return BOSS_DOOM_SCROLLER;
        case ENEMY_ALGORITHM:     return BOSS_ALGORITHM;
        case ENEMY_BRAINROT_GOD:  return BOSS_BRAINROT_GOD;
        default:                  return (BossId)-1;
    }
}

void OnZombieDeath(GameState* game, int idx) {
    game->zombies[idx].active = false;
    for (int a = 0; a < MAX_ASH_EFFECTS; a++) {
        if (!game->ashEffects[a].active) {
            game->ashEffects[a].gridX = game->zombies[idx].col;
            game->ashEffects[a].gridY = game->zombies[idx].row;
            game->ashEffects[a].timer = 1.5f;
            game->ashEffects[a].active = true;
            break;
        }
    }

    BossId bid = GetBossId(game->zombies[idx].type);
    if (bid != (BossId)-1) {
        game->bosses[bid].defeated = true;
        if (bid == BOSS_RAT_KING || bid == BOSS_DOOM_SCROLLER) {
            game->bosses[bid].showLog = true;
        }
        for (int z = 0; z < MAX_ZOMBIES; z++) {
            game->zombies[z].active = false;
        }

        if (bid == BOSS_RAT_KING) {
            int r = game->zombies[idx].row;
            int c = game->zombies[idx].col;
            game->map.tiles[r][c] = 236;
            if (c + 1 < MAP_WIDTH) game->map.tiles[r][c + 1] = 237;
        }
        else if (bid == BOSS_DOOM_SCROLLER) {
            SpawnEnemy(game, ENEMY_ALGORITHM, -1, -1);
            PlaySound(game->blastSound);
        }
        else if (bid == BOSS_ALGORITHM) {
            game->bosses[bid].showLog = true;
            SpawnEnemy(game, ENEMY_BRAINROT_GOD, -1, -1);
            SpawnMemoryFragments(game);
            PlaySound(game->blastSound);
        }
        else if (bid == BOSS_BRAINROT_GOD) {
            GameHistory_SaveEntry(game->playerName, 3, false);
            currentLevel = 3;
            UnloadTexture(game->map.tileset);
            Tilemap_Load(&game->map, "finalmap.csv", "finalmap_packed.png");
            
            Player_Init(&game->player);
            game->player.position.x = 2.0f * TILE_PX;
            game->player.position.y = 2.0f * TILE_PX;
            game->player.gridX = 2;
            game->player.gridY = 2;
            
            game->state = STATE_SURVIVAL;
            game->zombieTimer = 0.0f;
            game->zombieSpawnTimer = 0.0f;
            for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
            game->startTextTimer = 4.0f;
            
            game->checkpointPosition = game->player.position;
            game->checkpointLevel = currentLevel;
            game->checkpointState = STATE_SURVIVAL;
            game->checkpointActive = true;
            PlaySound(game->blastSound);
        }
    }
}

bool IsZombieHit(const GameState* game, int i, Vector2 hitPos, float size) {
    if (!game->zombies[i].active) return false;
    EnemyProperties props = GetEnemyProperties(game->zombies[i].type, currentLevel, game->difficulty);
    float zSize = TILE_PX * props.scale;
    float offset = (props.scale - 1.0f) / 2.0f;
    Rectangle zRec = { game->zombies[i].position.x - TILE_PX * offset, game->zombies[i].position.y - TILE_PX * offset, zSize, zSize };
    
    Rectangle hitRec = { hitPos.x - size / 2.0f, hitPos.y - size / 2.0f, size, size };
    return CheckCollisionRecs(zRec, hitRec);
}

void DamageZombie(GameState* game, int idx, float damage) {
    if (!game->zombies[idx].active) return;
    if (game->zombies[idx].type == ENEMY_BRAINROT_GOD && damage != 200.0f) {
        return;
    }
    game->zombies[idx].health -= damage;
    if (game->zombies[idx].health <= 0.0f) {
        game->zombies[idx].health = 0.0f;
        OnZombieDeath(game, idx);
    }
}

void SpawnGun(GameState* game) {
    game->gunSpawned = false;
    for (int attempt = 0; attempt < 100; attempt++) {
        int r = GetRandomValue(1, MAP_HEIGHT - 2);
        int c = GetRandomValue(1, MAP_WIDTH - 2);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            int pCol = (int)(game->player.position.x / TILE_PX);
            int pRow = (int)(game->player.position.y / TILE_PX);
            if (abs(r - pRow) + abs(c - pCol) >= 3) {
                game->gunRow = r;
                game->gunCol = c;
                game->gunSpawned = true;
                break;
            }
        }
    }
}

EnemyProperties GetEnemyProperties(EnemyType type, int level, int difficulty) {
    EnemyProperties props = { 30.0f, 55.0f, 331, 1.0f, WHITE, 0.0f };
    switch (type) {
        case ENEMY_SNAKE:
            props.maxHealth = 30.0f;
            props.moveSpeed = 55.0f;
            props.baseTileId = (level == 1) ? 20 : 331;
            break;
        case ENEMY_SPIDER:
            props.maxHealth = 30.0f;
            props.moveSpeed = 55.0f;
            props.baseTileId = 23;
            break;
        case ENEMY_GHOST:
            props.maxHealth = 40.0f;
            props.moveSpeed = 65.0f;
            props.baseTileId = 25;
            break;
        case ENEMY_RAT_KING:
            props.maxHealth = 300.0f;
            props.moveSpeed = 40.0f;
            props.baseTileId = 23;
            props.scale = 2.5f;
            props.color = PURPLE;
            props.hitboxOffset = 20.0f;
            break;
        case ENEMY_DOOM_SCROLLER:
            props.maxHealth = 400.0f;
            props.moveSpeed = 0.0f;
            props.baseTileId = 306;
            props.scale = 3.0f;
            props.color = RED;
            props.hitboxOffset = 28.0f;
            break;
        case ENEMY_ALGORITHM:
            props.maxHealth = 200.0f;
            props.moveSpeed = 45.0f;
            props.baseTileId = 308;
            props.scale = 2.5f;
            props.color = VIOLET;
            props.hitboxOffset = 20.0f;
            break;
        case ENEMY_BRAINROT_GOD:
            props.maxHealth = 600.0f;
            props.moveSpeed = 50.0f;
            props.baseTileId = 27;
            props.scale = 4.0f;
            props.color = WHITE;
            props.hitboxOffset = 40.0f;
            break;
    }

    float diffSpeedMult = (difficulty == 0) ? 0.8f : (difficulty == 1) ? 1.0f : 1.25f;
    float diffHpMult = (difficulty == 0) ? 0.75f : (difficulty == 1) ? 1.0f : 1.35f;

    float levelSpeedMult = 1.0f + (level * 0.2f);
    float levelHpMult = 1.0f + (level * 0.2f);

    if (type == ENEMY_SNAKE || type == ENEMY_SPIDER || type == ENEMY_GHOST) {
        props.moveSpeed *= (diffSpeedMult * levelSpeedMult);
        props.maxHealth *= (diffHpMult * levelHpMult);
    } else {
        float bossSpeedMult = (difficulty == 0) ? 0.8f : (difficulty == 1) ? 1.1f : 1.3f;
        float bossHpMult = (difficulty == 0) ? 0.8f : (difficulty == 1) ? 1.1f : 1.5f;
        props.moveSpeed *= bossSpeedMult;
        props.maxHealth *= bossHpMult;
    }

    return props;
}
