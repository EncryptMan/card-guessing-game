#include "CardGame.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <random>
#include <sstream>

#include "raylib.h"

namespace {
const int kWindowWidth = 1000;
const int kWindowHeight = 760;
const int kBoardLeft = 60;
const int kBoardTop = 130;
const int kBoardAreaSize = 560;
const int kCardGap = 10;

const GameBoard* gLastBoardDrawn = nullptr;

Rectangle getCardRectangle(int row, int col, int gridSize) {
    const int cardSize = (kBoardAreaSize - (gridSize + 1) * kCardGap) / gridSize;
    const float x = static_cast<float>(kBoardLeft + kCardGap + col * (cardSize + kCardGap));
    const float y = static_cast<float>(kBoardTop + kCardGap + row * (cardSize + kCardGap));
    return Rectangle{x, y, static_cast<float>(cardSize), static_cast<float>(cardSize)};
}

std::string trimName(const std::string& rawName) {
    std::string clean;
    for (char ch : rawName) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            clean.push_back(ch);
        }
    }
    return clean;
}
} // namespace

Card::Card() : value(""), isFlipped(false), isMatched(false) {}

Card::Card(const std::string& cardValue) : value(cardValue), isFlipped(false), isMatched(false) {}

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
    isMatched = true;
    isFlipped = true;
}

GameBoard::GameBoard() : gridSize(4) {}

void GameBoard::initializeBoard(int size) {
    gridSize = size;
    cards.assign(gridSize, std::vector<Card>(gridSize));

    std::vector<std::string> values;
    const int totalCards = gridSize * gridSize;

    for (int i = 0; i < totalCards / 2; ++i) {
        std::string label;
        if (i < 26) {
            label = std::string(1, static_cast<char>('A' + i));
        } else {
            label = "C" + std::to_string(i - 25);
        }
        values.push_back(label);
        values.push_back(label);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(values.begin(), values.end(), gen);

    int index = 0;
    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            cards[row][col] = Card(values[index]);
            index++;
        }
    }
}

Card GameBoard::flipCard(int row, int col) {
    cards[row][col].flip();
    return cards[row][col];
}

bool GameBoard::checkMatch(const Card& card1, const Card& card2) {
    return card1.value == card2.value;
}

void GameBoard::hideCards(Card& card1, Card& card2) {
    card1.hide();
    card2.hide();
}

bool GameBoard::allCardsMatched() {
    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            if (!cards[row][col].isMatched) {
                return false;
            }
        }
    }
    return true;
}

Player::Player() : name("Player"), attempts(0) {}

void Player::makeGuess(int row1, int col1, int row2, int col2) {
    (void)row1;
    (void)col1;
    (void)row2;
    (void)col2;
    attempts++;
}

ScoreSystem::ScoreSystem() : currentScore(0) {}

void ScoreSystem::updateScore(bool match) {
    if (match) {
        currentScore += 10;
    } else {
        currentScore = std::max(0, currentScore - 2);
    }
}

void ScoreSystem::saveHighScore(const std::string& playerName, int score) {
    const std::string cleanedName = trimName(playerName);
    const std::string nameToStore = cleanedName.empty() ? "Player" : cleanedName;

    std::map<std::string, int>::iterator found = highScores.find(nameToStore);
    if (found == highScores.end() || score > found->second) {
        highScores[nameToStore] = score;
    }

    std::ofstream outFile("highscores.txt");
    if (!outFile.is_open()) {
        return;
    }

    for (std::map<std::string, int>::const_iterator it = highScores.begin(); it != highScores.end(); ++it) {
        outFile << it->first << " " << it->second << "\n";
    }
}

std::map<std::string, int> ScoreSystem::loadHighScores() {
    highScores.clear();

    std::ifstream inFile("highscores.txt");
    if (!inFile.is_open()) {
        return highScores;
    }

    std::string name;
    int score = 0;
    while (inFile >> name >> score) {
        std::map<std::string, int>::iterator found = highScores.find(name);
        if (found == highScores.end() || score > found->second) {
            highScores[name] = score;
        }
    }

    return highScores;
}

