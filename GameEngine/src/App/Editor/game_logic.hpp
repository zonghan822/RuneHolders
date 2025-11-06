/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         game_logic.hpp
* @author       Name (@digipen.edu)
* @date         DD-MM-YYYY
* @brief        Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor
*               incididunt ut labore et dolore magna aliqua.
*//*========================================================================================*/

#pragma once

#include "engine/eno_engine.hpp"
#include "../../Engine/editor/eno_scene_tree.hpp"
#include <iostream>
#include <memory>
#include <imgui.h>
#include "movementTest.hpp"
#include "player/player_config.hpp"
#include "player/player.hpp"


// Enum for different editor states
enum EditorState {
    Stopped = 1,
    Playing,
    Paused,
    NextKeyFrame
};

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

    // Font
    std::shared_ptr<Font::EnoFont> pixelifyFont_;
    std::shared_ptr<Font::EnoFont> timesFont_;

    // Player
    Player::PlayerConfig playerConfig_;

    // Editor State
    EditorState g_editorState = Stopped;

    // Scene Tree (note: must be initialized after engine_ is initialized)
    std::unique_ptr<Editor::EnoSceneTree> sceneTree_;

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

    // Yun Qi's IMGUI sandbox
    //void setPlayerTexture(std::shared_ptr<Graphics::EnoTexture> newTex);
    void applyTextureToPlayer(const Graphics::TextureData& texData);

    // Accessor and mutators to access editor state
    EditorState getEditorState() const;
    EditorState& getEditorState();

    void setEditorState(EditorState state);
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


private:
    // Setup (called during initialize())
    void setupGameEntities();   // ECS code
    void setupTreeTesting();    // Editor tree testing code
    void setupGameAudio();      // audio setup code
    void setupGameEvents();     // event bus setup code
    void setupGameMath();       // math demonstration code
    void setupImGui();          // setup ImGui code
    void setupAnimations();

    // Update methods (called every frame)
    void updateGameLogic();     // Called every frame in game loop
    void updateEntities();      // Update entity logic per frame
    void updateImGui();         // Update code for imGui

    void drawPlayerAndIndicator();
    

    // Helper function to store entity positions during change in editor state
    void UpdateResetPos();

    void drawShapes(); 

    void drawText(Math::eVec2<float> windowSize);
};
