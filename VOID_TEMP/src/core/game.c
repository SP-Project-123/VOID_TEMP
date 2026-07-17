#include "raylib.h"
#include "common.h"
#include <stdio.h>

// Define global tracking variables
int currentLevel = 0;

// Helper drawing function for rendering tiles
static void DrawTile(Texture2D tileset, int tilesPerRow, int tileID, float x, float y) {
    float xco = (float)((tileID % tilesPerRow) * TILE_SIZE);
    float yco = (float)((tileID / tilesPerRow) * TILE_SIZE);
    Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
    Rectangle dest = { x, y, (float)TILE_PX, (float)TILE_PX };
    DrawTexturePro(tileset, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

// --- GameState Implementations ---

void GameState_Init(GameState* self) {
    currentLevel = 0;
    Tilemap_Load(&self->map, "map1.csv", "tilemap_packed.png");
    Player_Init(&self->player);
    self->state = STATE_MENU;
    self->zombieTimer = 0.0f;
    self->zombieSpawnTimer = 0.0f;
    self->gameOverSelection = 0;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        self->zombies[i].active = false;
    }
    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        self->ashEffects[i].active = false;
    }
}

// --- State Machine Update Routine ---
void UpdateGame(GameState* game, float dt) {
    int pCol = (int)(game->player.position.x / TILE_PX);
    int pRow = (int)(game->player.position.y / TILE_PX);

    // Update Ash effects
    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        if (game->ashEffects[i].active) {
            game->ashEffects[i].timer -= dt;
            if (game->ashEffects[i].timer <= 0.0f) {
                game->ashEffects[i].active = false;
            }
        }
    }

    // Helper to spawn ash
    void SpawnAsh(int c, int r) {
        for (int a = 0; a < MAX_ASH_EFFECTS; a++) {
            if (!game->ashEffects[a].active) {
                game->ashEffects[a].gridX = c;
                game->ashEffects[a].gridY = r;
                game->ashEffects[a].timer = 1.5f;
                game->ashEffects[a].active = true;
                break;
            }
        }
    }

    // Combat Sweep Attack Logic
    if (IsKeyPressed(KEY_SPACE) && game->player.hasWeapon && !game->player.isAttacking) {
        if (game->player.lightAttackCooldown <= 0.0f) {
            game->player.isAttacking = true;
            game->player.attackTimer = 0.15f;
            game->player.lightAttackCooldown = 0.33f;
            
            int dx = 0, dy = 0;
            if (game->player.direction == DIR_UP) dy = -1;
            else if (game->player.direction == DIR_DOWN) dy = 1;
            else if (game->player.direction == DIR_LEFT) dx = -1;
            else if (game->player.direction == DIR_RIGHT) dx = 1;

            int t1X = pCol + dx;
            int t1Y = pRow + dy;
            int t2X = pCol + dx * 2;
            int t2Y = pRow + dy * 2;

            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (game->zombies[i].active) {
                    if ((game->zombies[i].col == t1X && game->zombies[i].row == t1Y) ||
                        (game->zombies[i].col == t2X && game->zombies[i].row == t2Y)) {
                        game->zombies[i].active = false;
                        SpawnAsh(game->zombies[i].col, game->zombies[i].row);
                    }
                }
            }
        }
    }

    switch (game->state) {
        case STATE_MENU: {
            if (IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_EXPLORING;
            }
            break;
        }
        
        case STATE_EXPLORING: {
            Player_Update(&game->player, &game->map, dt);

            // Switch to STATE_CUTSCENE when stepping on the Mayor's tile
            if (pRow >= 0 && pRow < MAP_HEIGHT && pCol >= 0 && pCol < MAP_WIDTH) {
                int tileID = game->map.tiles[pRow][pCol];
                if (GetTileType(tileID) == TILE_MAYOR) {
                    game->state = STATE_CUTSCENE;
                }
            }
            break;
        }

        case STATE_CUTSCENE: {
            // Cutscene: Wait for user to hit KEY_ENTER to start survival mode
            if (IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_SURVIVAL;
                game->zombieTimer = 0.0f;
                game->zombieSpawnTimer = 0.0f;
                // Clear any leftover zombies
                for (int i = 0; i < MAX_ZOMBIES; i++) {
                    game->zombies[i].active = false;
                }
                // Spawn first wave
                SpawnZombie(game);
                SpawnZombie(game);
                SpawnZombie(game);
            }
            break;
        }

        case STATE_SURVIVAL: {
            Player_Update(&game->player, &game->map, dt);

            // Spawn zombies periodically
            game->zombieSpawnTimer += dt;
            if (game->zombieSpawnTimer >= 2.0f) {
                game->zombieSpawnTimer = 0.0f;
                SpawnZombie(game);
            }

            // Path & update zombies closer to player
            UpdateZombies(game, dt);

            // Level Transition Engine: check if reaching Cave
            if (pRow >= 0 && pRow < MAP_HEIGHT && pCol >= 0 && pCol < MAP_WIDTH) {
                int tileID = game->map.tiles[pRow][pCol];
                if (GetTileType(tileID) == TILE_CAVE) {
                    // Reset zombie array
                    for (int i = 0; i < MAX_ZOMBIES; i++) {
                        game->zombies[i].active = false;
                    }

                    if (currentLevel == 0) {
                        // Increment level and load Map 2
                        currentLevel = 1;
                        UnloadTexture(game->map.tileset); // Clean texture from Map 1
                        Tilemap_Load(&game->map, "map2.csv", "tilemap_packed.png");
                        
                        // Reset player spawn coordinates to Map 2 starting position
                        Player_Init(&game->player);
                        game->state = STATE_EXPLORING;
                    } else {
                        // Reached cave on Level 2: Win! Reset back to Level 1
                        currentLevel = 0;
                        UnloadTexture(game->map.tileset);
                        Tilemap_Load(&game->map, "map1.csv", "tilemap_packed.png");
                        Player_Init(&game->player);
                        game->state = STATE_EXPLORING;
                    }
                }
            }

            // Survival Radius Superpower Logic
            game->player.survivalTimer += dt;
            if (game->player.survivalTimer >= 7.0f && game->player.radiusCooldown <= 0.0f) {
                game->player.radiusPowerupReady = true;
            }

            if (game->player.radiusPowerupReady && IsKeyDown(KEY_F)) {
                game->player.isAimingSuperpower = true;
            } else {
                game->player.isAimingSuperpower = false;
            }

            if (game->player.radiusPowerupReady && IsKeyReleased(KEY_F) && game->player.radiusCooldown <= 0.0f) {
                game->player.radiusPowerupReady = false;
                game->player.radiusCooldown = 8.0f;
                game->player.radiusBlastTimer = 0.5f;
                
                // Kill sweep within 5 grid blocks
                for (int i = 0; i < MAX_ZOMBIES; i++) {
                    if (game->zombies[i].active) {
                        int diffX = game->zombies[i].col - pCol;
                        int diffY = game->zombies[i].row - pRow;
                        if (diffX * diffX + diffY * diffY <= 25) { // 5-block radius roughly
                            game->zombies[i].active = false;
                            SpawnAsh(game->zombies[i].col, game->zombies[i].row);
                        }
                    }
                }
            }

            // Check if player collided with a zombie: transition to game over
            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (game->zombies[i].active && game->zombies[i].row == pRow && game->zombies[i].col == pCol) {
                    game->state = STATE_GAMEOVER;
                    game->gameOverSelection = 0;
                    break;
                }
            }
            break;
        }

        case STATE_GAMEOVER: {
            if (IsKeyPressed(KEY_UP)) game->gameOverSelection = 0;
            if (IsKeyPressed(KEY_DOWN)) game->gameOverSelection = 1;
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (game->gameOverSelection == 0) {
                    // Retry
                    GameState_Init(game);
                } else {
                    // Quit
                    CloseWindow(); // Triggers window close in next frame
                }
            }
            break;
        }
    }

    // Left mouse click debug utility: Print tile info
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        int col = (int)(mousePos.x / TILE_PX);
        int row = (int)(mousePos.y / TILE_PX);
        if (row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH) {
            printf("Clicked -> Row: %d | Col: %d | TILE ID: %d | TYPE: %d\n", 
                   row, col, game->map.tiles[row][col], GetTileType(game->map.tiles[row][col]));
        }
    }
}

