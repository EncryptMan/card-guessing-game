#include "CardGame.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <random>
#include <sstream>

#include "raylib.h"

using namespace std;

static const int WindowWidth = 1000;
static const int WindowHeight = 760;
static const int BoardLeft = 60;
static const int BoardTop = 130;
static const int BoardAreaSize = 560;
static const int CardGap = 10;

static const GameBoard* LastBoardDrawn = nullptr;

// Constructs rectangle object for a given card position on the board, used for drawing and click detection
static Rectangle getCardRectangle(int row, int col, int gridSize) {
    const int cardSize = (BoardAreaSize - (gridSize + 1) * CardGap) / gridSize;
    const float x = static_cast<float>(BoardLeft + CardGap + col * (cardSize + CardGap));
    const float y = static_cast<float>(BoardTop + CardGap + row * (cardSize + CardGap));
    return Rectangle{x, y, static_cast<float>(cardSize), static_cast<float>(cardSize)};
}

static string trimName(const string& rawName) {
    string clean;
    for (char ch : rawName) {
        if (!isspace(static_cast<unsigned char>(ch))) {
            clean.push_back(ch);
        }
    }
    return clean;
}

Card::Card() : value(""), isFlipped(false), isMatched(false) {}

Card::Card(const string& cardValue) : value(cardValue), isFlipped(false), isMatched(false) {
    if (cardValue.empty()) {
        throw InvalidCardException("Card value cannot be empty");
    }
}

const string& Card::getValue() const {
    return value;
}

bool Card::getIsFlipped() const {
    return isFlipped;
}

bool Card::getIsMatched() const {
    return isMatched;
}

void Card::flip() {
    if (!isMatched) {
        isFlipped = true;
    }
}

void Card::hide() {
    if (!isMatched) {
        isFlipped = false;
    }
}

void Card::match() {
    if (isMatched) {
        throw InvalidCardException("Card is already matched");
    }
    isMatched = true;
    isFlipped = true;
}

GameBoard::GameBoard() : gridSize(4) {}

int GameBoard::getGridSize() const {
    return gridSize;
}

const Card& GameBoard::getCard(int row, int col) const {
    validateBoardPosition(row, col);
    return cards[row][col];
}

Card& GameBoard::getCardMutable(int row, int col) {
    validateBoardPosition(row, col);
    return cards[row][col];
}

void GameBoard::validateBoardPosition(int row, int col) const {
    if (row < 0 || row >= gridSize || col < 0 || col >= gridSize) {
        throw InvalidCardException("Card position is out of bounds");
    }
}

void GameBoard::initializeBoard(int size) {
    try {
        if (size < 2 || size > 8 || size % 2 != 0) {
            throw BoardInitializationException(
                "Board size must be even and between 2 and 8");
        }

        gridSize = size;
        string values[32];
        int valueCount = 0;
        const int totalCards = gridSize * gridSize;

        for (int i = 0; i < totalCards / 2; ++i) {
            string label;
            if (i < 26) {
                label = string(1, static_cast<char>('A' + i));
            } else {
                label = "C" + to_string(i - 25);
            }
            values[valueCount++] = label;
            values[valueCount++] = label;
        }

        random_device rd;
        mt19937 gen(rd());
        shuffle(values, values + valueCount, gen);

        int index = 0;
        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
                cards[row][col] = Card(values[index]);
                index++;
            }
        }
    } catch (const GameException& e) {
        throw;
    } catch (const exception& e) {
        throw BoardInitializationException(e.what());
    }
}

Card GameBoard::flipCard(int row, int col) {
    validateBoardPosition(row, col);
    cards[row][col].flip();
    return cards[row][col];
}

bool GameBoard::checkMatch(const Card& card1, const Card& card2) const {
    return card1.getValue() == card2.getValue();
}

void GameBoard::hideCards(Card& card1, Card& card2) {
    card1.hide();
    card2.hide();
}

bool GameBoard::allCardsMatched() const {
    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            if (!cards[row][col].getIsMatched()) {
                return false;
            }
        }
    }
    return true;
}

Player::Player() : name("Player"), attempts(0) {}

Player::Player(const string& playerName) 
    : name(playerName), attempts(0) {
    if (playerName.empty()) {
        throw GameException("Player name cannot be empty");
    }
}

const string& Player::getName() const {
    return name;
}

int Player::getAttempts() const {
    return attempts;
}

