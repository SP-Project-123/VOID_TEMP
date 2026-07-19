#include "raylib.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int currentLevel = 0;

#define CUT1_FRAMES 152
#define CUT2_FRAMES 62
#define CUT_CAVE_FRAMES 240
#define ENDSCENE_FRAMES 12

static void UpdatePlayerWeapon(GameState* game, float dt);
static void UpdatePlayerProjectiles(GameState* game, float dt);
static void UpdateMenuState(GameState* game);
static void UpdateNamePromptState(GameState* game);
static void UpdateIntroState(GameState* game, float dt);
static void UpdateExploringState(GameState* game, float dt);
static void UpdateCutsceneState(GameState* game);
static void UpdateSurvivalState(GameState* game, float dt);
static void UpdateGameOverState(GameState* game);
static void UpdateWinState(GameState* game);
static void DrawGameplayWorld(const GameState* game);

void GameState_LoadCutscenes(GameState* game) {
    if (game->cutscene.loaded) return;
    
    BeginDrawing();
    ClearBackground(BLACK);
    DrawCustomText(game, "LOADING CUTSCENE...", (float)GetScreenWidth() / 2.0f - 180.0f, (float)GetScreenHeight() / 2.0f - 20.0f, 24.0f, RED);
    EndDrawing();

    char path[128];
    for (int i = 0; i < CUT1_FRAMES; i++) {
        sprintf(path, "cutscenes/frames/cut1_%03d.png", i + 1);
        game->cutscene.cut1Textures[i] = LoadTexture(path);
    }
    for (int i = 0; i < CUT2_FRAMES; i++) {
        sprintf(path, "cutscenes/frames/cut2_%03d.png", i + 1);
        game->cutscene.cut2Textures[i] = LoadTexture(path);
    }
    for (int i = 0; i < CUT_CAVE_FRAMES; i++) {
        sprintf(path, "cutscenes/frames/cutscene_cave_%03d.png", i + 1);
        game->cutscene.cutCaveTextures[i] = LoadTexture(path);
    }
    for (int i = 0; i < ENDSCENE_FRAMES; i++) {
        sprintf(path, "cutscenes/endscenes/end%d.png", i + 1);
        game->cutscene.endsceneTextures[i] = LoadTexture(path);
    }
    game->cutscene.loaded = true;
}

void GameState_UnloadCutscenes(GameState* game) {
    if (!game->cutscene.loaded) return;
    for (int i = 0; i < CUT1_FRAMES; i++) {
        if (game->cutscene.cut1Textures[i].id > 0) {
            UnloadTexture(game->cutscene.cut1Textures[i]);
            game->cutscene.cut1Textures[i].id = 0;
        }
    }
    for (int i = 0; i < CUT2_FRAMES; i++) {
        if (game->cutscene.cut2Textures[i].id > 0) {
            UnloadTexture(game->cutscene.cut2Textures[i]);
            game->cutscene.cut2Textures[i].id = 0;
        }
    }
    for (int i = 0; i < CUT_CAVE_FRAMES; i++) {
        if (game->cutscene.cutCaveTextures[i].id > 0) {
            UnloadTexture(game->cutscene.cutCaveTextures[i]);
            game->cutscene.cutCaveTextures[i].id = 0;
        }
    }
    for (int i = 0; i < ENDSCENE_FRAMES; i++) {
        if (game->cutscene.endsceneTextures[i].id > 0) {
            UnloadTexture(game->cutscene.endsceneTextures[i]);
            game->cutscene.endsceneTextures[i].id = 0;
        }
    }
    game->cutscene.loaded = false;
}

void GameState_Init(GameState* self) {
    self->audio.bgMusic = LoadMusicStream("resources/bgm.mp3");
    if (FileExists("resources/slash.ogg")) {
        self->audio.slash = LoadSound("resources/slash.ogg");
    } else {
        self->audio.slash = LoadSound("resources/slash.wav");
    }
    self->audio.blast = LoadSound("resources/blast.wav");
    self->audio.hit = LoadSound("resources/hit.wav");
    self->audio.hitTimer = 0.0f;
    self->cutscene.cut1Audio = LoadSound("cutscenes/cut1.wav");
    self->cutscene.cut2Audio = LoadSound("cutscenes/cut2.wav");
    self->cutscene.cutCaveAudio = LoadSound("cutscenes/cutscene_cave.wav");
    self->cutscene.part = 0;
    self->cutscene.time = 0.0f;
    self->cutscene.loaded = false;
    self->levelInfo.startTextTimer = 0.0f;

    self->playerTileset = LoadTexture("tilemap_packed.png");
    self->playerTilesPerRow = self->playerTileset.width / TILE_SIZE;
    if (FileExists("spirites_tilepacked.png")) {
        Image img = LoadImage("spirites_tilepacked.png");
        ImageColorReplace(&img, (Color){34, 35, 35, 255}, BLANK);
        self->spritesTileset = LoadTextureFromImage(img);
        UnloadImage(img);
    } else {
        self->spritesTileset = self->playerTileset;
    }
    self->spritesTilesPerRow = self->spritesTileset.width / TILE_SIZE;

    if (FileExists("C:/Windows/Fonts/consola.ttf")) {
        self->gameFont = LoadFontEx("C:/Windows/Fonts/consola.ttf", 64, NULL, 0);
    } else if (FileExists("C:/Windows/Fonts/lucon.ttf")) {
        self->gameFont = LoadFontEx("C:/Windows/Fonts/lucon.ttf", 64, NULL, 0);
    } else {
        self->gameFont = GetFontDefault();
    }

    self->map.tileset.id = 0;
    self->menu.difficulty = 1;
    self->playerInfo.name[0] = '\0';
    self->playerInfo.nameLength = 0;
    self->playerInfo.lives = PLAYER_INITIAL_LIVES;
    self->playerInfo.checkpointActive = false;
    self->playerInfo.checkpointPosition = (Vector2){ 0, 0 };
    self->playerInfo.checkpointLevel = 0;
    self->playerInfo.checkpointState = STATE_EXPLORING;
    self->menu.gameOverSelection = 0;
    self->menu.menuSelection = 0;
    self->menu.storyPage = 0;
    self->cutscene.targetState = (GameMode)-1;
    self->cutscene.targetLevel = 0;
    for (int i = 0; i < 4; i++) {
        self->enemies.status[i].spawned = false;
        self->enemies.status[i].defeated = false;
        self->enemies.status[i].showLog = false;
        self->enemies.status[i].tileId = 0;
    }
    self->enemies.status[BOSS_RAT_KING].tileId = 23;
    self->enemies.status[BOSS_DOOM_SCROLLER].tileId = 306;
    self->enemies.status[BOSS_ALGORITHM].tileId = 308;
    self->enemies.status[BOSS_BRAINROT_GOD].tileId = 27;
    for (int i = 0; i < 3; i++) {
        self->enemies.fragments[i].activated = false;
        self->enemies.fragments[i].position = (Vector2){0, 0};
        self->enemies.fragments[i].name = "";
    }

    LoadLevel(self, 0);
    self->state = STATE_MENU;

    PlayMusicStream(self->audio.bgMusic);
}

