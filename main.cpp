#include "CardGame.h"
#include "GameException.h"
#include <iostream>

int main() {
    try {
        Game game;
        game.startGame();
    } catch (const GameException& e) {
        std::cerr << "Game Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}