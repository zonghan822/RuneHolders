#pragma once

#include "engine/eno_engine.hpp"
#include <iostream>

class GameApplication {
private:
    // Engine instance
    Engine::EnoEngine engine_;

    // Game state variables
    bool gameRunning_ = true;
    bool musicStarted_ = false;

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

private:
    // Setup (called during initialize())
    void setupGameEntities();   // ECS code
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
    void setupGameMath();       // math demonstration code

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void updateAudio();         // Update audio per frame

    // Demonstration methods
    void demonstrateSignals();
};