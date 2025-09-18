#pragma once

// GLFW
#include <GLFW/glfw3.h>

// Engine Component(s)
#include "../event/eno_event_bus.hpp"

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
		EnoInputManager(GLFWwindow* window, Event::EnoEventBus& bus) : bus_ { bus }
		{
			glfwSetWindowUserPointer(window, this);

			glfwSetKeyCallback(window, KeyCallback);
		}

		// Set up a key in the input map
		void setupKey(const std::string& inputName, int key = 0, int scancode = 0, int action = 0, int mods = 0)
		{
			inputMap_.insert(inputName, KeyBinding{ key, scancode, action, mods });
		}

		template <typename EventType>
		void setupEvent(const std::string& inputName)
		{
			eventMap_[inputName] = [this]() { bus_.publish(EventType{}); };
		}

	private:
		Event::EnoEventBus& bus_;

		Utility::EnoBiMap<
			std::string, KeyBinding,
			std::hash<std::string>, std::equal_to<std::string>,
			KeyBindingHasher, std::equal_to<KeyBinding>> inputMap_;

		std::unordered_map<std::string, std::function<void()>> eventMap_;

		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			// Delegate to a non-static function to handle input
			auto mgr = static_cast<EnoInputManager*>(glfwGetWindowUserPointer(window));
			mgr->handleKey(key, scancode, action, mods);
		}

		// Handle key inputs
		void handleKey(int key, int scancode, int action, int mods)
		{
			auto name = inputMap_.findk(KeyBinding{ key, scancode, action, mods });
			if (name.has_value())
			{
				// Handle publishing events
				auto it = eventMap_.find(name.value());
				if (it == eventMap_.end())
				{
					throw std::runtime_error("No Event is associated with this input!");
				}

				it->second();
			}
		}
	};
}
