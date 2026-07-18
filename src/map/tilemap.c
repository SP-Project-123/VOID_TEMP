#include "raylib.h"
#include "common.h"
#include <stdio.h>

// Declare external level tracking variable
extern int currentLevel;

// Helper drawing function for rendering tiles
static void DrawTile(Texture2D tileset, int tilesPerRow, int tileID, float x, float y) {
    float xco = (float)((tileID % tilesPerRow) * TILE_SIZE);
    float yco = (float)((tileID / tilesPerRow) * TILE_SIZE);
    Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
    Rectangle dest = { x, y, (float)TILE_PX, (float)TILE_PX };
    DrawTexturePro(tileset, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

// --- Tile Type Classification ---
int GetTileType(int tileID) {
    // 355 is designated as the Mayor tile
    if (tileID == 355) return TILE_MAYOR;
    // 283 is designated as the Cave tile
    if (tileID == 283) return TILE_CAVE;
    
    // Designated list of car tile IDs from map1.csv
    static const int carTiles[] = { 263, 287, 306, 329, 330, 353, 354,313,181,101,288,183 };
    for (int i = 0; i < (int)(sizeof(carTiles) / sizeof(carTiles[0])); i++) {
        if (carTiles[i] == tileID) return TILE_CAR;
    }

    // Walkable / Ground tiles list
    static const int walkable[] = {
        19, 20, 43, 44, 45, 67, 68, 69,
        121, 126, 265, 288, 290, 293, 337, 344
    };
    for (int i = 0; i < (int)(sizeof(walkable) / sizeof(walkable[0])); i++) {
        if (walkable[i] == tileID) return TILE_GROUND;
    }

    // Default to Wall for non-walkable tiles
    return TILE_WALL;
}

// --- Tilemap Implementations ---

void Tilemap_Load(Tilemap* self, const char* csvPath, const char* texturePath) {
    FILE *file = fopen(csvPath, "r");
    if (file) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                fscanf(file, "%d,", &self->tiles[y][x]);
            }
        }
        fclose(file);
    } else {
        printf("Failed to load map file: %s\n", csvPath);
    }

    // Inject Level 2 specific configurations dynamically into Map 2
    if (currentLevel == 1) {
        // Place a cave in Map 2 at Row 15, Col 29 to allow Level 2 escape
        self->tiles[15][29] = 283;
        // Inject a few obstacles/walls in Level 2 for zombie pathing variety
        for (int i = 0; i < 6; i++) {
            self->tiles[8][10 + i] = 28;
            self->tiles[16][14 + i] = 28;
        }
    }

    self->tileset = LoadTexture(texturePath);
    self->tilesPerRow = self->tileset.width / TILE_SIZE;
}

void Tilemap_Draw(const Tilemap* self) {
    // Note: Main viewport drawing is overridden by DrawGame's 5x5 centering camera loop.
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int tileID = self->tiles[y][x];
            DrawTile(self->tileset, self->tilesPerRow, tileID, (float)(x * TILE_PX), (float)(y * TILE_PX));
        }
    }
}

bool Tilemap_IsWalkable(const Tilemap* self, float targetX, float targetY) {
    int col = (int)(targetX / TILE_PX);
    int row = (int)(targetY / TILE_PX);

    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) return false;

    int tileID = self->tiles[row][col];
    int type = GetTileType(tileID);

    // Predictive Movement: Reject movement into walls or cars
    if (type == TILE_WALL || type == TILE_CAR) return false;
    return true;
}
