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

void OnEnemyDeath(GameState* game, int idx) {
    game->enemies.killedCount++;
    game->enemies.list[idx].active = false;
    for (int a = 0; a < MAX_ASH_EFFECTS; a++) {
        if (!game->enemies.ashEffects[a].active) {
            game->enemies.ashEffects[a].gridX = game->enemies.list[idx].col;
            game->enemies.ashEffects[a].gridY = game->enemies.list[idx].row;
            game->enemies.ashEffects[a].timer = 1.5f;
            game->enemies.ashEffects[a].active = true;
            break;
        }
    }

    BossId bid = GetBossId(game->enemies.list[idx].type);
    if (bid != (BossId)-1) {
        game->enemies.status[bid].defeated = true;
        if (bid == BOSS_RAT_KING || bid == BOSS_DOOM_SCROLLER) {
            game->enemies.status[bid].showLog = true;
        }
        for (int z = 0; z < MAX_ZOMBIES; z++) {
            game->enemies.list[z].active = false;
        }

        if (bid == BOSS_RAT_KING) {
            int r = game->enemies.list[idx].row;
            int c = game->enemies.list[idx].col;
            game->map.tiles[r][c] = 236;
            if (c + 1 < MAP_WIDTH) game->map.tiles[r][c + 1] = 237;
        }
        else if (bid == BOSS_DOOM_SCROLLER) {
            SpawnEnemy(game, ENEMY_ALGORITHM, 16, 20);
            PlaySound(game->audio.blast);
        }
        else if (bid == BOSS_ALGORITHM) {
            game->enemies.status[bid].showLog = true;
            SpawnEnemy(game, ENEMY_BRAINROT_GOD, 16, 20);
            SpawnMemoryFragments(game);
            PlaySound(game->audio.blast);
        }
        else if (bid == BOSS_BRAINROT_GOD) {
            GameHistory_SaveEntry(game->playerInfo.name, 3, false);
            currentLevel = 3;
            UnloadTexture(game->map.tileset);
            Tilemap_Load(&game->map, "finalmap.csv", "finalmap_packed.png");
            
            Player_Init(&game->player);
            game->player.hasWeapon = true;
            game->player.position.x = 2.0f * TILE_PX;
            game->player.position.y = 2.0f * TILE_PX;
            game->player.gridX = 2;
            game->player.gridY = 2;
            
            game->state = STATE_SURVIVAL;
            game->enemies.timer = 0.0f;
            game->enemies.spawnTimer = 0.0f;
            for (int z = 0; z < MAX_ZOMBIES; z++) game->enemies.list[z].active = false;
            for (int s = 0; s < 6; s++) SpawnZombie(game);
            
            game->levelInfo.startTextTimer = 4.0f;
            
            game->playerInfo.checkpointPosition = game->player.position;
            game->playerInfo.checkpointLevel = currentLevel;
            game->playerInfo.checkpointState = STATE_SURVIVAL;
            game->playerInfo.checkpointActive = true;
            PlaySound(game->audio.blast);
        }
    }
}

bool IsZombieHit(const GameState* game, int i, Vector2 hitPos, float size) {
    if (!game->enemies.list[i].active) return false;
    EnemyProperties props = GetEnemyProperties(game->enemies.list[i].type, currentLevel, game->menu.difficulty);
    float zSize = TILE_PX * props.scale;
    float offset = (props.scale - 1.0f) / 2.0f;
    Rectangle zRec = { game->enemies.list[i].position.x - TILE_PX * offset, game->enemies.list[i].position.y - TILE_PX * offset, zSize, zSize };
    
    Rectangle hitRec = { hitPos.x - size / 2.0f, hitPos.y - size / 2.0f, size, size };
    return CheckCollisionRecs(zRec, hitRec);
}

void DamageZombie(GameState* game, int idx, float damage) {
    if (!game->enemies.list[idx].active) return;
    if (game->enemies.list[idx].type == ENEMY_BRAINROT_GOD && damage != BRAINROT_GOD_PUZZLE_DAMAGE) {
        return;
    }
    game->enemies.list[idx].health -= damage;
    if (game->enemies.list[idx].health <= 0.0f) {
        game->enemies.list[idx].health = 0.0f;
        OnEnemyDeath(game, idx);
    }
}

EnemyProperties GetEnemyProperties(EnemyType type, int level, int difficulty) {
    EnemyProperties props = { SNAKE_BASE_HP, SNAKE_BASE_SPEED, 331, 1.0f, WHITE, 0.0f };
    switch (type) {
        case ENEMY_SNAKE:
            props.maxHealth = SNAKE_BASE_HP;
            props.moveSpeed = SNAKE_BASE_SPEED;
            props.baseTileId = (level == 1) ? 20 : 331;
            break;
        case ENEMY_SPIDER:
            props.maxHealth = SPIDER_BASE_HP;
            props.moveSpeed = SPIDER_BASE_SPEED;
            props.baseTileId = 23;
            break;
        case ENEMY_GHOST:
            props.maxHealth = GHOST_BASE_HP;
            props.moveSpeed = GHOST_BASE_SPEED;
            props.baseTileId = 25;
            break;
        case ENEMY_RAT_KING:
            props.maxHealth = RAT_KING_BASE_HP;
            props.moveSpeed = RAT_KING_BASE_SPEED;
            props.baseTileId = 23;
            props.scale = 2.5f;
            props.color = PURPLE;
            props.hitboxOffset = 20.0f;
            break;
        case ENEMY_DOOM_SCROLLER:
            props.maxHealth = DOOM_SCROLLER_BASE_HP;
            props.moveSpeed = DOOM_SCROLLER_BASE_SPEED;
            props.baseTileId = 9;
            props.scale = 3.0f;
            props.color = RED;
            props.hitboxOffset = 28.0f;
            break;
        case ENEMY_ALGORITHM:
            props.maxHealth = ALGORITHM_BASE_HP;
            props.moveSpeed = ALGORITHM_BASE_SPEED;
            props.baseTileId = 13;
            props.scale = 2.5f;
            props.color = VIOLET;
            props.hitboxOffset = 20.0f;
            break;
        case ENEMY_BRAINROT_GOD:
            props.maxHealth = BRAINROT_GOD_BASE_HP;
            props.moveSpeed = BRAINROT_GOD_BASE_SPEED;
            props.baseTileId = 11;
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