void Player::makeGuess(int row1, int col1, int row2, int col2) {
    try {
        validateCoordinates(row1, col1, 8);
        validateCoordinates(row2, col2, 8);
        attempts++;
    } catch (const GameException& e) {
        throw;
    }
}

void Player::resetAttempts() {
    attempts = 0;
}

void Player::validateCoordinates(int row, int col, int gridSize) const {
    if (row < 0 || row >= gridSize || col < 0 || col >= gridSize) {
        throw GameException("Invalid card coordinates");
    }
}

ScoreSystem::ScoreSystem()
    : currentGameScore(0), difficultyMultiplier(1.0), highScoreFile("highscores.txt"), highScoreCount(0) {
    // Load existing high scores if any into memory
    try {
        highScoreCount = loadHighScores();
    } catch (...) {
        highScoreCount = 0;
    }
}

void ScoreSystem::updateScore(bool match, int elapsedSeconds, int hintsUsed, int attempts,
                              int totalPairs) {
    if (match) {
        int points = static_cast<int>(10.0 * difficultyMultiplier);
        currentGameScore += points;
    }
}

int ScoreSystem::getCurrentScore() const {
    return currentGameScore;
}

void ScoreSystem::resetScore() {
    currentGameScore = 0;
}

void ScoreSystem::setDifficultyMultiplier(double multiplier) {
    difficultyMultiplier = multiplier;
}

void ScoreSystem::saveHighScore(const string& playerName, int score) {
    try {
        const string name = trimName(playerName).empty() ? "Player" : trimName(playerName);
        // keep the best score for each player
        size_t existingIndex = highScoreCount;
        for (size_t i = 0; i < highScoreCount; ++i) {
            if (highScoreNames[i] == name) {
                existingIndex = i;
                break;
            }
        }

        if (existingIndex == highScoreCount) {
            if (highScoreCount < MaxHighScores) {
                highScoreNames[highScoreCount] = name;
                highScoreValues[highScoreCount] = score;
                ++highScoreCount;
            }
        } else if (score > highScoreValues[existingIndex]) {
            highScoreValues[existingIndex] = score;
        }

        ofstream out(highScoreFile);
        if (!out.is_open()) throw FileIOException("Cannot open highscores.txt for writing");
        for (size_t i = 0; i < highScoreCount; ++i) {
            out << highScoreNames[i] << " " << highScoreValues[i] << "\n";
        }
    } catch (const exception& e) {
        throw FileIOException(string("Save high score failed: ") + e.what());
    }
}

size_t ScoreSystem::loadHighScores() {
    highScoreCount = 0;
    ifstream in(highScoreFile);
    if (!in.is_open()) return highScoreCount; // no file yet
    string name;
    int value;
    while (in >> name >> value) {
        size_t existingIndex = highScoreCount;
        for (size_t i = 0; i < highScoreCount; ++i) {
            if (highScoreNames[i] == name) {
                existingIndex = i;
                break;
            }
        }

        if (existingIndex == highScoreCount) {
            if (highScoreCount < MaxHighScores) {
                highScoreNames[highScoreCount] = name;
                highScoreValues[highScoreCount] = value;
                ++highScoreCount;
            }
        } else if (value > highScoreValues[existingIndex]) {
            highScoreValues[existingIndex] = value;
        }
    }
    return highScoreCount;
}

size_t ScoreSystem::getTopScores(string names[], int scores[], size_t limit) const {
    const size_t count = min(limit, highScoreCount);
    for (size_t i = 0; i < count; ++i) {
        names[i] = highScoreNames[i];
        scores[i] = highScoreValues[i];
    }

    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j) {
            if (scores[j] > scores[i]) {
                swap(scores[i], scores[j]);
                swap(names[i], names[j]);
            }
        }
    }

    return count;
}

