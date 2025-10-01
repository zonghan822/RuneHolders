#pragma once
// OpenGL api
#include <GLFW/glfw3.h>
#include <GL/glew.h>

// stds
#include <iostream>
#include <utility>


namespace Window
{
	class EnoWindow 
	{
	public:
		EnoWindow() = default;

		// Non-copyable, movable
		EnoWindow(const EnoWindow&) = delete;
		EnoWindow& operator=(const EnoWindow&) = delete;
		EnoWindow(EnoWindow&&) = default;
		EnoWindow& operator=(EnoWindow&&) = default;

		~EnoWindow() { shutdown(); }

		// Create a window and make its GL context current.
		// Throws std::runtime_error on failure.
		void initialize(const std::string& title, int width, int height,
						int glMajor = 4, int glMinor = 5);

		// Destroys window and terminates GLFW if created by this manager.
		void shutdown();
		
		// Window accessor
		GLFWwindow* getWindow() const;

		// Window lifecycle
		void pollEvents() const { if (window_) glfwPollEvents(); }
		void swapBuffers() const { if (window_) glfwSwapBuffers(window_); }
		bool shouldClose() const { return window_ == nullptr ? true : glfwWindowShouldClose(window_); }
		void setShouldClose(bool value) { if (window_) glfwSetWindowShouldClose(window_, value ? GLFW_TRUE : GLFW_FALSE); }

		std::pair<int, int> getWindowSize() const {
			int w = 0, h = 0;
			if (window_) glfwGetWindowSize(window_, &w, &h);
			return { w,h };
		}

	private:
		void setupEventCallbacks();

	private:
		GLFWwindow* window_ = nullptr;
	};
}