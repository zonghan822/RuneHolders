#include "game_logic.hpp"

void GameApplication::initialize() {
    // Setup engine systems first
    engine_.setup();

    // Setup all your game content (moved from your current setup() method)
    setupGameEntities();
    setupGameAudio();
    setupGameEvents();
    setupGameMath();

    std::cout << "Game initialized successfully!" << std::endl;
}

void GameApplication::run() {
    std::cout << "Starting game..." << std::endl;

    // Start the engine with game update callback
    engine_.run([this]() {
        updateGameLogic();  // This gets called every frame
        });
}

void GameApplication::setupGameEntities() {
    // To setup the entities in our game
}

void GameApplication::setupGameAudio() {
    // Setup audio
}

void GameApplication::setupGameEvents() {
    // setup event bus
}

void GameApplication::setupGameMath() {
    // math lib and maybe collision
}

void GameApplication::demonstrateSignals() {
    // signalling code
}

void GameApplication::updateGameLogic() {
    // This is called every frame (replaces your while loop in eno_engine.cpp run())
    updateEntities();
    updateAudio();

    // Add any other per-frame game logic here
}

void GameApplication::updateEntities() {
    // Update your game entities each frame
    // Add entity movement, AI, collision detection, etc. here
}

void GameApplication::updateAudio() {
    // audio update
}