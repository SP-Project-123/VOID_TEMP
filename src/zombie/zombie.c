#include "raylib.h"
#include "common.h"
#include <stdlib.h> // For abs()
#include <math.h>   // For sqrtf()

extern int currentLevel;

// --- Zombie Spawning & Tracking Mechanics ---

void SpawnZombie(GameState* game) {
    // Find an inactive slot
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return; // Fixed enemy array is full

    int playerCol = (int)(game->player.position.x / TILE_PX);
    int playerRow = (int)(game->player.position.y / TILE_PX);

    // Pick a random row/col and verify it is a TILE_GROUND at a Manhattan distance >= 6
    for (int attempt = 0; attempt < 100; attempt++) {
        int r = GetRandomValue(0, MAP_HEIGHT - 1);
        int c = GetRandomValue(0, MAP_WIDTH - 1);

        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            int dist = abs(c - playerCol) + abs(r - playerRow);
            if (dist >= 6) {
                game->zombies[slot].row = r;
                game->zombies[slot].col = c;
                game->zombies[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
                game->zombies[slot].active = true;
                game->zombies[slot].health = 30.0f;
                game->zombies[slot].maxHealth = 30.0f;
                game->zombies[slot].shootTimer = (float)GetRandomValue(0, 100) / 100.0f;
                
                // Map 2: assign Snake (tile 20) or Fireshooter Spider (tile 23)
                if (currentLevel == 1) {
                    game->zombies[slot].type = (GetRandomValue(0, 1) == 0) ? ENEMY_SNAKE : ENEMY_SPIDER;
                } else {
                    game->zombies[slot].type = ENEMY_SNAKE;
                }
                break;
            }
        }
    }
}

void SpawnRatKing(GameState* game, int r, int c) {
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    game->zombies[slot].row = r;
    game->zombies[slot].col = c;
    game->zombies[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
    game->zombies[slot].active = true;
    game->zombies[slot].health = 300.0f;
    game->zombies[slot].maxHealth = 300.0f;
    game->zombies[slot].type = ENEMY_RAT_KING;
    game->zombies[slot].shootTimer = 0.0f;
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
    // 1. Spider & Rat King Firing Logic
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) continue;

        if (game->zombies[i].type == ENEMY_SPIDER || game->zombies[i].type == ENEMY_RAT_KING) {
            game->zombies[i].shootTimer += dt;
            float interval = (game->zombies[i].type == ENEMY_RAT_KING) ? 1.5f : 1.8f;
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
                ShootSpiderProjectile(game, startPos, targetPos, (game->zombies[i].type == ENEMY_RAT_KING));
            }
        }
    }

    // 2. Projectile Updates & Damage Logic
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!game->enemyProjectiles[i].active) continue;

        game->enemyProjectiles[i].position.x += game->enemyProjectiles[i].velocity.x * dt;
        game->enemyProjectiles[i].position.y += game->enemyProjectiles[i].velocity.y * dt;

        Vector2 pos = game->enemyProjectiles[i].position;
        if (pos.x < 0 || pos.x > MAP_WIDTH * TILE_PX || pos.y < 0 || pos.y > MAP_HEIGHT * TILE_PX) {
            game->enemyProjectiles[i].active = false;
            continue;
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

        // Custom Boss attacks cycle
        if (game->zombies[i].type == ENEMY_BRAINROT_GOD) {
            game->zombies[i].shootTimer += dt;
            float t = game->zombies[i].shootTimer;
            
            // 1. Skibidi Blast (Rapid fireballs at 1.0s, 1.2s, 1.4s)
            if ((t >= 1.0f && t < 1.05f) || (t >= 1.2f && t < 1.25f) || (t >= 1.4f && t < 1.45f)) {
                static float lastBlastTime = 0.0f;
                if (GetTime() - lastBlastTime > 0.15f) {
                    lastBlastTime = GetTime();
                    Vector2 startPos = { game->zombies[i].position.x + TILE_PX/2.0f, game->zombies[i].position.y + TILE_PX/2.0f };
                    Vector2 targetPos = { game->player.position.x + TILE_PX/2.0f, game->player.position.y + TILE_PX/2.0f };
                    ShootSpiderProjectile(game, startPos, targetPos, true);
                }
            }
            
            // 2. Ohio Storm (Falling lightning at 2.5s)
            if (t >= 2.5f && t < 2.55f) {
                static float lastStormTime = 0.0f;
                if (GetTime() - lastStormTime > 0.5f) {
                    lastStormTime = GetTime();
                    for (int pNum = 0; pNum < 3; pNum++) {
                        float offsetX = (pNum - 1) * 32.0f;
                        Vector2 startPos = { game->player.position.x + TILE_PX/2.0f + offsetX, game->player.position.y - 120.0f };
                        Vector2 targetPos = { game->player.position.x + TILE_PX/2.0f + offsetX, game->player.position.y + TILE_PX/2.0f };
                        ShootSpiderProjectile(game, startPos, targetPos, true);
                    }
                }
            }
            
            // 3. Sigma Shockwave (Expanding energy circle at 4.0s)
            if (t >= 4.0f && t < 4.05f) {
                static float lastWaveTime = 0.0f;
                if (GetTime() - lastWaveTime > 0.5f) {
                    lastWaveTime = GetTime();
                    Vector2 zCenter = { game->zombies[i].position.x + TILE_PX/2.0f, game->zombies[i].position.y + TILE_PX/2.0f };
                    Vector2 pCenter = { game->player.position.x + TILE_PX/2.0f, game->player.position.y + TILE_PX/2.0f };
                    float dist = sqrtf((zCenter.x - pCenter.x)*(zCenter.x - pCenter.x) + (zCenter.y - pCenter.y)*(zCenter.y - pCenter.y));
                    if (dist < 120.0f) {
                        game->player.health -= 30.0f;
                        PlaySound(game->hitSound);
                    }
                }
            }
            
            if (t >= 5.0f) {
                game->zombies[i].shootTimer = 0.0f;
            }
        }
        else if (game->zombies[i].type == ENEMY_DOOM_SCROLLER) {
            game->zombies[i].shootTimer += dt;
            if (game->zombies[i].shootTimer >= 1.5f) {
                game->zombies[i].shootTimer = 0.0f;
                Vector2 startPos = {
                    game->zombies[i].position.x + TILE_PX / 2.0f,
                    game->zombies[i].position.y + TILE_PX / 2.0f
                };
                Vector2 dirs[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
                for (int d = 0; d < 4; d++) {
                    int projSlot = -1;
                    for (int p = 0; p < MAX_ENEMY_PROJECTILES; p++) {
                        if (!game->enemyProjectiles[p].active) {
                            projSlot = p;
                            break;
                        }
                    }
                    if (projSlot != -1) {
                        float speed = 160.0f;
                        game->enemyProjectiles[projSlot].position = startPos;
                        game->enemyProjectiles[projSlot].velocity = (Vector2){ dirs[d].x * speed, dirs[d].y * speed };
                        game->enemyProjectiles[projSlot].active = true;
                        game->enemyProjectiles[projSlot].isBig = false;
                    }
                }
            }
        }

        float dx = game->player.position.x - game->zombies[i].position.x;
        float dy = game->player.position.y - game->zombies[i].position.y;
        float len = sqrtf(dx * dx + dy * dy);

        if (len > 2.0f) {
            float moveSpeed = 55.0f;
            if (game->zombies[i].type == ENEMY_RAT_KING) moveSpeed = 40.0f;
            else if (game->zombies[i].type == ENEMY_DOOM_SCROLLER) moveSpeed = 0.0f;
            else if (game->zombies[i].type == ENEMY_ALGORITHM) moveSpeed = 45.0f;
            else if (game->zombies[i].type == ENEMY_BRAINROT_GOD) moveSpeed = 50.0f;

            if (moveSpeed > 0.0f) {
                float stepX = (dx / len) * moveSpeed * dt;
                float stepY = (dy / len) * moveSpeed * dt;

                float nextX = game->zombies[i].position.x + stepX;
                float nextY = game->zombies[i].position.y + stepY;

                if (Zombie_CanMoveTo(&game->map, nextX, game->zombies[i].position.y)) {
                    game->zombies[i].position.x = nextX;
                }
                if (Zombie_CanMoveTo(&game->map, game->zombies[i].position.x, nextY)) {
                    game->zombies[i].position.y = nextY;
                }
            }

            // Sync grid cell indices
            game->zombies[i].row = (int)((game->zombies[i].position.y + TILE_PX / 2.0f) / TILE_PX);
            game->zombies[i].col = (int)((game->zombies[i].position.x + TILE_PX / 2.0f) / TILE_PX);
        }
    }
}