const LevelConfig g_levelConfigs[4] = {
    { "map1.csv",     "tilemap_packed.png",        "OBJ: Find Mayor in Neo Ohio" },
    { "map2.csv",     "spirites_tilepacked.png",   "OBJ: Defeat Ohio Rat King" },
    { "map3.csv",     "tilemap_packed3.png",       "OBJ: Defeat Doom Scroller & Brainrot God" },
    { "finalmap.csv", "finalmap_packed.png",       "OBJ: Escape the Collapsing Center!" }
};

/*
 * LoadLevel - Loads map CSVs, configures tile properties, resets player coordinates,
 * and sets up level-specific spawn points for boss phases and standard enemies.
 */
void LoadLevel(GameState* game, int levelIndex) {
    if (levelIndex < 0 || levelIndex >= 4) return;
    currentLevel = levelIndex;

    // Unload previous map tileset texture from VRAM
    if (game->map.tileset.id > 0) {
        UnloadTexture(game->map.tileset);
        game->map.tileset.id = 0;
    }
    Tilemap_Load(&game->map, g_levelConfigs[levelIndex].csvMap, g_levelConfigs[levelIndex].tilesetPng);

    Player_Init(&game->player);
    if (levelIndex > 0) game->player.hasWeapon = true;

    // Default starting grid positions per level
    int startCol = 5, startRow = 5;
    if (levelIndex == 1) { startCol = 5; startRow = 3; }
    else if (levelIndex == 2) { startCol = 5; startRow = 5; }
    else if (levelIndex == 3) { startCol = 2; startRow = 2; }

    // Fallback search to guarantee player spawns on a valid walkable path tile
    if (!Tilemap_IsWalkable(&game->map, startCol * TILE_PX, startRow * TILE_PX)) {
        bool found = false;
        for (int r = 0; r < MAP_HEIGHT && !found; r++) {
            for (int c = 0; c < MAP_WIDTH && !found; c++) {
                if (Tilemap_IsWalkable(&game->map, c * TILE_PX, r * TILE_PX)) {
                    startCol = c;
                    startRow = r;
                    found = true;
                }
            }
        }
    }

    game->player.position.x = startCol * TILE_PX;
    game->player.position.y = startRow * TILE_PX;
    game->player.gridX = startCol;
    game->player.gridY = startRow;

    game->state = (levelIndex == 0) ? STATE_EXPLORING : STATE_SURVIVAL;

    // Set up Mayor on level 0
    if (levelIndex == 0) {
        game->enemies.killedCount = 0;
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                int tid = game->map.tiles[y][x];
                if (tid == 283 || tid == 306 || tid == 308 || tid == 355) {
                    game->map.tiles[y][x] = 20; //delete duplication on respawn
                }
            }
        }

        game->levelInfo.mayorRow = 10;
        game->levelInfo.mayorCol = 10;
        game->map.tiles[game->levelInfo.mayorRow][game->levelInfo.mayorCol] = 283;
    }

    game->enemies.timer = 0.0f;
    game->enemies.spawnTimer = 0.0f;
    for (int i = 0; i < MAX_ZOMBIES; i++) game->enemies.list[i].active = false;
    for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) game->playerInfo.projectiles[p].active = false;
    for (int e = 0; e < MAX_ENEMY_PROJECTILES; e++) game->enemies.projectiles[e].active = false;

    game->levelInfo.bossRow = -1;
    game->levelInfo.bossCol = -1;
    if (levelIndex == 1) {
        game->levelInfo.bossRow = 10;
        game->levelInfo.bossCol = 10;
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (game->map.tiles[y][x] == 23) {
                    game->levelInfo.bossRow = y;
                    game->levelInfo.bossCol = x;
                    game->map.tiles[y][x] = 17; // Replace static boss tile with walkable floor
                }
            }
        }
        SpawnZombie(game);
        SpawnZombie(game);
        SpawnEnemy(game, ENEMY_RAT_KING, game->levelInfo.bossRow, game->levelInfo.bossCol);
    } else if (levelIndex == 2) {
        game->levelInfo.bossRow = 16;
        game->levelInfo.bossCol = 20;
        SpawnEnemy(game, ENEMY_DOOM_SCROLLER, game->levelInfo.bossRow, game->levelInfo.bossCol);
    } else if (levelIndex == 3) {
        for (int s = 0; s < 6; s++) SpawnZombie(game);
    }

    game->playerInfo.hasGun = false;
    game->playerInfo.gunSpawned = false;
    game->playerInfo.gunSpawnTimer = 4.0f;
    game->playerInfo.gunAbilityTimer = 0.0f;
    game->levelInfo.startTextTimer = 4.0f;
    game->levelInfo.escapeTimer = 120.0f;

    // Spawn potion on each map
    game->levelInfo.potionSpawned = false;
    int potionRow = -1, potionCol = -1;
    for (int attempt = 0; attempt < 200; attempt++) {
        int r = GetRandomValue(2, MAP_HEIGHT - 3);
        int c = GetRandomValue(2, MAP_WIDTH - 3);
        if (GetTileType(game->map.tiles[r][c]) == TILE_GROUND) {
            int dist = abs(c - startCol) + abs(r - startRow);
            if (dist >= 5) {
                potionRow = r;
                potionCol = c;
                game->levelInfo.potionSpawned = true;
                break;
            }
        }
    }
    if (!game->levelInfo.potionSpawned) {
        game->levelInfo.potionRow = 8;
        game->levelInfo.potionCol = 8;
        game->levelInfo.potionSpawned = true;
    } else {
        game->levelInfo.potionRow = potionRow;
        game->levelInfo.potionCol = potionCol;
    }

    game->playerInfo.checkpointPosition = game->player.position;
    game->playerInfo.checkpointLevel = currentLevel;
    game->playerInfo.checkpointState = game->state;
    game->playerInfo.checkpointActive = true;
}

void GameState_TransitionFromCutscene(GameState* game) {
    if (game->cutscene.targetState != (GameMode)-1) {
        if (game->cutscene.targetState == STATE_WIN) {
            game->state = STATE_WIN;
        } else {
            LoadLevel(game, game->cutscene.targetLevel);
            game->state = game->cutscene.targetState;
        }
        game->cutscene.targetState = (GameMode)-1;
    } else {
        LoadLevel(game, 0);
        game->state = STATE_EXPLORING;
        game->playerInfo.lives = PLAYER_INITIAL_LIVES;
    }
    PlayMusicStream(game->audio.bgMusic);
}

