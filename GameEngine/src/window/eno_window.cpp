#include "eno_window.hpp"


namespace Window
{
	EnoWindow::EnoWindow()
	{
		glfwInit();

		init();
	}

	GLFWwindow* EnoWindow::getWindow() const { return window_; }

	void EnoWindow::init()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window_ = glfwCreateWindow(WIDTH, HEIGHT, "Eno Engine Window", nullptr, nullptr);
	}
}