void SpawnDoomScroller(GameState* game) {
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    for (int attempt = 0; attempt < 100; attempt++) {
        int r = GetRandomValue(2, MAP_HEIGHT - 3);
        int c = GetRandomValue(2, MAP_WIDTH - 3);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            game->zombies[slot].row = r;
            game->zombies[slot].col = c;
            game->zombies[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
            game->zombies[slot].active = true;
            game->zombies[slot].health = 400.0f;
            game->zombies[slot].maxHealth = 400.0f;
            game->zombies[slot].type = ENEMY_DOOM_SCROLLER;
            game->zombies[slot].shootTimer = 0.0f;
            game->doomScrollerSpawned = true;
            break;
        }
    }
}

void SpawnAlgorithm(GameState* game) {
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    for (int attempt = 0; attempt < 100; attempt++) {
        int r = GetRandomValue(2, MAP_HEIGHT - 3);
        int c = GetRandomValue(2, MAP_WIDTH - 3);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            game->zombies[slot].row = r;
            game->zombies[slot].col = c;
            game->zombies[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
            game->zombies[slot].active = true;
            game->zombies[slot].health = 200.0f;
            game->zombies[slot].maxHealth = 200.0f;
            game->zombies[slot].type = ENEMY_ALGORITHM;
            game->zombies[slot].shootTimer = 0.0f;
            game->algorithmSpawned = true;
            break;
        }
    }
}

void SpawnBrainrotGod(GameState* game) {
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->zombies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    for (int attempt = 0; attempt < 100; attempt++) {
        int r = GetRandomValue(2, MAP_HEIGHT - 3);
        int c = GetRandomValue(2, MAP_WIDTH - 3);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            game->zombies[slot].row = r;
            game->zombies[slot].col = c;
            game->zombies[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
            game->zombies[slot].active = true;
            game->zombies[slot].health = 600.0f;
            game->zombies[slot].maxHealth = 600.0f;
            game->zombies[slot].type = ENEMY_BRAINROT_GOD;
            game->zombies[slot].shootTimer = 0.0f;
            game->brainrotGodSpawned = true;
            break;
        }
    }
}

void SpawnMemoryFragments(GameState* game) {
    game->curiosityActivated = false;
    game->knowledgeActivated = false;
    game->truthActivated = false;
    
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
    
    game->curiosityPos = spots[0];
    game->knowledgePos = spots[1];
    game->truthPos = spots[2];
}