// --- State Machine Update Routine ---
static void UpdatePlayerWeapon(GameState* game, float dt) {
    if (IsKeyPressed(KEY_SPACE) && game->player.hasWeapon && !game->player.isAttacking) {
        if (game->player.lightAttackCooldown <= 0.0f) {
            PlaySound(game->audio.slash);
            game->player.isAttacking = true;
            game->player.attackTimer = 0.15f;
            game->player.lightAttackCooldown = 0.33f;

            if (game->playerInfo.hasGun) {
                int projSlot = -1;
                for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) {
                    if (!game->playerInfo.projectiles[p].active) {
                        projSlot = p;
                        break;
                    }
                }
                if (projSlot != -1) {
                    float pSpeed = 350.0f;
                    float startX = game->player.position.x + TILE_PX / 2.0f;
                    float startY = game->player.position.y + TILE_PX / 2.0f;
                    Vector2 vel = { 0, 0 };
                    if (game->player.direction == DIR_UP) vel.y = -pSpeed;
                    else if (game->player.direction == DIR_DOWN) vel.y = pSpeed;
                    else if (game->player.direction == DIR_LEFT) vel.x = -pSpeed;
                    else if (game->player.direction == DIR_RIGHT) vel.x = pSpeed;

                    game->playerInfo.projectiles[projSlot].position = (Vector2){ startX, startY };
                    game->playerInfo.projectiles[projSlot].velocity = vel;
                    game->playerInfo.projectiles[projSlot].active = true;
                }
            }

            float rx = game->player.position.x;
            float ry = game->player.position.y;
            float rw = TILE_PX;
            float rh = TILE_PX;
            if (game->player.direction == DIR_UP) ry -= TILE_PX;
            else if (game->player.direction == DIR_DOWN) ry += TILE_PX;
            else if (game->player.direction == DIR_LEFT) rx -= TILE_PX;
            else if (game->player.direction == DIR_RIGHT) rx += TILE_PX;
            Rectangle attackRec = { rx, ry, rw, rh };

            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (game->enemies.list[i].active) {
                    EnemyProperties props = GetEnemyProperties(game->enemies.list[i].type, currentLevel, game->menu.difficulty);
                    float zSize = TILE_PX * props.scale;
                    float offset = (props.scale - 1.0f) / 2.0f;
                    Rectangle zRec = { game->enemies.list[i].position.x - TILE_PX * offset, game->enemies.list[i].position.y - TILE_PX * offset, zSize, zSize };
                    if (CheckCollisionRecs(attackRec, zRec)) {
                        DamageZombie(game, i, PLAYER_SWORD_DAMAGE);
                    }
                }
            }
        }
    }

    if (game->playerInfo.gunSpawned) {
        float gx = game->playerInfo.gunCol * TILE_PX + TILE_PX / 2.0f;
        float gy = game->playerInfo.gunRow * TILE_PX + TILE_PX / 2.0f;
        float px = game->player.position.x + TILE_PX / 2.0f;
        float py = game->player.position.y + TILE_PX / 2.0f;
        float dx = px - gx;
        float dy = py - gy;
        if (sqrtf(dx*dx + dy*dy) < 32.0f) {
            game->playerInfo.gunSpawned = false;
            game->playerInfo.hasGun = true;
            game->playerInfo.gunAbilityTimer = 4.0f;
            PlaySound(game->audio.blast);
        }
    }

    if (game->playerInfo.hasGun) {
        game->playerInfo.gunAbilityTimer -= dt;
        if (game->playerInfo.gunAbilityTimer <= 0.0f) game->playerInfo.hasGun = false;
    }

    if (!game->playerInfo.gunSpawned && !game->playerInfo.hasGun && (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL)) {
        game->playerInfo.gunSpawnTimer -= dt;
        if (game->playerInfo.gunSpawnTimer <= 0.0f) {
            SpawnGun(game);
            game->playerInfo.gunSpawnTimer = 4.0f;
        }
    }

    if (game->levelInfo.potionSpawned) {
        float gx = game->levelInfo.potionCol * TILE_PX + TILE_PX / 2.0f;
        float gy = game->levelInfo.potionRow * TILE_PX + TILE_PX / 2.0f;
        float px = game->player.position.x + TILE_PX / 2.0f;
        float py = game->player.position.y + TILE_PX / 2.0f;
        float dx = px - gx;
        float dy = py - gy;
        if (sqrtf(dx*dx + dy*dy) < 32.0f) {
            game->levelInfo.potionSpawned = false;
            game->player.health = game->player.maxHealth;
            PlaySound(game->audio.blast);
        }
    }
}

static void UpdatePlayerProjectiles(GameState* game, float dt) {
    for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) {
        if (!game->playerInfo.projectiles[p].active) continue;

        game->playerInfo.projectiles[p].position.x += game->playerInfo.projectiles[p].velocity.x * dt;
        game->playerInfo.projectiles[p].position.y += game->playerInfo.projectiles[p].velocity.y * dt;

        Vector2 pos = game->playerInfo.projectiles[p].position;
        if (pos.x < 0 || pos.x > MAP_WIDTH * TILE_PX || pos.y < 0 || pos.y > MAP_HEIGHT * TILE_PX) {
            game->playerInfo.projectiles[p].active = false;
            continue;
        }

        int cellX = (int)(pos.x / TILE_PX);
        int cellY = (int)(pos.y / TILE_PX);
        if (cellX >= 0 && cellX < MAP_WIDTH && cellY >= 0 && cellY < MAP_HEIGHT) {
            int type = GetTileType(game->map.tiles[cellY][cellX]);
            if (type == TILE_WALL) {
                game->playerInfo.projectiles[p].active = false;
                continue;
            }
        }

        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (IsZombieHit(game, i, pos, 16.0f)) {
                game->playerInfo.projectiles[p].active = false;
                PlaySound(game->audio.hit);
                DamageZombie(game, i, PLAYER_PROJECTILE_DAMAGE);
                break;
            }
        }
    }
}

static void UpdateMenuState(GameState* game) {
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        game->menu.menuSelection--;
        if (game->menu.menuSelection < 0) game->menu.menuSelection = 4;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        game->menu.menuSelection++;
        if (game->menu.menuSelection > 4) game->menu.menuSelection = 0;
    }
    if (game->menu.menuSelection == 1) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            game->menu.difficulty = (game->menu.difficulty + 1) % 3;
        } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            game->menu.difficulty = (game->menu.difficulty + 2) % 3;
        }
    } else if (IsKeyPressed(KEY_ENTER)) {
        if (game->menu.menuSelection == 0) game->state = STATE_NAME_PROMPT;
        else if (game->menu.menuSelection == 2) game->state = STATE_HISTORY;
        else if (game->menu.menuSelection == 3) game->state = STATE_TEAM;
        else if (game->menu.menuSelection == 4) CloseWindow();
    }
}

