# Card Guessing Game

A beginner-friendly C++ card guessing game built around six core classes: `Game`, `GameBoard`, `Card`, `Player`, `ScoreSystem`, and `UI`.

## Features

### Core Features
- **Memory Matching Gameplay** - Classic card-flipping memory game
- **4 Difficulty Levels** - Easy, Medium, Hard, Expert with simple presets
- **Simple Timer** - Built directly into `Game` for easier reading
- **Simple Hint Counter** - Hint usage is tracked in `Game`
- **Easy Scoring** - Straightforward score updates in `ScoreSystem`
- **High Score Tracking** - Persistent high scores saved to file
- **raylib UI** - Graphical interface with real-time stats

### Difficulty Levels

| Level | Board | Time | Hints | Multiplier |
|-------|-------|------|-------|-----------|
| EASY | 2x2 | 5 min | 5 | 0.8x |
| MEDIUM | 4x4 | 3 min | 3 | 1.0x |
| HARD | 6x6 | 2 min | 2 | 1.5x |
| EXPERT | 8x8 | 90s | 1 | 2.0x |


## Requirements

- **C++17** or later
- **raylib** graphics library
- **macOS/Linux/Windows** with GCC/Clang compiler

## Installation & Build

### 1. Install raylib

**macOS:**
```bash
brew install raylib
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libraylib-dev
```

**Windows:**
Visit [raylib.com](https://www.raylib.com/) for installation instructions

### 2. Build the Game

**Manual compilation:**
```bash
g++ -std=c++14 -Wall -Wextra \
    main.cpp CardGame.cpp \
    -o card-guessing-game $(pkg-config --libs --cflags raylib)

./card-guessing-game
```
