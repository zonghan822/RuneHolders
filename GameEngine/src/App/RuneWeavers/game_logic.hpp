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

    // Interrupt handling
    bool windowFocused_ = true;
    bool windowMinimized_ = false;

    // Texture and anim loading
    std::shared_ptr<Graphics::EnoTexture> playerTexture_;
    std::shared_ptr<Graphics::EnoTexture> backgroundTexture_;
    std::shared_ptr<Graphics::EnoTexture> playerWalkTexture_;
    Animation::AnimationController playerAnimController_;
    //Animation::AnimationController enemyAnimController_;
    int playerFacingDirection_ = 1;

    std::unique_ptr<Graphics::EnoFramebuffer> gameFramebuffer_;

    float worldWidth_ = 0.f;
    float worldHeight_ = 0.f;

    // Font
    std::shared_ptr<Font::EnoFont> pixelifyFont_;
    std::shared_ptr<Font::EnoFont> timesFont_;
    std::shared_ptr<Font::EnoFont> philosopherBoldFont;
    std::shared_ptr<Font::EnoFont> philosopherRegularFont;

    // Player
    Player::PlayerConfig playerConfig_;
    std::unique_ptr<Player::PlayerSystem> playerSystem_;
    uint32_t playerEntityID_ = 0;
    bool playerSpawned_ = false;

    // Debug/Display
    bool showColliders_ = true;
    std::unordered_map<std::string, double> systemTimes_;

    // Rune System
    uint32_t runeSystemEntityID_ = 0;  // ID of entity holding RuneSystemStateComponent
    bool runeSystemInitialized_ = false;
    std::shared_ptr<Graphics::EnoTexture> runeTexture_;  // Cached rune spritesheet texture

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
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
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
    void updateCamera();  // Update camera position and projection

    void drawPlayerAndIndicator();

    void drawShapes();
    void displayFPS();

    // Rune system methods
    void initializeRuneSystem();                                    // Initialize rune system entity
    void updateRuneSystem();                                        // Update rune system logic each frame
    void spawnRunes(const Math::eVec2<float>& mouseWorldPos);       // Spawn all 6 runes in circle
    void destroyRunes();                                            // Destroy spawned runes
    void updateRuneHover(const Math::eVec2<float>& mouseWorldPos);  // Check if mouse hovers over runes
    void drawRuneConnectionLine();                                  // Draw line connecting runes
    void spawnFireball(const Math::eVec2<float>& pos);              // Spawn fireball when runes connect
    void spawnFloatingText(const Math::eVec2<float>& pos, const char* text);  // Spawn floating text message
    void updateFloatingTexts(float deltaTime);                      // Update floating text positions and alpha
    void spawnRuptureMark(const Math::eVec2<float>& pos, ECSComponents::SpellType spellType);  // Spawn rupture mark spell
    void updateRuptureMarks(float deltaTime);                       // Update rupture mark state machines

    // Combat system methods
    void updateCombatSystem(float deltaTime);       // Handle damage dealing and health updates

    // AI system methods
    void updateEnemyAI(float deltaTime);            // Update enemy AI movement and pathfinding

    // Pause Menu and Spellbook methods
    void spawnSpellbook();                          // Spawn spellbook UI entity
    void destroySpellbook();                        // Destroy spellbook UI entity
    void spawnPauseMenu();                          // Spawn pause menu UI entity
    void destroyPauseMenu();                        // Destroy pause menu UI entity
    void hidePlayer();                              // Hide player by setting scale to 0
    void showPlayer();                              // Show player by restoring scale

    // Main Menu Button System
    void setupMainMenuButtons();                    // Setup invisible buttons for main menu
    void updateMainMenuButtons();                   // Update button interactions
    void handlePlayButton();                        // Handle play button click
    void handleControlsButton();                    // Handle controls button click
    void handleQuitButton();                        // Handle quit button click
    void handleControlsCloseButton();               // Handle controls close button click
    void handleQuitNoButton();                      // Handle quit no button click
    void handleQuitYesButton();                     // Handle quit yes button click

    // UI entities for controls and quit confirm menus
    uint32_t controlsDisplayEntityID_ = 0;
    uint32_t quitConfirmEntityID_ = 0;

    // Scene Transition System
    void setupSceneTransitions();                   // Setup scene transition triggers
    void updateSceneTransitions();                  // Check for scene transition collisions
    uint32_t scene1ToScene2TriggerID_ = 0;         // Scene transition trigger entity ID

    // Pause Menu Button System
    void setupPauseMenuButtons();                   // Setup pause menu buttons (called when spawning pause menu)
    void handleResumeButton();                      // Handle resume button click
    void handleMainMenuButton();                    // Handle main menu button click
    uint32_t resumeButtonEntityID_ = 0;
    uint32_t mainMenuButtonEntityID_ = 0;

    bool isDisplayFPS_ = false;
};
