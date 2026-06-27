#include "raylib.h"
#include <stdio.h>

#define mapwidth 35
#define mapheight 24
#define tilesize 8
#define zoom 4

int main(void) {
    InitWindow(mapwidth*tilesize*zoom, mapheight*tilesize*zoom, "GameRot");
    SetTargetFPS(60);


    int map[mapheight][mapwidth];

    FILE *file = fopen("map1.csv", "r");

        for (int y = 0; y < mapheight; y++) {
            for (int x = 0; x < mapwidth; x++) {
                fscanf(file, "%d,", &map[y][x]);
            }
        }
        fclose(file);

    Texture2D tileset = LoadTexture("tilemap_packed.png");
    int tilesrow = tileset.width / tilesize;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // 3. Draw the map
        for (int y = 0; y < mapheight; y++) {
            for (int x = 0; x < mapwidth; x++) {
                int tileID = map[y][x] ; 

                // Find the source rectangle coordinates inside the tileset image
                float xco= (float)((tileID % tilesrow) * tilesize);
                float yco = (float)((tileID / tilesrow) * tilesize);

                Rectangle tileinmap = { xco, yco, (float)tilesize, (float)tilesize };
                Rectangle pastetile = { (float)(x * tilesize * zoom), (float)(y * tilesize * zoom), (float)(tilesize * zoom), (float)(tilesize * zoom) };

                DrawTexturePro(tileset, tileinmap, pastetile, (Vector2){0, 0}, 0.0f, WHITE);
            }
        }

        EndDrawing();
    }

    UnloadTexture(tileset);
    CloseWindow();
    return 0;
}