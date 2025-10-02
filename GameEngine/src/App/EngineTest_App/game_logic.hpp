#pragma once

#include "engine/eno_engine.hpp"
#include "../../Engine/resources/animation.hpp"
#include <iostream>
#include <memory>  
#include "movementTest.hpp"

namespace {
    //scene selection
    enum SceneSelect
    {
        PLAYER_SCENE = 1,
        DYANMIC_COL_SCENE

    } currScene;

}


class GameApplication {
private:
    // Engine instance
    Engine::EnoEngine engine_;

    // Game state variables
    bool gameRunning_ = true;
    bool musicStarted_ = false;

    // For delta time calculation
    float lastFrameTime_ = 0.0f;  

    // Caching
    std::vector<ECS::Entity> cachedDrawableEntities_;
    std::vector<ECS::Entity> cachedSceneTwoEntities_;
    bool entitiesDirty_ = true;

    // Texture and anim loading
    std::shared_ptr<Graphics::EnoTexture> playerTexture_;
    std::shared_ptr<Graphics::EnoTexture> backgroundTexture_;
    std::shared_ptr<Graphics::EnoTexture> playerWalkTexture_;
    Animation::AnimationController playerAnimController_;
    int playerFacingDirection_ = 1;

public:
    // Constructors
    GameApplication() = default;
    GameApplication(const GameApplication&) = delete;            // Disable copy
    GameApplication& operator=(const GameApplication&) = delete; // Disable copy assignment
    GameApplication(GameApplication&&) = delete;                 // Disable move
    GameApplication& operator=(GameApplication&&) = delete;      // Disable move assignment

    // Main game methods
    void initialize();  // Called once at start
    void run();         // Starts the game loop
    void markEntitiesDirty() { entitiesDirty_ = true; }


private:
    // Setup (called during initialize())
    void setupGameEntities();   // ECS code
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
    void setupGameMath();       // math demonstration code
    void setupImGui();          // setup ImGui code
    void setupAnimations();

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void updateAudio();         // Update audio per frame
    void updateImGui();         // Update code for imGui

    void drawPlayerAndIndicator();

    void drawShapes(); // jsut to test

    // Demonstration methods
    void demonstrateSignals();
};
