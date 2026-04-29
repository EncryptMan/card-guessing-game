# Card Guessing Game

This project is a simple C++ card guessing game built as a semester project. The idea is straightforward: the player makes guesses, the game responds with feedback, and scoring is tracked across rounds.

The focus of the project is clean object-oriented design with classes for game flow, board handling, cards, scoring, and user interaction.

## UI Library

This version uses `raylib` for a graphical UI.

## Project Structure

- `CardGame.h` contains class declarations.
- `CardGame.cpp` contains class method implementations and raylib-based UI rendering.
- `main.cpp` only starts the game.

## Build and Run (macOS)

If raylib is not installed yet:

```bash
brew install raylib
```

Build:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp CardGame.cpp -o card-guessing-game $(pkg-config --libs --cflags raylib)
```

Run:

```bash
./card-guessing-game
```

Gameplay basics:

- Type your name on the start screen.
- Press `2`, `4`, or `6` to choose board size.
- Click cards to find matching pairs.
- Match = `+10` points, mismatch = `-2` points (minimum score is `0`).

## Team Members

- Group: S26-05
- Muhammad Umar (25L-0017)
- Syed Naday Ali Zaidi (25L-0002)
- Muhammad Huzaifa Aslam (25L-2558)