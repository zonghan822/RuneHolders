#include "game_logic.hpp"
#include <GLFW/glfw3.h>

void GameApplication::initialize() {
    // Setup engine systems first
    engine_.setup();

    // Setup all game content
    setupGameEntities();
    setupGameAudio();
    setupGameEvents();
    setupGameMath();
    setupAnimations();

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

void GameApplication::setupAnimations() {
    // Load the sprite sheet texture
    testTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Idle-Sheet");

    if (!testTexture_)
    {
        std::cout << "Warning: Could not load animation sprite sheet!" << std::endl;
        return;
    }

    // Create attack animation
    // Adjust these values based on YOUR sprite sheet:
    // - Frame size: 89x89 pixels (your frames appear to be square based on height)
    // - Sheet size: 502x89 pixels
    // - Start frame: 0
    // - Frame count: 5 (502 / 89 = 5.6, so probably 5 frames)
    // - Duration: 0.1 seconds per frame

    auto attackAnim = Animation::createSpriteSheetAnimation(
        96,          // Frame width
        96,          // Frame height
        2304,         // Sheet width
        96,          // Sheet height
        0,           // Start frame
        24,           // Number of frames
        0.1f         // Duration per frame (seconds)
    );

    attackAnim.setLooping(true);
    animController_.addAnimation("attack", attackAnim);

    // Set the initial animation
    animController_.setAnimation("attack");

    std::cout << "Animation setup complete!" << std::endl;
}

void GameApplication::demonstrateSignals() {
    // signalling code
}

void GameApplication::updateGameLogic() {
    // This is called every frame (replaces your while loop in eno_engine.cpp run())

    // Calculate delta time
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastFrameTime_;
    lastFrameTime_ = currentTime;

    // Update animation with delta time
    if (testTexture_)
    {
        animController_.update(deltaTime);
    }

    updateEntities();
    updateAudio();

    // Add any other per-frame game logic here
    drawShapes();
    drawAnimatedSprite();
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

    // Draw rotated red rect (primitive shape test)
    graphics.drawRect(
        Math::eVec2<float>{100.0f, 100.0f},  // Position
        Math::eVec2<float>{200.0f, 150.0f},  // Size 200 wide 150 tall
        Graphics::Color::Red,                 // Colour
        true,                                 // Filled
        M_PI/2.f                             // Rotation in PI
    );
    graphics.drawRect(
        Math::eVec2<float>{100.0f, 300.0f},  // Position
        Math::eVec2<float>{200.0f, 150.0f},  // Size 200 wide 150 tall
        Graphics::Color::Red,                 // Colour
        true                                 // Filled
    );

    // Draw green circ (primitive shape test)
    //graphics.drawCircle(
    //    Math::eVec2<float>{500.0f, 200.0f},  // Center
    //    75.0f,                               // Radius
    //    Graphics::Color::Green,              // Colour
    //    false,                                // Filled
    //    32                                   // Segments
    //);

    //// Draw blue tri (primitive shape test)
    //graphics.drawTriangle(
    //    Math::eVec2<float>{300.0f, 400.0f},  // Bot left
    //    Math::eVec2<float>{450.0f, 300.0f},  // Top
    //    Math::eVec2<float>{600.0f, 400.0f},  // Bot right
    //    Graphics::Color::Blue,               // Colour
    //    false                                // Filled
    //);

    // TEXTURE TEST: Draw a textured rectangle
    if (testTexture_ && testTexture_->isValid())
    {
        graphics.drawTexturedQuad(
            Math::eVec2<float>{400.0f, 100.0f},  // Position
            Math::eVec2<float>{150.0f, 150.0f},  // Size (width x height)
            testTexture_,
            Graphics::Color::White                // White = no tint, shows original colors
        );

        
        //graphics.drawTexturedQuad(
        //    Math::eVec2<float>{600.0f, 100.0f},  // Different position
        //    Math::eVec2<float>{150.0f, 150.0f},
        //    testTexture_,
        //    Graphics::Color::Red                  // Red tint
        //);

        //graphics.drawTexturedQuad(
        //    Math::eVec2<float>{400.0f, 300.0f},  // Different position
        //    Math::eVec2<float>{150.0f, 150.0f},
        //    testTexture_,
        //    Graphics::Color::Green                // Green tint
        //);
    }
}

void GameApplication::drawAnimatedSprite() {
    if (!testTexture_ || !testTexture_->isValid())
        return;

    auto& graphics = engine_.getGraphicsManager();

    // Get the current frame from the animation
    auto frame = animController_.getCurrentFrame();

    // CRITICAL: Round to whole pixels to prevent sub-pixel jittering
    float xPos = std::floor(400.0f);
    float yPos = std::floor(300.0f);

    // Draw the animated sprite
    graphics.drawTexturedQuadUV(
        Math::eVec2<float>{xPos, yPos},
        Math::eVec2<float>{192.0f, 192.0f},  // Size (2x scale for visibility)
        testTexture_,
        frame.uvStart,  // UV start from current frame
        frame.uvEnd,    // UV end from current frame
        Graphics::Color::White
    );
}