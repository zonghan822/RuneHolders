/*============================================================================================
* @file         eno_input.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author		Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief		Input management system for handling keyboard and mouse input events.
*               Integrates with the event bus to map key/mouse actions to engine events,
*               and provides utility functions to query input states.
*//*========================================================================================*/

#pragma once

// GLFW
#include <GLFW/glfw3.h>

// Engine Component(s)
#include "../event/event_bus.hpp"
#include "../window/window_events.hpp"

// Events
#include "eno_input_events.hpp"

// Utility
#include "../utility/eno_bimap.hpp"

// stds
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>


namespace Input
{
	/*============================================================================================
	* \struct	KeyBinding
	* \brief	Structure that describes a key binding with key, scancode, action, and modifier.
	*//*========================================================================================*/
	struct KeyBinding
	{
		int key_;       ///< GLFW key code
		int scancode_;  ///< Platform-specific scancode
		int action_;    ///< GLFW action (press/release/repeat)
		int mods_;      ///< Modifier bitmask (e.g., shift, ctrl, alt)

		bool operator==(const KeyBinding& other) const
		{
			return	key_ == other.key_ &&
				scancode_ == other.scancode_ &&
				action_ == other.action_ &&
				mods_ == other.mods_;
		}
	};

	/*============================================================================================
	* \struct	KeyBindingHasher
	* \brief	Custom hash functor for KeyBinding to be used in unordered containers.
	*//*========================================================================================*/
	struct KeyBindingHasher
	{
		size_t operator()(const KeyBinding& k) const noexcept
		{
			size_t seed = 0;
			auto hash_combine = [&](size_t v) {
				seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
				};
			hash_combine(std::hash<int>{}(k.key_));
			hash_combine(std::hash<int>{}(k.scancode_));
			hash_combine(std::hash<int>{}(k.action_));
			hash_combine(std::hash<int>{}(k.mods_));
			return seed;
		}
	};

	/*============================================================================================
	* \class	EnoInputManager
	* \brief	Manages input handling for keyboard and mouse events.
	*
	*			Provides functions for key and mouse state queries,
	*			as well as event mapping through an event bus.
	*//*========================================================================================*/
	class EnoInputManager
	{
	public:

		/*============================================================================================
		* \brief   Construct the input manager and bind it to an event bus.
		* \param[in] bus Reference to the engine-wide event bus.
		*//*=========================================================================================*/
		EnoInputManager(Event::EnoEventBus& bus);

		/*============================================================================================
		* \brief   Subscribe to window events (keyboard, mouse, scroll) through the event bus.
		*//*========================================================================================*/
		void initialize();

		/*============================================================================================
		* \brief   Set up a key binding in the input map.
		* \param[in] inputName User-defined name for the key binding.
		* \param[in] key GLFW key code.
		* \param[in] scancode Platform-specific scancode.
		* \param[in] action GLFW action (press/release/repeat).
		* \param[in] mods Modifier bitmask.
		*//*========================================================================================*/
		void setupKey(const std::string& inputName, int key = 0, int scancode = 0, int action = 0, int mods = 0);

		/*============================================================================================
		* \brief   Associate an event type with an input binding.
		* \tparam  EventType The type of event to publish.
		* \tparam  Data Variadic arguments for constructing the event.
		* \param[in] inputName Name of the input to associate with the event.
		* \param[in] d Arguments for the event.
		*//*========================================================================================*/
		template <typename EventType, typename... Data>
		void setupEvent(const std::string& inputName, Data&&... d) 
		{
			eventMap_[inputName] = [this, ... args = std::forward<Data>(d)]()
				{ bus_.publish(EventType{ args... }); };
		}

		/*============================================================================================
		* \brief   End the input frame, updating previous states for comparison in the next frame.
		*//*========================================================================================*/
		void endFrame();

		/****************************************************
		* Keyboard											*
		*****************************************************/

		/*============================================================================================
		* \brief   Check if a key is currently pressed.
		* \param[in] key GLFW key code.
		* \return  True if the key is pressed, false otherwise.
		*//*========================================================================================*/
		bool isKeyPressed(int key) const;

		/*============================================================================================
		* \brief   Check if a key was pressed in this frame (key down event).
		* \param[in] key GLFW key code.
		* \return  True if the key was pressed this frame, false otherwise.
		*//*========================================================================================*/
		bool isKeyDown(int key) const;

		/*============================================================================================
		* \brief   Check if a key was released in this frame (key up event).
		* \param[in] key GLFW key code.
		* \return  True if the key was released this frame, false otherwise.
		*//*========================================================================================*/
		bool isKeyUp(int key) const;

		/*============================================================================================
		* \brief   Check if a modifier key is active.
		* \param[in] mod GLFW modifier mask (e.g., GLFW_MOD_SHIFT).
		* \return  True if the modifier is active, false otherwise.
		*//*========================================================================================*/
		bool isModifierActive(int mod) const;

