# COSMIC OBLIVION

> A polished arcade-style space shooter built in pure C with raylib.

![C](https://img.shields.io/badge/Language-C-blue)
![raylib](https://img.shields.io/badge/Graphics-raylib-green)
![Linux](https://img.shields.io/badge/Platform-Linux-lightgrey)
![Modular](https://img.shields.io/badge/Architecture-Modular-orange)

------------------------------------------------------------------------

## Screenshots

|Main Menu | Gameplay |
| :--- | :--- |
|![Main Menu Screenshot](screenshots/menu.png) | ![Gameplay Screenshot](screenshots/gameplay.png)|

------------------------------------------------------------------------

## About The Game

**Cosmic Oblivion** is a fast-paced arcade-style space shooter built
entirely in C using the raylib graphics library.

**The game features:** 
- A state-driven UI system
- Three selectable spaceships 
- Procedural meteor generation 
- Particle-based visual effects 
- Score & persistent highscore system 
- Smooth animations and screen effects

**Gameplay loop:**

> Spawn → Dodge → Shoot → Survive → Score → Game Over → Retry

------------------------------------------------------------------------

## Features

### Gameplay

-   60 FPS smooth gameplay
-   Delta-time based movement
-   Acceleration & velocity-based controls
-   HP system
-   Combo multiplier
-   Progressive difficulty scaling

### Spaceships

-   **Interceptor** --- Fast, lightweight, rapid-fire
-   **Destroyer** --- Balanced stats
-   **Titan** --- Slow but heavy damage

**Each ship includes:** 
- Unique visual design 
- Engine glow effects 
- Custom fire rate 
- Distinct stats

### Meteors

-   Small, Medium, Large types
-   Procedural irregular shapes
-   Rotation while falling
-   Break into smaller fragments
-   Difficulty scales over time

### Visual Effects

-   Custom particle system
-   Engine trails
-   Explosion bursts
-   Glow simulation
-   Screen shake effects
-   Animated parallax starfield background

### UI System

-   Animated main menu
-   Mouse & keyboard navigation
-   Ship selection screen
-   Pause screen
-   Game Over screen
-   Smooth state transitions

------------------------------------------------------------------------

## Project Structure

```
cosmic-oblivion/
├── src/
│   ├── main.c       # Entry point, game loop
│   ├── helpers.c    # Utility functions
│   ├── stars.c      # Background starfield
│   ├── particles.c  # Particle effects
│   ├── button.c     # UI buttons
│   ├── ship.c      # Ship rendering
│   ├── meteor.c    # Meteor system
│   ├── game.c      # Core game logic
│   └── ui.c        # Screen functions
├── include/
│   ├── constants.h # Constants, enums, type definitions
│   ├── helpers.h   # Helper function declarations
│   ├── stars.h     # Starfield declarations
│   ├── particles.h # Particle declarations
│   ├── button.h    # Button declarations
│   ├── ship.h      # Ship declarations
│   ├── meteor.h    # Meteor declarations
│   ├── game.h      # Game logic declarations
│   └── ui.h        # UI declarations
├── tests/           # Unit tests
├── screenshots/    # Game screenshots
├── Makefile        # Build automation
├── AGENTS.md       # Developer documentation
└── README.md       # This file
```

------------------------------------------------------------------------

## Requirements

-   Linux (X11)
-   GCC
-   raylib (latest stable)

Compatible with Windows and macOS if raylib is installed correctly.

------------------------------------------------------------------------

## Installing raylib

### Arch Linux

    sudo pacman -S raylib

### Ubuntu / Debian (Build from source)

    git clone https://github.com/raysan5/raylib.git
    cd raylib
    mkdir build && cd build
    cmake ..
    make
    sudo make install

### Windows & macOS
Follow the official [raylib installation guide](https://github.com/raysan5/raylib?tab=readme-ov-file#build-and-installation) for detailed setup instructions on these platforms.

------------------------------------------------------------------------

## Compile & Run

### Using Makefile (recommended)
```bash
make           # Compile game
make run       # Compile and run
make debug     # Debug build
make release   # Release build
make lint      # Run cppcheck
make test      # Run unit tests
make clean     # Clean build artifacts
```

### Manual Compile
```bash
gcc src/*.c -I. -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic
./cosmic
```

**Flags Summary:**
* `-lraylib`: Link the raylib graphics library.
* `-lm`: Link the math library.
* `-lpthread`: Enable POSIX threads support.
* `-ldl`: Dynamic linking loader library.
* `-lrt`: Realtime extensions library.
* `-lX11`: X Window System support.

------------------------------------------------------------------------

## Controls

| Action | Key |
| :--- | :--- |
| **Move** | WASD / Arrow Keys |
| **Shoot** | Space |
| **Pause** | ESC |
| **Select** | Enter |

------------------------------------------------------------------------

## Highscore System

-   Stored in `highscore.txt`
-   Auto-created if missing
-   Updated when a new record is achieved

------------------------------------------------------------------------

## Testing

### Unit Tests
```bash
make test      # Run unit tests (currently 15 tests)
```

Tests are in `tests/` directory and cover helper functions (Clampf, Rf, CAlpha, CLerp).

### Manual Testing
Run `./cosmic` and test: menu, ship select, gameplay, pause, game over.

---

## Development Philosophy

-   Built in pure C (C99 compatible)
-   No external game engine
-   Minimal dependencies
-   Modular architecture (10 source files)
-   Clean state-based architecture

------------------------------------------------------------------------

## License

This project is open-source under the **MIT License**.
