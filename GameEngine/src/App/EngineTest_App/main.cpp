#include "game_logic.hpp"

int main() {
    try {
        // Create game instance
        GameApplication game;

        // Initialize all game systems
        game.initialize();

        // Run the game
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Game error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}