#include "raylib.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

void DrawTile(Texture2D tileset, int tilesPerRow, int tileID, float x, float y) {
    float xco = (float)((tileID % tilesPerRow) * TILE_SIZE);
    float yco = (float)((tileID / tilesPerRow) * TILE_SIZE);
    Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
    Rectangle dest = { x, y, (float)TILE_PX, (float)TILE_PX };
    DrawTexturePro(tileset, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

void DrawCustomText(const GameState* game, const char* text, float posX, float posY, float fontSize, Color color) {
    Vector2 pos = { posX, posY };
    DrawTextEx(game->gameFont, text, pos, fontSize, 1.0f, color);
}

float MeasureCustomText(const GameState* game, const char* text, float fontSize) {
    Vector2 size = MeasureTextEx(game->gameFont, text, fontSize, 1.0f);
    return size.x;
}

void GameHistory_SaveEntry(const char* name, int levelCleared, bool victory) {
    FILE* file = fopen("history.txt", "a");
    if (file != NULL) {
        if (victory) {
            fprintf(file, "%s - Level %d Cleared (Victory)\n", name, levelCleared);
        } else {
            fprintf(file, "%s - Level %d Cleared\n", name, levelCleared);
        }
        fclose(file);
    }
}

void GameHistory_Load(GameHistory* history) {
    history->count = 0;
    FILE* file = fopen("history.txt", "r");
    if (file == NULL) return;

    char line[128];
    char tempEntries[100][64];
    int totalCount = 0;
    while (fgets(line, sizeof(line), file) && totalCount < 100) {
        line[strcspn(line, "\r\n")] = '\0';
        strncpy(tempEntries[totalCount], line, 63);
        tempEntries[totalCount][63] = '\0';
        totalCount++;
    }
    fclose(file);

    int start = totalCount - 1;
    for (int i = 0; i < MAX_HISTORY_ENTRIES && start >= 0; i++, start--) {
        strcpy(history->entries[i], tempEntries[start]);
        history->count++;
    }
}

void DrawMenuScreen(const GameState* game) {
    ClearBackground((Color){ 15, 15, 20, 255 });
    
    const char* titleText = "THE GAMEROT";
    int titleWidth = MeasureText(titleText, 56);
    DrawText(titleText, (GetScreenWidth() - titleWidth) / 2 + 4, GetScreenHeight() / 4 + 4, 56, BLACK);
    DrawText(titleText, (GetScreenWidth() - titleWidth) / 2, GetScreenHeight() / 4, 56, (Color){ 180, 0, 0, 255 });
    

    Color playColor = (game->menu.menuSelection == 0) ? RED : DARKGRAY;
    Color diffColor = (game->menu.menuSelection == 1) ? RED : DARKGRAY;
    Color historyColor = (game->menu.menuSelection == 2) ? RED : DARKGRAY;
    Color teamColor = (game->menu.menuSelection == 3) ? RED : DARKGRAY;
    Color quitColor = (game->menu.menuSelection == 4) ? RED : DARKGRAY;
    
    const char* playText = "PLAY GAME";
    char diffText[48];
    sprintf(diffText, "DIFFICULTY: %s", (game->menu.difficulty == 0) ? "EASY" : (game->menu.difficulty == 1) ? "NORMAL" : "HARD");
    const char* historyText = "GAME HISTORY";
    const char* teamText = "DEVELOPMENT TEAM";
    const char* quitText = "QUIT";
    
    int playWidth = MeasureText(playText, 24);
    int diffWidth = MeasureText(diffText, 24);
    int historyWidth = MeasureText(historyText, 24);
    int teamWidth = MeasureText(teamText, 24);
    int quitWidth = MeasureText(quitText, 24);
    
    int startY = GetScreenHeight() / 2 - 40;
    int selectY = startY + (game->menu.menuSelection * 35);
    DrawRectangle((GetScreenWidth() - 360) / 2, selectY - 8, 360, 32, ColorAlpha(DARKGRAY, 0.2f));
    DrawRectangleLines((GetScreenWidth() - 360) / 2, selectY - 8, 360, 32, RED);

    DrawText(playText, (GetScreenWidth() - playWidth) / 2, startY, 20, playColor);
    DrawText(diffText, (GetScreenWidth() - diffWidth) / 2, startY + 35, 20, diffColor);
    DrawText(historyText, (GetScreenWidth() - historyWidth) / 2, startY + 70, 20, historyColor);
    DrawText(teamText, (GetScreenWidth() - teamWidth) / 2, startY + 105, 20, teamColor);
    DrawText(quitText, (GetScreenWidth() - quitWidth) / 2, startY + 140, 20, quitColor);

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

void DrawNamePromptScreen(const GameState* game) {
    ClearBackground((Color){ 15, 15, 20, 255 });

    int boxWidth = 500;
    int boxHeight = 250;
    int boxX = (GetScreenWidth() - boxWidth) / 2;
    int boxY = (GetScreenHeight() - boxHeight) / 2;

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.85f));
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, RED);

    const char* promptHeader = "ENTER PLAYER NAME";
    int headerWidth = MeasureText(promptHeader, 24);
    DrawText(promptHeader, (GetScreenWidth() - headerWidth) / 2, boxY + 30, 24, RED);
    DrawText("--------------------------------", boxX + 50, boxY + 65, 16, GRAY);

    int inputW = 360;
    int inputH = 50;
    int inputX = (GetScreenWidth() - inputW) / 2;
    int inputY = boxY + 100;
    DrawRectangle(inputX, inputY, inputW, inputH, ColorAlpha(DARKGRAY, 0.3f));
    DrawRectangleLines(inputX, inputY, inputW, inputH, GRAY);

    char nameDisplay[32];
    if (game->playerInfo.nameLength > 0) {
        strcpy(nameDisplay, game->playerInfo.name);
    } else {
        strcpy(nameDisplay, "");
    }
    
    bool showCursor = (int)(GetTime() * 2.0f) % 2 == 0;
    if (showCursor && game->playerInfo.nameLength < 15) {
        strcat(nameDisplay, "_");
    }

    int textW = MeasureText(nameDisplay, 22);
    DrawText(nameDisplay, inputX + (inputW - textW) / 2, inputY + 15, 22, GREEN);

    const char* footerText = "Press [ENTER] to Begin | [ESC] to Cancel";
    int footerW = MeasureText(footerText, 14);
    DrawText(footerText, (GetScreenWidth() - footerW) / 2, boxY + 185, 14, YELLOW);
}



