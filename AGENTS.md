# AGENTS.md - Cosmic Oblivion

## Project Overview
- **Project**: Cosmic Oblivion - Arcade Space Shooter
- **Language**: Pure C (C99 compatible)
- **Framework**: raylib
- **Architecture**: Modular (14 source files, 14 header files)

---

## Build & Run Commands

### Using Makefile (recommended)
```bash
make           # Compile game
make run       # Compile and run
make debug     # Debug build with -g -O0
make release   # Release build with -O2 -DNDEBUG
make lint      # Run cppcheck static analysis
make analyze   # Run gcc -fanalyzer
make clean     # Clean build artifacts
```

### Manual Compile
```bash
gcc src/*.c -I. -Wall -Wextra -Wpedantic -std=c99 \
    -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic
```

### Running the Game
- Requires X11 display (Linux)
- On headless systems: `xvfb-run -a ./cosmic`
- Highscore stored in `highscore.txt` (auto-created)

---

## Code Style

### Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Line length**: Max ~120 characters
- **Trailing whitespace**: None
- **Statements per line**: One

### Naming Conventions
| Type | Convention | Examples |
|------|------------|----------|
| Constants | UPPER_SNAKE_CASE | `MAX_BULLETS`, `SW`, `SH` |
| Types/Enums | PascalCase | `GameScreen`, `ShipType`, `Particle` |
| Variables | camelCase | `playerPos`, `shakeTimer`, `G` |
| Functions | camelCase | `UpdateStars`, `SpawnMeteor` |
| Static functions | lowercase + prefixes | `initStars`, `drawButton` |
| Files | lowercase | `game.c`, `constants.h` |

### Imports Order
```c
#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "include/constants.h"
#include "include/helpers.h"
```
- **raylib first**, then **stdlib** (stdio, stdlib, math, string), then **local headers**
- Use `<>` for stdlib, `""` for local headers
- Always use `include/` prefix for local headers

### Types
| Type | Usage |
|------|-------|
| `Vector2` | Positions, velocities |
| `Rectangle` | Hitboxes, UI bounds |
| `Color` | Colors (RGBA) |
| `bool` | Boolean flags |
| `float` | Physics, timers, interpolation |
| `int` | Counters, indices, scores |
| `unsigned char` | Color components |

### Structs
```c
typedef struct {
    Vector2 pos, vel;
    float speed, fireRate;
    int hp, maxHp;
    bool alive;
} Player;
```
- Group related fields together
- Use raylib composite types (Vector2, Color)
- Keep structs POD (plain old data) where possible

### Functions
- **Prefixes**:
  - `Init*` - Initialization (e.g., `InitGame`, `InitStars`)
  - `Update*` - Game logic per frame (e.g., `UpdatePlayer`, `UpdateMeteors`)
  - `Draw*` - Rendering (e.g., `DrawParticles`, `DrawUI`)
  - `Spawn*` - Entity creation (e.g., `SpawnMeteor`, `SpawnHealthStar`)
- Keep functions focused and under ~50 lines
- Use `static` for internal helper functions
- Public functions go in header files; static helpers stay in .c

### Error Handling
- Return early on failures
- Check file operations: `if (!f) return;` or `if (!f) return 0;`
- Fail gracefully (e.g., missing highscore = 0)
- No assertions or `exit()` in production code

---

## Architecture

### Global State
- Global `GameState G` defined in `main.c` (accessed via `extern`)
- Single source of truth for all game data
- Fixed-size arrays with `MAX_*` constants (no dynamic allocation)

### Entity Lifecycle
```
Init*()   →  Update*(dt)  →  Draw*()
   ↑              ↑              ↑
  Setup      per-frame      per-frame
           logic/state     rendering
```

### Delta Time
- All movement uses `dt` (delta time) for frame-rate independence
- `dt` passed from main game loop
- Values in "units per second"

---

## File Structure

### Source Files (src/)
| File | Purpose |
|------|---------|
| `main.c` | Entry point, game loop, screen switching |
| `game.c` | Core game logic, player, collision detection |
| `helpers.c` | Utilities (Rf, Clampf, CAlpha, CLerp, LoadHS, SaveHS) |
| `stars.c` | Parallax starfield background |
| `particles.c` | Particle effects system |
| `healthstar.c` | Health pickup drops |
| `shieldpickup.c` | Shield pickup drops |
| `floatingtext.c` | Score/HP feedback text |
| `button.c` | UI button components |
| `ship.c` | Ship rendering and stats |
| `enemy.c` | Enemy ship AI, spawning, shooting |
| `meteor.c` | Meteor spawning, movement, destruction |
| `ui.c` | Menu, pause, game over screens |

### Header Files (include/)
Each .c has a corresponding .h in `include/`

---

## Adding New Features

### Adding a new entity type:
1. Add `MAX_*` constant and struct typedef to `constants.h`
2. Add array to `GameState` struct in `constants.h`
3. Create `src/entityname.c` and `include/entityname.h`
4. Implement `InitEntity`, `UpdateEntity`, `DrawEntity`, `SpawnEntity`
5. In `game.c`: include header, call Init in InitGame, call Update/Draw in main loop

### Adding a new screen:
1. Add enum value to `GameScreen` in `constants.h`
2. Implement drawing in `ui.c` or new file
3. Add state handling in `main.c` game loop

---

## Audio System
- Uses raylib's AudioDevice module
- Music: `PlayMusicStream()` with `UpdateMusicStream()` per frame
- Sound effects: `PlaySound()` for one-shot effects
- Audio files in `audio/` directory organized by category
- Global mute toggle available via `G.audioEnabled`

---

## Common Issues

| Issue | Solution |
|-------|----------|
| Linker errors | Install raylib: `sudo pacman -S raylib` |
| Runtime errors | Requires X11; use `xvfb-run` on headless |
| Missing highscore | Auto-created on first save |
| Segfault on exit | Ensure all fclose() calls have null checks |

---

## Development Notes

- No external dependencies beyond raylib and standard C
- Uses fixed-size arrays; entities pool reused
- Screen shake via `G.shakeTimer` and `G.shakeMag`
- Combo system for score multipliers
- Difficulty scales over time via `G.difficulty`
- Enemy ships: Scout (fast), Fighter (balanced), Bomber (slow/heavy)
