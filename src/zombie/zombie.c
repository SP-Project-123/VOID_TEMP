#include "raylib.h"
#include "common.h"
#include <stdlib.h> // For abs()
#include <math.h>   // For sqrtf()

extern int currentLevel;

// --- Zombie Spawning & Tracking Mechanics ---

void SpawnEnemy(GameState* game, EnemyType type, int r, int c) {
    int slot = -1;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->enemies.list[i].active) {
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

    EnemyProperties props = GetEnemyProperties(type, currentLevel, game->menu.difficulty);
    game->enemies.list[slot].row = r;
    game->enemies.list[slot].col = c;
    game->enemies.list[slot].position = (Vector2){ (float)c * TILE_PX, (float)r * TILE_PX };
    game->enemies.list[slot].active = true;
    game->enemies.list[slot].health = props.maxHealth;
    game->enemies.list[slot].maxHealth = props.maxHealth;
    game->enemies.list[slot].type = type;
    
    if (type == ENEMY_SNAKE || type == ENEMY_SPIDER) {
        game->enemies.list[slot].shootTimer = (float)GetRandomValue(0, 100) / 100.0f;
    } else {
        game->enemies.list[slot].shootTimer = 0.0f;
        BossId bid = GetBossId(type);
        if (bid != (BossId)-1) {
            game->enemies.status[bid].spawned = true;
            game->enemies.status[bid].defeated = false;
            game->enemies.status[bid].showLog = false;
        }
    }

    if (type == ENEMY_GHOST) {
        if (GetRandomValue(0, 1) == 0) {
            game->enemies.list[slot].ghostVel.x = (GetRandomValue(0, 1) == 0) ? props.moveSpeed : -props.moveSpeed;
            game->enemies.list[slot].ghostVel.y = 0.0f;
        } else {
            game->enemies.list[slot].ghostVel.y = (GetRandomValue(0, 1) == 0) ? props.moveSpeed : -props.moveSpeed;
            game->enemies.list[slot].ghostVel.x = 0.0f;
        }
    } else {
        game->enemies.list[slot].ghostVel = (Vector2){ 0, 0 };
    }
}

void SpawnZombie(GameState* game) {
    EnemyType type = ENEMY_SNAKE;
    if (currentLevel == 1 || currentLevel == 2) {
        int roll = GetRandomValue(0, 2);
        type = (roll == 0) ? ENEMY_SNAKE : (roll == 1) ? ENEMY_SPIDER : ENEMY_GHOST;
    } else if (currentLevel == 3) {
        type = ENEMY_GHOST;
    }
    SpawnEnemy(game, type, -1, -1);
}

static void ShootSpiderProjectile(GameState* game, Vector2 startPos, Vector2 targetPos, bool isBig) {
    int projSlot = -1;
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!game->enemies.projectiles[i].active) {
            projSlot = i;
            break;
        }
    }
    if (projSlot == -1) return;

    float dx = targetPos.x - startPos.x;
    float dy = targetPos.y - startPos.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) len = 1.0f;

    float speed = isBig ? ENEMY_PROJECTILE_SPEED_BIG : ENEMY_PROJECTILE_SPEED_SMALL;
    game->enemies.projectiles[projSlot].position = startPos;
    game->enemies.projectiles[projSlot].velocity = (Vector2){ (dx / len) * speed, (dy / len) * speed };
    game->enemies.projectiles[projSlot].active = true;
    game->enemies.projectiles[projSlot].isBig = isBig;
    game->enemies.projectiles[projSlot].lifeTimer = ENEMY_PROJECTILE_LIFETIME;
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
        if (type == TILE_WALL) return false;
    }
    return true;
}

