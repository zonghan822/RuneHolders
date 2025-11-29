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

    // Rune System
    uint32_t runeSystemEntityID_;  // ID of entity holding RuneSystemStateComponent
    bool runeSystemInitialized_ = false;

    // Pause Menu and Spellbook System
    bool isPaused_ = false;
    bool showSpellbook_ = false;
    uint32_t pauseMenuEntityID_ = 0;
    uint32_t spellbookEntityID_ = 0;
    Math::eVec2<float> savedPlayerScale_ = { 2.0f, 2.0f };  // Save player scale when hiding (default to common scale)

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
    void setupCamera();         // setup camera system
    void setupCombatSystem();   // Add health/damage to player and enemies
    void playSceneBGM();        // Play appropriate BGM based on current scene

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void loadEntityTextures();  // Load textures for entities with texture paths
    void updateCamera(float deltaTime);  // Update camera position and projection

    void drawPlayerAndIndicator();

    void drawShapes();

    // Rune system methods
    void initializeRuneSystem();                    // Initialize rune system entity
    void updateRuneSystem();                        // Update rune system logic each frame
    void spawnRunes(const Math::eVec2<float>& mouseWorldPos);   // Spawn left and right runes
    void destroyRunes();                            // Destroy spawned runes
    void updateRuneHover(const Math::eVec2<float>& mouseWorldPos);  // Check if mouse hovers over runes
    void drawRuneConnectionLine();                  // Draw line connecting runes
    void spawnFireball(const Math::eVec2<float>& pos);  // Spawn fireball when runes connect

    // Combat system methods
    void updateCombatSystem(float deltaTime);       // Handle damage dealing and health updates

    // Pause Menu and Spellbook methods
    void spawnSpellbook();                          // Spawn spellbook UI entity
    void destroySpellbook();                        // Destroy spellbook UI entity
    void spawnPauseMenu();                          // Spawn pause menu UI entity
    void destroyPauseMenu();                        // Destroy pause menu UI entity
    void hidePlayer();                              // Hide player by setting scale to 0
    void showPlayer();                              // Show player by restoring scale
};