static void UpdateNamePromptState(GameState* game) {
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && (game->playerInfo.nameLength < 15)) {
            game->playerInfo.name[game->playerInfo.nameLength] = (char)key;
            game->playerInfo.name[game->playerInfo.nameLength + 1] = '\0';
            game->playerInfo.nameLength++;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        game->playerInfo.nameLength--;
        if (game->playerInfo.nameLength < 0) game->playerInfo.nameLength = 0;
        game->playerInfo.name[game->playerInfo.nameLength] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER) && game->playerInfo.nameLength > 0) {
        StopMusicStream(game->audio.bgMusic);
        GameState_LoadCutscenes(game);
        game->state = STATE_INTRO;
        game->cutscene.part = 0;
        game->cutscene.time = 0.0f;
        PlaySound(game->cutscene.cut1Audio);
    }
    if (IsKeyPressed(KEY_ESCAPE)) game->state = STATE_MENU;
}

static void UpdateIntroState(GameState* game, float dt) {
    game->cutscene.time += dt;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
        StopSound(game->cutscene.cut1Audio);
        StopSound(game->cutscene.cut2Audio);
        StopSound(game->cutscene.cutCaveAudio);
        GameState_UnloadCutscenes(game);
        GameState_TransitionFromCutscene(game);
        return;
    }
    if (game->cutscene.part == 0 && game->cutscene.time >= 10.0f) {
        game->cutscene.part = 1;
        game->cutscene.time = 0.0f;
        StopSound(game->cutscene.cut1Audio);
        PlaySound(game->cutscene.cut2Audio);
    }
    if (game->cutscene.part == 1 && game->cutscene.time >= 4.0f) {
        StopSound(game->cutscene.cut2Audio);
        GameState_UnloadCutscenes(game);
        GameState_TransitionFromCutscene(game);
    }
    if (game->cutscene.part == 2 && game->cutscene.time >= 10.0f) {
        StopSound(game->cutscene.cutCaveAudio);
        GameState_UnloadCutscenes(game);
        GameState_TransitionFromCutscene(game);
    }
    if (game->cutscene.part == 3 && game->cutscene.time >= 12.0f) {
        GameState_UnloadCutscenes(game);
        GameState_TransitionFromCutscene(game);
    }
}

static void UpdateExploringState(GameState* game, float dt) {
    Player_Update(&game->player, &game->map, dt);
    int pCol = (int)((game->player.position.x + TILE_PX / 2.0f) / TILE_PX);
    int pRow = (int)((game->player.position.y + TILE_PX / 2.0f) / TILE_PX);
    if (pRow >= 0 && pRow < MAP_HEIGHT && pCol >= 0 && pCol < MAP_WIDTH) {
        int tileID = game->map.tiles[pRow][pCol];
        if (GetTileType(tileID) == TILE_MAYOR) {
            game->state = STATE_CUTSCENE;
        }
    }
}

static void UpdateCutsceneState(GameState* game) {
    if (IsKeyPressed(KEY_ENTER)) {
        game->state = STATE_SURVIVAL;
        game->enemies.timer = 0.0f;
        game->enemies.spawnTimer = 0.0f;
        
        if (currentLevel == 0) {
            // Clean up Mayor tile from the old spawn position
            game->map.tiles[game->levelInfo.mayorRow][game->levelInfo.mayorCol] = 20;

            // Spawn Cave Exit at hardcoded position (2, 2)
            game->levelInfo.mayorRow = 2;
            game->levelInfo.mayorCol = 2;
            game->map.tiles[2][2] = 236;
            game->map.tiles[2][3] = 237;

            game->player.position.x = 25.0f * TILE_PX;
            game->player.position.y = 23.0f * TILE_PX;
            for (int i = 0; i < MAX_ZOMBIES; i++) game->enemies.list[i].active = false;
            for (int s = 0; s < 6; s++) SpawnZombie(game);
        } else {
            for (int i = 0; i < MAX_ZOMBIES; i++) game->enemies.list[i].active = false;
            for (int s = 0; s < 6; s++) SpawnZombie(game);
        }

        game->playerInfo.checkpointPosition = game->player.position;
        game->playerInfo.checkpointLevel = currentLevel;
        game->playerInfo.checkpointState = STATE_SURVIVAL;
        game->playerInfo.checkpointActive = true;
    }
}

