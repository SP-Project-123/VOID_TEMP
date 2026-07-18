#include "raylib.h"
#include "common.h"
#include <stdio.h>

// Declare external level tracking variable
extern int currentLevel;


// --- Tile Type Classification ---
int GetTileType(int tileID) {
    // 283 is designated as the Mayor tile
    if (tileID == 283) return TILE_MAYOR;
    // 236, 237, 306, 308, and 158 are designated as the Cave exit gate tiles
    if (tileID == 236 || tileID == 237 || tileID == 306 || tileID == 308 || tileID == 158) return TILE_CAVE;
    
    // Designated list of car tile IDs from map1.csv
    static const int carTiles[] = { 263, 287, 329, 330, 353,324,323,275,276, 354,313,181,101,288,183,347,348,349,350 };
    for (int i = 0; i < (int)(sizeof(carTiles) / sizeof(carTiles[0])); i++) {
        if (carTiles[i] == tileID) return TILE_CAR;
    }

    // Walkable lists separated by currentLevel
    static const int walkableL1[] = {
        3,27,51,19, 20, 21, 22, 23, 43, 44, 45, 46, 47, 67, 68, 69, 70, 71, 83, 101, 121, 125, 126, 135, 170, 171, 185, 193, 198, 209, 210, 264,265,266,267,268,269, 288, 289, 290, 291, 292, 293,294,295,296,297,298, 312, 313,314,315,316,317,318,319,320,321,322, 314, 336, 337, 338, 344,336,337,338,339,340,341,342,343,344,345,346
    };
    static const int walkableL2[] = {
        17, 64, 65, 158, 159,68,31,34,146,152
    };
    static const int walkableL3[] = {
        192,193,194,195,196,197,198,199,200,201,202,203,216,217,218,219,220,221,222,223,224,225,226,227,133,132,180,181,182,183,184,185,156,157
    };
    static const int walkableL4[] = {
        17, 64, 65, 158, 159
    };

    const int* list = NULL;
    int size = 0;
    if (currentLevel == 0) {
        list = walkableL1;
        size = sizeof(walkableL1) / sizeof(walkableL1[0]);
    } else if (currentLevel == 1) {
        list = walkableL2;
        size = sizeof(walkableL2) / sizeof(walkableL2[0]);
    } else if (currentLevel == 2) {
        list = walkableL3;
        size = sizeof(walkableL3) / sizeof(walkableL3[0]);
    } else if (currentLevel == 3) {
        list = walkableL4;
        size = sizeof(walkableL4) / sizeof(walkableL4[0]);
    }

    if (list) {
        for (int i = 0; i < size; i++) {
            if (list[i] == tileID) return TILE_GROUND;
        }
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
        self->tiles[15][29] = 306;
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
