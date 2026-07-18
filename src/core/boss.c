#include "common.h"
#include <stdlib.h>

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

    if (game->zombies[idx].type == ENEMY_RAT_KING) {
        game->bossDefeated = true;
        game->showBossLog = true;
        for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
        int r = game->zombies[idx].row;
        int c = game->zombies[idx].col;
        game->map.tiles[r][c] = 236;
        if (c + 1 < MAP_WIDTH) game->map.tiles[r][c + 1] = 237;
    }
    else if (game->zombies[idx].type == ENEMY_DOOM_SCROLLER) {
        game->doomScrollerDefeated = true;
        game->showDoomScrollerLog = true;
        for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
        SpawnAlgorithm(game);
        PlaySound(game->blastSound);
    }
    else if (game->zombies[idx].type == ENEMY_ALGORITHM) {
        game->algorithmDefeated = true;
        for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
        SpawnBrainrotGod(game);
        SpawnMemoryFragments(game);
        PlaySound(game->blastSound);
    }
    else if (game->zombies[idx].type == ENEMY_BRAINROT_GOD) {
        game->brainrotGodDefeated = true;
        for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
        GameHistory_SaveEntry(game->playerName, 3, true);
        game->state = STATE_WIN;
    }
}

bool IsZombieHit(const GameState* game, int i, Vector2 hitPos, float baseRadius) {
    if (!game->zombies[i].active) return false;
    Vector2 zCenter = { game->zombies[i].position.x + TILE_PX / 2.0f, game->zombies[i].position.y + TILE_PX / 2.0f };
    float hitRadius = baseRadius;
    if (game->zombies[i].type == ENEMY_RAT_KING || game->zombies[i].type == ENEMY_ALGORITHM) hitRadius += 20.0f;
    else if (game->zombies[i].type == ENEMY_DOOM_SCROLLER) hitRadius += 28.0f;
    else if (game->zombies[i].type == ENEMY_BRAINROT_GOD) hitRadius += 40.0f;

    float dx = hitPos.x - zCenter.x;
    float dy = hitPos.y - zCenter.y;
    return (dx * dx + dy * dy) < (hitRadius * hitRadius);
}

void DamageZombie(GameState* game, int idx, float damage) {
    if (!game->zombies[idx].active) return;
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