static void UpdateSurvivalState(GameState* game, float dt) {
    Player_Update(&game->player, &game->map, dt);

    int pCol = (int)((game->player.position.x + TILE_PX / 2.0f) / TILE_PX);
    int pRow = (int)((game->player.position.y + TILE_PX / 2.0f) / TILE_PX);

    if (currentLevel == 3) {
        if (pRow == 2 && pCol == 20) {
            if (game->map.tiles[3][21] == 59) {
                game->map.tiles[3][21] = 17;
                game->map.tiles[3][22] = 17;
                PlaySound(game->audio.blast);
            }
        }
        
        if ((pRow == 2 && pCol == 23) || (pRow == 20 && pCol == 8)) {
            if (game->map.tiles[21][3] == 59) {
                game->map.tiles[21][3] = 17;
                game->map.tiles[21][4] = 17;
                game->map.tiles[22][3] = 17;
                game->map.tiles[22][4] = 17;
                PlaySound(game->audio.blast);
            }
        }
    }

    bool canSpawn = true;
    if (currentLevel == 1 && game->enemies.status[BOSS_RAT_KING].defeated) canSpawn = false;
    if (currentLevel == 2 && game->enemies.status[BOSS_BRAINROT_GOD].defeated) canSpawn = false;

    game->enemies.spawnTimer += dt;
    float spawnCooldown = (currentLevel == 0) ? LEVEL1_SPAWN_INTERVAL : (currentLevel == 1) ? LEVEL2_SPAWN_INTERVAL : LEVEL3_SPAWN_INTERVAL;
    if (game->enemies.spawnTimer >= spawnCooldown) {
        game->enemies.spawnTimer = 0.0f;
        if (canSpawn) {
            int spawnCount = 2 + currentLevel * 2;
            for (int s = 0; s < spawnCount; s++) {
                SpawnZombie(game);
            }
        }
    }

    UpdateZombies(game, dt);

    if (pRow >= 0 && pRow < MAP_HEIGHT && pCol >= 0 && pCol < MAP_WIDTH) {
        int tileID = game->map.tiles[pRow][pCol];
        if (GetTileType(tileID) == TILE_CAVE) {
            for (int i = 0; i < MAX_ZOMBIES; i++) game->enemies.list[i].active = false;

            if (currentLevel == 0) {
                GameHistory_SaveEntry(game->playerInfo.name, 1, false);
                
                GameState_LoadCutscenes(game);
                game->state = STATE_INTRO;
                game->cutscene.part = 2;
                game->cutscene.time = 0.0f;
                PlaySound(game->cutscene.cutCaveAudio);
                
                game->cutscene.targetLevel = 1;
                game->cutscene.targetState = STATE_SURVIVAL;
            } else if (currentLevel == 1) {
                GameHistory_SaveEntry(game->playerInfo.name, 2, false);
                LoadLevel(game, 2);
            } else if (currentLevel == 3) {
                GameHistory_SaveEntry(game->playerInfo.name, 4, true);
                GameState_LoadCutscenes(game);
                game->state = STATE_INTRO;
                game->cutscene.part = 3;
                game->cutscene.time = 0.0f;
                game->cutscene.targetLevel = 3;
                game->cutscene.targetState = STATE_WIN;
            }
        }
    }

    game->player.survivalTimer += dt;
    if (game->player.survivalTimer >= SUPERPOWER_READY_TIME && game->player.radiusCooldown <= 0.0f) {
        game->player.radiusPowerupReady = true;
    }

    if (game->player.radiusPowerupReady && IsKeyDown(KEY_F)) game->player.isAimingSuperpower = true;
    else game->player.isAimingSuperpower = false;

    if (game->player.radiusPowerupReady && IsKeyReleased(KEY_F) && game->player.radiusCooldown <= 0.0f) {
        PlaySound(game->audio.blast);
        game->player.radiusPowerupReady = false;
        game->player.radiusCooldown = SUPERPOWER_COOLDOWN_MAX;
        game->player.radiusBlastTimer = SUPERPOWER_BLAST_DURATION;
        
        Vector2 blastPos = { pCol * TILE_PX + TILE_PX/2.0f, pRow * TILE_PX + TILE_PX/2.0f };
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (game->enemies.list[i].active) {
                EnemyProperties props = GetEnemyProperties(game->enemies.list[i].type, currentLevel, game->menu.difficulty);
                float zSize = TILE_PX * props.scale;
                float offset = (props.scale - 1.0f) / 2.0f;
                Rectangle zRec = { game->enemies.list[i].position.x - TILE_PX * offset, game->enemies.list[i].position.y - TILE_PX * offset, zSize, zSize };
                if (CheckCollisionCircleRec(blastPos, SUPERPOWER_RADIUS, zRec)) {
                    DamageZombie(game, i, SUPERPOWER_DAMAGE);
                }
            }
        }
    }

    if (currentLevel == 2 && game->enemies.status[BOSS_BRAINROT_GOD].spawned && !game->enemies.status[BOSS_BRAINROT_GOD].defeated) {
        Vector2 pPos = { game->player.position.x + TILE_PX/2.0f, game->player.position.y + TILE_PX/2.0f };
        for (int i = 0; i < 3; i++) {
            if (!game->enemies.fragments[i].activated) {
                float dx = pPos.x - game->enemies.fragments[i].position.x;
                float dy = pPos.y - game->enemies.fragments[i].position.y;
                if (sqrtf(dx*dx + dy*dy) < 24.0f) {
                    game->enemies.fragments[i].activated = true;
                    PlaySound(game->audio.blast);
                    for (int z = 0; z < MAX_ZOMBIES; z++) {
                        if (game->enemies.list[z].type == ENEMY_BRAINROT_GOD) DamageZombie(game, z, BRAINROT_GOD_PUZZLE_DAMAGE);
                    }
                }
            }
        }
    }
    Rectangle pRec = { game->player.position.x + 2.0f, game->player.position.y + 2.0f, 28.0f, 28.0f };
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (game->enemies.list[i].active) {
            EnemyProperties props = GetEnemyProperties(game->enemies.list[i].type, currentLevel, game->menu.difficulty);
            float zSize = TILE_PX * props.scale;
            float offset = (props.scale - 1.0f) / 2.0f;
            Rectangle zRec = { game->enemies.list[i].position.x - TILE_PX * offset, game->enemies.list[i].position.y - TILE_PX * offset, zSize, zSize };
            if (CheckCollisionRecs(pRec, zRec)) {
                game->player.health -= ZOMBIE_DAMAGE_RATE * dt;
                if (game->audio.hitTimer <= 0.0f) {
                    PlaySound(game->audio.hit);
                    game->audio.hitTimer = 0.5f;
                }
            }
        }
    }

    if (game->player.health <= 0.0f) {
        game->player.health = 0.0f;
        if (game->playerInfo.checkpointActive && game->playerInfo.lives > 1) {
            game->playerInfo.lives--;
            game->player.health = PLAYER_INITIAL_HEALTH;
            
            if (currentLevel != game->playerInfo.checkpointLevel) {
                LoadLevel(game, game->playerInfo.checkpointLevel);
            } else {
                game->player.position = game->playerInfo.checkpointPosition;
                game->player.gridX = (int)(game->player.position.x / TILE_PX);
                game->player.gridY = (int)(game->player.position.y / TILE_PX);
                game->state = game->playerInfo.checkpointState;

                // Reset minor zombies, but preserve and reposition active bosses
                for (int z = 0; z < MAX_ZOMBIES; z++) {
                    if (game->enemies.list[z].active) {
                        BossId bid = GetBossId(game->enemies.list[z].type);
                        if (bid != (BossId)-1) {
                            if (game->levelInfo.bossRow != -1 && game->levelInfo.bossCol != -1) {
                                game->enemies.list[z].position = (Vector2){ game->levelInfo.bossCol * TILE_PX, game->levelInfo.bossRow * TILE_PX };
                                game->enemies.list[z].row = game->levelInfo.bossRow;
                                game->enemies.list[z].col = game->levelInfo.bossCol;
                            }
                        } else {
                            game->enemies.list[z].active = false;
                        }
                    }
                }

                if (currentLevel == 0 && game->playerInfo.checkpointState == STATE_SURVIVAL) {
                    for (int s = 0; s < 6; s++) SpawnZombie(game);
                }
            }
        } else {
            GameHistory_SaveEntry(game->playerInfo.name, currentLevel + 1, false);
            game->state = STATE_GAMEOVER;
        }
    }
}

static void UpdateGameOverState(GameState* game) {
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) game->menu.gameOverSelection = 0;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) game->menu.gameOverSelection = 1;
    if (IsKeyPressed(KEY_ENTER)) {
        if (game->menu.gameOverSelection == 0) {
            int levelToLoad = currentLevel;
            LoadLevel(game, levelToLoad);
            game->playerInfo.lives = PLAYER_INITIAL_LIVES;
        } else {
            game->state = STATE_MENU;
        }
    }
}

static void UpdateWinState(GameState* game) {
    if (IsKeyPressed(KEY_ENTER)) {
        GameState_Init(game);
        game->state = STATE_MENU;
    }
}