		/*============================================================================================
		* \brief   Get modifiers currently appplied
		* \param[in] GLFW mod
		* \return  current GLFW mod applied
		*//*========================================================================================*/
		int currentMods() const;

		/*============================================================================================
		* \brief   Check if a key combo (key + modifiers) is pressed.
		* \param[in] key GLFW key code.
		* \param[in] requiredMods Required modifier mask.
		* \return  True if combo is pressed, false otherwise.
		*//*========================================================================================*/
		bool isKeyComboPressed(int key, int requiredMods) const;

		/****************************************************
		* Mouse												*
		*****************************************************/

		/*============================================================================================
		* \brief   Check if a mouse button is currently pressed.
		* \param[in] button GLFW mouse button code.
		* \return  True if the button is pressed, false otherwise.
		*//*========================================================================================*/
		bool isMousePressed(int button) const;

		/*============================================================================================
		* \brief   Check if a mouse button was pressed in this frame (mouse down).
		* \param[in] button GLFW mouse button code.
		* \return  True if pressed this frame, false otherwise.
		*//*========================================================================================*/
		bool isMouseDown(int button) const;

		/*============================================================================================
		* \brief   Check if a mouse button was released in this frame (mouse up).
		* \param[in] button GLFW mouse button code.
		* \return  True if released this frame, false otherwise.
		*//*========================================================================================*/
		bool isMouseUp(int button) const;

		/*============================================================================================
		* \brief   Check if a mouse button + modifier combo is pressed.
		* \param[in] button GLFW mouse button code.
		* \param[in] requiredMods Required modifier mask.
		* \return  True if the combo is pressed, false otherwise.
		*//*========================================================================================*/
		bool isMouseCombo(int button, int requiredMods) const;

		/*============================================================================================
		* \brief   Get the current mouse position.
		* \return  Pair of (x, y) coordinates in screen space.
		*//*========================================================================================*/
		std::pair<double, double> getMousePosition() const;

		/*============================================================================================
		* \brief   Get the mouse movement delta since the last frame.
		* \return  Pair of (dx, dy) movement values.
		*//*========================================================================================*/
		std::pair<double, double> getMouseDelta() const;

		/*============================================================================================
		* \brief   Get the scroll offset since the last frame.
		* \return  Pair of (xOffset, yOffset).
		*//*========================================================================================*/
		std::pair<double, double> getScrollOffset() const;

	private:
		Event::EnoEventBus& bus_;

		Utility::EnoBiMap<
			std::string, KeyBinding,
			std::hash<std::string>, 
			std::equal_to<std::string>,
			KeyBindingHasher,
			std::equal_to<KeyBinding>
		> inputMap_;

		std::unordered_map<std::string, std::function<void()>> eventMap_;

		// Keyboard state tracking
		std::unordered_map<int, bool> keyStates_;
		std::unordered_map<int, bool> prevKeyStates_;
		int modsState_ = 0;

		// Mouse state tracking
		std::unordered_map<int, bool> mouseButtonStates_;
		std::unordered_map<int, bool> prevMouseButtonStates_;
		double mouseX_ = 0.0, mouseY_ = 0.0;
		double prevMouseX_ = 0.0, prevMouseY_ = 0.0;
		double scrollX_ = 0.0, scrollY_ = 0.0;

		/*============================================================================================
		* \brief   Handle keyboard events and update key state.
		* \param[in] key       GLFW key code.
		* \param[in] scancode  Platform-specific scancode.
		* \param[in] action    GLFW action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
		* \param[in] mods      Modifier bitmask (e.g., shift, ctrl, alt).
		*//*========================================================================================*/
		void handleKey(int key, int scancode, int action, int mods);

		/*============================================================================================
		* \brief   Handle mouse button events and update button state.
		* \param[in] button    GLFW mouse button code.
		* \param[in] action    GLFW action (GLFW_PRESS or GLFW_RELEASE).
		* \param[in] mods      Modifier bitmask (e.g., shift, ctrl, alt).
		*//*========================================================================================*/
		void handleMouseButton(int button, int action, int mods);

		/*============================================================================================
		* \brief   Handle mouse movement events and update cursor position.
		* \param[in] xpos  X position of the mouse in screen coordinates.
		* \param[in] ypos  Y position of the mouse in screen coordinates.
		*//*========================================================================================*/
		void handleMouseCursor(double xpos, double ypos);

		/*============================================================================================
		* \brief   Handle scroll events and update scroll offset values.
		* \param[in] xoffset  Horizontal scroll offset.
		* \param[in] yoffset  Vertical scroll offset.
		*//*========================================================================================*/
		void handleMouseScroll(double xoffset, double yoffset);

		/*============================================================================================
		* \brief   Publishes an input event for subscribers to listen to
		* \return  Pair of (xOffset, yOffset).
		*//*========================================================================================*/
		void fireInput(const int&, const int&, const int&, const int&);
	};
}
