/*============================================================================================
* @file         window.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration file for the window manager
*//*========================================================================================*/


#pragma once
// OpenGL api
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Engine Component(s)
#include "../event/event_bus.hpp"
#include "window_events.hpp"

// stds
#include <iostream>
#include <utility>


namespace Window
{
	class EnoWindow 
	{
	public:
		EnoWindow(Event::EnoEventBus& bus);

		// Non-copyable, movable
		EnoWindow(const EnoWindow&) = delete;
		EnoWindow& operator=(const EnoWindow&) = delete;
		EnoWindow(EnoWindow&&) = default;
		EnoWindow& operator=(EnoWindow&&) = default;
		
		/*============================================================================================
		 * \brief				Window manager constructor
		 * \param				Window dimensions (Width, Height)
		*//*========================================================================================*/
		EnoWindow(std::pair<int, int>);

		/*============================================================================================
		 * \brief				Window manager constructor
		 * \param				Window dimensions (Width, Height)
		*//*========================================================================================*/
		EnoWindow(std::pair<int, int>);

		/*============================================================================================
		 * \brief				Window manager destructor
		*//*========================================================================================*/
		~EnoWindow();

		// Create a window and make its GL context current.
		// Throws std::runtime_error on failure.
		void initialize(const std::string& title, int width, int height,
						int glMajor = 4, int glMinor = 5);

		// Destroys window and terminates GLFW if created by this manager.
		void shutdown();
		
		// Window accessor
		GLFWwindow* getWindow() const;

		// Window lifecycle
		void pollEvents() const;
		void swapBuffers() const;
		bool shouldClose() const;
		void setShouldClose(bool value);

		std::pair<int, int> getFramebufferSize() const;
		std::pair<int, int> getWindowSize() const;

		// VSync
		void setVSync(bool enabled) const;
		
		/*============================================================================================
		 * \brief				Window getter
		*//*========================================================================================*/
		GLFWwindow* getWindow() const;

		/*============================================================================================
		 * \brief				Window getter
		*//*========================================================================================*/
		GLFWwindow* getWindow() const;
		
		/*============================================================================================
		 * \brief				Initialize and create the window
		*//*========================================================================================*/
		void windowInit(GLFWwindow*&);

		/*========================================= Events ===========================================

		/*============================================================================================
		 * \brief				Creating the game window
		*//*========================================================================================*/
		void gameWindowInit();

		/*============================================================================================
		 * \brief				Destroying the game window
		*//*========================================================================================*/
		void destroyGameWindow();


	private:
		// event callbacks
		static void ErrorCallback(int err, char const* description);
		static void FbSizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void MousePosCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		// setup callbacks
		void setupEventCallbacks();


	private:
		GLFWwindow* window_ = nullptr;
		Event::EnoEventBus& bus_;
	};
}