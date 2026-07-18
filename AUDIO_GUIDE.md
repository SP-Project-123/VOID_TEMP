# Audio Guide - Adding Sounds and Music Later

This guide details what audio assets are required by the game, where to place them, and where to download high-quality retro game audio assets for free.

---

## 📁 Required Audio Assets

The game expects all audio files to be placed in the `resources/` folder in the root of the project workspace. 

Ensure the file formats and filenames match exactly:

| Filename | Type | Game Event Trigger | Recommended Format |
| :--- | :--- | :--- | :--- |
| `resources/bgm.mp3` | Music Stream | Main Menu & Gameplay background track | MP3 (or OGG/WAV) |
| `resources/slash.wav` | Sound Effect | Swung sword attack (Pressing `SPACE`) | WAV |
| `resources/blast.wav` | Sound Effect | Superpower blast ring discharge (Releasing `F`) | WAV |
| `resources/hit.wav` | Sound Effect | Player takes damage from a Zombie | WAV |

---

## 🌐 Where to Find Free Game Music & SFX

Here are the best platforms to download permissive, royalty-free audio assets:

### 1. Kenney.nl (Highly Recommended)
* **What**: Clean, cohesive 8-bit and 16-bit retro audio packs.
* **Link**: [Kenney Audio Assets](https://kenney.nl/assets/category:Audio)
* **License**: **CC0 (Public Domain)** — Completely free, no attribution required.

### 2. OpenGameArt.org
* **What**: Huge community portal of 2D/3D art and game audio. Search for keywords like *"retro background track"*, *"sword attack sound"*, or *"chiptune loop"*.
* **Link**: [OpenGameArt Audio](https://opengameart.org/art-search-advanced?keys=&field_art_type_tid[]=13)
* **License**: Check individual assets (most are CC0 or CC-BY).

### 3. Itch.io (Sound Assets)
* **What**: Permissive retro synth and chiptune sound/music packs uploaded by indie game developers.
* **Link**: [Itch.io Free Game Audio](https://itch.io/game-assets/free/tag-music)
* **License**: Usually free for both personal and commercial projects.

### 4. Freesound.org
* **What**: Huge repository of recorded audio clips. Perfect for searching raw sounds like a *"sword slash"* or *"growl"*.
* **Link**: [Freesound](https://freesound.org/)
* **License**: Filter searches by Creative Commons 0 (CC0) to avoid attribution requirements.

---

## 🛠️ Code Reference (How it works under the hood)

* **Initialization**: The game calls `InitAudioDevice();` in [main.c](file:///c:/Users/User/Downloads/VOID_TEMP-main/VOID_TEMP-main/VOID_TEMP/src/main.c) on startup.
* **Loading Assets**: Audio handles are loaded in `GameState_Init()` in [game.c](file:///c:/Users/User/Downloads/VOID_TEMP-main/VOID_TEMP-main/VOID_TEMP/src/core/game.c). If a file is missing, Raylib logs a console error but **will not crash** (it loads a dummy buffer instead).
* **Music Buffer Update**: Raylib streams music files in chunks. The game updates this buffer each frame using `UpdateMusicStream(game->bgMusic);` in the `UpdateGame` loop.
* **Unloading**: When quitting, `GameState_Unload()` is called to free the audio memory safely and avoid leaks.
