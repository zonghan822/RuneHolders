/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         imGui.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        This file contains the functions that are required for Dear ImGui displays,
*				which includes the init, update and terminate functions.
*     			There are also functions for spawning and counting entities.
*
*  NOTE: System pointers (collision, physics, player) are no longer global variables.
*        Access them through engine_.getXXXSystem() methods.
*//*========================================================================================*/

#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <chrono>
#include <random>
#include <unordered_map>
#include <algorithm>


#include "player/player.hpp"
#include "game_logic.hpp"
#include "../../Engine/prefab/prefab.hpp"
#include "../../Engine/raycast/raycast_renderer.hpp"

// Variables
static std::mt19937 rng(std::random_device{}());

// NOTE: Systems are now managed by EnoEngine, not as globals
// Access via engine_.getCollisionSystem(), engine_.getPhysicsSystem(), engine_.getPlayerSystem()

extern bool showColliders_;
extern GameApplication* g_game;

// Player entity ID to track player
extern uint32_t playerEntityID_;
extern bool playerSpawned_;

// Containers to store system timings
extern std::unordered_map<std::string, double> systemTimes_; //actual system time
extern std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)

// System Timers abstracted for neatness and accuracy
struct SystemTimer {
    using Clock = std::chrono::high_resolution_clock;

    std::string name;
    std::unordered_map<std::string, double>& systemTimes;
    std::chrono::time_point<Clock> start;

    SystemTimer(std::string name, std::unordered_map<std::string, double>& systemTimes)
        : name(std::move(name)), systemTimes(systemTimes), start(Clock::now()) {
    }

    ~SystemTimer() {
        auto end = Clock::now();
        systemTimes[name] = std::chrono::duration<double, std::micro>(end - start).count();
    }
};

//YunQi
struct AnimationPreset {
    std::string name;

    Graphics::TextureData idleTex;
    Graphics::TextureData walkTex;
    Graphics::TextureData attackTex;
};

// ImGui functions
/*============================================================================================
 * \brief       Initialises Dear ImGui
 * \param[in]   GLFWwindow* window
 * \return      void
*//*========================================================================================*/
void imGuiInit(GLFWwindow* window);

/*============================================================================================
 * \brief       Updates and renders the ImGui control panel
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \param[in]   Editor::UndoManager& undoManager
 * \return      void
*//*========================================================================================*/
void imGuiDisplay(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager);

/*============================================================================================
 * \brief       Terminates Dear ImGui
 * \param[in]   void
 * \return      void
*//*========================================================================================*/
void imGuiTerminate();


/*============================================================================================
 * \brief       Returns a random float between min and max value
 * \param[in]   float min, float max
 * \return      float
*//*========================================================================================*/
float randomFloat(float min, float max);

/*============================================================================================
 * \brief       Displays total active entity count
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayEntityCount(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays Edit tab
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \return      void
*//*========================================================================================*/
void displayEditTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree);

/*============================================================================================
 * \brief       Displays Hierachy Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \param[in]   Editor::UndoManager& undoManager
 * \return      void
*//*========================================================================================*/
void displayHierarchyTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager);

/*============================================================================================
 * \brief       Displays all Nodes in a recursive order
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \param[in]   uint32_t nodeID
 * \param[in]   ECS::EntityManager& entityManager
 * \return      void
*//*========================================================================================*/
void drawNodeRecursive(Editor::EnoSceneTree& sceneTree, uint32_t nodeID, ECS::EntityManager& entityManager);

/*============================================================================================
 * \brief       Displays All Entities View Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   Editor::UndoManager& undoManager
 * \return      void
*//*========================================================================================*/
void displayAllEntitiesTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager);

/*============================================================================================
 * \brief       Displays Entity Inspector
 * \param[in]   ECS::Entity&
 * \param[in]   Editor::UndoManager& undoManager
 * \return      void
*//*========================================================================================*/
void displayEntityInspector(Engine::EnoEngine& engine_, ECS::Entity& entity, Editor::UndoManager& undoManager, Editor::EnoSceneTree& sceneTree);

/*============================================================================================
 * \brief       Displays Prefabs in Prefabs Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \param[in]   Editor::UndoManager& undoManager
 * \return      void
*//*========================================================================================*/
void displayPrefabsTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager);


void assetPresetsTab(Engine::EnoEngine& engine_, ECS::Entity& entity);
//void assetPresetsTab(Engine::EnoEngine& engine_, Node* Node);

/*============================================================================================
 * \brief       Displays Assets in the Assets menu
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayAssetTab(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays all characteristics of each entity available
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayEntityCharacteristics(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays FPS tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayFPSTab(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays Scene Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displaySceneTab(Editor::EnoEditor& editor_);

/*============================================================================================
 * \brief       Helper function to load music and sounds for Audio Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void LoadAudioAssets(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays Audio Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayAudioTab(Engine::EnoEngine& engine_);


/*============================================================================================
 * \brief       Display game view inside of imgui
 * \return      void
*//*========================================================================================*/
void displayGameView();

