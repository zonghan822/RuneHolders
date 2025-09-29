#pragma once

#include "engine/eno_engine.hpp"
#include "player/player.hpp"
#include <iostream>
#include <memory>  


//yunqi testing
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h> 
#include <chrono>

class GameApplication {
private:
    // Engine instance
    Engine::EnoEngine engine_;

    // Game state variables
    bool gameRunning_ = true;
    bool musicStarted_ = false;


    // indicator control variables
    float indicatorX_ = 600.0f;
    float indicatorY_ = 425.0f;
    float moveSpeed_ = 200.0f; // pixels per second

    //collision system
    std::unique_ptr<Collision::CollisionSystem> collisionSystem_;
    std::unique_ptr<Player::PlayerSystem> playerSystem_;   
    bool showColliders_ = true;

    // player entity ID (to track the player)
    uint32_t playerEntityID_ = 0;
    bool playerSpawned_ = false;


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
    void setupImGui();

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void updateAudio();         // Update audio per frame
    void updateImGui();

    void drawShapes(); // jsut to test

    // Demonstration methods
    void demonstrateSignals();
};