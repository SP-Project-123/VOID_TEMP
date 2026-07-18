#include "raylib.h"
#include "common.h"

int main(void) {
    // Window width and height matches layout dimensions
    InitWindow(MAP_WIDTH * TILE_PX, MAP_HEIGHT * TILE_PX, "GameRot");
    InitAudioDevice(); // Initialize audio device
    SetTargetFPS(60);

    GameState game;
    GameState_Init(&game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        UpdateGame(&game, dt);
        DrawGame(&game);
    }

    GameState_Unload(&game); // Unload loaded sounds/music
    UnloadTexture(game.map.tileset);
    CloseAudioDevice(); // Close audio device
    CloseWindow();
    return 0;
}

// UCRT compatibility wrapper for raylib
#include <sys/stat.h>
int stat64i32(const char *path, struct _stat *buffer) {
    return _stat(path, buffer);
}
