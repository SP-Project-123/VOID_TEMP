#include "raylib.h"
#include <stdio.h>

#define mapwidth 35
#define mapheight 24
#define tilesize 8
#define zoom 4

int main(void) {
    InitWindow(800, 600, "GameRot");

    int monitor = GetCurrentMonitor();

    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));

    float scaleX = (float)GetScreenWidth() / (mapwidth * tilesize);
    float scaleY = (float)GetScreenHeight() / (mapheight * tilesize);


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
        if (IsKeyPressed(KEY_F11))
{
    ToggleFullscreen();
}
        BeginDrawing();
        ClearBackground(BLACK);

        for (int y = 0; y < mapheight; y++) {
            for (int x = 0; x < mapwidth; x++) {
                int tileID = map[y][x] ; 

                float xco= (float)((tileID % tilesrow) * tilesize);
                float yco = (float)((tileID / tilesrow) * tilesize);

                Rectangle tileinmap = { xco, yco, (float)tilesize, (float)tilesize };
                Rectangle pastetile = {
                    x * tilesize * scaleX,
                    y * tilesize * scaleY,
                    tilesize * scaleX,
                    tilesize * scaleY
                };

                DrawTexturePro(tileset, tileinmap, pastetile, (Vector2){0, 0}, 0.0f, WHITE);
            }
        }

        EndDrawing();
    }

    UnloadTexture(tileset);
    CloseWindow();
    return 0;
}