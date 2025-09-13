#pragma once

// GLFW
#include <GLFW/glfw3.h>

// stds
#include <iostream>


namespace Input
{
	class EnoInputManager
	{
	public:

		EnoInputManager(GLFWwindow* window)
		{
			glfwSetKeyCallback(window, keyCallback);
		}

		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_E && action == GLFW_PRESS)
			{
				std::cout << "User has pressed E" << std::endl;
			}
		}
	};
}