void UI::displayBoard(const GameBoard& board) {
    LastBoardDrawn = &board;

    DrawRectangle(BoardLeft - 8, BoardTop - 8, BoardAreaSize + 16, BoardAreaSize + 16, Color{35, 42, 53, 255});
    DrawRectangle(BoardLeft, BoardTop, BoardAreaSize, BoardAreaSize, Color{16, 22, 30, 255});

    for (int row = 0; row < board.getGridSize(); ++row) {
        for (int col = 0; col < board.getGridSize(); ++col) {
            const Card& card = board.getCard(row, col);
            const Rectangle rect = getCardRectangle(row, col, board.getGridSize());

            Color fillColor = Color{64, 96, 160, 255};
            if (card.getIsMatched()) {
                fillColor = Color{56, 153, 112, 255};
            } else if (card.getIsFlipped()) {
                fillColor = Color{228, 206, 116, 255};
            }

            DrawRectangleRounded(rect, 0.16f, 8, fillColor);
            DrawRectangleRoundedLinesEx(rect, 0.16f, 8, 2.0f, Color{15, 19, 26, 255});

            if (card.getIsFlipped() || card.getIsMatched()) {
                const int fontSize = 30;
                const int textWidth = MeasureText(card.getValue().c_str(), fontSize);
                const int textX = static_cast<int>(rect.x + rect.width / 2.0f) - textWidth / 2;
                const int textY = static_cast<int>(rect.y + rect.height / 2.0f) - fontSize / 2;
                DrawText(card.getValue().c_str(), textX, textY, fontSize, Color{20, 24, 32, 255});
            }
        }
    }
}

void UI::displayScore(int score) {
    ostringstream stream;
    stream << "Score: " << score;
    DrawText(stream.str().c_str(), 680, 180, 28, Color{230, 232, 235, 255});
}

void UI::displayAttempts(int attempts) {
    ostringstream stream;
    stream << "Attempts: " << attempts;
    DrawText(stream.str().c_str(), 680, 220, 28, Color{230, 232, 235, 255});
}

void UI::getCardSelection(int selection[2]) {
    selection[0] = -1;
    selection[1] = -1;

    if (LastBoardDrawn == nullptr) {
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    for (int row = 0; row < LastBoardDrawn->getGridSize(); ++row) {
        for (int col = 0; col < LastBoardDrawn->getGridSize(); ++col) {
            const Rectangle rect = getCardRectangle(row, col, LastBoardDrawn->getGridSize());
            if (CheckCollisionPointRec(mouse, rect)) {
                selection[0] = row;
                selection[1] = col;
                return;
            }
        }
    }
}

void UI::displayMessage(const string& message) {
    DrawRectangle(45, 40, 910, 62, Color{19, 26, 37, 255});
    DrawRectangleLinesEx(Rectangle{45, 40, 910, 62}, 2.0f, Color{46, 77, 120, 255});
    DrawText(message.c_str(), 58, 58, 24, Color{238, 242, 245, 255});
}

Game::Game() : gameState("idle") {}

int Game::getHintsRemaining() const {
    return hintsRemaining;
}

int Game::getSelectedBoardSize() const {
    return selectedBoardSize;
}

string Game::validatePlayerName(const string& rawName) {
    try {
        const string cleanedName = trimName(rawName);
        if (cleanedName.empty()) {
            throw GameException("Player name cannot be empty");
        }
        return cleanedName;
    } catch (const GameException& e) {
        throw;
    } catch (const exception& e) {
        throw GameException(string("Name validation failed: ") + e.what());
    }
}

void Game::displayDifficultyMenu() {

    int selectedIndex = 2; // default MEDIUM
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ONE)) selectedIndex = 1;
        if (IsKeyPressed(KEY_TWO)) selectedIndex = 2;
        if (IsKeyPressed(KEY_THREE)) selectedIndex = 3;
        if (IsKeyPressed(KEY_FOUR)) selectedIndex = 4;

        const Rectangle startButton = Rectangle{380, 600, 240, 70};
        const bool clickStart = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                                CheckCollisionPointRec(GetMousePosition(), startButton);
        const bool keyStart = IsKeyPressed(KEY_ENTER);

        if (clickStart || keyStart) {
            // Map choices to simple configs
            switch (selectedIndex) {
                case 1:
                    selectedBoardSize = 2; selectedTimeLimit = 300; hintsRemaining = 5;
                    scoreSystem.setDifficultyMultiplier(0.8);
                    break;
                case 2:
                default:
                    selectedBoardSize = 4; selectedTimeLimit = 180; hintsRemaining = 3;
                    scoreSystem.setDifficultyMultiplier(1.0);
                    break;
                case 3:
                    selectedBoardSize = 6; selectedTimeLimit = 120; hintsRemaining = 2;
                    scoreSystem.setDifficultyMultiplier(1.5);
                    break;
                case 4:
                    selectedBoardSize = 8; selectedTimeLimit = 90; hintsRemaining = 1;
                    scoreSystem.setDifficultyMultiplier(2.0);
                    break;
            }
            hintsUsed = 0;
            hintActive = false;
            return;
        }

        BeginDrawing();
        ClearBackground(Color{12, 18, 26, 255});
        DrawText("Select Difficulty / Board Size", 220, 80, 48, Color{245, 248, 252, 255});
        DrawText("1 - EASY: 2x2, 5 min, 5 hints", 150, 180, 24, selectedIndex==1?Color{238,211,111,255}:Color{159,196,225,255});
        DrawText("2 - MEDIUM: 4x4, 3 min, 3 hints", 150, 240, 24, selectedIndex==2?Color{238,211,111,255}:Color{159,196,225,255});
        DrawText("3 - HARD: 6x6, 2 min, 2 hints", 150, 300, 24, selectedIndex==3?Color{238,211,111,255}:Color{159,196,225,255});
        DrawText("4 - EXPERT: 8x8, 90s, 1 hint", 150, 360, 24, selectedIndex==4?Color{238,211,111,255}:Color{159,196,225,255});

        DrawRectangleRounded(startButton, 0.22f, 8, Color{50, 132, 86, 255});
        DrawText("START", 455, 622, 34, Color{250, 252, 255, 255});
        DrawText("Press ENTER or click START to begin", 300, 680, 26, Color{162, 202, 174, 255});
        EndDrawing();
    }
}

