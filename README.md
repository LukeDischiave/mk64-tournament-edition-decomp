# Refactor of Mario Kart 64 Multiplayer ROM Hack (Tournament Edition)
Refactor of Mario Kart 64 ROM hack with configuration menu

## Features
### TITLE MENU Options
- Game Pacing: Default/original (15-30fps)* / 30fps** / 60fps
- Widescreen: 4:3*(default) / 16:9**
- MP Music: Disabled*(Default) / Enabled**
- VS Detail: Original*(Default) / Enabled** / Both
  - Multiplayer DKJP Boat: Enabled* / Disabled**
  - Multiplayer KD Train: Original* / Full**
- Practice mode: Disabled*(Default) / Enabled**
- Extra mode: Disabled*(Default) / Enabled**
- Force Minimap: Disabled*(Default) / prog view**, map
- Anti-aliasing: Disabled*(Default) / Enabled**

### FEATURES
- Tie-logic: Based off the in-game timer instead of player port
- Lightning: Single white flash so easier on the eyes
- VS timer: Players' finish timers are displayed on the points screen
- Sounds: No c-up or c-right sounds during the race
- Slow pause: players must hold pause for roughly .5 sec to mitigate accidental pauses

### CHARACTER SELECT Options
- Character Stats: Original / All Yoshi / All Wario

### VS MAP SELECT Screen
- Track Selection: VA*(Original) / Kaillera**(in-order) / Random

### VS MATCH RANKING Screen
- Include total score by player as 3/2/1 (3P) or 3/2/1/0 (4P)
- Include ability to adjust VS scores if all players hold L + R bumpers and use C-UP and C-DOWN to adjust scores

## TODO

### TITLE MENU Options

### GAME SELECT Menu Options
- Course Z Scaler: Original / Minimum / Maximum
- Randomizer
- Engine (150cc): 150cc / 200cc
- Engine (Extra): 100cc / 150cc / 200cc
- Practice mode bots

### VS MAP SELECT Screen
- VS Track Order: All Cups (Random Order)
- VS Bomb Karts: Enabled / Disabled
- VS CPUs: None / 1 / 2 / 3 / 4 / 5 / 6 / 7

### VS MATCH RANKING Screen


### CREDITS
- Megamech and the mk64 decomp team 
Please credit Climatee, Zserf, and dntn31 if you decided to fork this specific repo

## Building (Windows)
This decompilation of Mario Kart 64 pursues historical and educational elements within the game found via taking it apart and putting it back together. Inspiration to do so not only emanates from the game's hardware and technology but also its immensely positive effects on the cultures and families of nearly every nationality.

This repository does not contain assets. Compiling requires asset extraction from a prior copy of the game.

It supports and builds the following versions:

| ROM Output      | Revision| SHA-1 Checksum                           |
|-----------------|---------|------------------------------------------|
| mk64.us.z64     | USA     | 579c48e211ae952530ffc8738709f078d5dd215e |
| mk64.eu.v10.z64 | EUR 1.0 | a729039453210b84f17019dda3f248d5888f7690 |
| mk64.eu.v11.z64 | EUR 1.1 | f6b5f519dd57ea59e9f013cc64816e9d273b2329 |

## Progress

[![Linux Compile](https://github.com/n64decomp/mk64/actions/workflows/linux-compile.yml/badge.svg)](https://github.com/n64decomp/mk64/actions/workflows/linux-compile.yml)

![Build Status](https://n64decomp.github.io/mk64/total_progress.svg)

## Quick Start

```
git submodule update --init --recursive
OR
git clone https://github.com/n64decomp/mk64 --recurse-submodules
```
Extract assets
```
make -C tools -j
make assets -j
make -j
```

## Building

Build instructions located in the [API docs](https://n64decomp.github.io/mk64/compiling.html)  
See also, the comprehensive API and documentation which includes an overview of the code-base while also discussing significant concepts, features, and modding.

## Current State

The code-base may change significanty overtime as naming and documentation continues.
Adjustments to the game require compiling with AVOID_UB=1 to enable shiftability. Use DEBUG=1 to enable the games debug mode.

Some menu textures are compressed using a format called tkmk00. A byte-matching compressor/decompressor does not yet exist.   

## Project Structure
	
	mk64
	├── asm: Handwritten assembly code, rom header and boot
	│   ├── non_matchings: Assembly for non-matching sections
	│   └── os: Libultra handwritten assembly code
	├── assets: Textures
	├── courses: Course data, geography, display lists and staff ghosts
	├── build: Output directory
	├── data: Misc data, text, audio banks, and instrument sets
	├── docs: Build guides
	├── include: Header files
	├── music: Sequences
	├── src: C source code for the game
	|   ├── actors: Individual actors split out from other files
	│   ├── audio: Sample tables and audio code
	│   ├── data: Misc data referenced in other C files
	|   ├── debug: Custom debug code
	|   ├── ending: Podium ceremony and credits code
	│   ├── os: Libultra C code
	|   └── racing: Race and game engine code
	├── textures: Texture data, bitmaps
	|   ├── common: Textures common to many courses
	|   ├── courses: Course specific textures
	|   ├── crash screen: Crash screen font image
	│   ├── raw: Raw textures
    │   ├── standalone: Whole textures
	|   ├── startup_logo: Reflection map
	|   └── trophy: Ceremony cutscene podium and trophy textures
	└── tools: build tools

## Documentation

Documentation available online at [https://n64decomp.github.io/mk64/](https://n64decomp.github.io/mk64/).

Documentation generated using [Doxygen](https://www.doxygen.nl/index.html). Run `make doc` to test locally. Documentation resides in `docs/html`. Open index.html to view the site.

## Contributing

Pull requests are welcome. For major changes, please discuss in the Discord.

Run `make format` to ensure it meets the project's coding standards.
If code needs to avoid formatting place `// clang-format off` before the code and `// clang-format on` after.

Official Discord: [discord.gg/DuYH3Fh](https://discord.gg/DuYH3Fh)
