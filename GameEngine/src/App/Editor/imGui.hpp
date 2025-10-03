/*============================================================================================
* @file         imGui.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        This file contains the functions that are required for Dear ImGui displays,
*				which includes the init, update and terminate functions. 
*     			There are also functions for spawning and counting entities.
*//*========================================================================================*/

#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h> 
#include <chrono>
#include <random>
#include <unordered_map>


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

// Containers to store system timings
extern std::unordered_map<std::string, double> systemTimes_; //actual system time
extern std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)

// indicator control variables
extern float indicatorX_;
extern float indicatorY_;
extern float moveSpeed_ ; // pixels per second


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
 * \brief       Displays total active entity count
 * \param[in]   Engine::EnoEngine& engine_
 * \return      void 
*//*========================================================================================*/
void displayEntityCount(Engine::EnoEngine& engine_);