#include "raylib.h"
#include "common.h"
#include <stdlib.h> // For abs()
#include <math.h>   // For sqrtf()

extern int currentLevel;

// --- Zombie Spawning & Tracking Mechanics ---

void SpawnEnemy(GameState* game, EnemyType type, int r, int c) {
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    if (r == -1 || c == -1) {
        int playerCol = (int)(game->player.position.x / TILE_PX);
        int playerRow = (int)(game->player.position.y / TILE_PX);
        for (int attempt = 0; attempt < 100; attempt++) {
            int tr = GetRandomValue(2, MAP_HEIGHT - 3);
            int tc = GetRandomValue(2, MAP_WIDTH - 3);
            if (GetTileType(game->map.tiles[tr][tc]) == TILE_GROUND) {
                int dist = abs(tc - playerCol) + abs(tr - playerRow);
                if (type == ENEMY_SNAKE || type == ENEMY_SPIDER) {
                    if (dist >= 6) {
                        r = tr; c = tc;
                        break;
                    }
                } else {
                    if (dist >= 3) {
                        r = tr; c = tc;
                        break;
                    }
                }
            }
        }
        if (r == -1 || c == -1) {
            r = 10; c = 10;
        }
    }

    EnemyProperties props = GetEnemyProperties(type, currentLevel, game->difficulty);
    game->zombies[slot].row = r;
    game->zombies[slot].col = c;
    game->zombies[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
    game->zombies[slot].active = true;
    game->zombies[slot].health = props.maxHealth;
    game->zombies[slot].maxHealth = props.maxHealth;
    game->zombies[slot].type = type;
    
    if (type == ENEMY_SNAKE || type == ENEMY_SPIDER) {
        game->zombies[slot].shootTimer = (float)GetRandomValue(0, 100) / 100.0f;
    } else {
        game->zombies[slot].shootTimer = 0.0f;
        BossId bid = GetBossId(type);
        if (bid != (BossId)-1) {
            game->bosses[bid].spawned = true;
            game->bosses[bid].defeated = false;
            game->bosses[bid].showLog = false;
        }
    }

    if (type == ENEMY_GHOST) {
        if (GetRandomValue(0, 1) == 0) {
            game->zombies[slot].ghostVel.x = (GetRandomValue(0, 1) == 0) ? props.moveSpeed : -props.moveSpeed;
            game->zombies[slot].ghostVel.y = 0.0f;
        } else {
            game->zombies[slot].ghostVel.y = (GetRandomValue(0, 1) == 0) ? props.moveSpeed : -props.moveSpeed;
            game->zombies[slot].ghostVel.x = 0.0f;
        }
    } else {
        game->zombies[slot].ghostVel = (Vector2){ 0, 0 };
    }
}

void SpawnZombie(GameState* game) {
    EnemyType type = ENEMY_SNAKE;
    if (currentLevel == 1 || currentLevel == 2) {
        int roll = GetRandomValue(0, 2);
        type = (roll == 0) ? ENEMY_SNAKE : (roll == 1) ? ENEMY_SPIDER : ENEMY_GHOST;
    }
    SpawnEnemy(game, type, -1, -1);
}

static void ShootSpiderProjectile(GameState* game, Vector2 startPos, Vector2 targetPos, bool isBig) {
    int projSlot = -1;
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!game->enemyProjectiles[i].active) {
            projSlot = i;
            break;
        }
    }
    if (projSlot == -1) return;

    float dx = targetPos.x - startPos.x;
    float dy = targetPos.y - startPos.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) len = 1.0f;

    float speed = isBig ? 150.0f : 180.0f;
    game->enemyProjectiles[projSlot].position = startPos;
    game->enemyProjectiles[projSlot].velocity = (Vector2){ (dx / len) * speed, (dy / len) * speed };
    game->enemyProjectiles[projSlot].active = true;
    game->enemyProjectiles[projSlot].isBig = isBig;
    game->enemyProjectiles[projSlot].lifeTimer = 2.5f;
}

