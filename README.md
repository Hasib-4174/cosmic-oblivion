# COSMIC OBLIVION

> A polished arcade-style space shooter built in pure C with raylib.

![C](https://img.shields.io/badge/Language-C-blue)
![raylib](https://img.shields.io/badge/Graphics-raylib-green)
![Linux](https://img.shields.io/badge/Platform-Linux-lightgrey) ![Single
File](https://img.shields.io/badge/Architecture-Single%20File-orange)

------------------------------------------------------------------------

## 📸 Screenshots

|Main Menu | Gameplay |
| :--- | :--- |
|![Main Menu Screenshot](screenshots/menu.png) | ![Gameplay Screenshot](screenshots/gameplay.png)|

------------------------------------------------------------------------

## 🚀 About The Game

**Cosmic Oblivion** is a fast-paced arcade-style space shooter built
entirely in C using the raylib graphics library.

**The game features:** 
- A state-driven UI system - Three selectable
spaceships 
- Procedural meteor generation - Particle-based visual
effects 
- Score & persistent highscore system 
- Smooth animations and
screen effects

**Gameplay loop:**

> Spawn → Dodge → Shoot → Survive → Score → Game Over → Retry

------------------------------------------------------------------------

## 🎮 Features

### Gameplay

-   60 FPS smooth gameplay
-   Delta-time based movement
-   Acceleration & velocity-based controls
-   HP system
-   Combo multiplier
-   Progressive difficulty scaling

### 🚀 Spaceships

-   **Interceptor** --- Fast, lightweight, rapid-fire
-   **Destroyer** --- Balanced stats
-   **Titan** --- Slow but heavy damage

**Each ship includes:** 
- Unique visual design 
- Engine glow effects 
- Custom fire rate 
- Distinct stats

### ☄️ Meteors

-   Small, Medium, Large types
-   Procedural irregular shapes
-   Rotation while falling
-   Break into smaller fragments
-   Difficulty scales over time

### ✨ Visual Effects

-   Custom particle system
-   Engine trails
-   Explosion bursts
-   Glow simulation
-   Screen shake effects
-   Animated parallax starfield background

### 🖥 UI System

-   Animated main menu
-   Mouse & keyboard navigation
-   Ship selection screen
-   Pause screen
-   Game Over screen
-   Smooth state transitions

------------------------------------------------------------------------

## 📁 Project Structure

Cosmic-Oblivion/

├── main.c\
├── highscore.txt\
└── screenshots/

This is a single-file C project using fixed-size arrays and no dynamic
memory allocation.

------------------------------------------------------------------------

## 📦 Requirements

-   Linux (X11)
-   GCC
-   raylib (latest stable)

Compatible with Windows and macOS if raylib is installed correctly.

------------------------------------------------------------------------

## 🔧 Installing raylib

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

## 📥 Getting The Project

### Clone via Git

    git clone https://github.com/Hasib-4174/cosmic-oblivion.git
    cd cosmic-oblivion

### Download ZIP

Download the repository ZIP from GitHub and extract it.

------------------------------------------------------------------------

## 🛠 Compile & Run


```bash
gcc main.c -lraylib -lm -lpthread -ldl -lrt -lX11 -o cosmic
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

## 🎮 Controls

| Action | Key |
| :--- | :--- |
| **Move** | WASD / Arrow Keys |
| **Shoot** | Space |
| **Pause** | ESC |
| **Select** | Enter |

------------------------------------------------------------------------

## 🏆 Highscore System

-   Stored in `highscore.txt`
-   Auto-created if missing
-   Updated when a new record is achieved

------------------------------------------------------------------------

## 🧠 Development Philosophy

-   Built in pure C (C99 compatible)
-   No external game engine
-   Minimal dependencies
-   Clean state-based architecture
-   Structured development workflow using **Vibecode**
-   Developed with **Google Antigravity** agentic editor

------------------------------------------------------------------------

## 📜 License

This project is open-source under the **MIT License**.

