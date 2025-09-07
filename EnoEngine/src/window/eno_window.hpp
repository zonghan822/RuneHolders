#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


// stds
#include "iostream"


namespace Window
{
	class EnoWindow
	{
	public:
		EnoWindow();

		void init();

		GLFWwindow* getWindow() const;

	private:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		GLFWwindow* window_;
	};
}