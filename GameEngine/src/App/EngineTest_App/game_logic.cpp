#include "game_logic.hpp"

void GameApplication::initialize() {
    // Setup engine systems first
    engine_.setup();

    // Setup all game content
    setupGameEntities();
    setupGameAudio();
    setupGameEvents();
    setupGameMath();

    // DEBUG: Print the path
    std::cout << "Art folder path: " << engine_.getResourceLoader().getImagePath() << std::endl;

    testTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Animation-Sheet-Attack");

    if (testTexture_)
    {
        std::cout << "Test texture loaded successfully!" << std::endl;
    }
    else
    {
        std::cout << "Warning: Test texture not found. Make sure you have 'test.png' in your art folder." << std::endl;
    }

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

    // Draw red rect (primitive shape test)
    graphics.drawRect(
        Math::eVec2<float>{100.0f, 100.0f},  // Position
        Math::eVec2<float>{200.0f, 150.0f},  // Size 200 wide 150 tall
        Graphics::Color::Red,                 // Colour
        false                                 // Filled
    );

    // Draw green circ (primitive shape test)
    graphics.drawCircle(
        Math::eVec2<float>{500.0f, 200.0f},  // Center
        75.0f,                               // Radius
        Graphics::Color::Green,              // Colour
        false,                                // Filled
        32                                   // Segments
    );

    // Draw blue tri (primitive shape test)
    graphics.drawTriangle(
        Math::eVec2<float>{300.0f, 400.0f},  // Bot left
        Math::eVec2<float>{450.0f, 300.0f},  // Top
        Math::eVec2<float>{600.0f, 400.0f},  // Bot right
        Graphics::Color::Blue,               // Colour
        false                                // Filled
    );

    // TEXTURE TEST: Draw a textured rectangle
    if (testTexture_ && testTexture_->isValid())
    {
        graphics.drawTexturedQuad(
            Math::eVec2<float>{400.0f, 100.0f},  // Position
            Math::eVec2<float>{150.0f, 150.0f},  // Size (width x height)
            testTexture_,
            Graphics::Color::White                // White = no tint, shows original colors
        );

        
        graphics.drawTexturedQuad(
            Math::eVec2<float>{600.0f, 100.0f},  // Different position
            Math::eVec2<float>{150.0f, 150.0f},
            testTexture_,
            Graphics::Color::Red                  // Red tint
        );

        graphics.drawTexturedQuad(
            Math::eVec2<float>{400.0f, 300.0f},  // Different position
            Math::eVec2<float>{150.0f, 150.0f},
            testTexture_,
            Graphics::Color::Green                // Green tint
        );
    }
}