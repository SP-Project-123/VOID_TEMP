#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>

#define MAP_WIDTH 35
#define MAP_HEIGHT 24
#define TILE_SIZE 8
#define RENDER_ZOOM 4
#define TILE_PX (TILE_SIZE * RENDER_ZOOM)

#define MAX_ZOMBIES 30

// --- Gameplay Parameters & Configs ---
#define GUN_ABILITY_DURATION 4.0f
#define GUN_SPAWN_TIMER_INITIAL 4.0f
#define SUPERPOWER_COOLDOWN_MAX 6.0f
#define SUPERPOWER_BLAST_DURATION 0.5f
#define SUPERPOWER_READY_TIME 5.0f
#define SUPERPOWER_RADIUS 80.0f
#define SUPERPOWER_DAMAGE 200.0f
#define LEVEL1_SPAWN_INTERVAL 2.5f
#define LEVEL2_SPAWN_INTERVAL 1.8f
#define LEVEL3_SPAWN_INTERVAL 1.2f
#define TILE_MAYOR_ID 283
#define TILE_CAVE_EXIT_L2 306

// --- Ranged Enemy & Audio Parameters ---
#define ENEMY_PROJECTILE_SPEED_BIG 150.0f
#define ENEMY_PROJECTILE_SPEED_SMALL 180.0f
#define ENEMY_PROJECTILE_LIFETIME 2.5f
#define ENEMY_PROJECTILE_COLLISION_RADIUS_BIG 22.0f
#define ENEMY_PROJECTILE_COLLISION_RADIUS_SMALL 14.0f
#define HIT_SOUND_COOLDOWN 0.25f

// --- Gameplay Settings & Configurations ---
#define PLAYER_INITIAL_LIVES 3
#define PLAYER_INITIAL_HEALTH 200.0f
#define PLAYER_MAX_HEALTH 200.0f
#define PLAYER_SPEED 140.0f
#define PLAYER_SWORD_DAMAGE 100.0f
#define PLAYER_PROJECTILE_DAMAGE 200.0f

#define ZOMBIE_DAMAGE_RATE 30.0f
#define RANGED_ZOMBIE_PROJECTILE_DAMAGE_SMALL 12.0f
#define RANGED_ZOMBIE_PROJECTILE_DAMAGE_BIG 25.0f
#define BRAINROT_GOD_PUZZLE_DAMAGE 200.0f

// Enemy base health
#define SNAKE_BASE_HP 30.0f
#define SPIDER_BASE_HP 30.0f
#define GHOST_BASE_HP 40.0f
#define RAT_KING_BASE_HP 300.0f
#define DOOM_SCROLLER_BASE_HP 400.0f
#define ALGORITHM_BASE_HP 200.0f
#define BRAINROT_GOD_BASE_HP 600.0f

// Enemy base movement speed
#define SNAKE_BASE_SPEED 55.0f
#define SPIDER_BASE_SPEED 55.0f
#define GHOST_BASE_SPEED 65.0f
#define RAT_KING_BASE_SPEED 40.0f
#define DOOM_SCROLLER_BASE_SPEED 20.0f
#define ALGORITHM_BASE_SPEED 45.0f
#define BRAINROT_GOD_BASE_SPEED 50.0f

// --- Tile Types ---
#define TILE_GROUND 0
#define TILE_WALL 1
#define TILE_MAYOR 3
#define TILE_CAVE 4

// --- Game States ---
typedef enum {
    STATE_MENU,
    STATE_NAME_PROMPT,
    STATE_INTRO,
    STATE_HISTORY,
    STATE_TEAM,
    STATE_EXPLORING,
    STATE_CUTSCENE,
    STATE_SURVIVAL,
    STATE_GAMEOVER,
    STATE_WIN
} GameMode;

// --- Directions ---
typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

// --- Spatial Map Layout Structures ---
typedef struct {
    int tiles[MAP_HEIGHT][MAP_WIDTH];
    Texture2D tileset;
    int tilesPerRow;
} Tilemap;

// --- Enemy Type ---
typedef enum {
    ENEMY_SNAKE,
    ENEMY_SPIDER,
    ENEMY_GHOST,
    ENEMY_RAT_KING,
    ENEMY_DOOM_SCROLLER,
    ENEMY_ALGORITHM,
    ENEMY_BRAINROT_GOD
} EnemyType;

// --- Unified Boss Status System ---
typedef enum {
    BOSS_RAT_KING = 0,
    BOSS_DOOM_SCROLLER = 1,
    BOSS_ALGORITHM = 2,
    BOSS_BRAINROT_GOD = 3
} BossId;

typedef struct {
    bool spawned;
    bool defeated;
    bool showLog;
    int tileId;
} BossStatus;

// --- Memory Fragment System ---
typedef struct {
    bool activated;
    Vector2 position;
    const char* name;
} MemoryFragment;

