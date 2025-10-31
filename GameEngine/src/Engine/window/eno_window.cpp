/*============================================================================================
* @file         eno_window.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author		Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief		Implementation of GLFW window management and OpenGL context handling
* 
*				This file implements the window management system using GLFW for cross-platform
*				window creation and OpenGL context management. It handles window initialization,
*				event callback registration, OpenGL context creation with GLEW, and proper resource
*				cleanup. The implementation provides a robust wrapper around GLFW with error handling
*				and event forwarding to the engine's event system.
*//*========================================================================================*/

#include "eno_window.hpp"

namespace Window
{
	static std::vector<std::string> g_droppedFiles;


	EnoWindow::EnoWindow(Event::EnoEventBus& bus) : bus_{ bus } { /*empty by design*/ }

	EnoWindow::~EnoWindow() { shutdown(); }

	void EnoWindow::initialize(const std::string& title, int width, int height,
		int glMajor, int glMinor) {
		if (!glfwInit()) {
			throw std::runtime_error("GLFW initialization failed");
		}

		glfwSetErrorCallback(ErrorCallback);

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
		glfwSetWindowUserPointer(window_, this);

		// Make context current
		glfwMakeContextCurrent(window_);

		setupEventCallbacks();

		//(YunQi's file dropping)
		glfwSetDropCallback(window_, [](GLFWwindow* w, int count, const char** paths)
			{
				for (int i = 0; i < count; ++i)
					g_droppedFiles.emplace_back(paths[i]);   // store dropped file path
				std::cout << "[DEBUG] File drop callback triggered!\n";
				for (int i = 0; i < count; ++i)
					std::cout << "   " << paths[i] << "\n";
			});


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


	std::vector<std::string>& EnoWindow::GetDroppedFiles() { return g_droppedFiles; }

	void EnoWindow::shutdown() {
		if (window_) {
			glfwDestroyWindow(window_);
			window_ = nullptr;
		}
		glfwTerminate();
	}

	GLFWwindow* EnoWindow::getWindow() const { return window_; }

	void EnoWindow::pollEvents() const { if (window_) glfwPollEvents(); }

	void EnoWindow::swapBuffers() const { if (window_) glfwSwapBuffers(window_); }
	
	bool EnoWindow::shouldClose() const { return window_ == nullptr ? true : glfwWindowShouldClose(window_); }
	
	void EnoWindow::setShouldClose(bool value) { if (window_) glfwSetWindowShouldClose(window_, value ? GLFW_TRUE : GLFW_FALSE); }

	std::pair<int, int> EnoWindow::getFramebufferSize() const {
		int w = 0, h = 0;
		if (window_) glfwGetFramebufferSize(window_, &w, &h);
		return { w,h };
	}

	std::pair<int, int> EnoWindow::getWindowSize() const {
		int w = 0, h = 0;
		if (window_) glfwGetWindowSize(window_, &w, &h);
		return { w,h };
	}

	void EnoWindow::setVSync(bool enabled) const { 
		if (enabled) glfwSwapInterval(1); 
		else glfwSwapInterval(0); 
	}

	void EnoWindow::setupEventCallbacks() {
		glfwSetFramebufferSizeCallback(window_, FbSizeCallback);
		glfwSetKeyCallback(window_, KeyCallback);
		glfwSetMouseButtonCallback(window_, MouseButtonCallback);
		glfwSetCursorPosCallback(window_, MousePosCallback);
		glfwSetScrollCallback(window_, MouseScrollCallback);
	}

	void EnoWindow::ErrorCallback(int err, char const* description) {
		(void)err;
		std::string msg = "GLFW error: ";
		msg += description;
		throw std::runtime_error(msg);
	}

	void EnoWindow::FbSizeCallback(GLFWwindow* window, int width, int height) {
		auto* self = static_cast<EnoWindow*>(glfwGetWindowUserPointer(window));
		self->bus_.publish(FramebufferResizeEvent{ width, height });
	}

	void EnoWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto* self = static_cast<EnoWindow*>(glfwGetWindowUserPointer(window));
		self->bus_.publish(KeyEvent{ key, scancode, action, mods });
	}

	void EnoWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto* self = static_cast<EnoWindow*>(glfwGetWindowUserPointer(window));
		self->bus_.publish(MouseButtonEvent{ button, action, mods });
	}

	void EnoWindow::MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
		auto* self = static_cast<EnoWindow*>(glfwGetWindowUserPointer(window));
		self->bus_.publish(MouseCursorEvent{ xpos, ypos });
	}

	void EnoWindow::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		auto* self = static_cast<EnoWindow*>(glfwGetWindowUserPointer(window));
		self->bus_.publish(ScrollEvent{ xoffset, yoffset });
	}

}