static bool Zombie_CanMoveTo(const Tilemap* map, float tx, float ty) {
    float offset = 4.0f;
    float boxSize = TILE_PX - 2.0f * offset;
    float corners[4][2] = {
        { tx + offset, ty + offset },
        { tx + offset + boxSize, ty + offset },
        { tx + offset, ty + offset + boxSize },
        { tx + offset + boxSize, ty + offset + boxSize }
    };
    for (int i = 0; i < 4; i++) {
        int c = (int)(corners[i][0] / TILE_PX);
        int r = (int)(corners[i][1] / TILE_PX);
        if (r < 0 || r >= MAP_HEIGHT || c < 0 || c >= MAP_WIDTH) return false;
        int tileID = map->tiles[r][c];
        int type = GetTileType(tileID);
        if (type == TILE_WALL || type == TILE_CAR) return false;
    }
    return true;
}

void UpdateZombies(GameState* game, float dt) {
    // 1. Ranged Enemy & Boss Firing Logic (Single Unified Attack Type)
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) continue;

        if (game->zombies[i].type == ENEMY_SPIDER ||
            game->zombies[i].type == ENEMY_RAT_KING ||
            game->zombies[i].type == ENEMY_DOOM_SCROLLER ||
            game->zombies[i].type == ENEMY_BRAINROT_GOD) {
            game->zombies[i].shootTimer += dt;
            float interval = (game->zombies[i].type == ENEMY_SPIDER) ? 1.8f : 1.5f;
            if (game->zombies[i].shootTimer >= interval) {
                game->zombies[i].shootTimer = 0.0f;
                Vector2 startPos = {
                    game->zombies[i].position.x + TILE_PX / 2.0f,
                    game->zombies[i].position.y + TILE_PX / 2.0f
                };
                Vector2 targetPos = {
                    game->player.position.x + TILE_PX / 2.0f,
                    game->player.position.y + TILE_PX / 2.0f
                };
                bool isBig = (game->zombies[i].type == ENEMY_RAT_KING || game->zombies[i].type == ENEMY_BRAINROT_GOD);
                ShootSpiderProjectile(game, startPos, targetPos, isBig);
            }
        }
    }

    // 2. Projectile Updates & Damage Logic
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!game->enemyProjectiles[i].active) continue;

        game->enemyProjectiles[i].lifeTimer -= dt;
        if (game->enemyProjectiles[i].lifeTimer <= 0.0f) {
            game->enemyProjectiles[i].active = false;
            continue;
        }

        game->enemyProjectiles[i].position.x += game->enemyProjectiles[i].velocity.x * dt;
        game->enemyProjectiles[i].position.y += game->enemyProjectiles[i].velocity.y * dt;

        Vector2 pos = game->enemyProjectiles[i].position;
        if (pos.x < 0 || pos.x > MAP_WIDTH * TILE_PX || pos.y < 0 || pos.y > MAP_HEIGHT * TILE_PX) {
            game->enemyProjectiles[i].active = false;
            continue;
        }

        int col = (int)(pos.x / TILE_PX);
        int row = (int)(pos.y / TILE_PX);
        if (row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH) {
            int tileType = GetTileType(game->map.tiles[row][col]);
            if (tileType == TILE_WALL || tileType == TILE_CAR) {
                game->enemyProjectiles[i].active = false;
                continue;
            }
        }

        Vector2 pCenter = { game->player.position.x + TILE_PX / 2.0f, game->player.position.y + TILE_PX / 2.0f };
        float pDx = pos.x - pCenter.x;
        float pDy = pos.y - pCenter.y;
        float collisionRadius = game->enemyProjectiles[i].isBig ? 22.0f : 14.0f;
        float damageDealt = game->enemyProjectiles[i].isBig ? 25.0f : 12.0f;

        if (sqrtf(pDx * pDx + pDy * pDy) < collisionRadius) {
            game->enemyProjectiles[i].active = false;
            game->player.health -= damageDealt;
            if (game->hitSoundTimer <= 0.0f) {
                PlaySound(game->hitSound);
                game->hitSoundTimer = 0.25f;
            }
        }
    }

    // 3. Smooth Movement Updates
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) continue;

        if (game->zombies[i].type == ENEMY_GHOST) {
            game->zombies[i].position.x += game->zombies[i].ghostVel.x * dt;
            game->zombies[i].position.y += game->zombies[i].ghostVel.y * dt;

            // Bounce check at screen boundaries
            if (game->zombies[i].position.x < 0) {
                game->zombies[i].position.x = 0;
                game->zombies[i].ghostVel.x = -game->zombies[i].ghostVel.x;
            }
            else if (game->zombies[i].position.x > (MAP_WIDTH - 1) * TILE_PX) {
                game->zombies[i].position.x = (MAP_WIDTH - 1) * TILE_PX;
                game->zombies[i].ghostVel.x = -game->zombies[i].ghostVel.x;
            }

            if (game->zombies[i].position.y < 0) {
                game->zombies[i].position.y = 0;
                game->zombies[i].ghostVel.y = -game->zombies[i].ghostVel.y;
            }
            else if (game->zombies[i].position.y > (MAP_HEIGHT - 1) * TILE_PX) {
                game->zombies[i].position.y = (MAP_HEIGHT - 1) * TILE_PX;
                game->zombies[i].ghostVel.y = -game->zombies[i].ghostVel.y;
            }

            // Sync grid row/col
            game->zombies[i].row = (int)((game->zombies[i].position.y + TILE_PX / 2.0f) / TILE_PX);
            game->zombies[i].col = (int)((game->zombies[i].position.x + TILE_PX / 2.0f) / TILE_PX);
            continue;
        }



        float dx = game->player.position.x - game->zombies[i].position.x;
        float dy = game->player.position.y - game->zombies[i].position.y;
        float len = sqrtf(dx * dx + dy * dy);

        if (len > 2.0f) {
            EnemyProperties props = GetEnemyProperties(game->zombies[i].type, currentLevel, game->difficulty);
            float moveSpeed = props.moveSpeed;

            if (moveSpeed > 0.0f) {
                float stepX = (dx / len) * moveSpeed * dt;
                float stepY = (dy / len) * moveSpeed * dt;

                float nextX = game->zombies[i].position.x + stepX;
                float nextY = game->zombies[i].position.y + stepY;

                if (Zombie_CanMoveTo(&game->map, nextX, nextY)) {
                    game->zombies[i].position.x = nextX;
                    game->zombies[i].position.y = nextY;
                } else {
                    if (Zombie_CanMoveTo(&game->map, nextX, game->zombies[i].position.y)) {
                        game->zombies[i].position.x = nextX;
                    }
                    if (Zombie_CanMoveTo(&game->map, game->zombies[i].position.x, nextY)) {
                        game->zombies[i].position.y = nextY;
                    }
                }
            }

            // Sync grid cell indices
            game->zombies[i].row = (int)((game->zombies[i].position.y + TILE_PX / 2.0f) / TILE_PX);
            game->zombies[i].col = (int)((game->zombies[i].position.x + TILE_PX / 2.0f) / TILE_PX);
        }
    }
}

