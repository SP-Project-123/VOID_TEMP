#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>

#define MAP_WIDTH 35
#define MAP_HEIGHT 24
#define TILE_SIZE 8
#define RENDER_ZOOM 4
#define TILE_PX (TILE_SIZE * RENDER_ZOOM)

#define MAX_ZOMBIES 15

// --- Tile Types ---
#define TILE_GROUND 0
#define TILE_WALL 1
#define TILE_CAR 2
#define TILE_MAYOR 3
#define TILE_CAVE 4

// --- Game States ---
typedef enum {
    STATE_MENU,
    STATE_INFO,
    STATE_TEAM,
    STATE_EXPLORING,
    STATE_CUTSCENE,
    STATE_SURVIVAL,
    STATE_GAMEOVER
} GameMode;

// --- Directions ---
typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

// --- Spatial Map Layout Structures ---
typedef struct {
    int tiles[MAP_HEIGHT][MAP_WIDTH];
    Texture2D tileset;
    int tilesPerRow;
} Tilemap;

// --- Zombie / Enemy Struct ---
typedef struct {
    int row; 
    int col;
    bool active;
    float health;
    float maxHealth;
} Zombie;

// --- Environmental Effects ---
#define MAX_ASH_EFFECTS 50
typedef struct {
    int gridX;
    int gridY;
    float timer;
    bool active;
} AshTile;

// --- Player Character Object Structures ---
typedef struct {
    Vector2 position;
    float speed;
    int currentObjective;
    bool isMoving;
    int animFrame;
    int gridX;
    int gridY;
    Direction direction;
    bool hasWeapon;
    bool isAttacking;
    float attackTimer;
    float lightAttackCooldown;
    float survivalTimer;
    bool radiusPowerupReady;
    float radiusCooldown;
    float radiusBlastTimer;
    bool isAimingSuperpower;
    float health;
    float maxHealth;
} Player;

// --- Global Engine Core State ---
typedef struct {
    Tilemap map;
    Player player;
    GameMode state;
    Zombie zombies[MAX_ZOMBIES];
    float zombieTimer;
    float zombieSpawnTimer;
    AshTile ashEffects[MAX_ASH_EFFECTS];
    int gameOverSelection;
    int menuSelection;
    int storyPage;

    // --- Audio Handles ---
    Music bgMusic;
    Sound slashSound;
    Sound blastSound;
    Sound hitSound;
    float hitSoundTimer; // Cooldown timer so damage SFX doesn't trigger every frame
} GameState;

// --- OOP & Helper Function Implementations ---
int GetTileType(int tileID);

void Tilemap_Load(Tilemap* self, const char* csvPath, const char* texturePath);
void Tilemap_Draw(const Tilemap* self);
bool Tilemap_IsWalkable(const Tilemap* self, float targetX, float targetY);

void Player_Init(Player* self);
void Player_Update(Player* self, const Tilemap* map, float dt);
void Player_Draw(const Player* self, Texture2D tileset, int tilesPerRow);

void GameState_Init(GameState* self);
void GameState_Unload(GameState* self);
void UpdateGame(GameState* game, float dt);
void DrawGame(const GameState* game);

void SpawnZombie(GameState* game);
void UpdateZombies(GameState* game, float dt);

#endif