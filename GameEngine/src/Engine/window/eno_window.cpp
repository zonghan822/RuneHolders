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

	void EnoWindow::initialize(const std::string& title, int width, int height, bool fullscreen, bool vsync,
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

		isFullscreen_ = fullscreen;
		windowedW_ = width;
		windowedH_ = height;

		GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

		window_ = glfwCreateWindow(width, height, title.c_str(), monitor, NULL);
		if (!window_) {
			glfwTerminate();
			throw std::runtime_error("GLFW window creation failed");
		}
		glfwSetWindowUserPointer(window_, this);

		// Make context current
		glfwMakeContextCurrent(window_);

		buildResolutionTable();

		setupEventCallbacks();

		//(YunQi's file dropping)
		glfwSetDropCallback(window_, [](GLFWwindow* w, int count, const char** paths)
			{
				(void)w;
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
		glfwSwapInterval(vsync ? 1 : 0);
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

	void EnoWindow::buildResolutionTable() {
		resolutionTable_.clear();

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		int maxW = mode->width;
		int maxH = mode->height;

		// Standard resolutions (you can add/remove)
		std::vector<std::pair<int, int>> commonRes = {
			{1280, 720},
			{1366, 768},
			{1600, 900},
			{1920, 1080},
			{2560, 1440},
			{3840, 2160}
		};

		// Add only resolutions <= monitor resolution
		for (auto& r : commonRes)
			if (r.first <= maxW && r.second <= maxH)
				resolutionTable_.push_back(r);

		// Ensure at least one resolution
		if (resolutionTable_.empty())
			resolutionTable_.push_back({ maxW, maxH });

		// Find index matching current window size
		int w, h;
		glfwGetWindowSize(window_, &w, &h);

		for (int i = 0; i < (int)resolutionTable_.size(); i++) {
			if (resolutionTable_[i].first == w && resolutionTable_[i].second == h) {
				resolutionIndex_ = i;
				return;
			}
		}

		// fallback
		resolutionIndex_ = 0;
	}

	void EnoWindow::toggleFullscreen()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		if (!isFullscreen_) {

			// Save current windowed position + size
			glfwGetWindowPos(window_, &windowedX_, &windowedY_);
			glfwGetWindowSize(window_, &windowedW_, &windowedH_);

			// Go fullscreen
			glfwSetWindowMonitor(window_,
				monitor,
				0, 0,
				mode->width,
				mode->height,
				mode->refreshRate);

#ifdef _DEBUG
			std::cout << "[Window] Entering fullscreen: "
				<< mode->width << "x" << mode->height << "\n";
#endif

			isFullscreen_ = true;
		}
		else {
			// Return to windowed mode
			glfwSetWindowMonitor(window_,
				nullptr,
				windowedX_, windowedY_,
				windowedW_, windowedH_,
				0);

#ifdef _DEBUG
			std::cout << "[Window] Exiting fullscreen: "
				<< windowedW_ << "x" << windowedH_ << "\n";
#endif

			isFullscreen_ = false;
		}
	}

	void EnoWindow::setResolution(int width, int height) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		windowedW_ = width;
		windowedH_ = height;

		glfwGetWindowPos(window_, &windowedX_, &windowedY_);
		glfwSetWindowMonitor(window_, nullptr, windowedX_, windowedY_, width, height, 0);
	}

	void EnoWindow::increaseResolution()
	{
		if (resolutionIndex_ + 1 >= (int)resolutionTable_.size())
		{
#ifdef _DEBUG
			std::cout << "[Window] Already at maximum resolution.\n";
#endif
			return; // cannot increase further
		}

		resolutionIndex_++;
		auto [w, h] = resolutionTable_[resolutionIndex_];

#ifdef _DEBUG
		std::cout << "[Window] Increasing resolution to: "
			<< w << "x" << h << "\n";
#endif

		setResolution(w, h);
	}


	void EnoWindow::decreaseResolution()
	{
		if (resolutionIndex_ - 1 < 0)
		{
#ifdef _DEBUG
			std::cout << "[Window] Already at minimum resolution.\n";
#endif
			return; // cannot decrease further
		}

		resolutionIndex_--;
		auto [w, h] = resolutionTable_[resolutionIndex_];

#ifdef _DEBUG
		std::cout << "[Window] Decreasing resolution to: "
			<< w << "x" << h << "\n";
#endif

		setResolution(w, h);
	}
}