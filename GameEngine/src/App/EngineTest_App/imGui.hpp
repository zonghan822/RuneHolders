#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h> 
#include <chrono>
#include <random>
#include <unordered_map>

//trying to access GameApplication
#include "game_logic.hpp"

//variables
static std::mt19937 rng(std::random_device{}());
extern std::unique_ptr<Collision::CollisionSystem> collisionSystem_;
extern bool showColliders_;

//containers
extern std::unordered_map<std::string, double> systemTimes_; //actual system time
extern std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)



// returns a random float between min and max
float randomFloat(float min, float max);


// indicator control variables
extern float indicatorX_;
extern float indicatorY_;
extern float moveSpeed_ ; // pixels per second




//imGui functions

void imGuiInit(GLFWwindow* window);
//void imGuiDisplay(GLFWwindow* window, Engine::EnoEngine& engine_);
void imGuiDisplay(GLFWwindow* window, Engine::EnoEngine& engine_);

void imGuiTerminate();

