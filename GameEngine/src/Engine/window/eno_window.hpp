#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


// stds
#include <iostream>
#include <utility>


namespace Window
{
	class EnoWindow
	{
	public:
		EnoWindow(std::pair<int, int>);

		void init();

		GLFWwindow* getWindow() const;

	private:
		int width_;
		int height_;

		GLFWwindow* window_;
	};
}