void DrawHistoryScreen(const GameState* game) {
    ClearBackground((Color){ 15, 15, 20, 255 });
    int boxX = (GetScreenWidth() - 600) / 2;
    int boxY = (GetScreenHeight() - 500) / 2;
    DrawRectangle(boxX, boxY, 600, 500, ColorAlpha(BLACK, 0.95f));
    DrawRectangleLines(boxX, boxY, 600, 500, RED);

    DrawText("GAME RUN HISTORY", boxX + 165, boxY + 40, 28, RED);
    DrawText("-------------------------------------", boxX + 105, boxY + 80, 20, GRAY);

    GameHistory history;
    GameHistory_Load(&history);

    if (history.count == 0) {
        DrawText("No runs recorded yet.", boxX + 180, boxY + 220, 20, GRAY);
        DrawText("Play the game and clear levels to build history!", boxX + 80, boxY + 260, 16, LIGHTGRAY);
    } else {
        for (int i = 0; i < history.count; i++) {
            Color entryColor = LIGHTGRAY;
            if (strstr(history.entries[i], "Victory") != NULL) {
                entryColor = GREEN;
            }
            DrawText(history.entries[i], boxX + 60, boxY + 130 + (i * 35), 18, entryColor);
        }
    }

    DrawText("Press [ESC] or [ENTER] to return", boxX + 140, boxY + 440, 16, YELLOW);
}

