#include "raylib.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int currentLevel = 0;

#define CUT1_FRAMES 152
#define CUT2_FRAMES 62

void GameState_LoadCutscenes(GameState* game) {
    if (game->cutscenesLoaded) return;
    
    BeginDrawing();
    ClearBackground(BLACK);
    DrawCustomText(game, "LOADING CUTSCENE...", (float)GetScreenWidth() / 2.0f - 180.0f, (float)GetScreenHeight() / 2.0f - 20.0f, 24.0f, RED);
    EndDrawing();

    char path[128];
    for (int i = 0; i < CUT1_FRAMES; i++) {
        sprintf(path, "cutscenes/frames/cut1_%03d.png", i + 1);
        game->cut1Textures[i] = LoadTexture(path);
    }
    for (int i = 0; i < CUT2_FRAMES; i++) {
        sprintf(path, "cutscenes/frames/cut2_%03d.png", i + 1);
        game->cut2Textures[i] = LoadTexture(path);
    }
    game->cutscenesLoaded = true;
}

void GameState_UnloadCutscenes(GameState* game) {
    if (!game->cutscenesLoaded) return;
    for (int i = 0; i < CUT1_FRAMES; i++) {
        if (game->cut1Textures[i].id > 0) {
            UnloadTexture(game->cut1Textures[i]);
            game->cut1Textures[i].id = 0;
        }
    }
    for (int i = 0; i < CUT2_FRAMES; i++) {
        if (game->cut2Textures[i].id > 0) {
            UnloadTexture(game->cut2Textures[i]);
            game->cut2Textures[i].id = 0;
        }
    }
    game->cutscenesLoaded = false;
}

void GameState_Init(GameState* self) {
    currentLevel = 0;
    Tilemap_Load(&self->map, "map1.csv", "tilemap_packed.png");
    Player_Init(&self->player);

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int tid = self->map.tiles[y][x];
            if (tid == 306 || tid == 308 || tid == 355) {
                self->map.tiles[y][x] = 20;
            }
        }
    }

    typedef struct { int r, c; } CoordPair;
    CoordPair candidates[MAP_WIDTH * MAP_HEIGHT];
    int candidateCount = 0;

    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 2; x++) {
            int type1 = GetTileType(self->map.tiles[y][x]);
            int type2 = GetTileType(self->map.tiles[y][x+1]);
            if (type1 != TILE_WALL && type1 != TILE_CAR && type2 != TILE_WALL && type2 != TILE_CAR) {
                int distSq = (y - 2) * (y - 2) + (x - 2) * (x - 2);
                if (distSq >= 100) {
                    candidates[candidateCount].r = y;
                    candidates[candidateCount].c = x;
                    candidateCount++;
                }
            }
        }
    }

    if (candidateCount > 0) {
        int idx = GetRandomValue(0, candidateCount - 1);
        self->mayorRow = candidates[idx].r;
        self->mayorCol = candidates[idx].c;
    } else {
        self->mayorRow = 10;
        self->mayorCol = 10;
    }
    self->map.tiles[self->mayorRow][self->mayorCol] = 306;

    self->state = STATE_MENU;
    self->zombieTimer = 0.0f;
    self->zombieSpawnTimer = 0.0f;
    self->gameOverSelection = 0;
    self->menuSelection = 0;
    self->storyPage = 0;
    self->playerName[0] = '\0';
    self->playerNameLength = 0;
    self->lives = 3;
    self->checkpointActive = false;
    self->checkpointPosition = (Vector2){ 0, 0 };
    self->checkpointLevel = 0;
    self->checkpointState = STATE_EXPLORING;
    for (int i = 0; i < MAX_ZOMBIES; i++) self->zombies[i].active = false;
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) self->enemyProjectiles[i].active = false;
    for (int i = 0; i < MAX_PLAYER_PROJECTILES; i++) self->playerProjectiles[i].active = false;
    for (int i = 0; i < MAX_ASH_EFFECTS; i++) self->ashEffects[i].active = false;
    self->hasGun = false;
    self->gunSpawned = false;
    self->gunAbilityTimer = 0.0f;
    self->gunSpawnTimer = 4.0f;
    for (int i = 0; i < 6; i++) {
        self->bosses[i].spawned = false;
        self->bosses[i].defeated = false;
        self->bosses[i].showLog = false;
    }
    self->curiosityActivated = false;
    self->knowledgeActivated = false;
    self->truthActivated = false;
    self->curiosityPos = (Vector2){0, 0};
    self->knowledgePos = (Vector2){0, 0};
    self->truthPos = (Vector2){0, 0};

    self->bgMusic = LoadMusicStream("resources/bgm.mp3");
    if (FileExists("resources/slash.ogg")) {
        self->slashSound = LoadSound("resources/slash.ogg");
    } else {
        self->slashSound = LoadSound("resources/slash.wav");
    }
    self->blastSound = LoadSound("resources/blast.wav");
    self->hitSound = LoadSound("resources/hit.wav");
    self->hitSoundTimer = 0.0f;
    self->cut1Audio = LoadSound("cutscenes/cut1.wav");
    self->cut2Audio = LoadSound("cutscenes/cut2.wav");
    self->cutscenePart = 0;
    self->cutsceneTime = 0.0f;
    self->cutscenesLoaded = false;
    self->startTextTimer = 0.0f;

    self->playerTileset = LoadTexture("tilemap_packed.png");
    self->playerTilesPerRow = self->playerTileset.width / TILE_SIZE;
    if (FileExists("spirites_tilepacked.png")) {
        self->spritesTileset = LoadTexture("spirites_tilepacked.png");
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

    PlayMusicStream(self->bgMusic);
}

