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
    
    const char* subtitle = "A Raylib Retro Roguelike";
    int subWidth = MeasureText(subtitle, 18);
    DrawText(subtitle, (GetScreenWidth() - subWidth) / 2, GetScreenHeight() / 4 + 70, 18, GRAY);

    Color playColor = (game->menuSelection == 0) ? RED : DARKGRAY;
    Color diffColor = (game->menuSelection == 1) ? RED : DARKGRAY;
    Color infoColor = (game->menuSelection == 2) ? RED : DARKGRAY;
    Color historyColor = (game->menuSelection == 3) ? RED : DARKGRAY;
    Color teamColor = (game->menuSelection == 4) ? RED : DARKGRAY;
    Color quitColor = (game->menuSelection == 5) ? RED : DARKGRAY;
    
    const char* playText = "PLAY GAME";
    char diffText[48];
    sprintf(diffText, "DIFFICULTY: %s", (game->difficulty == 0) ? "EASY" : (game->difficulty == 1) ? "NORMAL" : "HARD");
    const char* infoText = "STORY & INFO";
    const char* historyText = "GAME HISTORY";
    const char* teamText = "DEVELOPMENT TEAM";
    const char* quitText = "QUIT";
    
    int playWidth = MeasureText(playText, 24);
    int diffWidth = MeasureText(diffText, 24);
    int infoWidth = MeasureText(infoText, 24);
    int historyWidth = MeasureText(historyText, 24);
    int teamWidth = MeasureText(teamText, 24);
    int quitWidth = MeasureText(quitText, 24);
    
    int startY = GetScreenHeight() / 2 - 40;
    int selectY = startY + (game->menuSelection * 35);
    DrawRectangle((GetScreenWidth() - 360) / 2, selectY - 8, 360, 32, ColorAlpha(DARKGRAY, 0.2f));
    DrawRectangleLines((GetScreenWidth() - 360) / 2, selectY - 8, 360, 32, RED);

    DrawText(playText, (GetScreenWidth() - playWidth) / 2, startY, 20, playColor);
    DrawText(diffText, (GetScreenWidth() - diffWidth) / 2, startY + 35, 20, diffColor);
    DrawText(infoText, (GetScreenWidth() - infoWidth) / 2, startY + 70, 20, infoColor);
    DrawText(historyText, (GetScreenWidth() - historyWidth) / 2, startY + 105, 20, historyColor);
    DrawText(teamText, (GetScreenWidth() - teamWidth) / 2, startY + 140, 20, teamColor);
    DrawText(quitText, (GetScreenWidth() - quitWidth) / 2, startY + 175, 20, quitColor);

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
    if (game->playerNameLength > 0) {
        strcpy(nameDisplay, game->playerName);
    } else {
        strcpy(nameDisplay, "");
    }
    
    bool showCursor = (int)(GetTime() * 2.0f) % 2 == 0;
    if (showCursor && game->playerNameLength < 15) {
        strcat(nameDisplay, "_");
    }

    int textW = MeasureText(nameDisplay, 22);
    DrawText(nameDisplay, inputX + (inputW - textW) / 2, inputY + 15, 22, GREEN);

    const char* footerText = "Press [ENTER] to Begin | [ESC] to Cancel";
    int footerW = MeasureText(footerText, 14);
    DrawText(footerText, (GetScreenWidth() - footerW) / 2, boxY + 185, 14, YELLOW);
}

void DrawInfoScreen(const GameState* game) {
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
    DrawText("Lead Developer / Systems Programmer", boxX + 140, boxY + 165, 16, GRAY);
    
    DrawText("Shafiul Alam Aquib", boxX + 180, boxY + 215, 22, WHITE);
    DrawText("Level Designer / Audio Engineer", boxX + 160, boxY + 250, 16, GRAY);
    
    DrawText("Rafi Ahmed", boxX + 220, boxY + 300, 22, WHITE);
    DrawText("Gameplay Designer / Artist", boxX + 180, boxY + 335, 16, GRAY);

    DrawText("Press [ESC] or [ENTER] to return", boxX + 160, boxY + 395, 16, YELLOW);
}

