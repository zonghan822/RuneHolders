/*============================================================================================
* @file         eno_window.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author		Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief		Window management class handling GLFW window creation and event callbacks
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
	/*============================================================================================
	* \class	EnoWindow
	* \brief	Manages GLFW window creation, destruction, and event handling
	*
	*			This class wraps GLFW window functionality and provides a clean interface
	*			for window management while forwarding input events to the event bus.
	*//*========================================================================================*/
	class EnoWindow 
	{
	public:

		/*============================================================================================
		* \brief	Constructs a window manager with reference to event bus
		* \param[in] bus	Reference to the event bus for forwarding window events
		*//*========================================================================================*/
		EnoWindow(Event::EnoEventBus& bus);

		// Non-copyable, movable
		EnoWindow(const EnoWindow&) = delete;
		EnoWindow& operator=(const EnoWindow&) = delete;
		EnoWindow(EnoWindow&&) = default;
		EnoWindow& operator=(EnoWindow&&) = default;

		/*============================================================================================
		* \brief	Destructor that ensures proper cleanup of GLFW resources
		*//*========================================================================================*/
		~EnoWindow();

		/*============================================================================================
		* \brief	Creates a window and initializes OpenGL context
		* \param[in] title		Window title
		* \param[in] width		Window width in pixels
		* \param[in] height		Window height in pixels
		* \param[in] glMajor	OpenGL major version (default: 4)
		* \param[in] glMinor	OpenGL minor version (default: 5)
		* \throws	std::runtime_error if GLFW initialization or window creation fails
		*//*========================================================================================*/
		void initialize(const std::string& title, int width, int height,
						int glMajor = 4, int glMinor = 5);

		/*============================================================================================
		* \brief	Destroys window and terminates GLFW if created by this manager
		*//*========================================================================================*/
		void shutdown();
		
		/*============================================================================================
		* \brief	Gets the underlying GLFW window pointer
		* \return	Pointer to GLFWwindow, or nullptr if not initialized
		*//*========================================================================================*/
		GLFWwindow* getWindow() const;

		// Window lifecycle

		/*============================================================================================
		* \brief	Processes all pending window events
		*//*========================================================================================*/
		void pollEvents() const;

		/*============================================================================================
		* \brief	Swaps the front and back buffers
		*//*========================================================================================*/
		void swapBuffers() const;

		/*============================================================================================
		* \brief	Checks if window should close
		* \return	true if window should close, false otherwise
		*//*========================================================================================*/
		bool shouldClose() const;

		/*============================================================================================
		* \brief	Sets the window close flag
		* \param[in] value	value true to request window close, false otherwise
		*//*========================================================================================*/
		void setShouldClose(bool value);

		/*============================================================================================
		* \brief	Gets the framebuffer size (actual rendering area)
		* \return	Pair of integers representing width and height
		*//*========================================================================================*/
		std::pair<int, int> getFramebufferSize() const;

		/*============================================================================================
		* \brief	Gets the window size (window frame including decorations)
		* \return	Pair of integers representing width and height
		*//*========================================================================================*/
		std::pair<int, int> getWindowSize() const;

		/*============================================================================================
		* \brief	Enables or disables vertical synchronization
		* \param[in] enabled	true to enable VSync, false to disable
		*//*========================================================================================*/
		void setVSync(bool enabled) const;

	private:
		// event callbacks

		/*============================================================================================
		* \brief	GLFW error callback
		* \param[in] err			Error code
		* \param[in] description	Error description
		*//*========================================================================================*/
		static void ErrorCallback(int err, char const* description);

		/*============================================================================================
		* \brief	Framebuffer resize callback
		* \param[in] window		GLFW window pointer
		* \param[in] width		New framebuffer width
		* \param[in] height		New framebuffer height
		*//*========================================================================================*/
		static void FbSizeCallback(GLFWwindow* window, int width, int height);

		/*============================================================================================
		* \brief	Keyboard input callback
		* \param[in] window		GLFW window pointer
		* \param[in] key		Key code
		* \param[in] scancode	System-specific scancode
		* \param[in] action		Key action (press/release/repeat)
		* \param[in] mods		Modifier keys state
		*//*========================================================================================*/
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		/*============================================================================================
		* \brief	Mouse button callback
		* \param[in] window		GLFW window pointer
		* \param[in] button		Mouse button code
		* \param[in] action		Button action (press/release)
		* \param[in] mods		Modifier keys state
		*//*========================================================================================*/
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		/*============================================================================================
		* \brief	Mouse cursor position callback
		* \param[in] window		GLFW window pointer
		* \param[in] xpos		Cursor x-coordinate
        * \param[in] ypos		Cursor y-coordinate
		*//*========================================================================================*/
		static void MousePosCallback(GLFWwindow* window, double xpos, double ypos);

		/*============================================================================================
		* \brief	Mouse scroll callback
		* \param[in] window		GLFW window pointer
		* \param[in] xoffset	Horizontal scroll offset
		* \param[in] yoffset	Vertical scroll offset
		*//*========================================================================================*/
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		/*============================================================================================
		* \brief	Sets up all GLFW event callbacks
		*//*========================================================================================*/
		void setupEventCallbacks();


	private:
		GLFWwindow* window_ = nullptr;
		Event::EnoEventBus& bus_;
	};
}