void UI::displayBoard(const GameBoard& board) {
    gLastBoardDrawn = &board;

    DrawRectangle(kBoardLeft - 8, kBoardTop - 8, kBoardAreaSize + 16, kBoardAreaSize + 16, Color{35, 42, 53, 255});
    DrawRectangle(kBoardLeft, kBoardTop, kBoardAreaSize, kBoardAreaSize, Color{16, 22, 30, 255});

    for (int row = 0; row < board.gridSize; ++row) {
        for (int col = 0; col < board.gridSize; ++col) {
            const Card& card = board.cards[row][col];
            const Rectangle rect = getCardRectangle(row, col, board.gridSize);

            Color fillColor = Color{64, 96, 160, 255};
            if (card.isMatched) {
                fillColor = Color{56, 153, 112, 255};
            } else if (card.isFlipped) {
                fillColor = Color{228, 206, 116, 255};
            }

            DrawRectangleRounded(rect, 0.16f, 8, fillColor);
            DrawRectangleRoundedLinesEx(rect, 0.16f, 8, 2.0f, Color{15, 19, 26, 255});

            if (card.isFlipped || card.isMatched) {
                const int fontSize = 30;
                const int textWidth = MeasureText(card.value.c_str(), fontSize);
                const int textX = static_cast<int>(rect.x + rect.width / 2.0f) - textWidth / 2;
                const int textY = static_cast<int>(rect.y + rect.height / 2.0f) - fontSize / 2;
                DrawText(card.value.c_str(), textX, textY, fontSize, Color{20, 24, 32, 255});
            }
        }
    }
}

void UI::displayScore(int score) {
    std::ostringstream stream;
    stream << "Score: " << score;
    DrawText(stream.str().c_str(), 680, 180, 28, Color{230, 232, 235, 255});
}

void UI::displayAttempts(int attempts) {
    std::ostringstream stream;
    stream << "Attempts: " << attempts;
    DrawText(stream.str().c_str(), 680, 220, 28, Color{230, 232, 235, 255});
}

std::vector<int> UI::getCardSelection() {
    if (gLastBoardDrawn == nullptr) {
        return std::vector<int>{-1, -1};
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return std::vector<int>{-1, -1};
    }

    const Vector2 mouse = GetMousePosition();
    for (int row = 0; row < gLastBoardDrawn->gridSize; ++row) {
        for (int col = 0; col < gLastBoardDrawn->gridSize; ++col) {
            const Rectangle rect = getCardRectangle(row, col, gLastBoardDrawn->gridSize);
            if (CheckCollisionPointRec(mouse, rect)) {
                return std::vector<int>{row, col};
            }
        }
    }

    return std::vector<int>{-1, -1};
}

void UI::displayMessage(const std::string& message) {
    DrawRectangle(45, 40, 910, 62, Color{19, 26, 37, 255});
    DrawRectangleLinesEx(Rectangle{45, 40, 910, 62}, 2.0f, Color{46, 77, 120, 255});
    DrawText(message.c_str(), 58, 58, 24, Color{238, 242, 245, 255});
}

Game::Game() : gameState("idle") {}

void Game::startGame() {
    gameState = "running";
    scoreSystem.loadHighScores();

    InitWindow(kWindowWidth, kWindowHeight, "Card Guessing Game - raylib");
    SetTargetFPS(60);

    int selectedSize = 4;
    std::string typedName;

    while (!WindowShouldClose()) {
        const int key = GetCharPressed();
        if (key >= 32 && key <= 126 && typedName.size() < 12) {
            typedName.push_back(static_cast<char>(key));
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !typedName.empty()) {
            typedName.pop_back();
        }

        if (IsKeyPressed(KEY_TWO)) {
            selectedSize = 2;
        }
        if (IsKeyPressed(KEY_FOUR)) {
            selectedSize = 4;
        }
        if (IsKeyPressed(KEY_SIX)) {
            selectedSize = 6;
        }

        const Rectangle startButton = Rectangle{380, 560, 240, 70};
        const bool clickStart = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                                CheckCollisionPointRec(GetMousePosition(), startButton);
        const bool keyStart = IsKeyPressed(KEY_ENTER);

        if ((clickStart || keyStart) && !trimName(typedName).empty()) {
            currentPlayer.name = trimName(typedName);
            gameBoard.initializeBoard(selectedSize);
            break;
        }

        BeginDrawing();
        ClearBackground(Color{12, 18, 26, 255});

        DrawText("Card Guessing Game", 300, 120, 52, Color{245, 248, 252, 255});
        DrawText("Beginner Friendly Memory Match", 325, 176, 24, Color{159, 196, 225, 255});

        DrawText("Type your name:", 270, 285, 30, Color{228, 235, 241, 255});
        DrawRectangle(500, 275, 250, 50, Color{22, 32, 45, 255});
        DrawRectangleLines(500, 275, 250, 50, Color{72, 112, 160, 255});
        DrawText(typedName.c_str(), 512, 288, 28, Color{246, 248, 250, 255});

        DrawText("Choose board size: press 2, 4, or 6", 270, 385, 30, Color{228, 235, 241, 255});
        std::ostringstream sizeText;
        sizeText << "Selected: " << selectedSize << " x " << selectedSize;
        DrawText(sizeText.str().c_str(), 360, 430, 36, Color{238, 211, 111, 255});

        DrawRectangleRounded(startButton, 0.22f, 8, Color{50, 132, 86, 255});
        DrawText("START", 455, 582, 34, Color{250, 252, 255, 255});

        if (trimName(typedName).empty()) {
            DrawText("Enter a name to start", 390, 650, 24, Color{230, 115, 115, 255});
        } else {
            DrawText("Press ENTER or click START", 350, 650, 24, Color{152, 213, 175, 255});
        }

        EndDrawing();
    }

    if (!WindowShouldClose()) {
        while (!gameBoard.allCardsMatched() && !WindowShouldClose()) {
            processTurn();
        }
    }

    if (!WindowShouldClose()) {
        endGame();
    }

    CloseWindow();
}

