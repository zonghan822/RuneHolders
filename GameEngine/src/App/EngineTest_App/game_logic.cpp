#include "game_logic.hpp"

void GameApplication::initialize() {
    // Setup engine systems first
    engine_.setup();

    // Setup all game content
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
    drawShapes();
}

void GameApplication::updateEntities() {
    // Update game entities each frame
    // Add entity movement, AI, collision detection etc here
}

void GameApplication::updateAudio() {
    // audio update
}

void GameApplication::drawShapes() { // For testing
    auto& graphics = engine_.getGraphicsManager();

    // Draw red rect
    graphics.drawRect(
        Math::eVec2<float>{100.0f, 100.0f},  // Position
        Math::eVec2<float>{200.0f, 150.0f},  // Size 200 wide 150 tall
        Graphics::Color::Red,                 // Colour
        true                                 // Filled
    );

    // Draw green circ
    graphics.drawCircle(
        Math::eVec2<float>{500.0f, 200.0f},  // Center
        75.0f,                               // Radius
        Graphics::Color::Green,              // Colour
        true,                                // Filled
        32                                   // Segments
    );

    // Draw blue tri
    graphics.drawTriangle(
        Math::eVec2<float>{300.0f, 400.0f},  // Bot left
        Math::eVec2<float>{450.0f, 300.0f},  // Top
        Math::eVec2<float>{600.0f, 400.0f},  // Bot right
        Graphics::Color::Blue,               // Colour
        true                                 // Filled
    );
}
