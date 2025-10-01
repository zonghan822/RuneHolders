#include "eno_window.hpp"


namespace Window
{
	void EnoWindow::initialize(const std::string& title, int width, int height,
		int glMajor, int glMinor) {
		if (!glfwInit()) {
			throw std::runtime_error("GLFW initialization failed");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_RED_BITS, 8); glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8); glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // window dimensions are static

		window_ = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		if (!window_) {
			glfwTerminate();
			throw std::runtime_error("GLFW window creation failed");
		}

		// Make context current
		glfwMakeContextCurrent(window_);

		// this is the default setting ...
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		// Part 2: Initialize entry points to OpenGL functions and extensions
		GLenum err = glewInit();
		if (GLEW_OK != err) {
			std::string msg = "GLEW initialization failed: ";
			msg += reinterpret_cast<const char*>(glewGetErrorString(err));
			glfwDestroyWindow(window_);
			window_ = nullptr;
			glfwTerminate();
			throw std::runtime_error(msg);
		}
		if (GLEW_VERSION_4_5) {
			std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
			std::cout << "Driver supports OpenGL " << glMajor << '.' << glMinor << '\n' << std::endl;
		}
		else {
			const GLubyte* glVersionStr = glGetString(GL_VERSION);
			if (glVersionStr) {
				int major = 0, minor = 0;
				if (sscanf(reinterpret_cast<const char*>(glVersionStr), "%d.%d", &major, &minor) == 2) {
					if (major > glMajor || (major == glMajor && minor >= glMinor)) {
						std::cout << "Driver supports OpenGL " << major << "." << minor << "\n" << std::endl;
					}
					else {
						std::cerr << "Warning: Detected OpenGL version " << major << "." << minor
							<< " � does not meet required version "
							<< glMajor << "." << glMinor << "." << std::endl;
					}
				}
				else {
					std::cerr << "Warning: Unable to parse OpenGL version string." << std::endl;
				}
			}
			else {
				std::cerr << "Warning: Unable to retrieve OpenGL version string." << std::endl;
			}
		}

		// Default swap interval 1 (vsync on)
		glfwSwapInterval(1);


	}


	GLFWwindow* EnoWindow::getWindow() const { return window_; }


}