void DrawTeamScreen(const GameState* game) {
    ClearBackground((Color){ 15, 15, 20, 255 });
    int boxX = (GetScreenWidth() - 600) / 2;
    int boxY = (GetScreenHeight() - 450) / 2;
    DrawRectangle(boxX, boxY, 600, 450, ColorAlpha(BLACK, 0.95f));
    DrawRectangleLines(boxX, boxY, 600, 450, RED);

    DrawText("DEVELOPMENT TEAM", boxX + 160, boxY + 40, 28, RED);
    DrawText("--------------------------------", boxX + 120, boxY + 80, 20, GRAY);
    
    DrawText("MD Fahad Islam", boxX + 200, boxY + 130, 22, WHITE);
    
    DrawText("Shafiul Alam Aquib", boxX + 180, boxY + 215, 22, WHITE);
    
    DrawText("Rafi Ahmed", boxX + 220, boxY + 300, 22, WHITE);

    DrawText("Press [ESC] or [ENTER] to return", boxX + 160, boxY + 395, 16, YELLOW);
}

void DrawGameOverScreen(const GameState* game) {
    int boxW = 300, boxH = 200;
    int bx = (GetScreenWidth() - boxW) / 2;
    int by = (GetScreenHeight() - boxH) / 2;
    DrawRectangle(bx, by, boxW, boxH, ColorAlpha(BLACK, 0.9f));
    DrawRectangleLines(bx, by, boxW, boxH, (Color){ 139, 0, 0, 255 });
    
    DrawText("DEFEAT", bx + 100, by + 40, 24, (Color){ 139, 0, 0, 255 });
    
    Color retryColor = (game->menu.gameOverSelection == 0) ? (Color){ 139, 0, 0, 255 } : DARKGRAY;
    Color quitColor = (game->menu.gameOverSelection == 1) ? (Color){ 139, 0, 0, 255 } : DARKGRAY;
    
    DrawText("RETRY", bx + 115, by + 100, 20, retryColor);
    DrawText("QUIT", bx + 125, by + 140, 20, quitColor);
}

void DrawWinScreen(const GameState* game) {
    ClearBackground((Color){ 15, 15, 20, 255 });
    int boxW = 500;
    int boxH = 200;
    int boxX = (GetScreenWidth() - boxW) / 2;
    int boxY = (GetScreenHeight() - boxH) / 2;
    DrawRectangle(boxX, boxY, boxW, boxH, ColorAlpha(BLACK, 0.95f));
    DrawRectangleLines(boxX, boxY, boxW, boxH, RED);

    DrawText("VICTORY!", boxX + 180, boxY + 40, 28, RED);
    DrawText("The infection has been cured.", boxX + 100, boxY + 90, 18, LIGHTGRAY);
    DrawText("Press ENTER to return to Main Menu", boxX + 70, boxY + 140, 18, YELLOW);
}

