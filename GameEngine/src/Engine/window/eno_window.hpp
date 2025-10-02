#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Events
#include "../game_instance/game_instance_events.hpp"

// stds
#include <iostream>
#include <utility>


namespace Window
{
	class EnoWindow
	{
	public:
		EnoWindow(std::pair<int, int>);

		~EnoWindow();

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