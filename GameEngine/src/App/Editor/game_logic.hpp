/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         game_logic.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         06-11-2025
* @brief        Game application layer that manages game-specific logic, rendering, and entity
*               management. Systems are now managed by EnoEngine for better architecture.
*//*========================================================================================*/

#pragma once

#include "engine/eno_engine.hpp"
#include <iostream>
#include <memory>
#include <imgui.h>
#include "movementTest.hpp"
#include "player/player_config.hpp"
#include "player/player.hpp"
#include "enemy/chasing_state.hpp"
#include "enemy/idle_state.hpp"

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

    std::unique_ptr<Graphics::EnoFramebuffer> gameFramebuffer_;
    std::unique_ptr<Graphics::EnoFramebuffer> editorFramebuffer_;

    // Fixed virtual resolution (game always renders to this size)
    static constexpr int VIRTUAL_WIDTH = 1280;
    static constexpr int VIRTUAL_HEIGHT = 720;

    // Editor camera settings (zoomed in view)
    static constexpr int EDITOR_WIDTH = 640;   // Half width for zoomed view
    static constexpr int EDITOR_HEIGHT = 360;   // Half height for zoomed view
    float editorCameraX_ = 0.0f;  // Camera offset X
    float editorCameraY_ = 0.0f;  // Camera offset Y
    float editorCameraSpeed_ = 300.0f;  // Camera movement speed

    // Camera panning state
    bool isPanning_ = false;
    float panStartY_ = 0.0f;
    float panTargetY_ = 0.0f;
    float panDuration_ = 1.0f;
    float panTimer_ = 0.0f;

    // Font
    std::shared_ptr<Font::EnoFont> pixelifyFont_;
    std::shared_ptr<Font::EnoFont> timesFont_;

    // Player
    Player::PlayerConfig playerConfig_;

    // NOTE: Systems (Physics, Collision, Player) are now managed by EnoEngine
    // Access them via engine_.getPhysicsSystem(), engine_.getCollisionSystem(), engine_.getPlayerSystem()

    // Scene Tree (note: must be initialized after engine_ is initialized)
    std::unique_ptr<Editor::EnoSceneTree> sceneTree_;

    // Pathfinding system
    std::unique_ptr<AI::PathfindingSystem> pathfinder_;

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
    void shutdown();    // Called when application closed

    void markEntitiesDirty() { entitiesDirty_ = true; }

    // Yun Qi's IMGUI sandbox
    //void setPlayerTexture(std::shared_ptr<Graphics::EnoTexture> newTex);
    void applyTextureToPlayer(const Graphics::TextureData& texData);


    Graphics::EnoFramebuffer* getGameFramebuffer() { return gameFramebuffer_.get(); }
    Graphics::EnoFramebuffer* getEditorFramebuffer() { return editorFramebuffer_.get(); }

    // Get fixed virtual resolution
    int getVirtualWidth() const { return VIRTUAL_WIDTH; }
    int getVirtualHeight() const { return VIRTUAL_HEIGHT; }
    int getEditorWidth() const { return EDITOR_WIDTH; }
    int getEditorHeight() const { return EDITOR_HEIGHT; }

    // Get editor camera position
    float getEditorCameraX() const { return editorCameraX_; }
    float getEditorCameraY() const { return editorCameraY_; }

    // Get pathfinding system
    AI::PathfindingSystem& getPathfinder() { return *pathfinder_; }
    // Camera panning for room transitions
    void startCameraPan(float targetY, float duration = 1.0f);
    bool isCameraPanning() const { return isPanning_; }


private:
    // Setup (called during initialize())
    void setupGameEntities();   // ECS code
    void setupTreeTesting();    // Editor tree testing code
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
    void setupGameMath();       // math demonstration code
    void setupImGui();          // setup ImGui code
    void setupAnimations();
    void setupScripting();
    void createTestObjectsAndAddToSceneTree();  // Create test objects and add them to scene tree

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void updateImGui();         // Update code for imGui
    void loadEntityTextures();  // Load textures for entities with texture paths

    void drawPlayerAndIndicator();

    void drawShapes(); 

    void drawText(Math::eVec2<float> windowSize);
};
