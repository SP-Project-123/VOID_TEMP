#include "raylib.h"
#include <stdio.h>

#define TILE_SIZE 8
#define ZOOM 4
#define TILE_PX (TILE_SIZE * ZOOM)

int main(void) {
    // Initial window size
    InitWindow(800, 600, "The GameRot - Tileset ID Viewer");
    SetTargetFPS(60);

    const char* file1 = "tilemap_packed.png";
    const char* file2 = "tilemap_packed2.png";
    const char* file3 = "spirites_tilepacked.png";
    const char* file4 = "mcpacked.png";
    const char* activeFile = file1;

    Texture2D tileset = LoadTexture(activeFile);
    int tilesPerRow = tileset.width / TILE_SIZE;

    while (!WindowShouldClose()) {
        // Toggle tileset file
        if (IsKeyPressed(KEY_ONE)) {
            UnloadTexture(tileset);
            activeFile = file1;
            tileset = LoadTexture(activeFile);
            tilesPerRow = tileset.width / TILE_SIZE;
        }
        if (IsKeyPressed(KEY_TWO)) {
            UnloadTexture(tileset);
            activeFile = file2;
            tileset = LoadTexture(activeFile);
            tilesPerRow = tileset.width / TILE_SIZE;
        }
        if (IsKeyPressed(KEY_THREE)) {
            UnloadTexture(tileset);
            activeFile = file3;
            tileset = LoadTexture(activeFile);
            tilesPerRow = tileset.width / TILE_SIZE;
        }
        if (IsKeyPressed(KEY_FOUR)) {
            UnloadTexture(tileset);
            activeFile = file4;
            tileset = LoadTexture(activeFile);
            tilesPerRow = tileset.width / TILE_SIZE;
        }

        Vector2 mousePos = GetMousePosition();
        int hoveredCol = (int)(mousePos.x / TILE_PX);
        int hoveredRow = (int)(mousePos.y / TILE_PX);
        
        int hoveredTileID = -1;
        // Verify inside bounds of the texture
        if (mousePos.x >= 0 && mousePos.x < tileset.width * ZOOM &&
            mousePos.y >= 0 && mousePos.y < tileset.height * ZOOM) {
            hoveredTileID = hoveredCol + hoveredRow * tilesPerRow;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoveredTileID != -1) {
            printf("CLICKED TILE ID: %d | Col: %d, Row: %d (in %s)\n", hoveredTileID, hoveredCol, hoveredRow, activeFile);
            fflush(stdout);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw tileset zoomed
        DrawTexturePro(
            tileset,
            (Rectangle){ 0, 0, (float)tileset.width, (float)tileset.height },
            (Rectangle){ 0, 0, (float)(tileset.width * ZOOM), (float)(tileset.height * ZOOM) },
            (Vector2){0, 0}, 0.0f, WHITE
        );

        // Draw grid overlay
        int cols = tileset.width / TILE_SIZE;
        int rows = tileset.height / TILE_SIZE;
        for (int x = 0; x <= cols; x++) {
            DrawLine(x * TILE_PX, 0, x * TILE_PX, rows * TILE_PX, ColorAlpha(DARKGRAY, 0.4f));
        }
        for (int y = 0; y <= rows; y++) {
            DrawLine(0, y * TILE_PX, cols * TILE_PX, y * TILE_PX, ColorAlpha(DARKGRAY, 0.4f));
        }

        // Highlight hovered tile
        if (hoveredTileID != -1) {
            DrawRectangleLines(hoveredCol * TILE_PX, hoveredRow * TILE_PX, TILE_PX, TILE_PX, RED);
        }

        // HUD panel showing status
        int hudW = 340;
        int hudH = 140;
        int hudX = GetScreenWidth() - hudW - 20;
        int hudY = GetScreenHeight() - hudH - 20;
        DrawRectangle(hudX, hudY, hudW, hudH, ColorAlpha(BLACK, 0.8f));
        DrawRectangleLines(hudX, hudY, hudW, hudH, RED);

        DrawText("TILESET ID VIEWER", hudX + 20, hudY + 15, 16, RED);
        DrawText(TextFormat("File: %s", activeFile), hudX + 20, hudY + 40, 12, LIGHTGRAY);
        DrawText("Keys: [1] Tile1 | [2] Tile2 | [3] Sprites | [4] MC", hudX + 20, hudY + 58, 11, YELLOW);

        if (hoveredTileID != -1) {
            DrawText(TextFormat("HOVERED TILE ID: %d", hoveredTileID), hudX + 20, hudY + 80, 14, GREEN);
            DrawText(TextFormat("Col: %d  |  Row: %d", hoveredCol, hoveredRow), hudX + 20, hudY + 100, 12, WHITE);
            DrawText("Click Left Mouse Button to print ID", hudX + 20, hudY + 115, 10, GRAY);
        } else {
            DrawText("Hover over a tile to inspect ID", hudX + 20, hudY + 85, 12, GRAY);
        }

        EndDrawing();
    }

    UnloadTexture(tileset);
    CloseWindow();
    return 0;
}