void Game::startGame() {
    try {
        gameState = "running";
        scoreSystem.loadHighScores();
        scoreSystem.resetScore();

        InitWindow(WindowWidth, WindowHeight, "Card Guessing Game");
        SetTargetFPS(60);

        string typedName;

        // Main menu loop
        while (!WindowShouldClose()) {
            const int key = GetCharPressed();
            if (key >= 32 && key <= 126 && typedName.size() < 12) {
                typedName.push_back(static_cast<char>(key));
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !typedName.empty()) {
                typedName.pop_back();
            }

            const Rectangle startButton = Rectangle{380, 440, 240, 70};
            const bool clickStart = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                                    CheckCollisionPointRec(GetMousePosition(), startButton);
            const bool keyStart = IsKeyPressed(KEY_ENTER);

            if ((clickStart || keyStart) && !trimName(typedName).empty()) {
                try {
                    currentPlayer = Player(validatePlayerName(typedName));
                } catch (const GameException& e) {
                    continue;
                }
                break;
            }

            BeginDrawing();
            ClearBackground(Color{12, 18, 26, 255});

            DrawText("Card Guessing Game", 300, 60, 52, Color{245, 248, 252, 255});
            DrawText("Enhanced Memory Match with Difficulty Levels", 200, 120, 24, Color{159, 196, 225, 255});

            DrawText("Type your name:", 270, 210, 30, Color{228, 235, 241, 255});
            DrawRectangle(500, 200, 250, 50, Color{22, 32, 45, 255});
            DrawRectangleLines(500, 200, 250, 50, Color{72, 112, 160, 255});
            DrawText(typedName.c_str(), 512, 213, 28, Color{246, 248, 250, 255});

            DrawRectangleRounded(startButton, 0.22f, 8, Color{50, 132, 86, 255});
            DrawText("NEXT", 450, 460, 34, Color{250, 252, 255, 255});

            if (trimName(typedName).empty()) {
                DrawText("Enter a name to continue", 360, 550, 24, Color{230, 115, 115, 255});
            } else {
                DrawText("Press ENTER or click NEXT", 340, 550, 24, Color{152, 213, 175, 255});
            }

            EndDrawing();
        }

        if (!WindowShouldClose()) {
            displayDifficultyMenu();
        }

        if (!WindowShouldClose()) {
            // Initialize game with the selected simple configuration
            try {
                gameBoard.initializeBoard(selectedBoardSize <= 0 ? 4 : selectedBoardSize);
                // Initialize simple timer and hint state
                hintsUsed = 0;
                hintActive = false;
                startTime = chrono::steady_clock::now();

                // Main game loop - will use processTurn to drive a single frame/block of turns
                while (!gameBoard.allCardsMatched() && !WindowShouldClose()) {
                    // check time limit
                    if (selectedTimeLimit > 0) {
                        auto elapsed = chrono::duration_cast<chrono::seconds>(
                            chrono::steady_clock::now() - startTime).count();
                        if (static_cast<int>(elapsed) >= selectedTimeLimit) {
                            break; // time expired
                        }
                    }
                    processTurn();
                }
            } catch (const GameException& e) {
                throw;
            }
        }

        if (!WindowShouldClose()) {
            endGame();
        }

        CloseWindow();
    } catch (const GameException& e) {
        // Handle game exceptions
            if (!WindowShouldClose()) {
                CloseWindow();
            }
        throw;
    } catch (const exception& e) {
        if (!WindowShouldClose()) {
            CloseWindow();
        }
        throw GameException(string("Game startup failed: ") + e.what());
    }
}

