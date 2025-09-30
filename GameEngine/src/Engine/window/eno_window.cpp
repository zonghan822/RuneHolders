#include "eno_window.hpp"


namespace Window
{
	EnoWindow::EnoWindow(std::pair<int, int> dim)
		:
		width_{ dim.first },
		height_{ dim.second }
	{
		glfwInit();

		windowInit(window_);
	}

	GLFWwindow* EnoWindow::getWindow() const { return window_; }

	void EnoWindow::windowInit(GLFWwindow*& window)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(width_, height_, "Eno Engine Window", nullptr, nullptr);

		if (!window)
		{
			throw std::runtime_error("Failed to create the window!");
		}
	}

	void EnoWindow::Update() {}

	// ----------------------- Events -----------------------
	void EnoWindow::gameWindowInit()
	{
		std::cout << "initializing game window" << std::endl;
		windowInit(gameWindow_);
	}

	void EnoWindow::destroyGameWindow()
	{
		std::cout << "destroying game window" << std::endl;
		glfwDestroyWindow(gameWindow_);
	}
}