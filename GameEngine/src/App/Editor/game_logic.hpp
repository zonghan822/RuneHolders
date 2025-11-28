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
#include <set>
#include <imgui.h>
#include "movementTest.hpp"
#include "player/player_config.hpp"
#include "player/player.hpp"
#include "map_editor/map_editor.hpp"


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
    std::unique_ptr<Graphics::EnoFramebuffer> editorFramebuffer_;

    float worldWidth_;
    float worldHeight_;

    // Editor camera settings (zoomed in view)
    static constexpr int EDITOR_WIDTH = 640;   // Base width for zoomed view
    static constexpr int EDITOR_HEIGHT = 360;   // Base height for zoomed view
    float editorCameraX_ = 0.0f;  // Camera offset X
    float editorCameraY_ = 0.0f;  // Camera offset Y
    float editorCameraSpeed_ = 300.0f;  // Camera movement speed
    float editorZoomLevel_ = 1.0f;  // Zoom level (1.0 = normal, >1.0 = zoomed in, <1.0 = zoomed out)

    // Track last framebuffer size to avoid recreating every frame
    int lastEditorFbWidth_ = 0;
    int lastEditorFbHeight_ = 0;

    // Editor view min/max coordinates
    Math::eVec2<float> editorViewMin_{ 0.f, 0.f };
    Math::eVec2<float> editorViewMax_{ 0.f, 0.f };

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
    std::unique_ptr<Player::PlayerSystem> playerSystem_;

    // Trigger tracking (for one-time entry/exit logging)
    std::set<uint32_t> currentTriggersPlayerIsIn_;
    std::set<uint32_t> previousTriggersPlayerWasIn_;

    // Scene Tree (note: must be initialized after engine_ is initialized)
    std::unique_ptr<Editor::EnoSceneTree> sceneTree_;

    // Map Editor (To be removed after set tilemap as node)
    std::unique_ptr<MapEditor> mapEditor_;

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
    float getWorldWidth() const { return worldWidth_; }
    float getWorldHeight() const { return worldHeight_; }
    int getEditorWidth() const { return static_cast<int>(EDITOR_WIDTH / editorZoomLevel_); }
    int getEditorHeight() const { return static_cast<int>(EDITOR_HEIGHT / editorZoomLevel_); }

    // Get editor camera position
    float getEditorCameraX() const { return editorCameraX_; }
    float getEditorCameraY() const { return editorCameraY_; }

    // Zoom control
    void setEditorZoom(float zoom) {
        // Calculate minimum zoom based on world size to prevent viewport exceeding world bounds
        float minZoomX = EDITOR_WIDTH / worldWidth_;
        float minZoomY = EDITOR_HEIGHT / worldHeight_;
        float minZoom = std::max(minZoomX, minZoomY);  // Use the larger to ensure we don't exceed in either dimension
        minZoom = std::max(minZoom, 0.1f);  // Absolute minimum of 0.1x

        editorZoomLevel_ = std::clamp(zoom, minZoom, 4.0f);
    }
    float getEditorZoom() const { return editorZoomLevel_; }
    void adjustEditorZoom(float delta) { setEditorZoom(editorZoomLevel_ + delta); }

    // Get/Set editor view min/max
    void setEditorViewRect(const Math::eVec2<float>& min, const Math::eVec2<float>& max) {
        editorViewMin_ = min;
        editorViewMax_ = max;
    }
    Math::eVec2<float> getEditorViewMin() const { return editorViewMin_; }
    Math::eVec2<float> getEditorViewMax() const { return editorViewMax_; }

    // Camera panning for room transitions
    void startCameraPan(float targetY, float duration = 1.0f);
    bool isCameraPanning() const { return isPanning_; }

    // Get map editor
    MapEditor& getMapEditor() { return *mapEditor_; }


private:
    // Setup (called during initialize())
    void setupPrefabs();        // Prefab instantiation
    void setupGameSystems();    // Register user systems (player, enemy, etc...)
    void setupTreeTesting();    // Editor tree testing code
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
    void setupGameMath();       // math demonstration code
    void setupImGui();          // setup ImGui code
    void setupAnimations();
    void setupScripting();
    void setupFont();           // setup font
    //void createTestObjectsAndAddToSceneTree();  // Create test objects and add them to scene tree

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void updateImGui();         // Update code for imGui
    void loadEntityTextures();  // Load textures for entities with texture paths

    void drawPlayerAndIndicator();

    void drawShapes();
};