void Game::processTurn() {
    try {
        UI ui;
        string message = "Select the first card.";

        int firstSelection[2] = {-1, -1};
        int secondSelection[2] = {-1, -1};

        bool resolvingMismatch = false;
        double mismatchStartTime = 0.0;

        while (!WindowShouldClose()) {
            // Check simplified timer
            int elapsedSeconds = static_cast<int>(
                chrono::duration_cast<chrono::seconds>(
                    chrono::steady_clock::now() - startTime).count());
            if (selectedTimeLimit > 0 && elapsedSeconds >= selectedTimeLimit) {
                message = "TIME'S UP! Game Over.";
            }

            int click[2];
            ui.getCardSelection(click);
            const int row = click[0];
            const int col = click[1];

            // Handle hint request space key
            if (IsKeyPressed(KEY_SPACE) && firstSelection[0] == -1 && hintsRemaining > 0) {
                try {
                    // Find first unmatched card and reveal it
                    bool found = false;
                    for (int r = 0; r < gameBoard.getGridSize() && !found; ++r) {
                        for (int c = 0; c < gameBoard.getGridSize() && !found; ++c) {
                            if (!gameBoard.getCard(r, c).getIsMatched()) {
                                gameBoard.flipCard(r, c);
                                hintsRemaining -= 1;
                                hintsUsed += 1;
                                message = "Hint: " + gameBoard.getCard(r, c).getValue() + " (" + to_string(hintsRemaining) + " hints left)";
                                found = true;
                            }
                        }
                    }
                } catch (const GameException& e) {
                    message = "No hints available!";
                }
            }

            if (row != -1 && col != -1 && !resolvingMismatch && !(selectedTimeLimit>0 && elapsedSeconds>=selectedTimeLimit)) {
                try {
                    if (gameBoard.getCard(row, col).getIsMatched()) {
                        message = "That card is already matched. Choose a different one.";
                    } else if (firstSelection[0] == -1) {
                        firstSelection[0] = click[0];
                        firstSelection[1] = click[1];
                        gameBoard.flipCard(row, col);
                        message = "Great. Now select the second card. (SPACE for hint)";
                    } else if (row == firstSelection[0] && col == firstSelection[1]) {
                        message = "You clicked the same card. Choose another one.";
                    } else {
                        secondSelection[0] = click[0];
                        secondSelection[1] = click[1];
                        gameBoard.flipCard(row, col);
                        currentPlayer.makeGuess(firstSelection[0], firstSelection[1], secondSelection[0], secondSelection[1]);

                        Card& firstCard = gameBoard.getCardMutable(firstSelection[0], firstSelection[1]);
                        Card& secondCard = gameBoard.getCardMutable(secondSelection[0], secondSelection[1]);
                        const bool isMatch = gameBoard.checkMatch(firstCard, secondCard);
                        // Scoring: use simplified score system
                        int totalPairs = (gameBoard.getGridSize() * gameBoard.getGridSize()) / 2;
                        scoreSystem.updateScore(isMatch, elapsedSeconds, hintsUsed, currentPlayer.getAttempts(), totalPairs);

                        if (isMatch) {
                            firstCard.match();
                            secondCard.match();
                            message = "Nice! You found a matching pair.";
                            return;
                        }

                        message = "No match. Watch the cards, then they will flip back.";
                        resolvingMismatch = true;
                        mismatchStartTime = GetTime();
                    }
                } catch (const InvalidCardException& e) {
                    message = string("Error: ") + e.what();
                }
            }

            if (resolvingMismatch && (GetTime() - mismatchStartTime) >= 1.1) {
                Card& firstCard = gameBoard.getCardMutable(firstSelection[0], firstSelection[1]);
                Card& secondCard = gameBoard.getCardMutable(secondSelection[0], secondSelection[1]);
                gameBoard.hideCards(firstCard, secondCard);
                hintActive = false;
                return;
            }

            BeginDrawing();
            ClearBackground(Color{11, 18, 27, 255});

            ui.displayMessage(message);
            ui.displayBoard(gameBoard);
            ui.displayScore(scoreSystem.getCurrentScore());
            ui.displayAttempts(currentPlayer.getAttempts());

            // Display timer
            DrawText("Time:", 680, 250, 26, Color{185, 203, 221, 255});
            int remaining = selectedTimeLimit > 0 ? max(0, selectedTimeLimit - elapsedSeconds) : 0;
            ostringstream timeBuf;
            if (selectedTimeLimit > 0) {
                timeBuf << remaining << "s";
            } else {
                timeBuf << elapsedSeconds << "s";
            }
            Color timeColor = (selectedTimeLimit>0 && remaining <= 10) ? Color{224, 157, 157, 255} : Color{147, 206, 169, 255};
            DrawText(timeBuf.str().c_str(), 680, 280, 36, timeColor);

            // Display hints
            ostringstream hintText;
            hintText << "Hints: " << hintsRemaining << " (used: " << hintsUsed << ")";
            DrawText(hintText.str().c_str(), 680, 330, 26, Color{238, 214, 138, 255});
            DrawText("(Press SPACE for hint)", 680, 360, 20, Color{200, 180, 100, 255});

            // Display difficulty
            DrawText("Board:", 680, 410, 20, Color{159, 196, 225, 255});
            ostringstream boardText;
            boardText << selectedBoardSize << " x " << selectedBoardSize;
            DrawText(boardText.str().c_str(), 680, 435, 24, Color{238, 211, 111, 255});

            DrawText("Click cards to flip them", 680, 490, 20, Color{185, 203, 221, 255});
            DrawText("Matched: +10 pts | Wrong: -2 pts", 680, 515, 18, Color{200, 200, 200, 255});

            EndDrawing();
        }
    } catch (const GameException& e) {
        throw;
    }
}