void SpawnMemoryFragments(GameState* game) {
    const char* names[3] = { "CURIOSITY", "KNOWLEDGE", "TRUTH" };
    for (int i = 0; i < 3; i++) {
        game->fragments[i].activated = false;
        game->fragments[i].name = names[i];
    }
    
    Vector2 spots[3];
    int spotCount = 0;
    for (int attempt = 0; attempt < 200 && spotCount < 3; attempt++) {
        int r = GetRandomValue(2, MAP_HEIGHT - 3);
        int c = GetRandomValue(2, MAP_WIDTH - 3);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            bool tooClose = false;
            for (int s = 0; s < spotCount; s++) {
                if (abs((int)(spots[s].x / TILE_PX) - c) + abs((int)(spots[s].y / TILE_PX) - r) < 6) {
                    tooClose = true;
                }
            }
            if (!tooClose) {
                spots[spotCount] = (Vector2){ c * TILE_PX, r * TILE_PX };
                spotCount++;
            }
        }
    }
    
    if (spotCount < 3) {
        spots[0] = (Vector2){ 5 * TILE_PX, 5 * TILE_PX };
        spots[1] = (Vector2){ 15 * TILE_PX, 15 * TILE_PX };
        spots[2] = (Vector2){ 25 * TILE_PX, 8 * TILE_PX };
    }
    
    game->fragments[0].position = spots[0];
    game->fragments[1].position = spots[1];
    game->fragments[2].position = spots[2];
}
