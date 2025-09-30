#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../interface/system.hpp"

// Events
#include "../game_instance/game_instance_events.hpp"

// stds
#include <iostream>
#include <utility>


namespace Window
{
	class EnoWindow : public Sys::ISystem
	{
	public:
		EnoWindow(std::pair<int, int>);

		GLFWwindow* getWindow() const;

		void windowInit(GLFWwindow*&);

		void gameWindowInit();
		void destroyGameWindow();

		void Update();

	private:
		int width_;
		int height_;

		GLFWwindow* window_;
		GLFWwindow* gameWindow_;
	};
}