void Game::endGame() {
    try {
        gameState = "finished";
        scoreSystem.saveHighScore(currentPlayer.getName(), scoreSystem.getCurrentScore());

        string topScoreNames[ScoreSystem::MaxHighScores];
        int topScoreValues[ScoreSystem::MaxHighScores];
        const size_t rowsToShow = scoreSystem.getTopScores(topScoreNames, topScoreValues, 8);
        // compute elapsed time
        int elapsedSeconds = static_cast<int>(
            chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - startTime).count());

        while (!WindowShouldClose()) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                break;
            }

            BeginDrawing();
            ClearBackground(Color{10, 16, 24, 255});

            DrawText("Game Complete!", 350, 40, 58, Color{247, 249, 252, 255});

            ostringstream scoreLine;
            scoreLine << "Final Score: " << scoreSystem.getCurrentScore();
            DrawText(scoreLine.str().c_str(), 380, 160, 34, Color{232, 214, 133, 255});

            ostringstream attemptsLine;
            attemptsLine << "Total Attempts: " << currentPlayer.getAttempts();
            DrawText(attemptsLine.str().c_str(), 365, 206, 34, Color{199, 215, 229, 255});

            ostringstream timeLine;
            timeLine << "Time Taken: " << elapsedSeconds << "s";
            DrawText(timeLine.str().c_str(), 380, 252, 28, Color{152, 213, 175, 255});

            ostringstream hintsLine;
            hintsLine << "Hints Used: " << hintsUsed;
            DrawText(hintsLine.str().c_str(), 420, 290, 24, Color{147, 206, 169, 255});

            DrawText("High Scores", 410, 350, 44, Color{157, 200, 236, 255});

            for (size_t i = 0; i < rowsToShow; ++i) {
                ostringstream line;
                line << (i + 1) << ". " << topScoreNames[i] << " - " << topScoreValues[i];
                DrawText(line.str().c_str(), 320, 410 + static_cast<int>(i) * 32, 26, Color{231, 237, 242, 255});
            }

            DrawText("Press ENTER or ESC to close", 350, 700, 26, Color{162, 202, 174, 255});

            EndDrawing();
        }
    } catch (const GameException& e) {
        throw;
    } catch (const exception& e) {
        throw GameException(string("End game error: ") + e.what());
    }
}
