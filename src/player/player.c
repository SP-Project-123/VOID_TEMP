#include "raylib.h"
#include "common.h"
#include <stdlib.h>

// Declare external level tracking variable
extern int currentLevel;


// --- Player Implementations ---

void Player_Init(Player* self) {
    // Reset player starting position based on level
    if (currentLevel == 0) {
        self->position.x = 25.0f * TILE_PX;
        self->position.y = 23.0f * TILE_PX;
    } else {
        self->position.x = 2.0f * TILE_PX;
        self->position.y = 2.0f * TILE_PX;
    }
    self->speed = PLAYER_SPEED;
    self->currentObjective = 0;
    self->isMoving = false;
    self->animFrame = 0;
    
    self->gridX = (int)(self->position.x / TILE_PX);
    self->gridY = (int)(self->position.y / TILE_PX);
    self->direction = DIR_DOWN;
    self->hasWeapon = (currentLevel > 0);
    self->isAttacking = false;
    self->attackTimer = 0.0f;
    self->lightAttackCooldown = 0.0f;
    self->survivalTimer = 0.0f;
    self->radiusPowerupReady = false;
    self->radiusCooldown = 0.0f;
    self->radiusBlastTimer = 0.0f;
    self->isAimingSuperpower = false;
    self->health = PLAYER_INITIAL_HEALTH;
    self->maxHealth = PLAYER_MAX_HEALTH;
}

static bool Player_CanMoveTo(const Tilemap* map, float x, float y) {
    float margin = 12.0f; // Small margin to prevent getting stuck on corners
    float size = 20.0f;  // Player size is 32x32, checking up to width/height of 30.0f
    
    // Check all 4 corners of the bounding box
    if (!Tilemap_IsWalkable(map, x + margin, y + margin)) return false;
    if (!Tilemap_IsWalkable(map, x + size, y + margin)) return false;
    if (!Tilemap_IsWalkable(map, x + margin, y + size)) return false;
    if (!Tilemap_IsWalkable(map, x + size, y + size)) return false;
    
    return true;
}

void Player_Update(Player* self, const Tilemap* map, float dt) {
    self->isMoving = false;
    float nextX = self->position.x;
    float nextY = self->position.y;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { nextY -= self->speed * dt; self->isMoving = true; self->direction = DIR_UP; }
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { nextY += self->speed * dt; self->isMoving = true; self->direction = DIR_DOWN; }
    else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { nextX -= self->speed * dt; self->isMoving = true; self->direction = DIR_LEFT; }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { nextX += self->speed * dt; self->isMoving = true; self->direction = DIR_RIGHT; }

    if (Player_CanMoveTo(map, nextX, nextY)) {
        self->position.x = nextX;
        self->position.y = nextY;
    } else {
        if (Player_CanMoveTo(map, nextX, self->position.y)) self->position.x = nextX;
        if (Player_CanMoveTo(map, self->position.x, nextY)) self->position.y = nextY;
    }
    
    self->gridX = (int)(self->position.x / TILE_PX);
    self->gridY = (int)(self->position.y / TILE_PX);
    
    // Attack timer update (if attacking)
    if (self->isAttacking && self->attackTimer > 0.0f) {
        self->attackTimer -= dt;
        if (self->attackTimer <= 0.0f) {
            self->isAttacking = false;
            self->attackTimer = 0.0f;
        }
    }
    if (self->lightAttackCooldown > 0.0f) self->lightAttackCooldown -= dt;
    if (self->radiusCooldown > 0.0f) self->radiusCooldown -= dt;
    if (self->radiusBlastTimer > 0.0f) self->radiusBlastTimer -= dt;
}

void Player_Draw(const Player* self, Texture2D tileset, int tilesPerRow) {
    int playerTileID = 331;
    if (self->isMoving) {
        playerTileID = ((int)(GetTime() * 8.0f) % 2 == 0) ? 332 : 333;
    }
    DrawTile(tileset, tilesPerRow, playerTileID, self->position.x, self->position.y);
    
    // Draw sword if idle and has weapon
    if (self->hasWeapon && !self->isAttacking && !self->isAimingSuperpower) {
        Rectangle swordRect = {0};
        float px = self->position.x;
        float py = self->position.y;
        float sw = 4.0f;
        float sh = 16.0f;
        
        if (self->direction == DIR_UP) {
            swordRect = (Rectangle){ px + TILE_PX/2.0f + 4.0f, py - sh/2.0f, sw, sh };
        } else if (self->direction == DIR_DOWN) {
            swordRect = (Rectangle){ px + TILE_PX/2.0f - 8.0f, py + TILE_PX - sh/2.0f, sw, sh };
        } else if (self->direction == DIR_LEFT) {
            swordRect = (Rectangle){ px - sh/2.0f, py + TILE_PX/2.0f + 4.0f, sh, sw };
        } else if (self->direction == DIR_RIGHT) {
            swordRect = (Rectangle){ px + TILE_PX - sh/2.0f, py + TILE_PX/2.0f + 4.0f, sh, sw };
        }
        
        // Multi-layered fiery gradient rendering
        DrawRectangleRec(swordRect, RED);
        if (self->direction == DIR_UP || self->direction == DIR_DOWN) {
            DrawRectangle(swordRect.x + 1, swordRect.y + 2, swordRect.width - 2, swordRect.height - 4, ORANGE);
            DrawRectangle(swordRect.x + 1, swordRect.y + 4, swordRect.width - 2, swordRect.height - 8, GOLD);
        } else {
            DrawRectangle(swordRect.x + 2, swordRect.y + 1, swordRect.width - 4, swordRect.height - 2, ORANGE);
            DrawRectangle(swordRect.x + 4, swordRect.y + 1, swordRect.width - 8, swordRect.height - 2, GOLD);
        }
    }
}

void SpawnGun(GameState* game) {
    game->playerInfo.gunSpawned = false;
    for (int attempt = 0; attempt < 100; attempt++) {
        int r = GetRandomValue(1, MAP_HEIGHT - 2);
        int c = GetRandomValue(1, MAP_WIDTH - 2);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            int pCol = (int)(game->player.position.x / TILE_PX);
            int pRow = (int)(game->player.position.y / TILE_PX);
            if (abs(r - pRow) + abs(c - pCol) >= 3) {
                game->playerInfo.gunRow = r;
                game->playerInfo.gunCol = c;
                game->playerInfo.gunSpawned = true;
                break;
            }
        }
    }
}
