#ifndef CARD_GAME_H
#define CARD_GAME_H

#include <map>
#include <string>
#include <vector>

class Card {
public:
    std::string value;
    bool isFlipped;
    bool isMatched;

    Card();
    explicit Card(const std::string& cardValue);

    void flip();
    void hide();
    void match();
};

class GameBoard {
public:
    std::vector<std::vector<Card>> cards;
    int gridSize;

    GameBoard();

    void initializeBoard(int size);
    Card flipCard(int row, int col);
    bool checkMatch(const Card& card1, const Card& card2);
    void hideCards(Card& card1, Card& card2);
    bool allCardsMatched();
};

class Player {
public:
    std::string name;
    int attempts;

    Player();

    void makeGuess(int row1, int col1, int row2, int col2);
};

class ScoreSystem {
public:
    int currentScore;
    std::map<std::string, int> highScores;

    ScoreSystem();

    void updateScore(bool match);
    void saveHighScore(const std::string& playerName, int score);
    std::map<std::string, int> loadHighScores();
};

class UI {
public:
    void displayBoard(const GameBoard& board);
    void displayScore(int score);
    void displayAttempts(int attempts);
    std::vector<int> getCardSelection();
    void displayMessage(const std::string& message);
};

class Game {
public:
    std::string gameState;
    Player currentPlayer;
    GameBoard gameBoard;
    ScoreSystem scoreSystem;

    Game();

    void startGame();
    void endGame();
    void processTurn();
};

#endif