/*============================================================================================
 * \brief       Displays editor view
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \param[in]   Editor::UndoManager& undoManager
*//*========================================================================================*/
void displayEditorView(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager);


/*============================================================================================
 * \brief       Sets up layout for the imGui
 * \return      void
*//*========================================================================================*/
void imGuiSetupDefaultLayout();

/*============================================================================================
 * \brief       Begin docking for imgui
*//*========================================================================================*/
void imGuiBeginDockspace();


/*============================================================================================
 * \brief       Terminates up layout for the imGui
 * \return      void
*//*========================================================================================*/
void imGuiEndDockspace();


/*============================================================================================
 * \brief       Gets entity under mouse cursor
 * \param[in]   Engine::EnoEngine& engine_
 * \param[in]   ImVec2 mouse
 * \return      std::optional<ECS::Entity>
*//*========================================================================================*/
std::optional<ECS::Entity> getEntityUnderMouse(Engine::EnoEngine& engine_, Math::eVec2<float> mouse);

/*============================================================================================
 * \brief       Moves child entities relative to parent movement (deprecated - use applyParentTransformToChildren)
 * \param[in]   Editor::Node* node
 * \param[in]   float deltaX
 * \param[in]   float deltaY
 * \param[in]   ECS::EntityManager& entityManager
 * \return      void
*//*========================================================================================*/
void moveChildrenRecursive(Editor::Node* node, float deltaX, float deltaY, ECS::EntityManager& entityManager);

/*============================================================================================
 * \brief       Applies rotation delta to all children recursively around parent position
 * \param[in]   Editor::Node* node
 * \param[in]   float deltaRotation - Rotation change in radians
 * \param[in]   Math::eVec2<float> parentPos - Parent's position (rotation pivot)
 * \param[in]   ECS::EntityManager& entityManager
 * \return      void
*//*========================================================================================*/
void rotateChildrenRecursive(Editor::Node* node, float deltaRotation, Math::eVec2<float> parentPos, ECS::EntityManager& entityManager);

/*============================================================================================
 * \brief       Applies scale delta to all children recursively from parent position
 * \param[in]   Editor::Node* node
 * \param[in]   float scaleFactorX - Scale multiplier for X axis
 * \param[in]   float scaleFactorY - Scale multiplier for Y axis
 * \param[in]   Math::eVec2<float> parentPos - Parent's position (scale origin)
 * \param[in]   ECS::EntityManager& entityManager
 * \return      void
*//*========================================================================================*/
void scaleChildrenRecursive(Editor::Node* node, float scaleFactorX, float scaleFactorY, Math::eVec2<float> parentPos, ECS::EntityManager& entityManager);

/*============================================================================================
 * \brief       Destroys all child entities recursively
 * \param[in]   Editor::Node* node
 * \param[in]   ECS::EntityManager& entityManager
 * \return      void
*//*========================================================================================*/
void destroyChildrenRecursive(Editor::Node* node, ECS::EntityManager& entityManager);

/*============================================================================================
 * \brief       Gets the currently selected entity ID
 * \return      uint32_t
*//*========================================================================================*/
uint32_t getSelectedEntityID();

/*============================================================================================
 * \brief       Clears the currently selected entity
 * \return      void
*//*========================================================================================*/
void clearSelectedEntity();

/*============================================================================================
 * \brief       Deletes the currently selected entity and its children
 * \param[in]   Editor::EnoSceneTree& sceneTree
 * \param[in]   ECS::EntityManager& entityManager
 * \return      void
*//*========================================================================================*/
void deleteSelectedEntity(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager, Editor::UndoManager& undoManager);

/*============================================================================================
 * \brief       Displays the Raycast 3D View panel (Wolfenstein-style first-person renderer)
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayRaycastVisualizerTab(Engine::EnoEngine& engine_);

// ============================================================
// Project Picker
// ============================================================

/*============================================================================================
 * \brief       Renders the project picker dialog (owns its own ImGui frame).
 *              Call instead of imGuiDisplay() when isProjectPickerOpen() is true.
 * \param[in]   engine_  Engine reference (needed for project switching)
 * \return      void
*//*========================================================================================*/
void imGuiProjectPicker(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Returns true if the project picker dialog is currently open.
*//*========================================================================================*/
bool isProjectPickerOpen();

/*============================================================================================
 * \brief       Opens or closes the project picker dialog.
 *              Passing true also rescans available projects and pre-selects the current one.
*//*========================================================================================*/
void setProjectPickerOpen(bool open);