// --- State Machine Update Routine ---
void UpdateGame(GameState* game, float dt) {
    if (game->bosses[ENEMY_RAT_KING].showLog) {
        if (IsKeyPressed(KEY_ENTER)) game->bosses[ENEMY_RAT_KING].showLog = false;
        return;
    }
    if (game->bosses[ENEMY_DOOM_SCROLLER].showLog) {
        if (IsKeyPressed(KEY_ENTER)) game->bosses[ENEMY_DOOM_SCROLLER].showLog = false;
        return;
    }

    UpdateMusicStream(game->bgMusic);

    if (game->hitSoundTimer > 0.0f) game->hitSoundTimer -= dt;
    if (game->startTextTimer > 0.0f) game->startTextTimer -= dt;

    if (IsKeyPressed(KEY_M) && (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL || game->state == STATE_CUTSCENE)) {
        if (currentLevel == 0) {
            currentLevel = 1;
            UnloadTexture(game->map.tileset);
            Tilemap_Load(&game->map, "map2.csv", "tilemap_packed2.png");
            Player_Init(&game->player);
            game->state = STATE_SURVIVAL;
            game->zombieTimer = 0.0f;
            game->zombieSpawnTimer = 0.0f;
            for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
            SpawnZombie(game);
            SpawnZombie(game);
            SpawnRatKing(game, 10, 10);
            game->hasGun = false;
            game->gunSpawned = false;
            game->gunSpawnTimer = 4.0f;
            game->gunAbilityTimer = 0.0f;
            for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) game->playerProjectiles[p].active = false;
            game->startTextTimer = 4.0f;
        } else if (currentLevel == 1) {
            currentLevel = 2;
            UnloadTexture(game->map.tileset);
            Tilemap_Load(&game->map, "map3.csv", "tilemap_packed3.png");
            Player_Init(&game->player);
            game->state = STATE_SURVIVAL;
            game->zombieTimer = 0.0f;
            game->zombieSpawnTimer = 0.0f;
            for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
            SpawnDoomScroller(game);
            game->hasGun = false;
            game->gunSpawned = false;
            game->gunSpawnTimer = 4.0f;
            game->gunAbilityTimer = 0.0f;
            for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) game->playerProjectiles[p].active = false;
            game->startTextTimer = 4.0f;
        } else {
            currentLevel = 0;
            game->hasGun = false;
            game->gunSpawned = false;
            game->gunSpawnTimer = 4.0f;
            game->gunAbilityTimer = 0.0f;
            for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) game->playerProjectiles[p].active = false;
            UnloadTexture(game->map.tileset);
            Tilemap_Load(&game->map, "map1.csv", "tilemap_packed.png");
            for (int y = 0; y < MAP_HEIGHT; y++) {
                for (int x = 0; x < MAP_WIDTH; x++) {
                    int tid = game->map.tiles[y][x];
                    if (tid == 306 || tid == 308 || tid == 355) game->map.tiles[y][x] = 20;
                }
            }
            game->map.tiles[game->mayorRow][game->mayorCol] = 306;
            Player_Init(&game->player);
            game->state = STATE_EXPLORING;
            game->zombieTimer = 0.0f;
            game->zombieSpawnTimer = 0.0f;
            for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
            game->startTextTimer = 4.0f;
        }
    }

    int pCol = (int)(game->player.position.x / TILE_PX);
    int pRow = (int)(game->player.position.y / TILE_PX);

    for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
        if (game->ashEffects[i].active) {
            game->ashEffects[i].timer -= dt;
            if (game->ashEffects[i].timer <= 0.0f) game->ashEffects[i].active = false;
        }
    }

    if (IsKeyPressed(KEY_SPACE) && game->player.hasWeapon && !game->player.isAttacking) {
        if (game->player.lightAttackCooldown <= 0.0f) {
            PlaySound(game->slashSound);
            game->player.isAttacking = true;
            game->player.attackTimer = 0.15f;
            game->player.lightAttackCooldown = 0.33f;

            if (game->hasGun) {
                int projSlot = -1;
                for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) {
                    if (!game->playerProjectiles[p].active) {
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

                    game->playerProjectiles[projSlot].position = (Vector2){ startX, startY };
                    game->playerProjectiles[projSlot].velocity = vel;
                    game->playerProjectiles[projSlot].active = true;
                }
            }

            Vector2 slashPos = {
                game->player.position.x + TILE_PX / 2.0f,
                game->player.position.y + TILE_PX / 2.0f
            };

            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (IsZombieHit(game, i, slashPos, 64.0f)) {
                    DamageZombie(game, i, 100.0f);
                }
            }
        }
    }

    if (game->gunSpawned && pRow == game->gunRow && pCol == game->gunCol) {
        game->gunSpawned = false;
        game->hasGun = true;
        game->gunAbilityTimer = 4.0f;
        PlaySound(game->blastSound);
    }

    if (game->hasGun) {
        game->gunAbilityTimer -= dt;
        if (game->gunAbilityTimer <= 0.0f) game->hasGun = false;
    }

    if (!game->gunSpawned && !game->hasGun && (game->state == STATE_EXPLORING || game->state == STATE_SURVIVAL)) {
        game->gunSpawnTimer -= dt;
        if (game->gunSpawnTimer <= 0.0f) {
            SpawnGun(game);
            game->gunSpawnTimer = 4.0f;
        }
    }

    for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) {
        if (!game->playerProjectiles[p].active) continue;

        game->playerProjectiles[p].position.x += game->playerProjectiles[p].velocity.x * dt;
        game->playerProjectiles[p].position.y += game->playerProjectiles[p].velocity.y * dt;

        Vector2 pos = game->playerProjectiles[p].position;
        if (pos.x < 0 || pos.x > MAP_WIDTH * TILE_PX || pos.y < 0 || pos.y > MAP_HEIGHT * TILE_PX) {
            game->playerProjectiles[p].active = false;
            continue;
        }

        int cellX = (int)(pos.x / TILE_PX);
        int cellY = (int)(pos.y / TILE_PX);
        if (cellX >= 0 && cellX < MAP_WIDTH && cellY >= 0 && cellY < MAP_HEIGHT) {
            int type = GetTileType(game->map.tiles[cellY][cellX]);
            if (type == TILE_WALL || type == TILE_CAR) {
                game->playerProjectiles[p].active = false;
                continue;
            }
        }

        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (IsZombieHit(game, i, pos, 16.0f)) {
                game->playerProjectiles[p].active = false;
                PlaySound(game->hitSound);
                DamageZombie(game, i, 500.0f);
                break;
            }
        }
    }

    switch (game->state) {
        case STATE_MENU: {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                game->menuSelection--;
                if (game->menuSelection < 0) game->menuSelection = 4;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                game->menuSelection++;
                if (game->menuSelection > 4) game->menuSelection = 0;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                if (game->menuSelection == 0) game->state = STATE_NAME_PROMPT;
                else if (game->menuSelection == 1) { game->state = STATE_INFO; game->storyPage = 0; }
                else if (game->menuSelection == 2) game->state = STATE_HISTORY;
                else if (game->menuSelection == 3) game->state = STATE_TEAM;
                else if (game->menuSelection == 4) CloseWindow();
            }
            break;
        }

        case STATE_NAME_PROMPT: {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (game->playerNameLength < 15)) {
                    game->playerName[game->playerNameLength] = (char)key;
                    game->playerName[game->playerNameLength + 1] = '\0';
                    game->playerNameLength++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                game->playerNameLength--;
                if (game->playerNameLength < 0) game->playerNameLength = 0;
                game->playerName[game->playerNameLength] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER) && game->playerNameLength > 0) {
                StopMusicStream(game->bgMusic);
                GameState_LoadCutscenes(game);
                game->state = STATE_INTRO;
                game->cutscenePart = 0;
                game->cutsceneTime = 0.0f;
                PlaySound(game->cut1Audio);
            }
            if (IsKeyPressed(KEY_ESCAPE)) game->state = STATE_MENU;
            break;
        }

        case STATE_INTRO: {
            game->cutsceneTime += dt;
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
                StopSound(game->cut1Audio);
                StopSound(game->cut2Audio);
                GameState_UnloadCutscenes(game);
                game->state = STATE_EXPLORING;
                game->lives = 3;
                game->checkpointPosition = game->player.position;
                game->checkpointLevel = currentLevel;
                game->checkpointState = STATE_EXPLORING;
                game->checkpointActive = true;
                game->startTextTimer = 4.0f;
                PlayMusicStream(game->bgMusic);
                break;
            }
            if (game->cutscenePart == 0 && game->cutsceneTime >= 10.0f) {
                game->cutscenePart = 1;
                game->cutsceneTime = 0.0f;
                StopSound(game->cut1Audio);
                PlaySound(game->cut2Audio);
            }
            if (game->cutscenePart == 1 && game->cutsceneTime >= 4.0f) {
                StopSound(game->cut2Audio);
                GameState_UnloadCutscenes(game);
                game->state = STATE_EXPLORING;
                game->lives = 3;
                game->checkpointPosition = game->player.position;
                game->checkpointLevel = currentLevel;
                game->checkpointState = STATE_EXPLORING;
                game->checkpointActive = true;
                game->startTextTimer = 4.0f;
                PlayMusicStream(game->bgMusic);
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
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) game->state = STATE_MENU;
            break;
        }

        case STATE_HISTORY:
        case STATE_TEAM: {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_MENU;
            }
            break;
        }

        case STATE_EXPLORING: {
            Player_Update(&game->player, &game->map, dt);
            if (pRow >= 0 && pRow < MAP_HEIGHT && pCol >= 0 && pCol < MAP_WIDTH) {
                int tileID = game->map.tiles[pRow][pCol];
                if (GetTileType(tileID) == TILE_MAYOR) {
                    game->state = STATE_CUTSCENE;
                }
            }
            break;
        }

        case STATE_CUTSCENE: {
            if (IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_SURVIVAL;
                game->zombieTimer = 0.0f;
                game->zombieSpawnTimer = 0.0f;
                
                if (currentLevel == 0) {
                    game->map.tiles[game->mayorRow][game->mayorCol] = 236;
                    game->map.tiles[game->mayorRow][game->mayorCol + 1] = 237;
                    for (int i = 0; i < MAX_ZOMBIES; i++) game->zombies[i].active = false;
                    SpawnZombie(game);
                    SpawnZombie(game);
                    SpawnZombie(game);
                } else {
                    for (int i = 0; i < MAX_ZOMBIES; i++) game->zombies[i].active = false;
                    SpawnZombie(game);
                    SpawnZombie(game);
                    SpawnZombie(game);
                }

                game->checkpointPosition = game->player.position;
                game->checkpointLevel = currentLevel;
                game->checkpointState = STATE_SURVIVAL;
                game->checkpointActive = true;
            }
            break;
        }

        case STATE_SURVIVAL: {
            Player_Update(&game->player, &game->map, dt);

            bool canSpawn = true;
            if (currentLevel == 1 && game->bosses[ENEMY_RAT_KING].defeated) canSpawn = false;
            if (currentLevel == 2 && game->bosses[ENEMY_BRAINROT_GOD].defeated) canSpawn = false;

            game->zombieSpawnTimer += dt;
            if (game->zombieSpawnTimer >= 2.0f) {
                game->zombieSpawnTimer = 0.0f;
                if (canSpawn) SpawnZombie(game);
            }

            UpdateZombies(game, dt);

            if (pRow >= 0 && pRow < MAP_HEIGHT && pCol >= 0 && pCol < MAP_WIDTH) {
                int tileID = game->map.tiles[pRow][pCol];
                if (GetTileType(tileID) == TILE_CAVE) {
                    for (int i = 0; i < MAX_ZOMBIES; i++) game->zombies[i].active = false;

                    if (currentLevel == 0) {
                        GameHistory_SaveEntry(game->playerName, 1, false);
                        currentLevel = 1;
                        UnloadTexture(game->map.tileset);
                        Tilemap_Load(&game->map, "map2.csv", "tilemap_packed2.png");
                        
                        Player_Init(&game->player);
                        game->state = STATE_SURVIVAL;
                        game->zombieTimer = 0.0f;
                        game->zombieSpawnTimer = 0.0f;
                        for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
                        SpawnZombie(game);
                        SpawnZombie(game);
                        SpawnRatKing(game, 10, 10);
                        game->hasGun = false;
                        game->gunSpawned = false;
                        game->gunSpawnTimer = 4.0f;
                        game->gunAbilityTimer = 0.0f;
                        for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) game->playerProjectiles[p].active = false;
                        game->startTextTimer = 4.0f;

                        game->checkpointPosition = game->player.position;
                        game->checkpointLevel = currentLevel;
                        game->checkpointState = STATE_SURVIVAL;
                        game->checkpointActive = true;
                    } else if (currentLevel == 1) {
                        GameHistory_SaveEntry(game->playerName, 2, false);
                        currentLevel = 2;
                        UnloadTexture(game->map.tileset);
                        Tilemap_Load(&game->map, "map3.csv", "tilemap_packed3.png");
                        
                        Player_Init(&game->player);
                        game->state = STATE_SURVIVAL;
                        game->zombieTimer = 0.0f;
                        game->zombieSpawnTimer = 0.0f;
                        for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
                        
                        SpawnDoomScroller(game);
                        game->hasGun = false;
                        game->gunSpawned = false;
                        game->gunSpawnTimer = 4.0f;
                        game->gunAbilityTimer = 0.0f;
                        for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) game->playerProjectiles[p].active = false;
                        game->startTextTimer = 4.0f;

                        game->checkpointPosition = game->player.position;
                        game->checkpointLevel = currentLevel;
                        game->checkpointState = STATE_SURVIVAL;
                        game->checkpointActive = true;
                    }
                }
            }

            game->player.survivalTimer += dt;
            if (game->player.survivalTimer >= 7.0f && game->player.radiusCooldown <= 0.0f) {
                game->player.radiusPowerupReady = true;
            }

            if (game->player.radiusPowerupReady && IsKeyDown(KEY_F)) game->player.isAimingSuperpower = true;
            else game->player.isAimingSuperpower = false;

            if (game->player.radiusPowerupReady && IsKeyReleased(KEY_F) && game->player.radiusCooldown <= 0.0f) {
                PlaySound(game->blastSound);
                game->player.radiusPowerupReady = false;
                game->player.radiusCooldown = 8.0f;
                game->player.radiusBlastTimer = 0.5f;
                
                Vector2 blastPos = { pCol * TILE_PX + TILE_PX/2.0f, pRow * TILE_PX + TILE_PX/2.0f };
                for (int i = 0; i < MAX_ZOMBIES; i++) {
                    if (IsZombieHit(game, i, blastPos, 160.0f)) DamageZombie(game, i, 50.0f);
                }
            }

            if (currentLevel == 2 && game->brainrotGodSpawned && !game->brainrotGodDefeated) {
                Vector2 pPos = { game->player.position.x + TILE_PX/2.0f, game->player.position.y + TILE_PX/2.0f };
                
                if (!game->curiosityActivated) {
                    float dx = pPos.x - game->curiosityPos.x;
                    float dy = pPos.y - game->curiosityPos.y;
                    if (sqrtf(dx*dx + dy*dy) < 24.0f) {
                        game->curiosityActivated = true;
                        PlaySound(game->blastSound);
                        for (int z = 0; z < MAX_ZOMBIES; z++) {
                            if (game->zombies[z].type == ENEMY_BRAINROT_GOD) DamageZombie(game, z, 200.0f);
                        }
                    }
                }
                if (!game->knowledgeActivated) {
                    float dx = pPos.x - game->knowledgePos.x;
                    float dy = pPos.y - game->knowledgePos.y;
                    if (sqrtf(dx*dx + dy*dy) < 24.0f) {
                        game->knowledgeActivated = true;
                        PlaySound(game->blastSound);
                        for (int z = 0; z < MAX_ZOMBIES; z++) {
                            if (game->zombies[z].type == ENEMY_BRAINROT_GOD) DamageZombie(game, z, 200.0f);
                        }
                    }
                }
                if (!game->truthActivated) {
                    float dx = pPos.x - game->truthPos.x;
                    float dy = pPos.y - game->truthPos.y;
                    if (sqrtf(dx*dx + dy*dy) < 24.0f) {
                        game->truthActivated = true;
                        PlaySound(game->blastSound);
                        for (int z = 0; z < MAX_ZOMBIES; z++) {
                            if (game->zombies[z].type == ENEMY_BRAINROT_GOD) DamageZombie(game, z, 200.0f);
                        }
                    }
                }
            }

            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (game->zombies[i].active && game->zombies[i].row == pRow && game->zombies[i].col == pCol) {
                    game->player.health -= 30.0f * dt;
                    if (game->hitSoundTimer <= 0.0f) {
                        PlaySound(game->hitSound);
                        game->hitSoundTimer = 0.5f;
                    }
                    if (game->player.health <= 0.0f) {
                        game->player.health = 0.0f;
                        
                        if (game->checkpointActive && game->lives > 1) {
                            game->lives--;
                            game->player.health = 100.0f;
                            
                            if (currentLevel != game->checkpointLevel) {
                                currentLevel = game->checkpointLevel;
                                UnloadTexture(game->map.tileset);
                                if (currentLevel == 0) Tilemap_Load(&game->map, "map1.csv", "tilemap_packed.png");
                                else if (currentLevel == 1) Tilemap_Load(&game->map, "map2.csv", "tilemap_packed2.png");
                                else Tilemap_Load(&game->map, "map3.csv", "tilemap_packed3.png");
                            }
                            
                            game->player.position = game->checkpointPosition;
                            game->player.gridX = (int)(game->player.position.x / TILE_PX);
                            game->player.gridY = (int)(game->player.position.y / TILE_PX);
                            game->state = game->checkpointState;
                            for (int z = 0; z < MAX_ZOMBIES; z++) game->zombies[z].active = false;
                        } else {
                            GameHistory_SaveEntry(game->playerName, currentLevel + 1, false);
                            game->state = STATE_GAMEOVER;
                        }
                    }
                }
            }
            break;
        }

        case STATE_GAMEOVER: {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) game->gameOverSelection = 0;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) game->gameOverSelection = 1;
            if (IsKeyPressed(KEY_ENTER)) {
                if (game->gameOverSelection == 0) {
                    GameState_Init(game);
                    game->state = STATE_SURVIVAL;
                } else game->state = STATE_MENU;
            }
            break;
        }

        case STATE_WIN: {
            if (IsKeyPressed(KEY_ENTER)) game->state = STATE_MENU;
            break;
        }
    }
}

