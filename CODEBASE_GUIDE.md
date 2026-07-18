# THE GAMEROT — Complete Codebase Documentation

> A ground-up reference covering every function, every formula, every mechanic, and viva Q&A.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Build & Run](#2-build--run)
3. [Architecture & File Map](#3-architecture--file-map)
4. [Core Constants & Configuration](#4-core-constants--configuration)
5. [Data Structures — Complete Reference](#5-data-structures--complete-reference)
6. [Enums — Game States, Enemies, Directions](#6-enums)
7. [Function Reference — Every Function Explained](#7-function-reference)
   - 7.1 main.c
   - 7.2 game.c
   - 7.3 boss.c
   - 7.4 player.c
   - 7.5 zombie.c
   - 7.6 tilemap.c
   - 7.7 ui.c
   - 7.8 tileset_viewer.c
8. [All Math Explained](#8-all-math-explained)
9. [Game State Machine — Full Flow Diagram](#9-game-state-machine)
10. [Enemy Behavior — Every Type](#10-enemy-behavior)
11. [Level Progression & What Changes](#11-level-progression)
12. ["Changing X Affects Y" Dependency Map](#12-dependency-map)
13. [Viva Q&A Preparation](#13-viva-qa)

---

## 1. Project Overview

**The GameRot** is a top-down 2D survival/exploration game built in **C99** using the **Raylib** game library. The player navigates tile-based maps, explores areas, fights waves of enemies, defeats bosses, and follows a narrative about an AI called "Mnemosyne" that replaced human knowledge with "brainrot" content.

| Property | Value |
|---|---|
| Language | C99 (`-std=c99`) |
| Graphics Library | Raylib |
| Map Size | 35 columns × 24 rows |
| Tile Size (source) | 8×8 pixels |
| Render Zoom | 4× |
| Effective Tile | 32×32 pixels on screen |
| Window Size | 1120 × 768 pixels (35×32 by 24×32) |
| Target FPS | 60 |
| Max Enemies | 15 simultaneous |
| Max Enemy Projectiles | 30 |
| Max Player Projectiles | 20 |
| Max Ash Effects | 50 |
| Player HP | 100 |
| Player Speed | 120 px/s |
| Lives | 3 |

---

## 2. Build & Run

### Makefile Breakdown

```makefile
CC = gcc
CFLAGS = -Wall -std=c99 -Wno-missing-braces
```

- **Raylib paths** default to `C:/raylib/raylib/src` but can be overridden via environment variables (`RAYLIB_PATH`, `RAYLIB_INCLUDE`, `RAYLIB_LIB`).
- **Include paths**: `-Isrc/include -I$(RAYLIB_INCLUDE)` — picks up `common.h` and Raylib headers.
- **Link flags**: `-lraylib -lopengl32 -lgdi32 -lwinmm` — links Raylib + Windows graphics/audio libs.

### Source Files Compiled

```
src/main.c
src/core/game.c
src/core/boss.c
src/ui/ui.c
src/map/tilemap.c
src/player/player.c
src/zombie/zombie.c
```

### Build Commands

| Command | What it does |
|---|---|
| `make` or `make all` | Compiles the game → `game.exe` |
| `make viewer` | Compiles the tileset viewer tool → `tileset_viewer.exe` |
| `make clean` | (declared but not implemented) |

### Runtime Dependencies

The game expects these files in the working directory:
- `map1.csv`, `map2.csv`, `map3.csv`, `finalmap.csv` — tilemap data
- `tilemap_packed.png`, `tilemap_packed2.png`, `tilemap_packed3.png`, `finalmap_packed.png` — tileset textures
- `spirites_tilepacked.png` — enemy/sprite tileset
- `resources/bgm.mp3` — background music
- `resources/slash.ogg` (or `.wav`) — sword slash sound
- `resources/blast.wav` — explosion/blast sound
- `resources/hit.wav` — damage taken sound
- `cutscenes/cut1.wav`, `cutscenes/cut2.wav` — cutscene audio
- `cutscenes/frames/cut1_001.png` to `cut1_152.png` — cutscene 1 frames
- `cutscenes/frames/cut2_001.png` to `cut2_062.png` — cutscene 2 frames
- `C:/Windows/Fonts/consola.ttf` (or `lucon.ttf`) — game font

---

## 3. Architecture & File Map

```
VOID_TEMP/
├── main.c                 Entry point — window init, game loop, cleanup
├── core/
│   ├── game.c             Master state machine (~1001 lines) — UpdateGame, DrawGame, init, transitions
│   └── boss.c             Boss logic, combat helpers, enemy stat table (~189 lines)
├── player/
│   └── player.c           Player init, movement, collision, drawing (~117 lines)
├── zombie/
│   └── zombie.c           Enemy spawning, AI, projectiles, movement (~358 lines)
├── map/
│   └── tilemap.c          CSV map loading, tile classification, walkability (~113 lines)
├── ui/
│   └── ui.c               All screens, HUD, drawing helpers, history (~469 lines)
├── include/
│   └── common.h           All shared types, constants, function declarations (~274 lines)
├── tools/
│   └── tileset_viewer.c   Standalone tileset inspection tool (not part of game)
├── Makefile               Build configuration
├── map1.csv - map3.csv, finalmap.csv   Tilemap data (35×24 grids of tile IDs)
├── tilemap_packed*.png    Tileset sprite sheets
├── spirites_tilepacked.png Enemy sprite sheet
├── resources/             Audio files
├── cutscenes/             Cutscene frames and audio
└── history.txt            Game run history log
```

### Dependency Graph (who includes/calls whom)

```
main.c
  └── common.h (types, constants, function declarations)
  └── calls: GameState_Init, UpdateGame, DrawGame, GameState_Unload

game.c
  └── common.h
  └── calls into: tilemap.c (Tilemap_Load, GetTileType)
                  player.c (Player_Init, Player_Update, Player_Draw)
                  zombie.c (SpawnZombie, SpawnEnemy, UpdateZombies, SpawnMemoryFragments)
                  boss.c (OnZombieDeath, IsZombieHit, DamageZombie, SpawnGun, GetEnemyProperties, GetBossId)
                  ui.c (DrawMenuScreen, DrawNamePromptScreen, DrawInfoScreen, etc.)

boss.c
  └── common.h
  └── calls: GetTileType, GetEnemyProperties, SpawnEnemy, SpawnMemoryFragments

player.c
  └── common.h
  └── calls: Tilemap_IsWalkable, DrawTile

zombie.c
  └── common.h
  └── calls: GetTileType, GetEnemyProperties, GetBossId, SpawnEnemy

tilemap.c
  └── common.h
  └── calls: DrawTile (from ui.c)

ui.c
  └── common.h
  └── calls: DrawTile (self-referencing within same file)
```

### The `extern int currentLevel` Global

Declared in `common.h` (line 217), defined in `game.c` (line 7). This is a **global variable** shared across all `.c` files via `extern`. It tracks which level (0-3) the player is currently on. It affects:
- Tile walkability classification in `GetTileType()`
- Enemy stat scaling in `GetEnemyProperties()`
- Spawn type selection in `SpawnZombie()`
- Starting position in `Player_Init()`
- Level transition logic in `UpdateGame()`
- HUD display in `DrawHUD()`

---

## 4. Core Constants & Configuration

### From `common.h`

| Constant | Value | Meaning |
|---|---|---|
| `MAP_WIDTH` | 35 | Tiles per row |
| `MAP_HEIGHT` | 24 | Tiles per column |
| `TILE_SIZE` | 8 | Source tile size in pixels |
| `RENDER_ZOOM` | 4 | Zoom factor |
| `TILE_PX` | 32 | Rendered tile size (8 × 4) |
| `MAX_ZOMBIES` | 15 | Max simultaneous enemies |
| `MAX_ENEMY_PROJECTILES` | 30 | Max enemy bullets on screen |
| `MAX_PLAYER_PROJECTILES` | 20 | Max player bullets on screen |
| `MAX_ASH_EFFECTS` | 50 | Max death visual effects |
| `MAX_HISTORY_ENTRIES` | 8 | Max history lines displayed |

### Tile Type IDs

| Constant | Value | Purpose |
|---|---|---|
| `TILE_GROUND` | 0 | Walkable floor |
| `TILE_WALL` | 1 | Impassable wall |
| `TILE_CAR` | 2 | Impassable car obstacle |
| `TILE_MAYOR` | 3 | NPC trigger (Mayor) |
| `TILE_CAVE` | 4 | Level exit/transition |

---

## 5. Data Structures — Complete Reference

### `GameState` (common.h:153-214) — The God Object

This is the **central struct** holding ALL game state. Every system reads/writes from it.

| Field | Type | Purpose |
|---|---|---|
| `map` | `Tilemap` | Current tilemap (tiles array + tileset texture) |
| `player` | `Player` | The player character |
| `state` | `GameMode` | Current game state (enum, 11 values) |
| `zombies[15]` | `Zombie[]` | All enemy slots |
| `zombieTimer` | `float` | Unused/general timer |
| `zombieSpawnTimer` | `float` | Countdown to next enemy wave spawn |
| `ashEffects[50]` | `AshTile[]` | Death visual effects |
| `gameOverSelection` | `int` | Game over menu cursor (0=Retry, 1=Quit) |
| `menuSelection` | `int` | Main menu cursor (0-5) |
| `storyPage` | `int` | Info screen page index (0-4) |
| `playerName[16]` | `char[]` | Player name string |
| `playerNameLength` | `int` | Current name string length |
| `lives` | `int` | Remaining lives (starts at 3) |
| `checkpointPosition` | `Vector2` | Saved position for respawn |
| `checkpointLevel` | `int` | Saved level for respawn |
| `checkpointState` | `GameMode` | Saved state for respawn |
| `checkpointActive` | `bool` | Whether checkpoint exists |
| `gameFont` | `Font` | Loaded Consolas/Lucida font |
| `bgMusic` | `Music` | Background music stream |
| `slashSound` | `Sound` | Sword attack sound |
| `blastSound` | `Sound` | Explosion/powerup sound |
| `hitSound` | `Sound` | Damage taken sound |
| `hitSoundTimer` | `float` | Cooldown between hit sounds (0.5s) |
| `cut1Audio` | `Sound` | Cutscene 1 audio |
| `cut2Audio` | `Sound` | Cutscene 2 audio |
| `cutscenePart` | `int` | 0=cutscene1, 1=cutscene2 |
| `cutsceneTime` | `float` | Elapsed time during cutscene |
| `cutscenesLoaded` | `bool` | Whether cutscene textures are loaded |
| `cut1Textures[152]` | `Texture2D[]` | Cutscene 1 frame textures |
| `cut2Textures[62]` | `Texture2D[]` | Cutscene 2 frame textures |
| `startTextTimer` | `float` | Level start banner fade timer (4s) |
| `mayorRow`, `mayorCol` | `int` | Randomly chosen Mayor spawn position |
| `playerTileset` | `Texture2D` | Player sprite sheet |
| `playerTilesPerRow` | `int` | Tiles per row in player tileset |
| `spritesTileset` | `Texture2D` | Enemy sprite sheet |
| `spritesTilesPerRow` | `int` | Tiles per row in sprite tileset |
| `enemyProjectiles[30]` | `EnemyProjectile[]` | Enemy bullet pool |
| `playerProjectiles[20]` | `PlayerProjectile[]` | Player bullet pool |
| `hasGun` | `bool` | Whether player currently has gun powerup |
| `gunSpawned` | `bool` | Whether gun pickup is on the map |
| `gunRow`, `gunCol` | `int` | Gun pickup grid position |
| `gunAbilityTimer` | `float` | Remaining gun duration (4s) |
| `gunSpawnTimer` | `float` | Countdown to next gun spawn (4s) |
| `bosses[4]` | `BossStatus[]` | Boss progress tracking |
| `fragments[3]` | `MemoryFragment[]` | Memory fragment positions/status |
| `difficulty` | `int` | 0=Easy, 1=Normal, 2=Hard |
| `cutsceneTargetState` | `GameMode` | State to transition to after cutscene |
| `cutsceneTargetLevel` | `int` | Level to load after cutscene |

### `Player` (common.h:122-142)

| Field | Type | Purpose |
|---|---|---|
| `position` | `Vector2` | Pixel position (x, y) |
| `speed` | `float` | Movement speed (120 px/s) |
| `currentObjective` | `int` | Unused objective tracker |
| `isMoving` | `bool` | Whether player is moving (for animation) |
| `animFrame` | `int` | Animation frame index |
| `gridX`, `gridY` | `int` | Current grid cell (derived from position) |
| `direction` | `Direction` | Facing direction (UP/DOWN/LEFT/RIGHT) |
| `hasWeapon` | `bool` | Whether player can attack |
| `isAttacking` | `bool` | Whether attack animation is active |
| `attackTimer` | `float` | Remaining attack animation time (0.15s) |
| `lightAttackCooldown` | `float` | Cooldown between attacks (0.33s) |
| `survivalTimer` | `float` | Time survived in survival mode (for radius powerup) |
| `radiusPowerupReady` | `bool` | Whether radius blast is available |
| `radiusCooldown` | `float` | Cooldown after using radius blast (8s) |
| `radiusBlastTimer` | `float` | Visual blast animation timer (0.5s) |
| `isAimingSuperpower` | `bool` | Whether F key is held (aiming indicator) |
| `health` | `float` | Current HP (0-100) |
| `maxHealth` | `float` | Maximum HP (100) |

### `Zombie` (common.h:90-101)

| Field | Type | Purpose |
|---|---|---|
| `row`, `col` | `int` | Grid position (synced from pixel position) |
| `active` | `bool` | Whether this enemy slot is in use |
| `health` | `float` | Current HP |
| `maxHealth` | `float` | Maximum HP |
| `type` | `EnemyType` | Which enemy type |
| `shootTimer` | `float` | Timer for ranged attacks |
| `position` | `Vector2` | Pixel position |
| `ghostVel` | `Vector2` | Velocity for Ghost-type enemies |

### `Tilemap` (common.h:41-45)

| Field | Type | Purpose |
|---|---|---|
| `tiles[24][35]` | `int[][]` | 2D array of tile IDs |
| `tileset` | `Texture2D` | Sprite sheet texture |
| `tilesPerRow` | `int` | How many tiles fit in one row of the sprite sheet |

### `EnemyProperties` (common.h:81-88)

| Field | Type | Purpose |
|---|---|---|
| `maxHealth` | `float` | Base HP (modified by difficulty/level) |
| `moveSpeed` | `float` | Base speed in px/s |
| `baseTileId` | `int` | Which sprite tile to render |
| `scale` | `float` | Visual/hitbox size multiplier |
| `color` | `Color` | Tint color for rendering |
| `hitboxOffset` | `float` | Extra hitbox padding for large enemies |

### `EnemyProjectile` (common.h:104-110)

| Field | Type | Purpose |
|---|---|---|
| `position` | `Vector2` | Current pixel position |
| `velocity` | `Vector2` | Direction × speed |
| `active` | `bool` | Whether this projectile slot is in use |
| `isBig` | `bool` | Big projectile (25 dmg, 22 radius) vs small (12 dmg, 14 radius) |

### `PlayerProjectile` (common.h:144-150)

| Field | Type | Purpose |
|---|---|---|
| `position` | `Vector2` | Current pixel position |
| `velocity` | `Vector2` | Direction × speed (350 px/s) |
| `active` | `bool` | Whether this slot is in use |

### `BossStatus` (common.h:66-71)

| Field | Type | Purpose |
|---|---|---|
| `spawned` | `bool` | Whether boss has been spawned this fight |
| `defeated` | `bool` | Whether boss has been killed |
| `showLog` | `bool` | Whether to show post-boss story log |
| `tileId` | `int` | Tile ID associated with this boss |

### `MemoryFragment` (common.h:74-78)

| Field | Type | Purpose |
|---|---|---|
| `activated` | `bool` | Whether player has collected it |
| `position` | `Vector2` | Pixel position on map |
| `name` | `const char*` | "CURIOSITY", "KNOWLEDGE", or "TRUTH" |

### `AshTile` (common.h:113-119)

| Field | Type | Purpose |
|---|---|---|
| `gridX`, `gridY` | `int` | Grid position of death effect |
| `timer` | `float` | Remaining display time (1.5s) |
| `active` | `bool` | Whether this slot is in use |

### `GameHistory` (common.h:220-224)

| Field | Type | Purpose |
|---|---|---|
| `entries[8][64]` | `char[][]` | Last 8 history lines |
| `count` | `int` | Number of entries loaded |

---

## 6. Enums

### `GameMode` — 11 States

| Value | Name | Description |
|---|---|---|
| 0 | `STATE_MENU` | Main menu screen |
| 1 | `STATE_NAME_PROMPT` | Player name entry |
| 2 | `STATE_INTRO` | Cutscene playback |
| 3 | `STATE_INFO` | Story/info pages |
| 4 | `STATE_HISTORY` | Game run history |
| 5 | `STATE_TEAM` | Credits/team screen |
| 6 | `STATE_EXPLORING` | Free-roam exploration |
| 7 | `STATE_CUTSCENE` | In-game dialogue (Mayor) |
| 8 | `STATE_SURVIVAL` | Combat/wave survival |
| 9 | `STATE_GAMEOVER` | Death screen |
| 10 | `STATE_WIN` | Victory screen |

### `EnemyType` — 7 Types

| Value | Name | Role |
|---|---|---|
| 0 | `ENEMY_SNAKE` | Basic melee enemy |
| 1 | `ENEMY_SPIDER` | Ranged enemy (shoots projectiles) |
| 2 | `ENEMY_GHOST` | Velocity-based movement, bounces |
| 3 | `ENEMY_RAT_KING` | Boss: big + ranged |
| 4 | `ENEMY_DOOM_SCROLLER` | Boss: stationary, 4-directional shots |
| 5 | `ENEMY_ALGORITHM` | Boss: chase + spawns final boss |
| 6 | `ENEMY_BRAINROT_GOD` | Final boss: 3-phase attack cycle |

### `BossId` — 4 Bosses

| Value | Name | Tile ID |
|---|---|---|
| 0 | `BOSS_RAT_KING` | 23 |
| 1 | `BOSS_DOOM_SCROLLER` | 306 |
| 2 | `BOSS_ALGORITHM` | 308 |
| 3 | `BOSS_BRAINROT_GOD` | 27 |

### `Direction` — 4 Directions

| Value | Name |
|---|---|
| 0 | `DIR_UP` |
| 1 | `DIR_DOWN` |
| 2 | `DIR_LEFT` |
| 3 | `DIR_RIGHT` |

---

## 7. Function Reference

### 7.1 `main.c`

#### `int main(void)` (line 4)
- **Purpose**: Entry point. Creates window, runs game loop, cleans up.
- **What it does**:
  1. `InitWindow(35*32, 24*32, "The GameRot")` — creates 1120×768 window
  2. `InitAudioDevice()` — enables sound
  3. `SetTargetFPS(60)` — locks to 60 FPS
  4. `GameState_Init(&game)` — initializes all game state
  5. Loop: `GetFrameTime()` → `UpdateGame()` → `DrawGame()` until window close
  6. `GameState_Unload()` → `CloseAudioDevice()` → `CloseWindow()`
- **Side effects**: Creates window, starts audio, runs until user closes

#### `int stat64i32(const char *path, struct _stat *buffer)` (line 28)
- **Purpose**: UCRT compatibility wrapper for Raylib's internal file stat calls on Windows.
- **What it does**: Delegates to `_stat()`.

---

### 7.2 `game.c` — The Master File (~1001 lines)

#### `void GameState_LoadCutscenes(GameState* game)` (line 12)
- **Purpose**: Loads all cutscene frame textures into memory.
- **What it does**: Shows "LOADING CUTSCENE..." text, then loads 152 PNGs for cutscene 1 and 62 PNGs for cutscene 2 from `cutscenes/frames/`.
- **Optimization**: Only loads once (`cutscenesLoaded` flag).

#### `void GameState_UnloadCutscenes(GameState* game)` (line 32)
- **Purpose**: Frees cutscene textures from GPU memory.
- **What it does**: Iterates all 214 textures, calls `UnloadTexture()` if valid.

#### `void GameState_Init(GameState* self)` (line 49)
- **Purpose**: Full game initialization. Sets up everything from scratch.
- **Step-by-step**:
  1. `currentLevel = 0` — starts at level 0
  2. Loads `map1.csv` + `tilemap_packed.png`
  3. `Player_Init()` — resets player
  4. Scans map for tiles 283/306/308/355 and replaces them with tile 20 (cleanup)
  5. Finds random valid position for Mayor (Manhattan distance ≥ 10 from top-left, not on walls/cars)
  6. Places Mayor tile (283) at chosen position
  7. Sets `state = STATE_MENU`
  8. Zeros all timers, arrays, projectiles, ash effects
  9. Sets boss tile IDs: Rat King=23, Doom Scroller=306, Algorithm=308, Brainrot God=27
  10. Loads audio: bgMusic, slash, blast, hit, cutscene audio
  11. Loads textures: playerTileset, spritesTileset (with color key removal for transparency)
  12. Loads font: tries Consolas → Lucida → default
  13. Starts background music

#### `void GameState_TransitionFromCutscene(GameState* game)` (line 171)
- **Purpose**: Handles what happens after a cutscene ends.
- **Logic**:
  - If `cutsceneTargetState != -1`: loads the target level (only handles level 1 transitions currently)
  - Otherwise: sets `STATE_EXPLORING`, resets lives to 3, sets checkpoint, shows start banner

#### `void UpdateGame(GameState* game, float dt)` (line 211)
- **Purpose**: THE master update function. ~525 lines. Handles ALL game logic.
- **Before the switch**:
  1. Checks if any boss log is showing → blocks all input if so
  2. Updates music stream
  3. Decrements `hitSoundTimer` and `startTextTimer`
  4. Handles **[M] key debug level switch**: cycles through levels 0→1→2→3→0, reloading maps and resetting state each time

- **STATE_MENU** (line 432):
  - UP/DOWN navigates menu (0-5)
  - LEFT/RIGHT changes difficulty on option 1
  - ENTER selects: Play → Name Prompt, Info, History, Team, Quit

- **STATE_NAME_PROMPT** (line 457):
  - Character input: ASCII 32-125, max 15 chars
  - BACKSPACE deletes last char
  - ENTER: stops music, loads cutscenes, starts intro
  - ESCAPE: back to menu

- **STATE_INTRO** (line 484):
  - Advances `cutsceneTime` by dt
  - Skippable with ENTER/SPACE/ESCAPE
  - Cutscene 1 lasts 10s, then auto-transitions to cutscene 2 (4s)
  - After cutscene 2: unloads cutscenes, calls `GameState_TransitionFromCutscene()`

- **STATE_INFO** (line 507): LEFT/RIGHT pages through 5 story pages, ESCAPE returns
- **STATE_HISTORY** / **STATE_TEAM** (line 520): Any key returns to menu

- **STATE_EXPLORING** (line 528):
  - `Player_Update()` — movement
  - Checks if player stepped on Mayor tile → transitions to `STATE_CUTSCENE`

- **STATE_CUTSCENE** (line 539):
  - ENTER: starts survival mode, spawns 3 zombies, sets checkpoint

- **STATE_SURVIVAL** (line 569): **THE BIG ONE** (~165 lines)
  - `Player_Update()`
  - Level 3 specific: checks player position for puzzle switches (tiles 59 → 17)
  - Wave spawn: increments `zombieSpawnTimer`, spawns enemies based on level cooldown:
    - Level 0: every 2.5s
    - Level 1: every 1.8s
    - Level 2: every 1.2s
  - Spawn count: `1 + currentLevel` per wave
  - Stops spawning when level boss is defeated (levels 1-2) or on level 3
  - `UpdateZombies()` — all enemy AI
  - Checks if player stepped on Cave tile → level transition
  - Radius powerup: charges after 7s survival, activated with F key, 8s cooldown
  - Brainrot God fragment mechanic: collecting fragments deals 200 damage to the boss
  - Collision damage: if enemy occupies same cell as player → 30 dmg/sec
  - Death: if health ≤ 0, either checkpoint respawn (if lives > 1) or game over
  - On game over: saves history entry

- **STATE_GAMEOVER** (line 737): UP/DOWN selects Retry (restarts) or Quit (menu)
- **STATE_WIN** (line 749): ENTER returns to menu

#### `void DrawGame(const GameState* game)` (line 760)
- **Purpose**: THE master draw function. ~225 lines.
- **Cutscene drawing**: renders frame texture scaled to screen
- **Game world drawing** (Exploring/Survival/Cutscene):
  1. Sets up Camera2D at 2.5x zoom, centered on player
  2. Draws all tiles
  3. Draws ash effects (fading)
  4. Draws all active enemies with health bars
  5. Draws memory fragments (pulsing circles)
  6. Draws enemy projectiles (multi-layered colored circles)
  7. Draws radius blast aim indicator (red circle) and blast animation
  8. `Player_Draw()`
  9. Draws attack flash effect (colored rectangles)
  10. Draws gun pickup (pulsing blue circle)
  11. Draws player projectiles (blue circles)
- **Overlay drawing**: Menu screens, HUD, level start banner, story overlays

#### `void GameState_Unload(GameState* self)` (line 986)
- **Purpose**: Frees ALL resources.
- **What it unloads**: cutscene textures, music, sounds (6), player tileset, sprite tileset (if different), font (if custom)

---

### 7.3 `boss.c` — Combat & Boss Logic

#### `BossId GetBossId(EnemyType type)` (line 4)
- **Purpose**: Maps enemy type to boss ID.
- **Returns**: `BOSS_RAT_KING` for Rat King, etc. Returns `-1` (cast to BossId) for non-bosses.

#### `void OnZombieDeath(GameState* game, int idx)` (line 14)
- **Purpose**: Handles ALL consequences of killing an enemy.
- **Step-by-step**:
  1. Deactivates enemy slot
  2. Creates ash death effect at enemy position
  3. Checks if enemy was a boss via `GetBossId()`
  4. If boss: marks `defeated = true`
     - Rat King / Doom Scroller: shows story log (`showLog = true`)
     - Rat King: replaces tiles at death position with cave exit (236, 237)
     - Doom Scroller: spawns Algorithm boss
     - Algorithm: shows log, spawns Brainrot God, spawns memory fragments
     - Brainrot God: saves history, loads level 3 (finalmap)
  5. Clears ALL active enemies on boss death (except Brainrot God → transitions to level 3)

#### `bool IsZombieHit(const GameState* game, int i, Vector2 hitPos, float size)` (line 79)
- **Purpose**: Checks if a hit area overlaps an enemy.
- **Math**: Creates two rectangles — enemy AABB (scaled by `props.scale`) and hit area — uses `CheckCollisionRecs()`.
- **Enemy AABB**: Position offset by `(scale-1)/2 * TILE_PX` to center the scaled sprite.

#### `void DamageZombie(GameState* game, int idx, float damage)` (line 90)
- **Purpose**: Applies damage to an enemy.
- **Special rule**: Brainrot God is **immune to all damage except 200.0** (the fragment damage). If `damage != 200.0f`, the function returns early.
- **On death**: Calls `OnZombieDeath()`.

#### `void SpawnGun(GameState* game)` (line 102)
- **Purpose**: Places a gun powerup pickup on the map.
- **Logic**: Tries up to 100 random ground tiles. Must be Manhattan distance ≥ 3 from player.

#### `EnemyProperties GetEnemyProperties(EnemyType type, int level, int difficulty)` (line 120)
- **Purpose**: Returns base stats for any enemy type, scaled by difficulty and level.
- **Base stats table**:

| Type | HP | Speed | Tile ID | Scale | Color | Hitbox Offset |
|---|---|---|---|---|---|---|
| Snake | 30 | 55 | 331 (or 20 on level 1) | 1.0 | WHITE | 0 |
| Spider | 30 | 55 | 23 | 1.0 | WHITE | 0 |
| Ghost | 40 | 65 | 25 | 1.0 | WHITE | 0 |
| Rat King | 300 | 40 | 23 | 2.5 | PURPLE | 20 |
| Doom Scroller | 400 | 0 | 306 | 3.0 | RED | 28 |
| Algorithm | 200 | 45 | 308 | 2.5 | VIOLET | 20 |
| Brainrot God | 600 | 50 | 27 | 4.0 | WHITE | 40 |

- **Scaling formulas**:
  - Regular enemies: `speed × diffSpeedMult × levelSpeedMult`, `hp × diffHpMult × levelHpMult`
  - Bosses: separate multipliers, no level scaling

---

### 7.4 `player.c` — Player Logic

#### `void Player_Init(Player* self)` (line 10)
- **Purpose**: Initializes player for current level.
- **Starting position**:
  - Level 0: (25, 23) in grid → (800, 736) in pixels
  - Other levels: (2, 2) in grid → (64, 64) in pixels
- **Weapon**: `hasWeapon = (currentLevel > 0)` — no weapon on level 0
- **Health**: 100 HP, speed: 120 px/s

#### `static bool Player_CanMoveTo(const Tilemap* map, float x, float y)` (line 40)
- **Purpose**: Checks if player can move to position (x, y).
- **Math**: Tests 4 corners of a 30×30 bounding box with 2px margin against `Tilemap_IsWalkable()`. This prevents corner-cutting through walls.

#### `void Player_Update(Player* self, const Tilemap* map, float dt)` (line 53)
- **Purpose**: Processes input and updates player state each frame.
- **Movement**: WASD or Arrow keys, speed × dt per frame
- **Axis-separated collision**: X and Y checked independently, allowing wall-sliding
- **Grid sync**: `gridX = (int)(position.x / TILE_PX)`, `gridY = (int)(position.y / TILE_PX)`
- **Timer decrements**: attackTimer, lightAttackCooldown, radiusCooldown, radiusBlastTimer

#### `void Player_Draw(const Player* self, Texture2D tileset, int tilesPerRow)` (line 82)
- **Purpose**: Renders the player character.
- **Animation**: Alternates between tiles 332/333 at 8 FPS when moving, tile 331 when idle.
- **Sword rendering**: When idle + has weapon + not attacking:
  - Draws a 4×16 (or 16×4) rectangle in the facing direction
  - Three-layer gradient: RED → ORANGE → GOLD (fiery effect)

---

### 7.5 `zombie.c` — Enemy AI & Spawning

#### `void SpawnEnemy(GameState* game, EnemyType type, int r, int c)` (line 10)
- **Purpose**: Spawns a specific enemy type at position (r, c) or random.
- **Random spawn logic** (when r=-1, c=-1):
  - Tries up to 100 random ground tiles
  - Manhattan distance check: regular enemies ≥ 6 tiles from player, bosses ≥ 3 tiles
  - Falls back to (10, 10) if no valid spot found
- **Sets up**: position, health (from `GetEnemyProperties`), type, shoot timer, boss tracking

#### `void SpawnZombie(GameState* game)` (line 80)
- **Purpose**: Spawns a random enemy type.
- **Type selection**:
  - Level 0: always Snake
  - Levels 1-2: 33% Snake, 33% Spider, 33% Ghost (random roll)

#### `static void ShootSpiderProjectile(GameState* game, Vector2 startPos, Vector2 targetPos, bool isBig)` (line 89)
- **Purpose**: Creates a projectile from startPos toward targetPos.
- **Math**: Normalizes direction vector (`dx/len, dy/len`), multiplies by speed (150 for big, 180 for small).

#### `static bool Zombie_CanMoveTo(const Tilemap* map, float tx, float ty)` (line 111)
- **Purpose**: Same 4-corner collision check as player, but with 4px offset and `TILE_PX - 8` box size.

#### `void UpdateZombies(GameState* game, float dt)` (line 131)
- **Purpose**: THE enemy AI update. ~190 lines. Handles ALL enemy behavior.
- **Phase 1 — Spider/Rat King firing** (line 133):
  - Spiders fire every 1.8s, Rat Kings every 1.5s
  - Creates projectile aimed at player center
- **Phase 2 — Projectile updates** (line 155):
  - Moves all active projectiles
  - Deactivates out-of-bounds ones
  - Checks collision with player: circle collision with radius 14 (small) or 22 (big)
  - Deals 12 (small) or 25 (big) damage
- **Phase 3 — Movement** (line 184):
  - **Ghost**: Moves in straight line, bounces off map boundaries, syncs grid position from center
  - **Brainrot God attack cycle** (5-second loop):
    - 1.0s-1.4s: "Skibidi Blast" — 3 rapid big fireballs at player
    - 2.5s: "Ohio Storm" — 3 lightning bolts from above
    - 4.0s: "Sigma Shockwave" — proximity damage if player within 120px
    - Resets at 5.0s
  - **Doom Scroller**: Fires 4 directional projectiles every 1.5s (up/down/left/right)
  - **All other enemies**: Chase player using normalized direction vector × moveSpeed × dt
  - X and Y axes checked independently for wall collision

#### `void SpawnMemoryFragments(GameState* game)` (line 323)
- **Purpose**: Places 3 memory fragments on the map for Brainrot God fight.
- **Logic**: Tries up to 200 random ground tiles, ensuring each fragment is Manhattan distance ≥ 6 from others.
- **Fallback positions**: (5,5), (15,15), (25,8) if placement fails.
- **Names**: "CURIOSITY", "KNOWLEDGE", "TRUTH"

---

### 7.6 `tilemap.c` — Map System

#### `int GetTileType(int tileID)` (line 10)
- **Purpose**: Classifies a tile ID into one of 5 types. THE core lookup function.
- **Classification order**:
  1. Tile 283 → `TILE_MAYOR`
  2. Tiles 236, 237, 306, 308, 158 → `TILE_CAVE`
  3. Check against hardcoded car tiles list (19 tiles) → `TILE_CAR`
  4. Check against level-specific walkable lists → `TILE_GROUND`
  5. Default → `TILE_WALL`
- **Level-specific walkable lists**:
  - Level 0: 42 walkable tile IDs
  - Level 1: 32 walkable tile IDs
  - Level 2: 28 walkable tile IDs
  - Level 3: 5 walkable tile IDs
- **Why this matters**: Changing these lists directly affects where enemies can spawn, where the player can walk, and where projectiles are blocked.

#### `void Tilemap_Load(Tilemap* self, const char* csvPath, const char* texturePath)` (line 64)
- **Purpose**: Loads a tilemap from CSV and its tileset texture.
- **CSV format**: Comma-separated integers, 35 per row, 24 rows.
- **Level 2 injection**: When loading level 1's map, dynamically places a cave exit at (15,29) and 6 wall obstacles for variety.

#### `void Tilemap_Draw(const Tilemap* self)` (line 92)
- **Purpose**: Draws every tile. (Not actually called — `DrawGame()` does this inline.)

#### `bool Tilemap_IsWalkable(const Tilemap* self, float targetX, float targetY)` (line 101)
- **Purpose**: Checks if a point can be walked on.
- **Math**: Converts pixel to grid, looks up tile type, rejects WALL and CAR.

---

### 7.7 `ui.c` — All UI Screens

#### `void DrawTile(Texture2D tileset, int tilesPerRow, int tileID, float x, float y)` (line 6)
- **Purpose**: Draws a single tile from a sprite sheet.
- **Math**:
  - Source X: `(tileID % tilesPerRow) * TILE_SIZE`
  - Source Y: `(tileID / tilesPerRow) * TILE_SIZE`
  - Source rect: 8×8 from sprite sheet
  - Dest rect: 32×32 on screen

#### `void DrawCustomText(const GameState* game, const char* text, float posX, float posY, float fontSize, Color color)` (line 14)
- **Purpose**: Renders text using the game's custom font.

#### `float MeasureCustomText(const GameState* game, const char* text, float fontSize)` (line 19)
- **Purpose**: Measures text width for centering.

#### `void GameHistory_SaveEntry(const char* name, int levelCleared, bool victory)` (line 24)
- **Purpose**: Appends a line to `history.txt`.
- **Format**: `"name - Level X Cleared (Victory)"` or `"name - Level X Cleared"`

#### `void GameHistory_Load(GameHistory* history)` (line 36)
- **Purpose**: Reads last 8 lines from `history.txt`.
- **Logic**: Reads all lines into temp array, then copies last 8 in reverse order (newest first).

#### `void DrawMenuScreen(const GameState* game)` (line 59)
- **Purpose**: Main menu with title, 6 options, and controls panel.
- **Options**: Play Game, Difficulty, Story & Info, Game History, Development Team, Quit
- **Selection highlight**: Semi-transparent rectangle + red border around selected option.

#### `void DrawNamePromptScreen(const GameState* game)` (line 119)
- **Purpose**: Name entry screen with blinking cursor.
- **Cursor**: `_` character toggles every 0.5s using `GetTime() * 2.0f % 2`.

#### `void DrawInfoScreen(const GameState* game)` (line 162)
- **Purpose**: 5-page story viewer.
- **Pages**: Prologue, Chapters 2-3, Chapters 4-5, Final Chapter, Ending.

#### `void DrawHistoryScreen(const GameState* game)` (line 257)
- **Purpose**: Shows game run history from `history.txt`.
- **Color coding**: Green for victories, light gray for normal clears.

#### `void DrawTeamScreen(const GameState* game)` (line 286)
- **Purpose**: Credits screen listing 3 team members.

#### `void DrawGameOverScreen(const GameState* game)` (line 308)
- **Purpose**: "DEFEAT" screen with Retry/Quit options.

#### `void DrawWinScreen(const GameState* game)` (line 324)
- **Purpose**: Victory screen with full story conclusion.

#### `void DrawHUD(const GameState* game)` (line 354)
- **Purpose**: In-game HUD overlay.
- **Elements**: Level name, player name, lives, HP bar (color-coded: green/orange/red), gun timer bar, objective text, enemy count.

#### `void DrawStoryOverlays(const GameState* game)` (line 416)
- **Purpose**: Post-boss story log popups.
- **Three overlays**: Rat King log, Doom Scroller log, Algorithm log.

---

### 7.8 `tileset_viewer.c` — Standalone Tool

Not part of the game. A utility for inspecting tile IDs in sprite sheets.
- Keys 1-6 switch between tileset files
- Hover shows tile ID, click prints to console
- Renders with grid overlay

---

## 8. All Math Explained

### Coordinate Systems

**Pixel coordinates** (`Vector2 position`): Where entities are on screen in pixels.
```
position.x = gridCol × TILE_PX    // gridCol × 32
position.y = gridRow × TILE_PX    // gridRow × 32
```

**Grid coordinates** (`gridX`, `gridY`): Which tile cell the entity is in.
```
gridX = (int)(position.x / TILE_PX)    // position.x / 32
gridY = (int)(position.y / TILE_PX)    // position.y / 32
```

### Tile Rendering Math

From sprite sheet (8px tiles) to screen (32px tiles):
```
sourceX = (tileID % tilesPerRow) × 8
sourceY = (tileID / tilesPerRow) × 8
sourceRect = { sourceX, sourceY, 8, 8 }
destRect   = { screenX, screenY, 32, 32 }
```

### Movement Math

Per-frame displacement:
```
nextX = position.x + (direction × speed × dt)
nextY = position.y + (direction × speed × dt)
```
Where `dt` = time since last frame (~0.0167s at 60 FPS).

### Direction Normalization (Unit Vector)

For enemies chasing the player:
```
dx = player.x - enemy.x
dy = player.y - enemy.y
len = sqrt(dx² + dy²)
unitX = dx / len
unitY = dy / len
stepX = unitX × moveSpeed × dt
stepY = unitY × moveSpeed × dt
```

### Manhattan Distance

Used for spawn distance checks (faster than Euclidean, no sqrt):
```
distance = |col1 - col2| + |row1 - row2|
```
- Regular enemies must be ≥ 6 tiles from player
- Bosses must be ≥ 3 tiles from player
- Gun must be ≥ 3 tiles from player
- Memory fragments must be ≥ 6 tiles from each other

### Euclidean Distance

Used for collision detection and proximity checks:
```
distance = sqrt((x1-x2)² + (y1-y2)²)
```
- Projectile-player collision
- Radius blast range (160px)
- Brainrot God shockwave (120px)
- Gun pickup range (32px)
- Memory fragment collection (24px)

### Collision Detection

**AABB (Axis-Aligned Bounding Box)**:
```
Two rectangles overlap if:
  rect1.left < rect2.right &&
  rect1.right > rect2.left &&
  rect1.top < rect2.bottom &&
  rect1.bottom > rect2.top
```
Used by: `CheckCollisionRecs()` for melee attacks, `IsZombieHit()`.

**Circle-Rectangle**:
```
Circle overlaps rectangle if:
  closest point on rect to circle center is within circle radius
```
Used by: `CheckCollisionCircleRec()` for melee attack → enemy collision.

**Circle-Circle** (manual implementation):
```
Two circles overlap if:
  sqrt((x1-x2)² + (y1-y2)²) < radius1 + radius2
```
Used by: projectile-player collision, proximity checks.

### 4-Corner Collision (Player/Enemy Movement)

Both player and enemies check 4 corners of their bounding box before moving:
```
corners = {
  (x + margin, y + margin),           // top-left
  (x + boxSize, y + margin),          // top-right
  (x + margin, y + boxSize),          // bottom-left
  (x + boxSize, y + boxSize)          // bottom-right
}
```
All 4 corners must be on walkable tiles. This prevents clipping through walls at corners.

Player: margin=2, boxSize=30
Enemy: offset=4, boxSize=TILE_PX-8=24

### Attack Rectangle Math

When player attacks in a direction:
```
dx, dy = direction offset (e.g., RIGHT → dx=1, dy=0)
rx = (playerCol + dx) × TILE_PX
ry = (playerRow + dy) × TILE_PX
rw = TILE_PX (or TILE_PX × 2 if horizontal)
rh = TILE_PX (or TILE_PX × 2 if vertical)
```
The attack area is 1 tile in front, stretched to 2 tiles in the movement direction.

### Health Bar Rendering

```
pct = health / maxHealth
barWidth = 278 (fixed HUD width)
filledWidth = 278 × pct
color: GREEN (≥60%), ORANGE (30-60%), RED (<30%)
```

### Difficulty Scaling

**Regular enemies**:
```
speed = baseSpeed × diffSpeedMult × levelSpeedMult
  diffSpeedMult: Easy=0.8, Normal=1.0, Hard=1.25
  levelSpeedMult: 1.0 + (level × 0.2)

hp = baseHP × diffHpMult × levelHpMult
  diffHpMult: Easy=0.75, Normal=1.0, Hard=1.35
  levelHpMult: 1.0 + (level × 0.2)
```

**Bosses**:
```
speed = baseSpeed × bossSpeedMult
  bossSpeedMult: Easy=0.8, Normal=1.1, Hard=1.3

hp = baseHP × bossHpMult
  bossHpMult: Easy=0.8, Normal=1.1, Hard=1.5
```

### Animation Frame Calculation

```
frameIndex = (int)(GetTime() * 8.0f) % 2
// GetTime() returns seconds since game started
// Multiplying by 8 gives 8 "ticks" per second
// Modulo 2 alternates between 0 and 1
// Used to swap between tiles 332 and 333 for walk animation
```

### Camera Math

```
camera.target  = player.position + (TILE_PX/2, TILE_PX/2)  // center on player
camera.offset  = (screenWidth/2, screenHeight/2)             // center of screen
camera.zoom    = 2.5
camera.rotation = 0
```

---

## 9. Game State Machine — Full Flow

```
                    ┌─────────────┐
                    │  STATE_MENU │
                    └──────┬──────┘
                           │ Play Game
                           ▼
                    ┌──────────────────┐
                    │ STATE_NAME_PROMPT│
                    └──────┬───────────┘
                           │ Enter name
                           ▼
                    ┌──────────────┐
                    │ STATE_INTRO  │ (cutscene 1: 10s, cutscene 2: 4s)
                    └──────┬───────┘
                           │ Cutscene ends
                           ▼
                    ┌───────────────────┐
                    │ STATE_EXPLORING   │ (Level 0: find Mayor)
                    └──────┬────────────┘
                           │ Step on Mayor tile
                           ▼
                    ┌──────────────────┐
                    │ STATE_CUTSCENE   │ (Mayor dialogue)
                    └──────┬───────────┘
                           │ Press ENTER
                           ▼
                    ┌──────────────────┐
                    │ STATE_SURVIVAL   │ ←──── checkpoint respawn
                    └──┬────┬────┬────┘
                       │    │    │
          Cave exit    │    │    │  Death (no lives)
                       ▼    │    ▼
              Level load    │  ┌────────────────┐
              (next map)    │  │ STATE_GAMEOVER │
                            │  └───┬────────┬───┘
                            │      │        │
                            │   Retry     Quit
                            │      │        │
                            │      ▼        ▼
                            │  Restart   STATE_MENU
                            │
                            ▼
                    ┌──────────────┐
                    │ STATE_WIN    │ (reached final cave)
                    └──────┬───────┘
                           │ ENTER
                           ▼
                    ┌──────────────┐
                    │ STATE_MENU  │
                    └─────────────┘
```

**Other menu paths**:
- Menu → STATE_INFO (5-page story viewer)
- Menu → STATE_HISTORY (game run log)
- Menu → STATE_TEAM (credits)
- Any of these → ESCAPE → STATE_MENU

---

## 10. Enemy Behavior — Every Type

### Snake (ENEMY_SNAKE)
- **Role**: Basic melee enemy
- **HP**: 30 (scaled)
- **Speed**: 55 px/s (scaled)
- **AI**: Chases player in straight line, X/Y independent movement
- **Special**: On level 0, uses tilemap sprites (looks different); on level 1+, uses sprite tileset
- **Damage**: Contact deals 30 dmg/sec

### Spider (ENEMY_SPIDER)
- **Role**: Ranged attacker
- **HP**: 30 (scaled)
- **Speed**: 55 px/s (scaled)
- **AI**: Chases player + fires projectiles every 1.8s
- **Projectile**: 12 damage, 180 px/s, collision radius 14px
- **Visual**: Small colored circle projectile

### Ghost (ENEMY_GHOST)
- **Role**: Fast, unpredictable enemy
- **HP**: 40 (scaled)
- **Speed**: 65 px/s (scaled)
- **AI**: Moves in a straight line in one direction, bounces off map edges. Does NOT chase player.
- **Spawn**: Random direction (horizontal or vertical, positive or negative)
- **Grid sync**: Uses center-based calculation (position + TILE_PX/2)

### Rat King (BOSS_RAT_KING)
- **Role**: Level 1 boss
- **HP**: 300 (scaled)
- **Speed**: 40 px/s (scaled)
- **Scale**: 2.5× (appears 2.5 tiles wide)
- **Color**: Purple
- **AI**: Chases player + fires big projectiles every 1.5s
- **Big projectile**: 25 damage, 150 px/s, collision radius 22px
- **On death**: Shows story log, places cave exit tiles, clears all enemies

### Doom Scroller (BOSS_DOOM_SCROLLER)
- **Role**: Level 2 boss (phase 1)
- **HP**: 400 (scaled)
- **Speed**: 0 (does NOT move)
- **Scale**: 3.0×
- **Color**: Red
- **AI**: Stationary. Fires 4 directional projectiles (up/down/left/right) every 1.5s
- **Projectile**: 12 damage, 160 px/s each
- **On death**: Shows story log, spawns Algorithm boss

### Algorithm (BOSS_ALGORITHM)
- **Role**: Level 2 boss (phase 2)
- **HP**: 200 (scaled)
- **Speed**: 45 px/s (scaled)
- **Scale**: 2.5×
- **Color**: Violet
- **AI**: Chases player
- **On death**: Shows story log, spawns Brainrot God, spawns 3 memory fragments

### Brainrot God (BOSS_BRAINROT_GOD)
- **Role**: Final boss
- **HP**: 600 (scaled)
- **Speed**: 50 px/s (scaled)
- **Scale**: 4.0× (4 tiles wide!)
- **Color**: White with pulsing effect (`sin(time*4) * 4px`)
- **AI**: 5-second attack cycle:
  1. **1.0s-1.4s — "Skibidi Blast"**: 3 rapid big fireballs at player
  2. **2.5s — "Ohio Storm"**: 3 lightning bolts from above (offset ±32px)
  3. **4.0s — "Sigma Shockwave"**: Deals 30 damage if player within 120px
  4. **5.0s**: Cycle resets
- **IMMUNE to all damage except memory fragments** (200 damage each)
- **3 fragments**: "CURIOSITY", "KNOWLEDGE", "TRUTH" — collecting all 3 kills it
- **On death**: Saves history, loads level 3 (final map)

---

## 11. Level Progression

### Level 0 — "Prologue: The Mayor's Request"
- **Map**: `map1.csv` + `tilemap_packed.png`
- **Tileset walkable tiles**: 42 specific tile IDs
- **Player**: Starts at (25, 23), NO weapon
- **Enemies**: Only Snakes, spawned every 2.5s, 1 per wave
- **Boss**: None
- **Objective**: Find the Mayor NPC
- **Transition**: Step on Mayor → cutscene → survival mode → find cave → level 1

### Level 1 — "Chapter 2: Sigma Research Facility"
- **Map**: `map2.csv` + `tilemap_packed2.png`
- **Dynamic injection**: Cave exit at (15,29), 6 wall obstacles
- **Tileset walkable tiles**: 32 specific tile IDs
- **Player**: Starts at (2, 2), HAS weapon
- **Enemies**: Snake/Spider/Ghost (random), spawned every 1.8s, 2 per wave
- **Boss**: Rat King (spawns at grid 10,10)
- **Objective**: Defeat Rat King
- **Transition**: Kill Rat King → story log → find cave → level 2

### Level 2 — "Chapter 3: The Forbidden Archive"
- **Map**: `map3.csv` + `tilemap_packed3.png`
- **Tileset walkable tiles**: 28 specific tile IDs
- **Player**: Starts at (2, 2), HAS weapon
- **Enemies**: Snake/Spider/Ghost (random), spawned every 1.2s, 3 per wave
- **Boss chain**: Doom Scroller → Algorithm → Brainrot God (sequential)
- **Special mechanic**: 3 memory fragments needed to kill Brainrot God
- **Objective**: Defeat all 3 bosses
- **Transition**: Kill Brainrot God → level 3

### Level 3 — "The Archive Core"
- **Map**: `finalmap.csv` + `finalmap_packed.png`
- **Tileset walkable tiles**: 5 specific tile IDs
- **Player**: Starts at (2, 2), HAS weapon
- **Enemies**: No spawning
- **Boss**: None
- **Objective**: Reach the cave exit to win
- **Special**: Puzzle switches — standing on specific tiles opens blocked passages
- **Transition**: Step on cave → STATE_WIN → victory screen

---

## 12. Dependency Map — "Changing X Affects Y"

### Core Constants

| Change | Affects |
|---|---|
| `TILE_PX` | Window size, all movement speeds, all collision math, all rendering positions, camera zoom |
| `TILE_SIZE` | Sprite sheet coordinate math, source rect sizes |
| `RENDER_ZOOM` | Visual scale, effective collision sizes |
| `MAP_WIDTH` / `MAP_HEIGHT` | Window size, map array bounds, spawn ranges, boundary checks |
| `MAX_ZOMBIES` | Array sizes, spawn slot availability, iteration counts |
| `MAX_ENEMY_PROJECTILES` | Projectile pool size |
| `MAX_PLAYER_PROJECTILES` | Projectile pool size |

### Game Logic

| Change | Affects |
|---|---|
| `currentLevel` | Tile classification, enemy stats, spawn types, starting position, tilemap loading, HUD display |
| `difficulty` | Enemy HP/speed multipliers (all enemies) |
| Player `speed` | Movement responsiveness |
| Player `health` / `maxHealth` | survivability, HP bar display |
| `lightAttackCooldown` (0.33s) | Attack rate |
| `attackTimer` (0.15s) | Attack animation duration |
| `radiusCooldown` (8s) | Superpower usage frequency |
| `survivalTimer` threshold (7s) | When radius powerup becomes available |

### Adding New Features

**Adding a new enemy type**:
1. Add to `EnemyType` enum in `common.h`
2. Add stats to `GetEnemyProperties()` in `boss.c`
3. Add AI behavior to `UpdateZombies()` in `zombie.c`
4. Add to `SpawnZombie()` type selection logic
5. Add drawing logic to `DrawGame()` if custom rendering needed

**Adding a new tile type**:
1. Add constant to `common.h` (e.g., `TILE_WATER 5`)
2. Add classification to `GetTileType()` in `tilemap.c`
3. Handle in walkability checks (`Tilemap_IsWalkable`, `Zombie_CanMoveTo`, `Player_CanMoveTo`)

**Adding a new game state**:
1. Add to `GameMode` enum
2. Add case to `UpdateGame()` switch
3. Add case to `DrawGame()` switch
4. Add transition logic from existing states

**Adding a new boss**:
1. Add to `EnemyType` and `BossId` enums
2. Add to `GetBossId()` mapping
3. Add stats to `GetEnemyProperties()`
4. Add AI to `UpdateZombies()`
5. Add death logic to `OnZombieDeath()`
6. Add `BossStatus` entry in `GameState_Init()`

---

## 13. Viva Q&A Preparation

### Architecture Questions

**Q: Why C instead of C++ or another language?**
A: C was chosen for simplicity and direct hardware access. Raylib has excellent C API support. No need for OOP overhead in a small game. The game uses a struct-based "OOP in C" pattern (structs with function pointers that take `self` as first parameter).

**Q: Why Raylib instead of SDL or Unity?**
A: Raylib is lightweight, has a simple API, handles graphics/audio/input in one library. Perfect for a small team project. No editor overhead — pure code.

**Q: Why tile-based maps?**
A: Tile-based maps are memory-efficient (35×24 int array), easy to edit (CSV files), and simplify collision detection (check tile type instead of complex geometry).

**Q: Why CSV for map data?**
A: Simple to parse with `fscanf`, human-readable, easy to edit in spreadsheet editors, no external dependencies.

**Q: Explain the game's architecture.**
A: The game follows a **single-threaded game loop** pattern:
1. `main()` creates window, runs init, enters loop
2. Each frame: `UpdateGame(dt)` processes logic, `DrawGame()` renders
3. A **finite state machine** (11 states) controls what updates and draws
4. All state lives in one `GameState` struct (god object pattern)
5. Systems are separated into files by domain (player, zombie, map, ui, boss)

### Design Questions

**Q: Why a state machine?**
A: Games have fundamentally different behaviors in different modes (menu vs gameplay vs cutscene). A state machine makes transitions explicit and prevents impossible state combinations.

**Q: Why AABB collision instead of pixel-perfect?**
A: For a tile-based game, AABB is sufficient and much faster. Enemies and player are approximately tile-sized, so rectangular collision is accurate enough.

**Q: Why Manhattan distance for spawning?**
A: Manhattan distance is faster to compute (no sqrt) and produces a diamond-shaped range, which feels more natural on a grid than circular Euclidean distance.

**Q: Why is the Brainrot God immune to normal damage?**
A: Game design choice — the Brainrot God represents an abstract concept (algorithmic corruption) that can't be defeated by physical weapons. Only "memory fragments" (representing human knowledge) can harm it. This reinforces the game's narrative theme.

**Q: Why does the Doom Scroller not move?**
A: It represents "infinite scroll" — the concept of endless content consumption. It's stationary because you don't move through infinite scroll; it comes to you (projectiles in all directions).

### Math Questions

**Q: How does collision detection work?**
A: Two methods:
1. **AABB vs AABB**: Used for melee attacks. Two rectangles overlap if their edges intersect.
2. **Circle vs Circle**: Used for projectile hits. Distance between centers < sum of radii.
3. **4-corner check**: Used for movement. All 4 corners of bounding box must be on walkable tiles.

**Q: How are projectiles normalized?**
A: Direction vector (dx, dy) is divided by its length to get a unit vector, then multiplied by speed. This ensures consistent speed regardless of direction or distance.

**Q: How does the camera follow the player?**
A: Raylib's Camera2D. Target = player center, offset = screen center, zoom = 2.5x. The camera automatically keeps the player centered on screen.

**Q: How does the HP bar color change?**
A: Percentage = health/maxHealth. Green ≥ 60%, Orange 30-60%, Red < 30%.

### Gameplay Questions

**Q: Walk through the full game flow.**
A: Menu → Enter name → Intro cutscene (14s) → Explore level 0 → Find Mayor → Dialogue → Survival mode → Kill enemies → Find cave → Level 1 → Fight Rat King → Level 2 → Fight Doom Scroller → Algorithm → Brainrot God (use fragments) → Level 3 → Reach exit → Victory.

**Q: How does the checkpoint system work?**
A: When entering survival mode, checkpoint saves position, level, and state. On death with lives remaining, player respawns at checkpoint with full HP. Lives decrease by 1. On 0 lives → game over.

**Q: How does the gun powerup work?**
A: Spawns every 4 seconds on a random ground tile (≥3 tiles from player). Player picks up by walking over it. Grants 4 seconds of ranged attack (press SPACE to fire projectile in facing direction). Projectile deals 500 damage, travels at 350 px/s.

**Q: How does the radius blast work?**
A: Charges after 7 seconds in survival mode. Hold F to aim (red circle indicator). Release to fire — deals 50 damage to all enemies within 160px radius. 8-second cooldown.

### Code Quality Questions

**Q: What would you refactor?**
A:
1. The `GameState` struct is a god object — could be split into subsystems
2. `UpdateGame()` is 525 lines — could be split per state
3. `GetTileType()` uses hardcoded arrays — could use a lookup table or CSV
4. `extern int currentLevel` is a global — could be a field in GameState
5. The `static float lastBlastTime` in `UpdateZombies()` is a bug-prone pattern (shared across all instances)

**Q: What are the limitations?**
A:
1. Single-threaded — no async loading
2. No save/load system — only in-memory checkpoints
3. Fixed map size (35×24) — no scrolling beyond screen
4. Max 15 enemies — limited by array size
5. No particle system — ash effects are simple rectangles
6. No networking — single player only

**Q: How would you add a new level?**
A:
1. Create new CSV map file + tileset texture
2. Add walkable tile list to `GetTileType()`
3. Add level transition logic in `UpdateGame()` (cave exit handling)
4. Add tilemap loading in `GameState_TransitionFromCutscene()`
5. Update HUD level names
6. Optionally add new enemy types/bosses

**Q: How would you add save/load?**
A: Serialize `GameState` fields (position, health, level, boss statuses, inventory) to a binary or JSON file. Load on game start. Would need to handle pointer-based resources (textures, sounds) by reloading from files.

**Q: How would you optimize performance?**
A:
1. Only draw tiles visible on screen (frustum culling)
2. Use spatial hashing for collision detection
3. Object pooling for particles
4. Reduce texture loads by caching
5. Profile with Raylib's built-in FPS counter

### Specific Function Questions

**Q: Walk through `UpdateGame()` in detail.**
A: [See Section 7.2 — full breakdown of every case]

**Q: How does `GetEnemyProperties()` scale difficulty?**
A: Two-layer scaling: difficulty multiplier (Easy/Normal/Hard) × level multiplier (1.0 + level × 0.2). Regular enemies and bosses use different multiplier tables.

**Q: Explain `OnZombieDeath()` chain reaction.**
A: On boss death, it triggers a sequence: marks defeated → shows log → clears all enemies → may spawn next boss (Doom Scroller → Algorithm → Brainrot God) → may transition to next level. This creates boss rush progression.

**Q: How does the tile classification work?**
A: `GetTileType()` checks in priority order: Mayor (283) → Cave (236/237/306/308/158) → Cars (19 hardcoded IDs) → Level-specific walkable lists → Default: Wall. This means certain tile IDs have hardcoded meanings across all levels.

---

*Document generated for "The GameRot" codebase. Total source: ~2,500 lines of C across 7 files.*