void DrawHUD(const GameState* game) {
    int hudHeight = (currentLevel == 3) ? 162 : 142;
    DrawRectangle(10, 10, 360, hudHeight, ColorAlpha(DARKGRAY, 0.8f));
    DrawRectangleLines(10, 10, 360, hudHeight, WHITE);
    
    const char* levelName = (currentLevel == 0) ? "LEVEL 1: Ohio Sewers" :
                            (currentLevel == 1) ? "LEVEL 2: Sigma Research Facility" :
                            "LEVEL 3: Archive Core";
    DrawText(levelName, 20, 20, 16, GREEN);
    
    DrawText(TextFormat("PLAYER: %s  |  LIVES: %d", game->playerInfo.name, game->playerInfo.lives), 20, 45, 14, WHITE);

    DrawText("HP", 20, 68, 16, RED);
    DrawRectangle(60, 68, 280, 16, ColorAlpha(BLACK, 0.6f));
    DrawRectangleLines(60, 68, 280, 16, DARKGRAY);
    
    float pct = game->player.health / game->player.maxHealth;
    if (pct < 0.0f) pct = 0.0f;
    Color hpColor = GREEN;
    if (pct < 0.3f) hpColor = RED;
    else if (pct < 0.6f) hpColor = ORANGE;
    DrawRectangle(61, 69, 278 * pct, 14, hpColor);

    DrawText(TextFormat("KILLS: %d", game->enemies.killedCount), 20, 118, 14, ORANGE);

    if (currentLevel == 3) {
        int minutes = (int)(game->levelInfo.escapeTimer) / 60;
        int seconds = (int)(game->levelInfo.escapeTimer) % 60;
        Color timerColor = (game->levelInfo.escapeTimer < 30.0f) ? RED : ORANGE;
        DrawText(TextFormat("COLLAPSE IN: %02d:%02d", minutes, seconds), 20, 138, 14, timerColor);
    }

    if (game->playerInfo.hasGun) {
        DrawText("GUN", 20, 95, 14, SKYBLUE);
        DrawRectangle(65, 95, 275, 14, ColorAlpha(BLACK, 0.6f));
        DrawRectangleLines(65, 95, 275, 14, DARKGRAY);
        float gunPct = game->playerInfo.gunAbilityTimer / 4.0f;
        if (gunPct < 0.0f) gunPct = 0.0f;
        DrawRectangle(66, 96, 273 * gunPct, 12, SKYBLUE);
    }
    else if (game->state == STATE_EXPLORING) {
        DrawText("OBJ: Find the Mayor", 20, 95, 14, YELLOW);
    } 
    else if (game->state == STATE_CUTSCENE) {
        int boxWidth = 520;
        int boxHeight = 130;
        int boxX = (GetScreenWidth() - boxWidth) / 2;
        int boxY = GetScreenHeight() - boxHeight - 40;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.9f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, RED);
        
        DrawText(TextFormat("MAYOR: %s! Neo Ohio is infected with Brainrot!", game->playerInfo.name), boxX + 25, boxY + 20, 18, RED);
        DrawText("Find the source of the infection in the sewers!", boxX + 25, boxY + 50, 16, YELLOW);
        DrawText(">>> Press ENTER to start Survival mode <<<", boxX + 25, boxY + 85, 16, LIGHTGRAY);
    } 
    else if (game->state == STATE_SURVIVAL) {
        int activeCount = 0;
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (game->enemies.list[i].active) activeCount++;
        }
        static const char* objs[] = {
            "OBJ: Defeat Ohio Rat King",
            "OBJ: Defeat Doom Scroller",
            "OBJ: Defeat Brainrot God",
            "OBJ: Escape the Collapsing Center!"
        };
        const char* obj = (currentLevel >= 0 && currentLevel < 4) ? objs[currentLevel] : "";
        DrawText(obj, 20, 95, 14, YELLOW);
        DrawText(TextFormat("(Enemies: %d)", activeCount), 220, 95, 14, ORANGE);
    }
}

void DrawStoryOverlays(const GameState* game) {
    int boxWidth = 500;
    int boxHeight = 130;
    int boxX = (GetScreenWidth() - boxWidth) / 2;
    int boxY = (GetScreenHeight() - boxHeight) / 2;

    if (game->enemies.status[BOSS_RAT_KING].showLog) {
        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GOLD);
        DrawText("REWARD: MEMORY FRAGMENT I", boxX + 25, boxY + 20, 20, GREEN);
        DrawText("LOG 01: \"Memory sectors are replacing themselves.\"", boxX + 25, boxY + 55, 16, YELLOW);
        DrawText(">>> Press ENTER to continue <<<", boxX + 110, boxY + 90, 14, GRAY);
    }
    
    if (game->enemies.status[BOSS_DOOM_SCROLLER].showLog) {
        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GOLD);
        DrawText("REWARD: MEMORY FRAGMENT II", boxX + 25, boxY + 20, 20, GREEN);
        DrawText("LOG 02: \"Attention is chosen over knowledge.\"", boxX + 25, boxY + 55, 16, YELLOW);
        DrawText(">>> Press ENTER to continue <<<", boxX + 110, boxY + 90, 14, GRAY);
    }
    
    if (game->enemies.status[BOSS_ALGORITHM].showLog) {
        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GOLD);
        DrawText("REWARD: MEMORY FRAGMENT III", boxX + 25, boxY + 20, 20, GREEN);
        DrawText("LOG 03: \"Truth and memories can stop Brainrot.\"", boxX + 25, boxY + 55, 16, YELLOW);
        DrawText(">>> Press ENTER to continue <<<", boxX + 110, boxY + 90, 14, GRAY);
    }
}
