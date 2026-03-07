# AGENTS.md - Cosmic Oblivion

## Project Overview
- **Project**: Cosmic Oblivion - Arcade Space Shooter
- **Language**: Pure C (C99 compatible)
- **Framework**: raylib
- **Architecture**: Modular (12 source files)

---

## Build & Run Commands

### Using Makefile (recommended)
```bash
make           # Compile game
make run       # Compile and run
make debug     # Debug build
make release   # Release build
make lint      # Run cppcheck static analysis
make analyze   # Run gcc -fanalyzer
make test      # Run unit tests (all tests)
make clean     # Clean build artifacts
```

### Manual Compile
```bash
gcc src/*.c -I. -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic && ./cosmic
```

---

## Testing

### Unit Tests
```bash
make test      # Run all unit tests (currently 15 tests)
```

Tests are in `tests/` directory. Test framework uses custom mocks in `tests/test_mocks.c`. To add tests:
1. Add test functions to `tests/test_*.c`
2. Tests use `printf` for output with `PASS/FAIL` format

### Manual Testing
Run `./cosmic` and test: menu, ship select, gameplay, pause, game over. 

**Controls:**
- WASD / Arrow keys: Move ship
- Space: Shoot
- ESC: Pause game

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
- raylib first, then stdlib
- Use `<>` for stdlib, `""` for local headers (e.g., `"include/constants.h"`)

### Types
- Use raylib types: `Vector2`, `Rectangle`, `Color`, `bool`
- `float` for physics, `int` for counters
- `unsigned char` for color components

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
- Inline where they fit

### Functions
- Use `Init*`, `Update*`, `Draw*` prefixes for lifecycle functions
- Use `Spawn*` prefix for entity spawning
- Keep functions small and focused
- Static for internal helper functions

### Error Handling
- Return early on failures
- Check file operations: `if(!f) return;`
- Fail gracefully (e.g., missing highscore = 0)

---

## Architecture

- Global `GameState G` in main.c (not static - needed by all modules via `extern`)
- Fixed-size arrays with MAX_* constants, no dynamic memory allocation
- Entity lifecycle: `Init*()` → `Update*(dt)` → `Draw*()`
- Use `dt` (delta time) for frame-rate independent physics

---

## File Structure
```
src/
  main.c         - entry point, game loop
  game.c         - core game logic, player, collision
  helpers.c      - Rf, Clampf, CAlpha, CLerp, LoadHS, SaveHS
  stars.c        - starfield background
  particles.c    - particle effects
  healthstar.c   - health pickup spawn/update/draw
  floatingtext.c - floating text feedback (+HP, etc)
  button.c       - UI buttons
  ship.c         - ship rendering
  meteor.c       - meteor spawning/rendering
  ui.c           - screens (menu, pause, game over)
include/
  constants.h    - constants, enums, types, GameState struct
  game.h         - game logic declarations
  helpers.h      - helper function declarations
  stars.h        - starfield declarations
  particles.h    - particle declarations
  healthstar.h   - health star declarations
  floatingtext.h - floating text declarations
  button.h       - button declarations
  ship.h         - ship declarations
  meteor.h       - meteor declarations
  ui.h           - UI declarations
tests/
  test_helpers.c - unit tests for helper functions
  test_mocks.c   - mock implementations for testing
```

---

## Adding New Features

### Adding a new entity type (e.g., power-ups, enemies):
1. Add MAX_* constant and struct to `constants.h`
2. Add array to GameState in `constants.h`
3. Create `entityname.c` and `entityname.h` in src/ and include/
4. Implement `Init*`, `Update*`, `Draw*`, `Spawn*` functions
5. Add to game.c: include header, call Init* in InitGame, call Update*/Draw* in game loop

### Adding to GameState:
```c
// In constants.h, add to GameState struct:
EntityName entities[MAX_ENTITIES];
```

---

## Common Issues
- **Linker errors**: Install raylib (`sudo pacman -S raylib` or build from source)
- **Runtime errors**: Requires X11 display (use Linux with display or Xvfb)
- **highscore.txt**: Auto-created on first save to current directory

---

## Extension Points
1. Add constants to `constants.h`
2. Extend `GameState` struct in `constants.h`
3. Add new enums for new types
4. Follow Init/Update/Draw pattern in appropriate module
5. Use existing particle system for visual effects via `SpawnP()`