static bool UpdateBossLogs(GameState* game) {
    for (int i = 0; i < 4; i++) {
        if (game->enemies.status[i].showLog) {
            if (IsKeyPressed(KEY_ENTER)) game->enemies.status[i].showLog = false;
            return true;
        }
    }
    return false;
}

static void UpdateDebugCheats(GameState* game) {
    if (IsKeyPressed(KEY_M) && (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL)) {
        for (int i = 0; i < MAX_ZOMBIES; i++) game->enemies.list[i].active = false;
        if (currentLevel == 0) {
            GameHistory_SaveEntry(game->playerInfo.name, 1, false);
            GameState_LoadCutscenes(game);
            game->state = STATE_INTRO;
            game->cutscene.part = 2;
            game->cutscene.time = 0.0f;
            PlaySound(game->cutscene.cutCaveAudio);
            game->cutscene.targetLevel = 1;
            game->cutscene.targetState = STATE_SURVIVAL;
        } else if (currentLevel == 1) {
            GameHistory_SaveEntry(game->playerInfo.name, 2, false);
            LoadLevel(game, 2);
        } else if (currentLevel == 2) {
            GameHistory_SaveEntry(game->playerInfo.name, 3, false);
            LoadLevel(game, 3);
        } else if (currentLevel == 3) {
            GameHistory_SaveEntry(game->playerInfo.name, 4, true);
            GameState_LoadCutscenes(game);
            game->state = STATE_INTRO;
            game->cutscene.part = 3;
            game->cutscene.time = 0.0f;
            game->cutscene.targetLevel = 3;
            game->cutscene.targetState = STATE_WIN;
        }
    }
}

static void UpdateLevelTimers(GameState* game, float dt) {
    if (game->audio.hitTimer > 0.0f) game->audio.hitTimer -= dt;
    if (game->levelInfo.startTextTimer > 0.0f && (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL)) {
        game->levelInfo.startTextTimer -= dt;
    }
    if (currentLevel == 3 && game->state == STATE_SURVIVAL) {
        game->levelInfo.escapeTimer -= dt;
        if (game->levelInfo.escapeTimer <= 0.0f) {
            game->levelInfo.escapeTimer = 0.0f;
            GameHistory_SaveEntry(game->playerInfo.name, 4, false);
            game->state = STATE_GAMEOVER;
        }
    }
}

static void UpdateAshEffects(GameState* game, float dt) {
    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        if (game->enemies.ashEffects[i].active) {
            game->enemies.ashEffects[i].timer -= dt;
            if (game->enemies.ashEffects[i].timer <= 0.0f) game->enemies.ashEffects[i].active = false;
        }
    }
}

void UpdateGame(GameState* game, float dt) {
    if (UpdateBossLogs(game)) return;

    UpdateMusicStream(game->audio.bgMusic);

    UpdateLevelTimers(game, dt);
    UpdateDebugCheats(game);
    UpdateAshEffects(game, dt);

    UpdatePlayerWeapon(game, dt);
    UpdatePlayerProjectiles(game, dt);

    switch (game->state) {
        case STATE_MENU:         UpdateMenuState(game); break;
        case STATE_NAME_PROMPT:  UpdateNamePromptState(game); break;
        case STATE_INTRO:        UpdateIntroState(game, dt); break;
        case STATE_HISTORY:
        case STATE_TEAM:
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_MENU;
            }
            break;
        case STATE_EXPLORING:    UpdateExploringState(game, dt); break;
        case STATE_CUTSCENE:     UpdateCutsceneState(game); break;
        case STATE_SURVIVAL:     UpdateSurvivalState(game, dt); break;
        case STATE_GAMEOVER:     UpdateGameOverState(game); break;
        case STATE_WIN:          UpdateWinState(game); break;
    }
}