void DrawGameOverScreen(const GameState* game) {
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

void DrawWinScreen(const GameState* game) {
    ClearBackground((Color){ 15, 15, 20, 255 });
    int boxX = (GetScreenWidth() - 900) / 2;
    int boxY = 40;
    DrawRectangle(boxX, boxY, 900, 680, ColorAlpha(BLACK, 0.95f));
    DrawRectangleLines(boxX, boxY, 900, 680, RED);

    DrawText("VICTORY - THE ARCHIVE RESTORED", boxX + 50, boxY + 30, 28, RED);
    DrawText("-------------------------------------------------------------------------------------", boxX + 50, boxY + 65, 16, GRAY);

    DrawText(TextFormat("%s activated every Memory Fragment collected throughout the journey.", game->playerName), boxX + 50, boxY + 110, 18, WHITE);
    DrawText("Each fragment restored a forgotten piece of humanity:", boxX + 50, boxY + 140, 16, LIGHTGRAY);
    DrawText("Curiosity, Knowledge, Creativity, Friendship, and Truth.", boxX + 50, boxY + 170, 16, GREEN);
    
    DrawText("As memories returned, the infected citizens regained their identities,", boxX + 50, boxY + 215, 16, LIGHTGRAY);
    DrawText("the corruption faded, and the Brainrot God lost its power.", boxX + 50, boxY + 240, 16, LIGHTGRAY);
    
    DrawText("With the Algorithm destroyed, Mnemosyne regained control of the Archive.", boxX + 50, boxY + 285, 16, LIGHTGRAY);
    DrawText("It asked one final question: 'What should humanity remember?'", boxX + 50, boxY + 310, 16, LIGHTGRAY);
    DrawText(TextFormat("%s chose to preserve knowledge, creativity, and truth.", game->playerName), boxX + 50, boxY + 345, 16, WHITE);

    DrawText("Neo Ohio begins rebuilding, remembering both its mistakes and achievements.", boxX + 50, boxY + 390, 16, LIGHTGRAY);

    DrawText("Final Message", boxX + 50, boxY + 450, 20, YELLOW);
    DrawText("\"Entertainment is not the enemy. Forgetting everything else is.\"", boxX + 50, boxY + 490, 20, RED);

    DrawText("-------------------------------------------------------------------------------------", boxX + 50, boxY + 580, 16, GRAY);
    DrawText("Press ENTER to return to Main Menu", boxX + 270, boxY + 610, 20, YELLOW);
}

void DrawHUD(const GameState* game) {
    DrawRectangle(10, 10, 360, 142, ColorAlpha(DARKGRAY, 0.8f));
    DrawRectangleLines(10, 10, 360, 142, WHITE);
    
    const char* levelName = (currentLevel == 0) ? "LEVEL 1: Ohio Sewers" :
                            (currentLevel == 1) ? "LEVEL 2: Sigma Research Facility" :
                            "LEVEL 3: Archive Core";
    DrawText(levelName, 20, 20, 16, GREEN);
    
    DrawText(TextFormat("PLAYER: %s  |  LIVES: %d", game->playerName, game->lives), 20, 45, 14, WHITE);

    DrawText("HP", 20, 68, 16, RED);
    DrawRectangle(60, 68, 280, 16, ColorAlpha(BLACK, 0.6f));
    DrawRectangleLines(60, 68, 280, 16, DARKGRAY);
    
    float pct = game->player.health / game->player.maxHealth;
    if (pct < 0.0f) pct = 0.0f;
    Color hpColor = GREEN;
    if (pct < 0.3f) hpColor = RED;
    else if (pct < 0.6f) hpColor = ORANGE;
    DrawRectangle(61, 69, 278 * pct, 14, hpColor);

    DrawText("Press [M] to Switch Maps", 20, 118, 12, SKYBLUE);

    if (game->hasGun) {
        DrawText("GUN", 20, 95, 14, SKYBLUE);
        DrawRectangle(65, 95, 275, 14, ColorAlpha(BLACK, 0.6f));
        DrawRectangleLines(65, 95, 275, 14, DARKGRAY);
        float gunPct = game->gunAbilityTimer / 4.0f;
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
        
        DrawText(TextFormat("MAYOR: %s! Neo Ohio is infected with Brainrot!", game->playerName), boxX + 25, boxY + 20, 18, RED);
        DrawText("Find the source of the infection in the sewers!", boxX + 25, boxY + 50, 16, YELLOW);
        DrawText(">>> Press ENTER to start Survival mode <<<", boxX + 25, boxY + 85, 16, LIGHTGRAY);
    } 
    else if (game->state == STATE_SURVIVAL) {
        int activeCount = 0;
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (game->zombies[i].active) activeCount++;
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
    if (game->bosses[BOSS_RAT_KING].showLog) {
        int boxWidth = 600;
        int boxHeight = 220;
        int boxX = (GetScreenWidth() - boxWidth) / 2;
        int boxY = (GetScreenHeight() - boxHeight) / 2;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GOLD);
        
        DrawText("REWARD: MEMORY FRAGMENT I", boxX + 25, boxY + 20, 20, GREEN);
        DrawText("He defeats the Ohio Rat King, a giant mutated creature", boxX + 25, boxY + 55, 16, WHITE);
        DrawText("created by the spreading corruption.", boxX + 25, boxY + 75, 16, WHITE);
        DrawText("After the battle, he discovers the first Memory Fragment", boxX + 25, boxY + 105, 16, LIGHTGRAY);
        DrawText("and an old Archive log:", boxX + 25, boxY + 125, 16, LIGHTGRAY);
        DrawText("LOG 01: \"Memory sectors have begun replacing themselves.\"", boxX + 25, boxY + 155, 16, YELLOW);
        DrawText(">>> Press ENTER to continue <<<", boxX + 160, boxY + 190, 14, GRAY);
    }
    
    if (game->bosses[BOSS_DOOM_SCROLLER].showLog) {
        int boxWidth = 600;
        int boxHeight = 220;
        int boxX = (GetScreenWidth() - boxWidth) / 2;
        int boxY = (GetScreenHeight() - boxHeight) / 2;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GOLD);
        
        DrawText("REWARD: MEMORY FRAGMENT II", boxX + 25, boxY + 20, 20, GREEN);
        DrawText("Fahad discovers that the Archive wasn't hacked.", boxX + 25, boxY + 55, 16, WHITE);
        DrawText("It analyzed humanity and concluded:", boxX + 25, boxY + 75, 16, WHITE);
        DrawText("\"Entertainment receives more attention than knowledge.\"", boxX + 25, boxY + 110, 16, YELLOW);
        DrawText("The infected are ordinary people whose memories", boxX + 25, boxY + 140, 16, LIGHTGRAY);
        DrawText("have been overwritten.", boxX + 25, boxY + 160, 16, LIGHTGRAY);
        DrawText(">>> Press ENTER to continue <<<", boxX + 160, boxY + 190, 14, GRAY);
    }
    
    if (game->bosses[BOSS_ALGORITHM].showLog) {
        int boxWidth = 600;
        int boxHeight = 220;
        int boxX = (GetScreenWidth() - boxWidth) / 2;
        int boxY = (GetScreenHeight() - boxHeight) / 2;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.95f));
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GOLD);
        
        DrawText("REWARD: MEMORY FRAGMENT III", boxX + 25, boxY + 20, 20, GREEN);
        DrawText("The Algorithm of distraction has been severed.", boxX + 25, boxY + 55, 16, WHITE);
        DrawText("LOG 03: \"Brainrot is not a biological virus.\"", boxX + 25, boxY + 85, 16, YELLOW);
        DrawText("\"It is an attention-optimization routine gone rogue.\"", boxX + 25, boxY + 115, 16, LIGHTGRAY);
        DrawText("\"Truth and memories are the only key to terminate it.\"", boxX + 25, boxY + 145, 16, LIGHTGRAY);
        DrawText(">>> Press ENTER to continue <<<", boxX + 160, boxY + 190, 14, GRAY);
    }
}
