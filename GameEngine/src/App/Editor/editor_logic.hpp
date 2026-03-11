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

#include "project_picker/project_picker.hpp"
#include <iostream>
#include <memory>
#include <set>
#include <imgui.h>
#include "map_editor/map_editor.hpp"
#include "stateMachine_editor/stateMachine_editor.hpp"


class EditorApplication {
private:
    // Engine instance
    Engine::EnoEngine engine_;

    // Game state variables
    bool gameRunning_ = true;
    bool musicStarted_ = false;

    // For delta time calculation
    float lastFrameTime_ = 0.0f;

    std::unique_ptr<Graphics::EnoFramebuffer> editorFramebuffer_;

    float worldWidth_ = 64.f;
    float worldHeight_ = 64.f;

    // Editor camera settings
    float editorCameraX_ = 0.0f;  // Camera offset X
    float editorCameraY_ = 0.0f;  // Camera offset Y
    float editorCameraSpeed_ = 600.0f;  // Camera movement speed
    bool  cameraInitialized_ = false;   // Deferred init: position (0,0) at bottom-left on first valid frame
    float editorZoomLevel_ = 1.0f;  // Zoom level (1.0 = normal, >1.0 = zoomed in, <1.0 = zoomed out)

    // Actual editor view panel size (pixels); updated each frame from ImGui
    int editorPanelWidth_  = 1;
    int editorPanelHeight_ = 1;

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

    // Scene Tree (note: must be initialized after engine_ is initialized)
    std::unique_ptr<Editor::EnoSceneTree> sceneTree_;

    // Map Editor (To be removed after set tilemap as node)
    std::unique_ptr<MapEditor> mapEditor_;

    // State Machine Editor
    std::unique_ptr<Editor::StateMachineEditor> stateMachineEditor_;


public:
    // Constructors
    EditorApplication() = default;
    EditorApplication(const EditorApplication&) = delete;            // Disable copy
    EditorApplication& operator=(const EditorApplication&) = delete; // Disable copy assignment
    EditorApplication(EditorApplication&&) = delete;                 // Disable move
    EditorApplication& operator=(EditorApplication&&) = delete;      // Disable move assignment

    // Main game methods
    void initialize();  // Called once at start
    void run();         // Starts the game loop
    void shutdown();    // Called when application closed

    void markEntitiesDirty() { engine_.markAnimationsDirty(); }
    ECS::EntityManager& getEntityManager() { return engine_.getEntityManager(); }
    void openProjectPicker() { setProjectPickerOpen(true); }

    Graphics::EnoFramebuffer* getEditorFramebuffer() { return editorFramebuffer_.get(); }

    // Get fixed virtual resolution
    float getWorldWidth() const { return worldWidth_; }
    float getWorldHeight() const { return worldHeight_; }
    // World units visible = panel pixels ÷ zoom (preserves zoom semantics)
    int getEditorWidth()  const { return static_cast<int>(editorPanelWidth_  / editorZoomLevel_); }
    int getEditorHeight() const { return static_cast<int>(editorPanelHeight_ / editorZoomLevel_); }

    // Raw pixel size of the editor panel
    int getEditorPanelWidth()  const { return editorPanelWidth_; }
    int getEditorPanelHeight() const { return editorPanelHeight_; }

    void setEditorPanelSize(int w, int h) {
        if (w > 0) editorPanelWidth_  = w;
        if (h > 0) editorPanelHeight_ = h;
    }

    // Get editor camera position
    float getEditorCameraX() const { return editorCameraX_; }
    float getEditorCameraY() const { return editorCameraY_; }
    void resetEditorCamera() { editorCameraX_ = 0.f; cameraInitialized_ = false; }

    // Zoom control
    void setEditorZoom(float zoom) {
        editorZoomLevel_ = std::clamp(zoom, 0.1f, 4.0f);
    }
    float getEditorZoom() const { return editorZoomLevel_; }
    void adjustEditorZoom(float delta) { setEditorZoom(editorZoomLevel_ + delta); }

    // Get/Set editor view min/max
    void setEditorViewRect(const Math::eVec2<float>& min, const Math::eVec2<float>& max) {
        editorViewMin_ = min;
        editorViewMax_ = max;
        engine_.setViewportBounds(min, max);
    }
    Math::eVec2<float> getEditorViewMin() const { return editorViewMin_; }
    Math::eVec2<float> getEditorViewMax() const { return editorViewMax_; }

    // Camera panning for room transitions
    void startCameraPan(float targetY, float duration = 1.0f);
    bool isCameraPanning() const { return isPanning_; }

    // Get map editor
    MapEditor& getMapEditor() { return *mapEditor_; }

    // Get map editor
    Editor::StateMachineEditor& getStateMachineEditor() { return *stateMachineEditor_; }


private:
    // Setup (called during initialize())
    void setupGameEvents();     // event bus setup code
    void setupImGui();          // setup ImGui code

    // Update methods (called every frame)
    void updateEditorLogic();   // ImGui rendering — called after renderFrame
    void updateEditorCamera();  // Camera sync, panning, write-back — called before renderFrame
    void updateEditorState();   // Hot reload, script begin() — called before beginFrame
    void updateImGui();         // Update code for imGui
};