// --- State Machine Draw Routine ---
void DrawGame(const GameState* game) {
    BeginDrawing();
    ClearBackground(BLACK);

    // Setup 2D Camera centered on the player
    Camera2D camera = { 0 };
    camera.target = (Vector2){ game->player.position.x + TILE_PX / 2.0f, game->player.position.y + TILE_PX / 2.0f };
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 2.5f; // Zoom centered viewport

    BeginMode2D(camera);

    // Draw entire map (Camera limits what is seen natively)
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int tileID = game->map.tiles[y][x];
            DrawTile(game->map.tileset, game->map.tilesPerRow, tileID, (float)(x * TILE_PX), (float)(y * TILE_PX));
        }
    }

    // Draw Ash Effects
    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        if (game->ashEffects[i].active) {
            float ax = game->ashEffects[i].gridX * TILE_PX;
            float ay = game->ashEffects[i].gridY * TILE_PX;
            float alpha = (game->ashEffects[i].timer / 1.5f) * 0.8f;
            DrawRectangle(ax, ay, TILE_PX, TILE_PX, ColorAlpha((Color){30, 30, 30, 255}, alpha));
        }
    }

    // Draw Zombies in visibility window
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (game->zombies[i].active) {
            int zRow = game->zombies[i].row;
            int zCol = game->zombies[i].col;
            float zx = zCol * TILE_PX;
            float zy = zRow * TILE_PX;
            // Render Zombie using player base template tinted red
            DrawTile(game->map.tileset, game->map.tilesPerRow, 331, zx, zy);
            DrawRectangle(zx, zy, TILE_PX, TILE_PX, ColorAlpha(RED, 0.45f));
        }
    }

    // Aiming superpower visual
    if (game->player.isAimingSuperpower) {
        float px = game->player.position.x + TILE_PX/2.0f;
        float py = game->player.position.y + TILE_PX/2.0f;
        DrawCircle(px, py, 5.0f * TILE_PX, ColorAlpha(RED, 0.3f));
    }

    // Blast Ring visual
    if (game->player.radiusBlastTimer > 0.0f) {
        float px = game->player.position.x + TILE_PX/2.0f;
        float py = game->player.position.y + TILE_PX/2.0f;
        float maxRadius = 5.0f * TILE_PX;
        float currentRadius = maxRadius * (1.0f - (game->player.radiusBlastTimer / 0.5f));
        DrawCircleLines(px, py, currentRadius, ORANGE);
        DrawCircleLines(px, py, currentRadius + 2.0f, RED);
    }

    // Draw Player
    Player_Draw(&game->player, game->map.tileset, game->map.tilesPerRow);

    // Attack Slash Overlay
    if (game->player.isAttacking && game->player.attackTimer > 0.0f) {
        int pCol = game->player.gridX;
        int pRow = game->player.gridY;
        int dx = 0, dy = 0;
        if (game->player.direction == DIR_UP) dy = -1;
        else if (game->player.direction == DIR_DOWN) dy = 1;
        else if (game->player.direction == DIR_LEFT) dx = -1;
        else if (game->player.direction == DIR_RIGHT) dx = 1;
        
        float rx = (pCol + dx) * TILE_PX;
        float ry = (pRow + dy) * TILE_PX;
        float rw = TILE_PX;
        float rh = TILE_PX;
        
        if (dx != 0) rw = TILE_PX * 2.0f;
        if (dy != 0) rh = TILE_PX * 2.0f;
        
        if (dx == -1) rx = (pCol - 2) * TILE_PX;
        if (dy == -1) ry = (pRow - 2) * TILE_PX;
        
        // Fiery slash multi-layer rendering
        for (int i = 0; i < 6; i++) {
            float ox = rx + GetRandomValue(-2, 2);
            float oy = ry + GetRandomValue(-2, 2);
            Color fColor = (i % 3 == 0) ? GOLD : ((i % 2 == 0) ? ORANGE : RED);
            DrawRectangle(ox, oy, rw, rh, ColorAlpha(fColor, 0.6f));
        }
    }

    EndMode2D();

    // UI Overlay (drawn in Screen Space)
    if (game->state == STATE_MENU) {
        DrawRectangle(0, GetScreenHeight() / 2 - 50, GetScreenWidth(), 100, BLACK);
        const char* title = "ENTER TO START";
        int tWidth = MeasureText(title, 40);
        DrawText(title, (GetScreenWidth() - tWidth) / 2, GetScreenHeight() / 2 - 20, 40, (Color){ 139, 0, 0, 255 });
    }
    else if (game->state == STATE_GAMEOVER) {
        int boxW = 300, boxH = 200;
        int bx = (GetScreenWidth() - boxW) / 2;
        int by = (GetScreenHeight() - boxH) / 2;
        DrawRectangle(bx, by, boxW, boxH, ColorAlpha(BLACK, 0.9f));
        DrawRectangleLines(bx, by, boxW, boxH, (Color){ 139, 0, 0, 255 });
        
        DrawText("DEFEAT", bx + 100, by + 40, 24, (Color){ 139, 0, 0, 255 });
        
        Color retryColor = (game->gameOverSelection == 0) ? (Color){ 139, 0, 0, 255 } : DARKGRAY;
        Color quitColor = (game->gameOverSelection == 1) ? (Color){ 139, 0, 0, 255 } : DARKGRAY;
        
        DrawText("RETRY", bx + 115, by + 100, 20, retryColor);
        DrawText("QUIT", bx + 125, by + 140, 20, quitColor);
    }
    else {
        DrawRectangle(10, 10, 360, 80, ColorAlpha(DARKGRAY, 0.8f));
        DrawRectangleLines(10, 10, 360, 80, WHITE);
        DrawText(TextFormat("LEVEL: %d / 2", currentLevel + 1), 20, 20, 20, GREEN);

        if (game->state == STATE_EXPLORING) {
            DrawText("OBJ: Find the Mayor", 20, 50, 16, WHITE);
        } 
        else if (game->state == STATE_CUTSCENE) {
            // Draw simple cutscene text box overlay
            int boxWidth = 520;
            int boxHeight = 130;
            int boxX = (GetScreenWidth() - boxWidth) / 2;
            int boxY = GetScreenHeight() - boxHeight - 40;

            DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.9f));
            DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, RED);
            
            DrawText("MAYOR: The streets are infested with zombies!", boxX + 25, boxY + 20, 18, RED);
            DrawText("Run to the Cave immediately!", boxX + 25, boxY + 50, 16, YELLOW);
            DrawText(">>> Press ENTER to start Survival mode <<<", boxX + 25, boxY + 85, 16, LIGHTGRAY);
        } 
        else if (game->state == STATE_SURVIVAL) {
            int activeCount = 0;
            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (game->zombies[i].active) activeCount++;
            }
            DrawText("OBJ: Find the Cave", 20, 50, 16, RED);
            DrawText(TextFormat("Zombies Chasing: %d", activeCount), 20, 70, 14, ORANGE);
        }
    }

    EndDrawing();
}
