#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <fstream>
#include "GameException.h"

// Simple representation of a single playing card
// Tracks its visible value and whether it is flipped or matched
class Card {
private:
    std::string value;
    bool isFlipped;
    bool isMatched;

public:
    Card();
    Card(const std::string& cardValue);

    // Getters
    const std::string& getValue() const;
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
    std::vector<std::vector<Card>> cards;
    int gridSize;

    // Validation helper
    void validateBoardPosition(int row, int col) const;

public:
    GameBoard();

    // Getters
    int getGridSize() const;
    const std::vector<std::vector<Card>>& getCards() const;
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
    std::string name;
    int attempts;

public:
    Player();
    Player(const std::string& playerName);

    // Getters
    const std::string& getName() const;
    int getAttempts() const;

    // Player operations
    void makeGuess(int row1, int col1, int row2, int col2);
    void resetAttempts();
    void validateCoordinates(int row, int col, int gridSize) const;
};

// Keeps a simple current score and reads writes high scores to a text file
class ScoreSystem {
private:
    int currentGameScore;
    double difficultyMultiplier;
    std::string highScoreFile;
    // Stored as name -> score
    std::map<std::string, int> highScores;

public:
    ScoreSystem();

    // Update the current score using a simple easy to understand formula.
    void updateScore(bool match, int elapsedSeconds, int hintsUsed, int attempts,
                     int totalPairs);
    int getCurrentScore() const;
    void resetScore();
    void setDifficultyMultiplier(double multiplier);

    // High score persistence
    void saveHighScore(const std::string& playerName, int score);
    std::map<std::string, int> loadHighScores();
    std::vector<std::pair<std::string, int>> getTopScores(size_t limit = 10) const;
};

// Thin wrapper around raylib drawing/input calls
class UI {
public:
    void displayBoard(const GameBoard& board);
    void displayScore(int score);
    void displayAttempts(int attempts);
    std::vector<int> getCardSelection();
    void displayMessage(const std::string& message);
};

// Coordinates the whole game: menus, main loop, and interactions between
class Game {
private:
    std::string gameState;
    Player currentPlayer;
    GameBoard gameBoard;
    ScoreSystem scoreSystem;

    // Simplified state that used to be spread across multiple helper classes.
    int selectedBoardSize;
    int selectedTimeLimit; // seconds, 0 = no limit
    int hintsRemaining;
    int hintsUsed;
    int lastHintR1, lastHintC1, lastHintR2, lastHintC2;
    bool hintActive;
    std::chrono::steady_clock::time_point startTime;

    // Helper for name validation
    std::string validatePlayerName(const std::string& rawName);
    
    // Difficulty and mode selection (simplified)
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