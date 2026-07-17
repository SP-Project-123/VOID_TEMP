#include "raylib.h"
#include "common.h"
#include <stdlib.h> // For abs()

// Helper drawing function for rendering tiles
static void DrawTile(Texture2D tileset, int tilesPerRow, int tileID, float x, float y) {
    float xco = (float)((tileID % tilesPerRow) * TILE_SIZE);
    float yco = (float)((tileID / tilesPerRow) * TILE_SIZE);
    Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
    Rectangle dest = { x, y, (float)TILE_PX, (float)TILE_PX };
    DrawTexturePro(tileset, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

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
                game->zombies[slot].active = true;
                break;
            }
        }
    }
}

void UpdateZombies(GameState* game, float dt) {
    game->zombieTimer += dt;
    // Controlled movement interval: Move zombies 1 tile closer every 0.6 seconds
    if (game->zombieTimer >= 0.6f) {
        game->zombieTimer = 0.0f;

        int playerCol = (int)(game->player.position.x / TILE_PX);
        int playerRow = (int)(game->player.position.y / TILE_PX);

        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (!game->zombies[i].active) continue;

            int zRow = game->zombies[i].row;
            int zCol = game->zombies[i].col;

            int nextRow = zRow;
            int nextCol = zCol;

            // Step 1 tile closer
            if (zRow < playerRow) nextRow++;
            else if (zRow > playerRow) nextRow--;
            
            if (zCol < playerCol) nextCol++;
            else if (zCol > playerCol) nextCol--;

            // Collision check: verify target tile is walkable ground
            if (GetTileType(game->map.tiles[nextRow][nextCol]) == TILE_GROUND) {
                game->zombies[i].row = nextRow;
                game->zombies[i].col = nextCol;
            } else if (GetTileType(game->map.tiles[zRow][nextCol]) == TILE_GROUND) {
                game->zombies[i].col = nextCol;
            } else if (GetTileType(game->map.tiles[nextRow][zCol]) == TILE_GROUND) {
                game->zombies[i].row = nextRow;
            }
        }
    }
}