void Game::processTurn() {
    UI ui;
    std::string message = "Select the first card.";

    std::vector<int> firstSelection{-1, -1};
    std::vector<int> secondSelection{-1, -1};

    bool resolvingMismatch = false;
    double mismatchStartTime = 0.0;

    while (!WindowShouldClose()) {
        std::vector<int> click = ui.getCardSelection();
        const int row = click[0];
        const int col = click[1];

        if (row != -1 && col != -1 && !resolvingMismatch) {
            if (gameBoard.cards[row][col].isMatched) {
                message = "That card is already matched. Choose a different one.";
            } else if (firstSelection[0] == -1) {
                firstSelection = click;
                gameBoard.flipCard(row, col);
                message = "Great. Now select the second card.";
            } else if (row == firstSelection[0] && col == firstSelection[1]) {
                message = "You clicked the same card. Choose another one.";
            } else {
                secondSelection = click;
                gameBoard.flipCard(row, col);
                currentPlayer.makeGuess(firstSelection[0], firstSelection[1], secondSelection[0], secondSelection[1]);

                Card& firstCard = gameBoard.cards[firstSelection[0]][firstSelection[1]];
                Card& secondCard = gameBoard.cards[secondSelection[0]][secondSelection[1]];
                const bool isMatch = gameBoard.checkMatch(firstCard, secondCard);
                scoreSystem.updateScore(isMatch);

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
        }

        if (resolvingMismatch && (GetTime() - mismatchStartTime) >= 1.1) {
            Card& firstCard = gameBoard.cards[firstSelection[0]][firstSelection[1]];
            Card& secondCard = gameBoard.cards[secondSelection[0]][secondSelection[1]];
            gameBoard.hideCards(firstCard, secondCard);
            return;
        }

        BeginDrawing();
        ClearBackground(Color{11, 18, 27, 255});

        ui.displayMessage(message);
        ui.displayBoard(gameBoard);
        ui.displayScore(scoreSystem.currentScore);
        ui.displayAttempts(currentPlayer.attempts);

        DrawText("Click cards to flip them", 680, 290, 26, Color{185, 203, 221, 255});
        DrawText("Matched cards stay green", 680, 328, 24, Color{147, 206, 169, 255});
        DrawText("Wrong guesses: -2 points", 680, 362, 24, Color{224, 157, 157, 255});
        DrawText("Correct guesses: +10 points", 680, 396, 24, Color{238, 214, 138, 255});

        EndDrawing();
    }
}

void Game::endGame() {
    gameState = "finished";
    scoreSystem.saveHighScore(currentPlayer.name, scoreSystem.currentScore);

    std::vector<std::pair<std::string, int>> sortedScores;
    for (std::map<std::string, int>::const_iterator it = scoreSystem.highScores.begin(); it != scoreSystem.highScores.end(); ++it) {
        sortedScores.push_back(*it);
    }

    std::sort(sortedScores.begin(), sortedScores.end(),
              [](const std::pair<std::string, int>& left, const std::pair<std::string, int>& right) {
                  if (left.second == right.second) {
                      return left.first < right.first;
                  }
                  return left.second > right.second;
              });

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            break;
        }

        BeginDrawing();
        ClearBackground(Color{10, 16, 24, 255});

        DrawText("Game Complete!", 350, 80, 58, Color{247, 249, 252, 255});

        std::ostringstream scoreLine;
        scoreLine << "Final Score: " << scoreSystem.currentScore;
        DrawText(scoreLine.str().c_str(), 380, 190, 34, Color{232, 214, 133, 255});

        std::ostringstream attemptsLine;
        attemptsLine << "Total Attempts: " << currentPlayer.attempts;
        DrawText(attemptsLine.str().c_str(), 365, 236, 34, Color{199, 215, 229, 255});

        DrawText("High Scores", 410, 320, 44, Color{157, 200, 236, 255});

        const int rowsToShow = std::min(static_cast<int>(sortedScores.size()), 8);
        for (int i = 0; i < rowsToShow; ++i) {
            std::ostringstream line;
            line << (i + 1) << ". " << sortedScores[i].first << " - " << sortedScores[i].second;
            DrawText(line.str().c_str(), 320, 380 + i * 38, 30, Color{231, 237, 242, 255});
        }

        DrawText("Press ENTER or ESC to close", 350, 700, 26, Color{162, 202, 174, 255});

        EndDrawing();
    }
}
