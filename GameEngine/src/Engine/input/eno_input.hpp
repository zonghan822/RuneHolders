#pragma once

// GLFW
#include <GLFW/glfw3.h>

// Engine Component(s)
#include "../event/event_bus.hpp"

// Utility
#include "../utility/eno_bimap.hpp"

// Events
#include "../game_instance/game_instance_events.hpp"

// stds
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>


namespace Input
{
	struct KeyBinding
	{
		int key_;
		int scancode_;
		int action_;
		int mods_;

		bool operator==(const KeyBinding& other) const
		{
			return	key_ == other.key_ &&
				scancode_ == other.scancode_ &&
				action_ == other.action_ &&
				mods_ == other.mods_;
		}
	};

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

	class EnoInputManager
	{
	public:
		EnoInputManager(GLFWwindow* window, Event::EnoEventBus& bus);

		// Set up a key in the input map
		void setupKey(const std::string& inputName, int key = 0, int scancode = 0, int action = 0, int mods = 0);

		template <typename EventType>
		void setupEvent(const std::string& inputName);

		void endFrame();

		/****************************************************
		* Keyboard											*
		*****************************************************/
		bool isKeyPressed(int key) const;

		bool isKeyDown(int key) const;

		bool isKeyUp(int key) const;

		bool isModifierActive(int mod) const;

		bool isKeyComboPressed(int key, int requiredMods) const;

		/****************************************************
		* Mouse												*
		*****************************************************/
		bool isMousePressed(int button) const;

		bool isMouseDown(int button) const;

		bool isMouseUp(int button) const;

		bool isMouseCombo(int button, int requiredMods) const;

		// Mouse position
		std::pair<double, double> getMousePosition() const;

		// Mouse delta since last frame
		std::pair<double, double> getMouseDelta() const;

		// Scroll offsets (reset each frame)
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

		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void MousePosCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


		// Handle key inputs
		void handleKey(int key, int scancode, int action, int mods);
		void handleMouseButton(int button, int action, int mods);
		void handleMouseCursor(double xpos, double ypos);
		void handleMouseScroll(double xoffset, double yoffset);

	};
}
