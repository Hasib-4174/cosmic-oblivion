# AGENTS.md - Cosmic Oblivion

## Project Overview
- **Project**: Cosmic Oblivion - Arcade Space Shooter
- **Language**: Pure C (C99 compatible)
- **Framework**: raylib
- **Architecture**: Modular (10 source files)

---

## Build & Run Commands

### Compile & Run
```bash
gcc main.c helpers.c stars.c particles.c button.c ship.c meteor.c game.c ui.c -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic && ./cosmic
```

### Debug Build
```bash
gcc -g -O0 main.c helpers.c stars.c particles.c button.c ship.c meteor.c game.c ui.c -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic_debug
```

### Release Build
```bash
gcc -O2 -Wall -Wextra main.c helpers.c stars.c particles.c button.c ship.c meteor.c game.c ui.c -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic_release
```

### Static Analysis
```bash
gcc -fanalyzer main.c helpers.c stars.c particles.c button.c ship.c meteor.c game.c ui.c -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic
```

---

## Testing
No unit tests. Manual testing required - run `./cosmic` and test: menu, ship select, gameplay, pause, game over. Controls: WASD/Arrows move, Space shoots, ESC pauses.

---

## Code Style

### Formatting
- 4-space indentation (no tabs)
- Opening brace on same line
- Max line ~120 chars, no trailing whitespace

### Naming
- **Constants**: UPPER_SNAKE_CASE (`MAX_BULLETS`, `SW`)
- **Types/Enums**: PascalCase (`GameScreen`, `ShipType`)
- **Variables/Functions**: camelCase (`playerPos`, `UpdateStars`)
- **Static functions**: lowercase with prefixes (`initStars`, `updateParticles`)

### Imports
```c
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
```
raylib first, then stdlib. Use `<>` for stdlib, `""` for local headers.

### Types
- Use raylib types: `Vector2`, `Rectangle`, `Color`, `bool`
- `float` for physics, `int` for counters, `unsigned char` for color components

### Structs
```c
typedef struct {
    Vector2 pos, vel;
    float speed, fireRate;
    int hp, maxHp;
    bool alive;
} Player;
```
Group related fields. Inline where they fit.

### Functions
- Use `Init*`, `Update*`, `Draw*` prefixes for lifecycle
- Static for internal helpers
- Keep small and focused

### Error Handling
- Return early on failures
- Check file ops: `if(!f) return;`
- Fail gracefully (missing highscore = 0)

---

## Architecture

- Global `GameState G` in main.c (not static - needed by all modules)
- Fixed-size arrays, no malloc
- Entity lifecycle: `Init*()` → `Update*(dt)` → `Draw*()`

---

## File Structure
```
main.c         - entry point, game loop
constants.h    - constants, enums, types
helpers.h/c    - Rf, Clampf, CAlpha, CLerp, LoadHS, SaveHS
stars.h/c      - starfield background
particles.h/c  - particle effects
button.h/c     - UI buttons
ship.h/c       - ship rendering
meteor.h/c     - meteor spawning/rendering
game.h/c       - core game logic
ui.h/c         - screens (menu, pause, game over)
```

---

## Common Issues
- **Linker errors**: Install raylib (`sudo pacman -S raylib` or build from source)
- **Runtime**: Requires X11 display. `highscore.txt` auto-created on first save.

---

## Extension Points
1. Add constants to constants.h
2. Extend GameState in constants.h
3. Add new enums for new types
4. Follow Init/Update/Draw pattern in appropriate module