// --- Unified Enemy/Boss Properties ---
typedef struct {
    float maxHealth;
    float moveSpeed;
    int baseTileId;
    float scale;
    Color color;
    float hitboxOffset;
} EnemyProperties;

// --- Zombie / Enemy Struct ---
typedef struct {
    int row; 
    int col;
    bool active;
    float health;
    float maxHealth;
    EnemyType type;
    float shootTimer;
    Vector2 position;
    Vector2 ghostVel;
} Zombie;

// --- Enemy Ranged Projectile ---
#define MAX_ENEMY_PROJECTILES 30
typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
    bool isBig;
    float lifeTimer;
} EnemyProjectile;

typedef struct {
    const char* csvMap;
    const char* tilesetPng;
    const char* objectiveText;
} LevelConfig;

extern const LevelConfig g_levelConfigs[4];

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

// --- Player Ranged Projectile ---
#define MAX_PLAYER_PROJECTILES 20
typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
} PlayerProjectile;

// --- Global Engine Core State ---
typedef struct {
    char name[16];
    int nameLength;
    int lives;
    Vector2 checkpointPosition;
    int checkpointLevel;
    GameMode checkpointState;
    bool checkpointActive;
    PlayerProjectile projectiles[MAX_PLAYER_PROJECTILES];
    bool hasGun;
    bool gunSpawned;
    int gunRow;
    int gunCol;
    float gunAbilityTimer;
    float gunSpawnTimer;
} PlayerData;

typedef struct {
    Sound cut1Audio;
    Sound cut2Audio;
    Sound cutCaveAudio;
    int part;
    float time;
    bool loaded;
    Texture2D cut1Textures[152];
    Texture2D cut2Textures[62];
    Texture2D cutCaveTextures[240];
    Texture2D endsceneTextures[12];
    GameMode targetState;
    int targetLevel;
} CutsceneData;

typedef struct {
    Music bgMusic;
    Sound slash;
    Sound blast;
    Sound hit;
    float hitTimer;
} AudioData;

typedef struct {
    int mayorRow;
    int mayorCol;
    int potionRow;
    int potionCol;
    bool potionSpawned;
    int bossRow;
    int bossCol;
    float escapeTimer;
    float startTextTimer;
} LevelData;

typedef struct {
    Zombie list[MAX_ZOMBIES];
    float timer;
    float spawnTimer;
    AshTile ashEffects[MAX_ASH_EFFECTS];
    EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES];
    BossStatus status[4];
    MemoryFragment fragments[3];
    int killedCount;
} EnemyData;

typedef struct {
    int gameOverSelection;
    int menuSelection;
    int storyPage;
    int difficulty;
} MenuData;

typedef struct {
    Tilemap map;
    Player player;
    GameMode state;
    PlayerData playerInfo;
    CutsceneData cutscene;
    AudioData audio;
    LevelData levelInfo;
    EnemyData enemies;
    MenuData menu;
    Font gameFont;
    Texture2D playerTileset;
    int playerTilesPerRow;
    Texture2D spritesTileset;
    int spritesTilesPerRow;
} GameState;

// --- Global Level Tracker ---
extern int currentLevel;
void LoadLevel(GameState* game, int levelIndex);

// --- Game History Structure & Functions ---
#define MAX_HISTORY_ENTRIES 8
typedef struct {
    char entries[MAX_HISTORY_ENTRIES][64];
    int count;
} GameHistory;

void GameHistory_SaveEntry(const char* name, int levelCleared, bool victory);
void GameHistory_Load(GameHistory* history);

// --- Boss & Combat Helper Functions ---
void OnEnemyDeath(GameState* game, int idx);
bool IsZombieHit(const GameState* game, int i, Vector2 hitPos, float size);
void DamageZombie(GameState* game, int idx, float damage);
void SpawnGun(GameState* game);

// --- Drawing Helpers ---
void DrawTile(Texture2D tileset, int tilesPerRow, int tileID, float x, float y);
void DrawCustomText(const GameState* game, const char* text, float posX, float posY, float fontSize, Color color);
float MeasureCustomText(const GameState* game, const char* text, float fontSize);

// --- UI Screen Draw Routines ---
void DrawMenuScreen(const GameState* game);
void DrawNamePromptScreen(const GameState* game);
void DrawHistoryScreen(const GameState* game);
void DrawTeamScreen(const GameState* game);
void DrawGameOverScreen(const GameState* game);
void DrawWinScreen(const GameState* game);
void DrawHUD(const GameState* game);
void DrawStoryOverlays(const GameState* game);

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
void SpawnEnemy(GameState* game, EnemyType type, int r, int c);
void SpawnMemoryFragments(GameState* game);
void UpdateZombies(GameState* game, float dt);
BossId GetBossId(EnemyType type);
EnemyProperties GetEnemyProperties(EnemyType type, int level, int difficulty);

#endif