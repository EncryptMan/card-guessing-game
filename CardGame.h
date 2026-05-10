#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <chrono>
#include <fstream>
#include "GameException.h"

using namespace std;


// Representation of a single playing card
// Tracks its visible value and whether it is flipped or matched
class Card {
private:
    string value;
    bool isFlipped;
    bool isMatched;

public:
    Card();
    Card(const string& cardValue);

    // Getters
    const string& getValue() const;
    bool getIsFlipped() const;
    bool getIsMatched() const;

    // Card operations with validation
    void flip();
    void hide();
    void match();
};

// Manages the grid of Card objects and provides basic board operations
class GameBoard {
private:
    Card cards[8][8];
    int gridSize;

    // Validation helper
    void validateBoardPosition(int row, int col) const;

public:
    GameBoard();

    // Getters
    int getGridSize() const;
    const Card& getCard(int row, int col) const;
    Card& getCardMutable(int row, int col);

    // Board operations with exception handling
    void initializeBoard(int size);
    Card flipCard(int row, int col);
    bool checkMatch(const Card& card1, const Card& card2) const;
    void hideCards(Card& card1, Card& card2);
    bool allCardsMatched() const;
};

// Represents the player and tracks the number of attempts they made
class Player {
private:
    string name;
    int attempts;

public:
    Player();
    Player(const string& playerName);

    // Getters
    const string& getName() const;
    int getAttempts() const;

    // Player operations
    void makeGuess(int row1, int col1, int row2, int col2);
    void resetAttempts();
    void validateCoordinates(int row, int col, int gridSize) const;
};

// Keeps a simple current score and reads writes high scores to a text file
class ScoreSystem {
public:
    static constexpr size_t MaxHighScores = 100;

private:
    int currentGameScore;
    double difficultyMultiplier;
    string highScoreFile;
    // Stored as raw parallel arrays to keep the structure simple.
    string highScoreNames[MaxHighScores];
    int highScoreValues[MaxHighScores];
    size_t highScoreCount;

public:
    ScoreSystem();

    // Update the current score using a simple easy to understand formula.
    void updateScore(bool match, int elapsedSeconds, int hintsUsed, int attempts,
                     int totalPairs);
    int getCurrentScore() const;
    void resetScore();
    void setDifficultyMultiplier(double multiplier);

    // High score persistence
    void saveHighScore(const string& playerName, int score);
    size_t loadHighScores();
    size_t getTopScores(string names[], int scores[], size_t limit = 10) const;
};

// Thin wrapper around raylib drawing calls
class UI {
public:
    void displayBoard(const GameBoard& board);
    void displayScore(int score);
    void displayAttempts(int attempts);
    void getCardSelection(int selection[2]);
    void displayMessage(const string& message);
};

// Coordinates the whole game including menus, main loop, and interactions between
class Game {
private:
    string gameState;
    Player currentPlayer;
    GameBoard gameBoard;
    ScoreSystem scoreSystem;

    // state that used to be spread across multiple helper classes.
    int selectedBoardSize;
    int selectedTimeLimit; // seconds, 0 for no limit
    int hintsRemaining;
    int hintsUsed;
    int lastHintR1, lastHintC1, lastHintR2, lastHintC2;
    bool hintActive;
    chrono::steady_clock::time_point startTime;

    // Helper for name validation
    string validatePlayerName(const string& rawName);
    
    // Difficulty and mode selection
    void displayDifficultyMenu();
    void displayBoardSizeMenu();

public:
    Game();

    // Game lifecycle
    void startGame();
    void endGame();
    void processTurn();
    
    // Getters for UI 
    int getHintsRemaining() const;
    int getSelectedBoardSize() const;
};