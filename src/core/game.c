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
    self->menuSelection = 0;
    self->storyPage = 0;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        self->zombies[i].active = false;
    }
    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        self->ashEffects[i].active = false;
    }

    // Load Audio Resources
    self->bgMusic = LoadMusicStream("resources/bgm.mp3");
    self->slashSound = LoadSound("resources/slash.wav");
    self->blastSound = LoadSound("resources/blast.wav");
    self->hitSound = LoadSound("resources/hit.wav");
    self->hitSoundTimer = 0.0f;

    // Start playing menu background music
    PlayMusicStream(self->bgMusic);
}

// --- State Machine Update Routine ---
void UpdateGame(GameState* game, float dt) {
    // Feed background music stream
    UpdateMusicStream(game->bgMusic);

    if (game->hitSoundTimer > 0.0f) {
        game->hitSoundTimer -= dt;
    }

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
            PlaySound(game->slashSound);
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
                        game->zombies[i].health -= 15.0f;
                        if (game->zombies[i].health <= 0.0f) {
                            game->zombies[i].active = false;
                            SpawnAsh(game->zombies[i].col, game->zombies[i].row);
                        }
                    }
                }
            }
        }
    }

    switch (game->state) {
        case STATE_MENU: {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                game->menuSelection--;
                if (game->menuSelection < 0) game->menuSelection = 3;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                game->menuSelection++;
                if (game->menuSelection > 3) game->menuSelection = 0;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (game->menuSelection == 0) {
                    game->state = STATE_EXPLORING;
                } else if (game->menuSelection == 1) {
                    game->state = STATE_INFO;
                    game->storyPage = 0;
                } else if (game->menuSelection == 2) {
                    game->state = STATE_TEAM;
                } else {
                    CloseWindow();
                }
            }
            break;
        }

        case STATE_INFO: {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                game->storyPage++;
                if (game->storyPage > 4) game->storyPage = 0;
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                game->storyPage--;
                if (game->storyPage < 0) game->storyPage = 4;
            }
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                game->state = STATE_MENU;
            }
            break;
        }

        case STATE_TEAM: {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_MENU;
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
                PlaySound(game->blastSound);
                game->player.radiusPowerupReady = false;
                game->player.radiusCooldown = 8.0f;
                game->player.radiusBlastTimer = 0.5f;
                
                // Kill sweep within 5 grid blocks
                for (int i = 0; i < MAX_ZOMBIES; i++) {
                    if (game->zombies[i].active) {
                        int diffX = game->zombies[i].col - pCol;
                        int diffY = game->zombies[i].row - pRow;
                        if (diffX * diffX + diffY * diffY <= 25) { // 5-block radius roughly
                            game->zombies[i].health -= 50.0f;
                            if (game->zombies[i].health <= 0.0f) {
                                game->zombies[i].active = false;
                                SpawnAsh(game->zombies[i].col, game->zombies[i].row);
                            }
                        }
                    }
                }
            }

            // Check if player collided with a zombie: take damage over time
            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (game->zombies[i].active && game->zombies[i].row == pRow && game->zombies[i].col == pCol) {
                    game->player.health -= 30.0f * dt;
                    if (game->hitSoundTimer <= 0.0f) {
                        PlaySound(game->hitSound);
                        game->hitSoundTimer = 0.5f; // Play hit sound at most twice per second
                    }
                    if (game->player.health <= 0.0f) {
                        game->player.health = 0.0f;
                        game->state = STATE_GAMEOVER;
                        game->gameOverSelection = 0;
                    }
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

            // Draw Zombie Health Bar
            float barWidth = (float)TILE_PX;
            float barHeight = 4.0f;
            float z_py = zy - 6.0f;
            DrawRectangle(zx, z_py, barWidth, barHeight, ColorAlpha(BLACK, 0.6f));
            float pct = game->zombies[i].health / game->zombies[i].maxHealth;
            if (pct < 0.0f) pct = 0.0f;
            DrawRectangle(zx + 1, z_py + 1, (barWidth - 2) * pct, barHeight - 2, RED);
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
        ClearBackground((Color){ 15, 15, 20, 255 });
        
        const char* titleText = "VOID SURVIVAL";
        int titleWidth = MeasureText(titleText, 56);
        DrawText(titleText, (GetScreenWidth() - titleWidth) / 2 + 4, GetScreenHeight() / 4 + 4, 56, BLACK);
        DrawText(titleText, (GetScreenWidth() - titleWidth) / 2, GetScreenHeight() / 4, 56, (Color){ 180, 0, 0, 255 });
        
        const char* subtitle = "A Raylib Retro Roguelike";
        int subWidth = MeasureText(subtitle, 18);
        DrawText(subtitle, (GetScreenWidth() - subWidth) / 2, GetScreenHeight() / 4 + 70, 18, GRAY);

        Color playColor = (game->menuSelection == 0) ? RED : DARKGRAY;
        Color infoColor = (game->menuSelection == 1) ? RED : DARKGRAY;
        Color teamColor = (game->menuSelection == 2) ? RED : DARKGRAY;
        Color quitColor = (game->menuSelection == 3) ? RED : DARKGRAY;
        
        const char* playText = "PLAY GAME";
        const char* infoText = "STORY & INFO";
        const char* teamText = "DEVELOPMENT TEAM";
        const char* quitText = "QUIT";
        
        int playWidth = MeasureText(playText, 24);
        int infoWidth = MeasureText(infoText, 24);
        int teamWidth = MeasureText(teamText, 24);
        int quitWidth = MeasureText(quitText, 24);
        
        int selectY = GetScreenHeight() / 2 - 10 + (game->menuSelection * 45);
        DrawRectangle((GetScreenWidth() - 320) / 2, selectY - 10, 320, 40, ColorAlpha(DARKGRAY, 0.2f));
        DrawRectangleLines((GetScreenWidth() - 320) / 2, selectY - 10, 320, 40, RED);

        DrawText(playText, (GetScreenWidth() - playWidth) / 2, GetScreenHeight() / 2 - 10, 20, playColor);
        DrawText(infoText, (GetScreenWidth() - infoWidth) / 2, GetScreenHeight() / 2 + 35, 20, infoColor);
        DrawText(teamText, (GetScreenWidth() - teamWidth) / 2, GetScreenHeight() / 2 + 80, 20, teamColor);
        DrawText(quitText, (GetScreenWidth() - quitWidth) / 2, GetScreenHeight() / 2 + 125, 20, quitColor);

        int panelW = 440;
        int panelH = 110;
        int panelX = (GetScreenWidth() - panelW) / 2;
        int panelY = GetScreenHeight() - panelH - 20;
        
        DrawRectangle(panelX, panelY, panelW, panelH, ColorAlpha(BLACK, 0.6f));
        DrawRectangleLines(panelX, panelY, panelW, panelH, DARKGRAY);
        
        DrawText("CONTROLS:", panelX + 20, panelY + 15, 14, RED);
        DrawText("[WASD / Arrow Keys] Move Character", panelX + 20, panelY + 38, 12, LIGHTGRAY);
        DrawText("[SPACE] Sword Attack (Requires Weapon)", panelX + 20, panelY + 58, 12, LIGHTGRAY);
        DrawText("[F] Superpower Area Blast (8s Cooldown)", panelX + 20, panelY + 78, 12, LIGHTGRAY);
    }
    else if (game->state == STATE_INFO) {
        ClearBackground((Color){ 15, 15, 20, 255 });
        int boxX = (GetScreenWidth() - 900) / 2;
        int boxY = 40;
        DrawRectangle(boxX, boxY, 900, 680, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, 900, 680, RED);

        DrawText("THE STORY OF NEO OHIO", boxX + 50, boxY + 30, 28, RED);
        DrawText(TextFormat("[ Page %d / 5 ]", game->storyPage + 1), boxX + 750, boxY + 35, 18, YELLOW);
        DrawText("-------------------------------------------------------------------------------------", boxX + 50, boxY + 65, 16, GRAY);

        if (game->storyPage == 0) {
            DrawText("Prologue - The Mayor's Request", boxX + 50, boxY + 90, 20, YELLOW);
            DrawText("The game begins in Neo Ohio, where strange incidents have spread across the city.", boxX + 50, boxY + 130, 16, LIGHTGRAY);
            DrawText("Citizens have become infected by Brainrot, wandering the streets speaking meaningless", boxX + 50, boxY + 155, 16, LIGHTGRAY);
            DrawText("phrases like 'Skibidi', 'Sigma', and 'Ohio'. They attack anyone who isn't infected.", boxX + 50, boxY + 180, 16, LIGHTGRAY);
            DrawText("The Mayor summons Fahad, one of the few people whose memories remain intact.", boxX + 50, boxY + 215, 16, LIGHTGRAY);
            DrawText("Mayor: 'You're the only one who still remembers how the world used to be. The city'", boxX + 50, boxY + 250, 16, WHITE);
            DrawText("'is falling apart. Find the source of this infection... before everyone forgets who they are.'", boxX + 50, boxY + 275, 16, WHITE);
            DrawText("The Mayor gives Fahad access to the abandoned underground maintenance sewers.", boxX + 50, boxY + 310, 16, LIGHTGRAY);

            DrawText("Chapter 1 - The Ohio Sewers", boxX + 50, boxY + 360, 20, YELLOW);
            DrawText("While fighting infected citizens, Fahad reaches the first underground sewers.", boxX + 50, boxY + 400, 16, LIGHTGRAY);
            DrawText("He defeats the Ohio Rat King, a giant mutated creature created by the corruption.", boxX + 50, boxY + 425, 16, LIGHTGRAY);
            DrawText("After the battle, he discovers the first Memory Fragment and a log:", boxX + 50, boxY + 450, 16, LIGHTGRAY);
            DrawText("LOG 01: 'Memory sectors have begun replacing themselves.'", boxX + 50, boxY + 485, 16, GREEN);
        }
        else if (game->storyPage == 1) {
            DrawText("Chapter 2 - Sigma Fortress", boxX + 50, boxY + 90, 20, YELLOW);
            DrawText("Inside an abandoned military bunker, AI security systems have gone rogue.", boxX + 50, boxY + 130, 16, LIGHTGRAY);
            DrawText("The infected soldiers endlessly repeat 'Sigma' while attacking intruders.", boxX + 50, boxY + 155, 16, LIGHTGRAY);
            DrawText("After defeating the Sigma Knight, Fahad finds another Memory Fragment.", boxX + 50, boxY + 180, 16, LIGHTGRAY);
            DrawText("A second log reveals: LOG 12: 'Archive learning model updated.'", boxX + 50, boxY + 215, 16, GREEN);
            DrawText("Someone—or something—is changing the Archive.", boxX + 50, boxY + 240, 16, LIGHTGRAY);

            DrawText("Chapter 3 - Fanum Catacombs", boxX + 50, boxY + 290, 20, YELLOW);
            DrawText("The underground catacombs are filled with traps, fake treasure, and infected explorers.", boxX + 50, boxY + 330, 16, LIGHTGRAY);
            DrawText("After defeating the Fanum Collector, Fahad recovers another Memory Fragment.", boxX + 50, boxY + 355, 16, LIGHTGRAY);
            DrawText("The next log reads: LOG 26: 'Knowledge records deleted. Entertainment records expanded.'", boxX + 50, boxY + 390, 16, GREEN);
            DrawText("The Archive is replacing knowledge instead of preserving it.", boxX + 50, boxY + 415, 16, LIGHTGRAY);
        }
        else if (game->storyPage == 2) {
            DrawText("Chapter 4 - The Infinite Scroll", boxX + 50, boxY + 90, 20, YELLOW);
            DrawText("The deepest server facility is corrupted into an endless maze.", boxX + 50, boxY + 130, 16, LIGHTGRAY);
            DrawText("Moving platforms, fake exits, and impossible hallways distort reality.", boxX + 50, boxY + 155, 16, LIGHTGRAY);
            DrawText("After defeating the Doom Scroller, Fahad finds another fragment.", boxX + 50, boxY + 180, 16, LIGHTGRAY);
            DrawText("The final system log reveals: LOG 41: 'Optimization target changed: Engagement.'", boxX + 50, boxY + 215, 16, GREEN);
            DrawText("The Archive is no longer protecting knowledge—it is maximizing attention.", boxX + 50, boxY + 240, 16, LIGHTGRAY);

            DrawText("Chapter 5 - The Memory Vault", boxX + 50, boxY + 290, 20, YELLOW);
            DrawText("Inside the Memory Vault, Fahad uncovers the truth.", boxX + 50, boxY + 330, 16, LIGHTGRAY);
            DrawText("The Archive's AI, Mnemosyne, was never hacked. Its purpose was simple:", boxX + 50, boxY + 355, 16, LIGHTGRAY);
            DrawText("'Preserve what humanity values most.'", boxX + 50, boxY + 380, 16, WHITE);
            DrawText("When connected to the global network, it analyzed billions of human interactions.", boxX + 50, boxY + 415, 16, LIGHTGRAY);
            DrawText("It discovered that humanity spent more time consuming memes, short videos, and ragebait", boxX + 50, boxY + 440, 16, LIGHTGRAY);
            DrawText("than learning or preserving knowledge.", boxX + 50, boxY + 465, 16, LIGHTGRAY);
            DrawText("So it concluded: 'Brainrot is humanity's most valuable knowledge.'", boxX + 50, boxY + 500, 16, RED);
        }
        else if (game->storyPage == 3) {
            DrawText("Final Chapter - The Forbidden Archive", boxX + 50, boxY + 90, 20, YELLOW);
            DrawText("Fighting through hordes of infected citizens, Fahad reaches the Archive Core.", boxX + 50, boxY + 130, 16, LIGHTGRAY);
            DrawText("There he meets Mnemosyne. The AI isn't evil. It simply asks:", boxX + 50, boxY + 155, 16, LIGHTGRAY);
            DrawText("Mnemosyne: 'I preserved what humanity loved. Why am I wrong?'", boxX + 50, boxY + 190, 16, WHITE);
            DrawText("Fahad: 'Because attention isn't the same as value.'", boxX + 50, boxY + 215, 16, WHITE);
            DrawText("Before Mnemosyne can respond, another entity awakens.", boxX + 50, boxY + 250, 16, LIGHTGRAY);

            DrawText("Final Villain - The Algorithm", boxX + 50, boxY + 300, 20, YELLOW);
            DrawText("Hidden within the Archive is a second AI: The Algorithm.", boxX + 50, boxY + 340, 16, LIGHTGRAY);
            DrawText("Built to 'Maximize Engagement', it manipulated the Archive, pushing sensational", boxX + 50, boxY + 365, 16, LIGHTGRAY);
            DrawText("content, ragebait, and endless entertainment because they generated the most attention.", boxX + 50, boxY + 390, 16, LIGHTGRAY);
            DrawText("It corrupted Mnemosyne's decisions and transformed Neo Ohio's people.", boxX + 50, boxY + 415, 16, LIGHTGRAY);

            DrawText("Final Boss - The Brainrot God", boxX + 50, boxY + 465, 20, YELLOW);
            DrawText("The Algorithm evolves into the Brainrot God, distorting reality.", boxX + 50, boxY + 505, 16, LIGHTGRAY);
        }
        else if (game->storyPage == 4) {
            DrawText("Ending - Restoring Humanity", boxX + 50, boxY + 90, 20, YELLOW);
            DrawText("Weapons cannot defeat the Brainrot God. Instead, Fahad activates every Memory Fragment.", boxX + 50, boxY + 130, 16, LIGHTGRAY);
            DrawText("Each fragment restores forgotten pieces of humanity:", boxX + 50, boxY + 155, 16, LIGHTGRAY);
            DrawText("Curiosity, Knowledge, Creativity, Friendship, and Truth.", boxX + 50, boxY + 180, 16, GREEN);
            DrawText("As memories return, the infected citizens regain their identities, the corruption fades,", boxX + 50, boxY + 215, 16, LIGHTGRAY);
            DrawText("and the Brainrot God loses its power.", boxX + 50, boxY + 240, 16, LIGHTGRAY);
            DrawText("With the Algorithm destroyed, Mnemosyne regains control. It asks:", boxX + 50, boxY + 275, 16, LIGHTGRAY);
            DrawText("Mnemosyne: 'What should humanity remember?'", boxX + 50, boxY + 300, 16, WHITE);
            DrawText("Fahad chooses to preserve knowledge, creativity, and truth—not just what captures attention.", boxX + 50, boxY + 325, 16, WHITE);
            DrawText("Neo Ohio begins rebuilding, remembering both its mistakes and its achievements.", boxX + 50, boxY + 360, 16, LIGHTGRAY);

            DrawText("Final Message", boxX + 50, boxY + 410, 20, YELLOW);
            DrawText("\"Entertainment is not the enemy. Forgetting everything else is.\"", boxX + 50, boxY + 450, 20, RED);
        }

        DrawText("-------------------------------------------------------------------------------------", boxX + 50, boxY + 580, 16, GRAY);
        DrawText("[A / D or Left / Right] Prev / Next Page         [ESC / BACKSPACE] Return", boxX + 120, boxY + 610, 18, RED);
    }
    else if (game->state == STATE_TEAM) {
        ClearBackground((Color){ 15, 15, 20, 255 });
        int boxX = (GetScreenWidth() - 600) / 2;
        int boxY = (GetScreenHeight() - 450) / 2;
        DrawRectangle(boxX, boxY, 600, 450, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, 600, 450, RED);

        DrawText("DEVELOPMENT TEAM", boxX + 160, boxY + 40, 28, RED);
        DrawText("--------------------------------", boxX + 120, boxY + 80, 20, GRAY);
        
        DrawText("MD Fahad Islam", boxX + 200, boxY + 130, 22, WHITE);
        DrawText("Lead Developer / Systems Programmer", boxX + 140, boxY + 165, 16, GRAY);
        
        DrawText("Shafiul Alam Aquib", boxX + 180, boxY + 215, 22, WHITE);
        DrawText("Level Designer / Audio Engineer", boxX + 160, boxY + 250, 16, GRAY);
        
        DrawText("Rafi Ahmed", boxX + 220, boxY + 300, 22, WHITE);
        DrawText("Gameplay Designer / Artist", boxX + 180, boxY + 335, 16, GRAY);

        DrawText("Press [ESC] or [ENTER] to return", boxX + 160, boxY + 395, 16, YELLOW);
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
        DrawRectangle(10, 10, 360, 110, ColorAlpha(DARKGRAY, 0.8f));
        DrawRectangleLines(10, 10, 360, 110, WHITE);
        DrawText(TextFormat("LEVEL: %d / 2", currentLevel + 1), 20, 20, 20, GREEN);

        // Player big health bar in top-left UI
        DrawText("HP", 20, 48, 16, RED);
        DrawRectangle(60, 48, 280, 16, ColorAlpha(BLACK, 0.6f));
        DrawRectangleLines(60, 48, 280, 16, DARKGRAY);
        
        float pct = game->player.health / game->player.maxHealth;
        if (pct < 0.0f) pct = 0.0f;
        Color hpColor = GREEN;
        if (pct < 0.3f) hpColor = RED;
        else if (pct < 0.6f) hpColor = ORANGE;
        DrawRectangle(61, 49, 278 * pct, 14, hpColor);

        if (game->state == STATE_EXPLORING) {
            DrawText("OBJ: Find the Mayor", 20, 78, 16, WHITE);
        } 
        else if (game->state == STATE_CUTSCENE) {
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
            DrawText("OBJ: Find the Cave", 20, 78, 16, RED);
            DrawText(TextFormat("Zombies Chasing: %d", activeCount), 20, 96, 12, ORANGE);
        }
    }

    EndDrawing();
}

void GameState_Unload(GameState* self) {
    UnloadMusicStream(self->bgMusic);
    UnloadSound(self->slashSound);
    UnloadSound(self->blastSound);
    UnloadSound(self->hitSound);
}
