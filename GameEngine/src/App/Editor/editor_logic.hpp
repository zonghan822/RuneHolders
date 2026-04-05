/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         editor_logic.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   75%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   17%
* @date         06-11-2025
* @brief        Game application layer that manages game-specific logic, rendering, and entity
*//*========================================================================================*/



#pragma once

#include "engine/eno_engine.hpp"

#include "project_picker/project_picker.hpp"
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
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

    // Saved editor camera state before entering play mode
    float prePlayCameraX_   = 0.0f;
    float prePlayCameraY_   = 0.0f;
    float prePlayZoomLevel_ = 1.0f;

    // Saved canvas positions before play (restored when play stops)
    std::unordered_map<uint32_t, Math::eVec2<float>> prePlayCanvasPositions_;

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

    /** @brief Initializes the engine, editor subsystems, and loads the initial scene. */
    void initialize();
    /** @brief Starts the main editor/game loop and runs until exit. */
    void run();
    /** @brief Shuts down all subsystems and releases resources. */
    void shutdown();

    /** @brief Marks all entity animation data as dirty, forcing a render cache refresh. */
    void markEntitiesDirty() { engine_.markAnimationsDirty(); }
    ECS::EntityManager& getEntityManager() { return engine_.getEntityManager(); }
    /** @brief Opens the project picker dialog for switching projects. */
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

    /** @brief Initiates a smooth camera pan to the target Y position over the given duration. */
    void startCameraPan(float targetY, float duration = 1.0f);
    bool isCameraPanning() const { return isPanning_; }

    // Get map editor
    MapEditor& getMapEditor() { return *mapEditor_; }

    // Get map editor
    Editor::StateMachineEditor& getStateMachineEditor() { return *stateMachineEditor_; }


private:
    /** @brief Registers editor event bus subscriptions during initialization. */
    void setupGameEvents();
    /** @brief Configures and initializes the ImGui context and style. */
    void setupImGui();

    /** @brief Renders ImGui editor panels; called after the engine render frame. */
    void updateEditorLogic();
    /** @brief Synchronizes the editor camera, applies panning, and writes back to the engine. */
    void updateEditorCamera();
    /** @brief Handles hot-reload checks and script begin() calls before the engine frame. */
    void updateEditorState();
    /** @brief Processes ImGui input and prepares the next ImGui frame. */
    void updateImGui();
};