static void DrawGameplayWorld(const GameState* game) {
    Camera2D camera = { 0 };
    camera.target = (Vector2){ game->player.position.x + TILE_PX / 2.0f, game->player.position.y + TILE_PX / 2.0f };
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 2.5f;

    BeginMode2D(camera);

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int tileID = game->map.tiles[y][x];
            if (tileID == 236 || tileID == 237) {
                float gx = x * TILE_PX;
                float gy = y * TILE_PX;
                float xco = (float)((34 % game->spritesTilesPerRow) * TILE_SIZE);
                float yco = (float)((34 / game->spritesTilesPerRow) * TILE_SIZE);
                Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
                Rectangle dest = { gx, gy, (float)TILE_PX, (float)TILE_PX };
                DrawTexturePro(game->spritesTileset, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawTile(game->map.tileset, game->map.tilesPerRow, tileID, (float)(x * TILE_PX), (float)(y * TILE_PX));
            }
        }
    }

    if (currentLevel == 3) {
        float ex = 33 * TILE_PX;
        float ey = 18 * TILE_PX;
        DrawText("EXIT", ex - MeasureText("EXIT", 10)/2.0f + TILE_PX/2.0f, ey - 12, 10, GOLD);
    }

    if (currentLevel == 0) {
        float mx = game->levelInfo.mayorCol * TILE_PX;
        float my = game->levelInfo.mayorRow * TILE_PX;
        if (game->state == STATE_EXPLORING) {
            DrawText("MAYOR", mx - MeasureText("MAYOR", 10)/2.0f + TILE_PX/2.0f, my - 12, 10, GREEN);
        } else if (game->state == STATE_SURVIVAL) {
            DrawText("EXIT", mx - MeasureText("EXIT", 10)/2.0f + TILE_PX/2.0f, my - 12, 10, GOLD);
        }
    }

    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        if (game->enemies.ashEffects[i].active) {
            float ax = game->enemies.ashEffects[i].gridX * TILE_PX;
            float ay = game->enemies.ashEffects[i].gridY * TILE_PX;
            float alpha = (game->enemies.ashEffects[i].timer / 1.5f) * 0.8f;
            DrawRectangle(ax, ay, TILE_PX, TILE_PX, ColorAlpha((Color){30, 30, 30, 255}, alpha));
        }
    }

    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (game->enemies.list[i].active) {
            float zx = game->enemies.list[i].position.x;
            float zy = game->enemies.list[i].position.y;
            
            EnemyProperties props = GetEnemyProperties(game->enemies.list[i].type, currentLevel, game->menu.difficulty);
            float pulse = 0.0f;
            if (game->enemies.list[i].type == ENEMY_BRAINROT_GOD) {
                pulse = sinf(GetTime() * 4.0f) * 4.0f;
            }

            float offset = (props.scale - 1.0f) / 2.0f;
            Texture2D tileset = game->spritesTileset;
            int tilesPerRow = game->spritesTilesPerRow;

            if (game->enemies.list[i].type == ENEMY_SNAKE && currentLevel != 1) {
                tileset = game->map.tileset;
                tilesPerRow = game->map.tilesPerRow;
            }

            DrawTexturePro(tileset,
                           (Rectangle){ (float)(props.baseTileId % tilesPerRow) * TILE_SIZE, (float)(props.baseTileId / tilesPerRow) * TILE_SIZE, TILE_SIZE, TILE_SIZE },
                           (Rectangle){ zx - TILE_PX * offset, zy - TILE_PX * offset, TILE_PX * props.scale + pulse, TILE_PX * props.scale + pulse },
                           (Vector2){ 0, 0 }, 0.0f, props.color);

            if (game->enemies.list[i].type == ENEMY_SNAKE && currentLevel != 1) {
                DrawRectangle(zx, zy, TILE_PX, TILE_PX, ColorAlpha(RED, 0.45f));
            }

            float barWidth = TILE_PX * props.scale;
            float barHeight = (props.scale > 1.0f) ? ((props.scale >= 4.0f) ? 8.0f : 6.0f) : 4.0f;
            float zx_offset = zx - (barWidth - TILE_PX)/2.0f;
            float z_py = zy - 10.0f;
            DrawRectangle(zx_offset, z_py, barWidth, barHeight, ColorAlpha(BLACK, 0.6f));
            float pct = game->enemies.list[i].health / game->enemies.list[i].maxHealth;
            if (pct < 0.0f) pct = 0.0f;
            DrawRectangle(zx_offset + 1, z_py + 1, (barWidth - 2) * pct, barHeight - 2, RED);

            if (game->enemies.list[i].type == ENEMY_BRAINROT_GOD) {
                DrawText("USE FRAGMENTS", zx - 24, z_py - 12, 10, YELLOW);
            }
        }
    }

    if (currentLevel == 2 && game->enemies.status[BOSS_BRAINROT_GOD].spawned && !game->enemies.status[BOSS_BRAINROT_GOD].defeated) {
        Color colors[3] = { GREEN, YELLOW, RAYWHITE };
        for (int i = 0; i < 3; i++) {
            if (!game->enemies.fragments[i].activated) {
                float pulse = sinf(GetTime() * 5.0f);
                Vector2 fPos = game->enemies.fragments[i].position;
                DrawCircleLines(fPos.x + TILE_PX/2.0f, fPos.y + TILE_PX/2.0f, 15.0f + pulse * 4.0f, colors[i]);
                DrawCircle(fPos.x + TILE_PX/2.0f, fPos.y + TILE_PX/2.0f, 6.0f, ColorAlpha(colors[i], 0.5f));
                DrawText(game->enemies.fragments[i].name, fPos.x - MeasureText(game->enemies.fragments[i].name, 10)/2.0f + TILE_PX/2.0f, fPos.y - 12, 10, colors[i]);
            }
        }
    }

    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (game->enemies.projectiles[i].active) {
            Vector2 p = game->enemies.projectiles[i].position;
            float r = game->enemies.projectiles[i].isBig ? 12.0f : 5.0f;
            Color innerCol = game->enemies.projectiles[i].isBig ? PURPLE : ORANGE;
            Color outerCol = game->enemies.projectiles[i].isBig ? MAGENTA : RED;
            DrawCircle(p.x, p.y, r, outerCol);
            DrawCircle(p.x, p.y, r * 0.6f, innerCol);
            DrawCircle(p.x, p.y, r * 0.3f, YELLOW);
        }
    }

    if (game->player.isAimingSuperpower) {
        float px = game->player.position.x + TILE_PX/2.0f;
        float py = game->player.position.y + TILE_PX/2.0f;
        DrawCircle(px, py, 5.0f * TILE_PX, ColorAlpha(RED, 0.3f));
    }

    if (game->player.radiusBlastTimer > 0.0f) {
        float px = game->player.position.x + TILE_PX/2.0f;
        float py = game->player.position.y + TILE_PX/2.0f;
        float maxRadius = 5.0f * TILE_PX;
        float currentRadius = maxRadius * (1.0f - (game->player.radiusBlastTimer / 0.5f));
        DrawCircleLines(px, py, currentRadius, ORANGE);
        DrawCircleLines(px, py, currentRadius + 2.0f, RED);
    }

    Player_Draw(&game->player, game->playerTileset, game->playerTilesPerRow);

    if (game->player.isAttacking && game->player.attackTimer > 0.0f) {
        float rx = game->player.position.x;
        float ry = game->player.position.y;
        float rw = TILE_PX;
        float rh = TILE_PX;
        if (game->player.direction == DIR_UP) ry -= TILE_PX;
        else if (game->player.direction == DIR_DOWN) ry += TILE_PX;
        else if (game->player.direction == DIR_LEFT) rx -= TILE_PX;
        else if (game->player.direction == DIR_RIGHT) rx += TILE_PX;
        
        for (int i = 0; i < 6; i++) {
            float ox = rx + GetRandomValue(-2, 2);
            float oy = ry + GetRandomValue(-2, 2);
            Color fColor = (i % 3 == 0) ? GOLD : ((i % 2 == 0) ? ORANGE : RED);
            DrawRectangle(ox, oy, rw, rh, ColorAlpha(fColor, 0.4f));
        }
    }

    if (game->playerInfo.gunSpawned) {
        float gx = game->playerInfo.gunCol * TILE_PX;
        float gy = game->playerInfo.gunRow * TILE_PX;
        float xco = (float)((133 % game->spritesTilesPerRow) * TILE_SIZE);
        float yco = (float)((133 / game->spritesTilesPerRow) * TILE_SIZE);
        Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
        Rectangle dest = { gx, gy, (float)TILE_PX, (float)TILE_PX };
        DrawTexturePro(game->spritesTileset, src, dest, (Vector2){0, 0}, 0.0f, ColorAlpha(WHITE, 0.65f));
        float pulse = sinf(GetTime() * 6.0f);
        DrawCircleLines(gx + TILE_PX/2.0f, gy + TILE_PX/2.0f, 10.0f + pulse * 4.0f, SKYBLUE);
        DrawCircleLines(gx + TILE_PX/2.0f, gy + TILE_PX/2.0f, 10.0f + pulse * 4.0f + 1.0f, BLUE);
    }

    if (game->levelInfo.potionSpawned) {
        float gx = game->levelInfo.potionCol * TILE_PX;
        float gy = game->levelInfo.potionRow * TILE_PX;
        float xco = (float)((135 % game->spritesTilesPerRow) * TILE_SIZE);
        float yco = (float)((135 / game->spritesTilesPerRow) * TILE_SIZE);
        Rectangle src = { xco, yco, (float)TILE_SIZE, (float)TILE_SIZE };
        Rectangle dest = { gx, gy, (float)TILE_PX, (float)TILE_PX };
        DrawTexturePro(game->spritesTileset, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
        float pulse = sinf(GetTime() * 6.0f);
        DrawCircleLines(gx + TILE_PX/2.0f, gy + TILE_PX/2.0f, 10.0f + pulse * 4.0f, GREEN);
        DrawCircleLines(gx + TILE_PX/2.0f, gy + TILE_PX/2.0f, 10.0f + pulse * 4.0f + 1.0f, LIME);
    }

    for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) {
        if (game->playerInfo.projectiles[p].active) {
            Vector2 pos = game->playerInfo.projectiles[p].position;
            DrawCircle(pos.x, pos.y, 6.0f, SKYBLUE);
            DrawCircle(pos.x, pos.y, 4.0f, BLUE);
            DrawCircle(pos.x, pos.y, 2.0f, WHITE);
        }
    }

    EndMode2D();
}