void UpdateZombies(GameState* game, float dt) {
    // 1. Ranged Enemy & Boss Firing Logic (Single Unified Attack Type)
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->enemies.list[i].active) continue;

        if (game->enemies.list[i].type == ENEMY_SPIDER ||
            game->enemies.list[i].type == ENEMY_RAT_KING ||
            game->enemies.list[i].type == ENEMY_DOOM_SCROLLER ||
            game->enemies.list[i].type == ENEMY_BRAINROT_GOD) {
            game->enemies.list[i].shootTimer += dt;
            float interval = (game->enemies.list[i].type == ENEMY_SPIDER) ? 1.8f : 1.5f;
            if (game->enemies.list[i].shootTimer >= interval) {
                game->enemies.list[i].shootTimer = 0.0f;
                Vector2 startPos = {
                    game->enemies.list[i].position.x + TILE_PX / 2.0f,
                    game->enemies.list[i].position.y + TILE_PX / 2.0f
                };
                Vector2 targetPos = {
                    game->player.position.x + TILE_PX / 2.0f,
                    game->player.position.y + TILE_PX / 2.0f
                };
                bool isBig = (game->enemies.list[i].type == ENEMY_RAT_KING || game->enemies.list[i].type == ENEMY_BRAINROT_GOD);
                ShootSpiderProjectile(game, startPos, targetPos, isBig);
            }
        }
    }

    // 2. Projectile Updates & Damage Logic
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!game->enemies.projectiles[i].active) continue;

        game->enemies.projectiles[i].lifeTimer -= dt;
        if (game->enemies.projectiles[i].lifeTimer <= 0.0f) {
            game->enemies.projectiles[i].active = false;
            continue;
        }

        game->enemies.projectiles[i].position.x += game->enemies.projectiles[i].velocity.x * dt;
        game->enemies.projectiles[i].position.y += game->enemies.projectiles[i].velocity.y * dt;

        Vector2 pos = game->enemies.projectiles[i].position;
        if (pos.x < 0 || pos.x > MAP_WIDTH * TILE_PX || pos.y < 0 || pos.y > MAP_HEIGHT * TILE_PX) {
            game->enemies.projectiles[i].active = false;
            continue;
        }

        int col = (int)(pos.x / TILE_PX);
        int row = (int)(pos.y / TILE_PX);
        if (row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH) {
            int tileType = GetTileType(game->map.tiles[row][col]);
            if (tileType == TILE_WALL) {
                game->enemies.projectiles[i].active = false;
                continue;
            }
        }

        Vector2 pCenter = { game->player.position.x + TILE_PX / 2.0f, game->player.position.y + TILE_PX / 2.0f };
        float pDx = pos.x - pCenter.x;
        float pDy = pos.y - pCenter.y;
        float collisionRadius = game->enemies.projectiles[i].isBig ? ENEMY_PROJECTILE_COLLISION_RADIUS_BIG : ENEMY_PROJECTILE_COLLISION_RADIUS_SMALL;
        float damageDealt = game->enemies.projectiles[i].isBig ? RANGED_ZOMBIE_PROJECTILE_DAMAGE_BIG : RANGED_ZOMBIE_PROJECTILE_DAMAGE_SMALL;

        if (sqrtf(pDx * pDx + pDy * pDy) < collisionRadius) {
            game->enemies.projectiles[i].active = false;
            game->player.health -= damageDealt;
            if (game->audio.hitTimer <= 0.0f) {
                PlaySound(game->audio.hit);
                game->audio.hitTimer = 0.25f;
            }
        }
    }

    // 3. Smooth Movement Updates
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!game->enemies.list[i].active) continue;

        if (game->enemies.list[i].type == ENEMY_GHOST) {
            game->enemies.list[i].position.x += game->enemies.list[i].ghostVel.x * dt;
            game->enemies.list[i].position.y += game->enemies.list[i].ghostVel.y * dt;

            // Bounce check at screen boundaries
            if (game->enemies.list[i].position.x < 0) {
                game->enemies.list[i].position.x = 0;
                game->enemies.list[i].ghostVel.x = -game->enemies.list[i].ghostVel.x;
            }
            else if (game->enemies.list[i].position.x > (MAP_WIDTH - 1) * TILE_PX) {
                game->enemies.list[i].position.x = (MAP_WIDTH - 1) * TILE_PX;
                game->enemies.list[i].ghostVel.x = -game->enemies.list[i].ghostVel.x;
            }

            if (game->enemies.list[i].position.y < 0) {
                game->enemies.list[i].position.y = 0;
                game->enemies.list[i].ghostVel.y = -game->enemies.list[i].ghostVel.y;
            }
            else if (game->enemies.list[i].position.y > (MAP_HEIGHT - 1) * TILE_PX) {
                game->enemies.list[i].position.y = (MAP_HEIGHT - 1) * TILE_PX;
                game->enemies.list[i].ghostVel.y = -game->enemies.list[i].ghostVel.y;
            }

            // Sync grid row/col
            game->enemies.list[i].row = (int)((game->enemies.list[i].position.y + TILE_PX / 2.0f) / TILE_PX);
            game->enemies.list[i].col = (int)((game->enemies.list[i].position.x + TILE_PX / 2.0f) / TILE_PX);
            continue;
        }



        float dx = game->player.position.x - game->enemies.list[i].position.x;
        float dy = game->player.position.y - game->enemies.list[i].position.y;
        float len = sqrtf(dx * dx + dy * dy);

        if (len > 2.0f) {
            EnemyProperties props = GetEnemyProperties(game->enemies.list[i].type, currentLevel, game->menu.difficulty);
            float moveSpeed = props.moveSpeed;

            if (moveSpeed > 0.0f) {
                float stepX = (dx / len) * moveSpeed;
                float stepY = (dy / len) * moveSpeed;

                float sepX = 0.0f;
                float sepY = 0.0f;
                float selfRadius = (TILE_PX / 2.0f) * props.scale;

                for (int j = 0; j < MAX_ZOMBIES; j++) {
                    if (j == i || !game->enemies.list[j].active) continue;
                    
                    float jdx = game->enemies.list[i].position.x - game->enemies.list[j].position.x;
                    float jdy = game->enemies.list[i].position.y - game->enemies.list[j].position.y;
                    float dist = sqrtf(jdx * jdx + jdy * jdy);
                    
                    EnemyProperties jProps = GetEnemyProperties(game->enemies.list[j].type, currentLevel, game->menu.difficulty);
                    float otherRadius = (TILE_PX / 2.0f) * jProps.scale;
                    float minDist = selfRadius + otherRadius - 4.0f;

                    if (dist > 0.1f && dist < minDist) {
                        sepX += (jdx / dist) * (minDist - dist) * 15.0f;
                        sepY += (jdy / dist) * (minDist - dist) * 15.0f;
                    }
                }

                float finalStepX = stepX + sepX;
                float finalStepY = stepY + sepY;
                float finalLen = sqrtf(finalStepX * finalStepX + finalStepY * finalStepY);
                if (finalLen > moveSpeed) {
                    finalStepX = (finalStepX / finalLen) * moveSpeed;
                    finalStepY = (finalStepY / finalLen) * moveSpeed;
                }

                float nextX = game->enemies.list[i].position.x + finalStepX * dt;
                float nextY = game->enemies.list[i].position.y + finalStepY * dt;

                if (Zombie_CanMoveTo(&game->map, nextX, nextY)) {
                    game->enemies.list[i].position.x = nextX;
                    game->enemies.list[i].position.y = nextY;
                } else {
                    if (Zombie_CanMoveTo(&game->map, nextX, game->enemies.list[i].position.y)) {
                        game->enemies.list[i].position.x = nextX;
                    }
                    if (Zombie_CanMoveTo(&game->map, game->enemies.list[i].position.x, nextY)) {
                        game->enemies.list[i].position.y = nextY;
                    }
                }
            }

            game->enemies.list[i].row = (int)((game->enemies.list[i].position.y + TILE_PX / 2.0f) / TILE_PX);
            game->enemies.list[i].col = (int)((game->enemies.list[i].position.x + TILE_PX / 2.0f) / TILE_PX);
        }
    }
}

void SpawnMemoryFragments(GameState* game) {
    const char* names[3] = { "CURIOSITY", "KNOWLEDGE", "TRUTH" };
    for (int i = 0; i < 3; i++) {
        game->enemies.fragments[i].activated = false;
        game->enemies.fragments[i].name = names[i];
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
    
    game->enemies.fragments[0].position = spots[0];
    game->enemies.fragments[1].position = spots[1];
    game->enemies.fragments[2].position = spots[2];
}