// --- State Machine Draw Routine ---
void DrawGame(const GameState* game) {
    if (game->state == STATE_INTRO) {
        ClearBackground(BLACK);
        int frameIndex = (int)(game->cutsceneTime * 15.0f);
        Texture2D frameTex = { 0 };
        if (game->cutscenePart == 0) {
            if (frameIndex >= 0 && frameIndex < CUT1_FRAMES) frameTex = game->cut1Textures[frameIndex];
        } else {
            if (frameIndex >= 0 && frameIndex < CUT2_FRAMES) frameTex = game->cut2Textures[frameIndex];
        }

        if (frameTex.id > 0) {
            DrawTexturePro(frameTex, 
                           (Rectangle){ 0, 0, (float)frameTex.width, (float)frameTex.height },
                           (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
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
        Camera2D camera = { 0 };
        camera.target = (Vector2){ game->player.position.x + TILE_PX / 2.0f, game->player.position.y + TILE_PX / 2.0f };
        camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        camera.rotation = 0.0f;
        camera.zoom = 2.5f;

        BeginMode2D(camera);

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                int tileID = game->map.tiles[y][x];
                DrawTile(game->map.tileset, game->map.tilesPerRow, tileID, (float)(x * TILE_PX), (float)(y * TILE_PX));
            }
        }

        for (int i = 0; i < MAX_ASH_EFFECTS; i++) {
            if (game->ashEffects[i].active) {
                float ax = game->ashEffects[i].gridX * TILE_PX;
                float ay = game->ashEffects[i].gridY * TILE_PX;
                float alpha = (game->ashEffects[i].timer / 1.5f) * 0.8f;
                DrawRectangle(ax, ay, TILE_PX, TILE_PX, ColorAlpha((Color){30, 30, 30, 255}, alpha));
            }
        }

        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (game->zombies[i].active) {
                float zx = game->zombies[i].position.x;
                float zy = game->zombies[i].position.y;
                
                if (currentLevel == 0) {
                    if (game->zombies[i].type == ENEMY_RAT_KING) {
                        DrawTexturePro(game->spritesTileset, 
                                       (Rectangle){ (float)(23 % game->spritesTilesPerRow) * TILE_SIZE, (float)(23 / game->spritesTilesPerRow) * TILE_SIZE, TILE_SIZE, TILE_SIZE },
                                       (Rectangle){ zx - TILE_PX*0.75f, zy - TILE_PX*0.75f, TILE_PX * 2.5f, TILE_PX * 2.5f },
                                       (Vector2){ 0, 0 }, 0.0f, PURPLE);
                    } else {
                        DrawTile(game->map.tileset, game->map.tilesPerRow, 331, zx, zy);
                        DrawRectangle(zx, zy, TILE_PX, TILE_PX, ColorAlpha(RED, 0.45f));
                    }
                } else if (currentLevel == 1) {
                    if (game->zombies[i].type == ENEMY_DOOM_SCROLLER) {
                        DrawTexturePro(game->spritesTileset,
                                       (Rectangle){ (float)(306 % game->spritesTilesPerRow) * TILE_SIZE, (float)(306 / game->spritesTilesPerRow) * TILE_SIZE, TILE_SIZE, TILE_SIZE },
                                       (Rectangle){ zx - TILE_PX*1.0f, zy - TILE_PX*1.0f, TILE_PX * 3.0f, TILE_PX * 3.0f },
                                       (Vector2){ 0, 0 }, 0.0f, RED);
                    } else {
                        int zTileID = (game->zombies[i].type == ENEMY_SPIDER) ? 23 : 20;
                        DrawTile(game->spritesTileset, game->spritesTilesPerRow, zTileID, zx, zy);
                    }
                } else {
                    if (game->zombies[i].type == ENEMY_ALGORITHM) {
                        DrawTexturePro(game->spritesTileset,
                                       (Rectangle){ (float)(308 % game->spritesTilesPerRow) * TILE_SIZE, (float)(308 / game->spritesTilesPerRow) * TILE_SIZE, TILE_SIZE, TILE_SIZE },
                                       (Rectangle){ zx - TILE_PX*0.75f, zy - TILE_PX*0.75f, TILE_PX * 2.5f, TILE_PX * 2.5f },
                                       (Vector2){ 0, 0 }, 0.0f, VIOLET);
                    } else if (game->zombies[i].type == ENEMY_BRAINROT_GOD) {
                        float pulse = sinf(GetTime() * 4.0f) * 4.0f;
                        DrawTexturePro(game->spritesTileset,
                                       (Rectangle){ (float)(27 % game->spritesTilesPerRow) * TILE_SIZE, (float)(27 / game->spritesTilesPerRow) * TILE_SIZE, TILE_SIZE, TILE_SIZE },
                                       (Rectangle){ zx - TILE_PX*1.5f, zy - TILE_PX*1.5f, TILE_PX * 4.0f + pulse, TILE_PX * 4.0f + pulse },
                                       (Vector2){ 0, 0 }, 0.0f, WHITE);
                    } else {
                        DrawTile(game->map.tileset, game->map.tilesPerRow, 331, zx, zy);
                        DrawRectangle(zx, zy, TILE_PX, TILE_PX, ColorAlpha(RED, 0.45f));
                    }
                }

                float barWidth = (float)TILE_PX;
                float barHeight = 4.0f;
                if (game->zombies[i].type == ENEMY_RAT_KING || game->zombies[i].type == ENEMY_ALGORITHM) {
                    barWidth = TILE_PX * 2.5f;
                    barHeight = 6.0f;
                } else if (game->zombies[i].type == ENEMY_DOOM_SCROLLER) {
                    barWidth = TILE_PX * 3.0f;
                    barHeight = 6.0f;
                } else if (game->zombies[i].type == ENEMY_BRAINROT_GOD) {
                    barWidth = TILE_PX * 4.0f;
                    barHeight = 8.0f;
                }
                float zx_offset = zx - (barWidth - TILE_PX)/2.0f;
                float z_py = zy - 10.0f;
                DrawRectangle(zx_offset, z_py, barWidth, barHeight, ColorAlpha(BLACK, 0.6f));
                float pct = game->zombies[i].health / game->zombies[i].maxHealth;
                if (pct < 0.0f) pct = 0.0f;
                DrawRectangle(zx_offset + 1, z_py + 1, (barWidth - 2) * pct, barHeight - 2, RED);

                if (game->zombies[i].type == ENEMY_BRAINROT_GOD) {
                    DrawText("USE FRAGMENTS", zx - 24, z_py - 12, 10, YELLOW);
                }
            }
        }

        if (currentLevel == 2 && game->brainrotGodSpawned && !game->brainrotGodDefeated) {
            if (!game->curiosityActivated) {
                float pulse = sinf(GetTime() * 5.0f);
                DrawCircleLines(game->curiosityPos.x + TILE_PX/2.0f, game->curiosityPos.y + TILE_PX/2.0f, 15.0f + pulse * 4.0f, GREEN);
                DrawCircle(game->curiosityPos.x + TILE_PX/2.0f, game->curiosityPos.y + TILE_PX/2.0f, 6.0f, ColorAlpha(GREEN, 0.5f));
                DrawText("CURIOSITY", game->curiosityPos.x - 12, game->curiosityPos.y - 12, 10, GREEN);
            }
            if (!game->knowledgeActivated) {
                float pulse = sinf(GetTime() * 5.0f);
                DrawCircleLines(game->knowledgePos.x + TILE_PX/2.0f, game->knowledgePos.y + TILE_PX/2.0f, 15.0f + pulse * 4.0f, YELLOW);
                DrawCircle(game->knowledgePos.x + TILE_PX/2.0f, game->knowledgePos.y + TILE_PX/2.0f, 6.0f, ColorAlpha(YELLOW, 0.5f));
                DrawText("KNOWLEDGE", game->knowledgePos.x - 15, game->knowledgePos.y - 12, 10, YELLOW);
            }
            if (!game->truthActivated) {
                float pulse = sinf(GetTime() * 5.0f);
                DrawCircleLines(game->truthPos.x + TILE_PX/2.0f, game->truthPos.y + TILE_PX/2.0f, 15.0f + pulse * 4.0f, RAYWHITE);
                DrawCircle(game->truthPos.x + TILE_PX/2.0f, game->truthPos.y + TILE_PX/2.0f, 6.0f, ColorAlpha(RAYWHITE, 0.5f));
                DrawText("TRUTH", game->truthPos.x - 2, game->truthPos.y - 12, 10, RAYWHITE);
            }
        }

        for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
            if (game->enemyProjectiles[i].active) {
                Vector2 p = game->enemyProjectiles[i].position;
                float r = game->enemyProjectiles[i].isBig ? 12.0f : 5.0f;
                Color innerCol = game->enemyProjectiles[i].isBig ? PURPLE : ORANGE;
                Color outerCol = game->enemyProjectiles[i].isBig ? MAGENTA : RED;
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
            
            for (int i = 0; i < 6; i++) {
                float ox = rx + GetRandomValue(-2, 2);
                float oy = ry + GetRandomValue(-2, 2);
                Color fColor = (i % 3 == 0) ? GOLD : ((i % 2 == 0) ? ORANGE : RED);
                DrawRectangle(ox, oy, rw, rh, ColorAlpha(fColor, 0.6f));
            }
        }

        if (game->gunSpawned) {
            float gx = game->gunCol * TILE_PX;
            float gy = game->gunRow * TILE_PX;
            DrawTile(game->spritesTileset, game->spritesTilesPerRow, 133, gx, gy);
            float pulse = sinf(GetTime() * 6.0f);
            DrawCircleLines(gx + TILE_PX/2.0f, gy + TILE_PX/2.0f, 10.0f + pulse * 4.0f, SKYBLUE);
            DrawCircleLines(gx + TILE_PX/2.0f, gy + TILE_PX/2.0f, 10.0f + pulse * 4.0f + 1.0f, BLUE);
        }

        for (int p = 0; p < MAX_PLAYER_PROJECTILES; p++) {
            if (game->playerProjectiles[p].active) {
                Vector2 pos = game->playerProjectiles[p].position;
                DrawCircle(pos.x, pos.y, 6.0f, SKYBLUE);
                DrawCircle(pos.x, pos.y, 4.0f, BLUE);
                DrawCircle(pos.x, pos.y, 2.0f, WHITE);
            }
        }

        EndMode2D();
    }

    if (game->state == STATE_MENU) DrawMenuScreen(game);
    else if (game->state == STATE_INFO) DrawInfoScreen(game);
    else if (game->state == STATE_HISTORY) DrawHistoryScreen(game);
    else if (game->state == STATE_TEAM) DrawTeamScreen(game);
    else if (game->state == STATE_GAMEOVER) DrawGameOverScreen(game);
    else if (game->state == STATE_WIN) DrawWinScreen(game);
    else DrawHUD(game);

    if (game->startTextTimer > 0.0f) {
        float alpha = 1.0f;
        if (game->startTextTimer > 3.0f) alpha = (4.0f - game->startTextTimer);
        else if (game->startTextTimer < 1.0f) alpha = game->startTextTimer;

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
    UnloadMusicStream(self->bgMusic);
    UnloadSound(self->slashSound);
    UnloadSound(self->blastSound);
    UnloadSound(self->hitSound);
    UnloadSound(self->cut1Audio);
    UnloadSound(self->cut2Audio);
    UnloadTexture(self->playerTileset);
    if (self->spritesTileset.id != self->playerTileset.id) {
        UnloadTexture(self->spritesTileset);
    }
    if (self->gameFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(self->gameFont);
    }
}