void DrawGame(const GameState* game) {
    if (game->state == STATE_INTRO) {
        ClearBackground(BLACK);
        int frameIndex = (game->cutscene.part == 3) ? (int)(game->cutscene.time / 1.0f) :
                         (game->cutscene.part == 2) ? (int)(game->cutscene.time * 24.0f) :
                                                     (int)(game->cutscene.time * 15.0f);
        Texture2D frameTex = { 0 };
        if (game->cutscene.part == 0) {
            if (frameIndex >= 0 && frameIndex < CUT1_FRAMES) frameTex = game->cutscene.cut1Textures[frameIndex];
        } else if (game->cutscene.part == 1) {
            if (frameIndex >= 0 && frameIndex < CUT2_FRAMES) frameTex = game->cutscene.cut2Textures[frameIndex];
        } else if (game->cutscene.part == 2) {
            if (frameIndex >= 0 && frameIndex < CUT_CAVE_FRAMES) frameTex = game->cutscene.cutCaveTextures[frameIndex];
        } else if (game->cutscene.part == 3) {
            if (frameIndex < 0) frameIndex = 0;
            if (frameIndex >= ENDSCENE_FRAMES) frameIndex = ENDSCENE_FRAMES - 1;
            frameTex = game->cutscene.endsceneTextures[frameIndex];
        }

        if (frameTex.id > 0) {
            float screenW = (float)GetScreenWidth();
            float screenH = (float)GetScreenHeight();
            float texW = (float)frameTex.width;
            float texH = (float)frameTex.height;

            float screenAspect = screenW / screenH;
            float texAspect = texW / texH;

            Rectangle srcRect;
            if (texAspect > screenAspect) {
                float cropW = texH * screenAspect;
                float cropX = (texW - cropW) / 2.0f;
                srcRect = (Rectangle){ cropX, 0.0f, cropW, texH };
            } else {
                float cropH = texW / screenAspect;
                float cropY = (texH - cropH) / 2.0f;
                srcRect = (Rectangle){ 0.0f, cropY, texW, cropH };
            }

            // Zoom the first intro cutscene (part 0) by 25%
            if (game->cutscene.part == 0) {
                float zoom = 1.25f;
                float newW = srcRect.width / zoom;
                float newH = srcRect.height / zoom;
                srcRect.x += (srcRect.width - newW) / 2.0f;
                srcRect.y += (srcRect.height - newH) / 2.0f;
                srcRect.width = newW;
                srcRect.height = newH;
            }

            DrawTexturePro(frameTex, 
                           srcRect,
                           (Rectangle){ 0, 0, screenW, screenH },
                           (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawText("PLAYING CUTSCENE...", GetScreenWidth() / 2 - 120, GetScreenHeight() / 2 - 10, 20, RED);
        }
        DrawText("Press [ENTER] to skip", GetScreenWidth() - 260, GetScreenHeight() - 40, 16, LIGHTGRAY);
        EndDrawing();
        return;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL || game->state == STATE_CUTSCENE) {
        DrawGameplayWorld(game);
    }

    if (game->state == STATE_MENU) DrawMenuScreen(game);
    else if (game->state == STATE_NAME_PROMPT) DrawNamePromptScreen(game);
    else if (game->state == STATE_HISTORY) DrawHistoryScreen(game);
    else if (game->state == STATE_TEAM) DrawTeamScreen(game);
    else if (game->state == STATE_GAMEOVER) DrawGameOverScreen(game);
    else if (game->state == STATE_WIN) DrawWinScreen(game);
    else DrawHUD(game);

    if (game->levelInfo.startTextTimer > 0.0f && (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL)) {
        float alpha = 1.0f;
        if (game->levelInfo.startTextTimer > 3.0f) alpha = (4.0f - game->levelInfo.startTextTimer);
        else if (game->levelInfo.startTextTimer < 1.0f) alpha = game->levelInfo.startTextTimer;

        int bannerH = 160;
        int bannerY = (GetScreenHeight() - bannerH) / 2;
        DrawRectangle(0, bannerY, GetScreenWidth(), bannerH, ColorAlpha(BLACK, 0.85f * alpha));
        DrawRectangleLines(0, bannerY, GetScreenWidth(), bannerH, ColorAlpha(RED, 0.7f * alpha));

        const char* subHeader = (currentLevel == 0) ? "PROLOGUE: THE MAYOR'S REQUEST" :
                                (currentLevel == 1) ? "CHAPTER 2: SIGMA RESEARCH FACILITY" :
                                "CHAPTER 3: THE FORBIDDEN ARCHIVE";
        int subW = MeasureText(subHeader, 18);
        DrawText(subHeader, (GetScreenWidth() - subW) / 2, bannerY + 30, 18, ColorAlpha(GOLD, alpha));

        const char* mainHeader = (currentLevel == 0) ? "CHAPTER 1: GO BEYOND THE ROT" :
                                 (currentLevel == 1) ? "THE CORRUPTED ARCHIVE" :
                                 "THE ARCHIVE CORE";
        int mainW = MeasureText(mainHeader, 28);
        DrawText(mainHeader, (GetScreenWidth() - mainW) / 2, bannerY + 70, 28, ColorAlpha(WHITE, alpha));
    }

    DrawStoryOverlays(game);

    EndDrawing();
}

void GameState_Unload(GameState* self) {
    GameState_UnloadCutscenes(self);
    UnloadMusicStream(self->audio.bgMusic);
    UnloadSound(self->audio.slash);
    UnloadSound(self->audio.blast);
    UnloadSound(self->audio.hit);
    UnloadSound(self->cutscene.cut1Audio);
    UnloadSound(self->cutscene.cut2Audio);
    UnloadSound(self->cutscene.cutCaveAudio);
    UnloadTexture(self->playerTileset);
    if (self->spritesTileset.id != self->playerTileset.id) {
        UnloadTexture(self->spritesTileset);
    }
    if (self->gameFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(self->gameFont);
    }
}
