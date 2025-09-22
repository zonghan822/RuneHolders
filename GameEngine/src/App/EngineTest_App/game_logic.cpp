#include "game_logic.hpp"
#include "math/math.hpp"

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
    std::cout << "Calling math function\n";
    Math::eVec2 vec{ 1.1f, 1.2f };
    Math::eVec2 vec2{ 1.3f, 2.f };
    vec += vec2;
    std::cout << "vector : " << vec.v1 << ", " << vec.v2 << std::endl;


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