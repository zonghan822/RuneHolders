/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         imGui.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        This file contains the functions that are required for Dear ImGui displays,
*				which includes the init, update and terminate functions. 
*     			There are also functions for spawning and counting entities.
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

// Variables
static std::mt19937 rng(std::random_device{}());
extern std::unique_ptr<Collision::CollisionSystem> collisionSystem_;
extern std::unique_ptr<Player::PlayerSystem> playerSystem_;
extern bool showColliders_;
extern GameApplication* g_game;

// Player entity ID to track player
extern uint32_t playerEntityID_;
extern bool playerSpawned_;

// Indicator control variables
extern float indicatorX_;
extern float indicatorY_;
extern float moveSpeed_ ; // pixels per second

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

// Enum for different editor states
//enum EditorState {
//    Stopped = 1,
//    Playing,
//    Paused,
//    NextKeyFrame
//};
//
//EditorState _editorState;


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
 * \return      void
*//*========================================================================================*/
void imGuiDisplay(Engine::EnoEngine& engine_);

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
 * \brief       Displays Main Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayMainTab(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays total active entity count
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void 
*//*========================================================================================*/
void displayEntityCount(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays Edit tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayEditTab(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays Hierachy Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayHierarchyTab(Engine::EnoEngine& engine_, GameInstance::EnoSceneTree& sceneTree);

/*============================================================================================
 * \brief       Displays all Nodes in a recursive order
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void drawNodeRecursive(GameInstance::EnoSceneTree& sceneTree, uint32_t nodeID);

/*============================================================================================
 * \brief       Displays All Entities View Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayAllEntitiesTab(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Displays Entity Inspector
 * \param[in]   ECS::Entity&
 * \return      void
*//*========================================================================================*/
void displayEntityInspector(ECS::EntityManager& entMng, ECS::Entity& entity);

/*============================================================================================
 * \brief       Displays Prefabs in Prefabs Tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
void displayPrefabsTab(Engine::EnoEngine& engine_);

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


// docking shi
void displayGameView(Engine::EnoEngine& engine_);
void displayEditorView(Engine::EnoEngine& engine_);

void imGuiSetupDefaultLayout();

void imGuiBeginDockspace();

void imGuiEndDockspace();


/*============================================================================================
 * \brief       Displays FPS tab
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void
*//*========================================================================================*/
std::optional<ECS::Entity> getEntityUnderMouse(Engine::EnoEngine& engine_, ImVec2 mouse);
