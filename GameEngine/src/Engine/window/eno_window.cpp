#include "eno_window.hpp"


namespace Window
{
	EnoWindow::EnoWindow(std::pair<int, int> dim) 
		: 
		width_{ dim.first }, 
		height_{ dim.second }
	{
		glfwInit();

		init();
	}

	GLFWwindow* EnoWindow::getWindow() const { return window_; }

	void EnoWindow::init()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window_ = glfwCreateWindow(width_, height_, "Eno Engine Window", nullptr, nullptr);

		if (!window_)
		{
			throw std::runtime_error("Failed to create the window!");
		}
	}
}