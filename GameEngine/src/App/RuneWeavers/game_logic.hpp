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
#include <unordered_map>
#include "movementTest.hpp"
#include "player/player_config.hpp"
#include "player/player.hpp"


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
    //Animation::AnimationController enemyAnimController_;
    int playerFacingDirection_ = 1;

    std::unique_ptr<Graphics::EnoFramebuffer> gameFramebuffer_;

    float worldWidth_;
    float worldHeight_;

    // Font
    std::shared_ptr<Font::EnoFont> pixelifyFont_;
    std::shared_ptr<Font::EnoFont> timesFont_;

    // Player
    Player::PlayerConfig playerConfig_;
    std::unique_ptr<Player::PlayerSystem> playerSystem_;
    uint32_t playerEntityID_ = 0;
    bool playerSpawned_ = false;

    // Debug/Display
    bool showColliders_ = true;
    std::unordered_map<std::string, double> systemTimes_;

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

    // Get fixed virtual resolution
    float getWorldWidth() const { return worldWidth_; }
    float getWorldHeight() const { return worldHeight_; }


private:
    // Setup (called during initialize())
    void setupPrefabs();        // Prefab instantiation
    void setupGameSystems();    // Register user systems (player, enemy, etc...)
    void setupTreeTesting();    // Editor tree testing code
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
    void setupGameMath();       // math demonstration code
    void setupAnimations();
    void setupScripting();
    void setupFont();           // setup font

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void loadEntityTextures();  // Load textures for entities with texture paths

    void drawPlayerAndIndicator();

    void drawShapes();
};
