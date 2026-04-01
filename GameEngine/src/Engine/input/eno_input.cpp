/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_input.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   64%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   18%
* @co-author    Caven Koh (caven.koh@digipen.edu)   18%
* @date         03-10-2025
* @brief        Implementation of input management system with keyboard and mouse handling
*//*========================================================================================*/



#include "eno_input.hpp"
#include <imgui.h>

namespace Input {

	EnoInputManager::EnoInputManager(Event::EnoEventBus& bus) : bus_{ bus } { /*empty by design*/ }

	void EnoInputManager::initialize() {
		bus_.subscribe<Window::KeyEvent>([this](const Window::KeyEvent& e) {
			handleKey(e.key, e.scancode, e.action, e.mods);
			});

		bus_.subscribe<Window::MouseButtonEvent>([this](const Window::MouseButtonEvent& e) {
			handleMouseButton(e.button, e.action, e.mods);
			});

		bus_.subscribe<Window::MouseCursorEvent>([this](const Window::MouseCursorEvent& e) {
			handleMouseCursor(e.xpos, e.ypos);
			});

		bus_.subscribe<Window::ScrollEvent>([this](const Window::ScrollEvent& e) {
			handleMouseScroll(e.xoffset, e.yoffset);
			});
	}

	void EnoInputManager::endFrame() {
		prevKeyStates_ = keyStates_;
		prevMouseButtonStates_ = mouseButtonStates_;
		prevMouseX_ = mouseX_;
		prevMouseY_ = mouseY_;

		// Reset scroll for next frame
		scrollX_ = 0.0;
		scrollY_ = 0.0;

		// Poll gamepad state
		prevGamepadState_ = gamepadState_;
		gamepadConnected_ = (glfwJoystickPresent(GLFW_JOYSTICK_1) &&
		                     glfwJoystickIsGamepad(GLFW_JOYSTICK_1) &&
		                     glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepadState_));
	}

	void EnoInputManager::clearAllKeyStates() {
		// Clear all key pressed states
		for (auto& pair : keyStates_) {
			pair.second = false;
		}

		// Clear mouse button states
		for (auto& pair : mouseButtonStates_) {
			pair.second = false;
		}
	}

	void EnoInputManager::setupKey(const std::string& inputName, int key, int scancode, int action, int mods)
	{
		inputMap_.insert(inputName, KeyBinding{ key, scancode, action, mods });
	}

	bool EnoInputManager::isKeyPressed(int key) const {
		auto it = keyStates_.find(key);
		return it != keyStates_.end() && it->second;
	}

	bool EnoInputManager::isKeyDown(int key) const {
		auto it = keyStates_.find(key);
		auto prev = prevKeyStates_.find(key);

		bool currPressed = (it != keyStates_.end() && it->second);
		bool prevPressed = (prev != prevKeyStates_.end() && prev->second);

		return currPressed && !prevPressed; // pressed this frame
	}

	bool EnoInputManager::isKeyUp(int key) const {
		auto it = keyStates_.find(key);
		auto prev = prevKeyStates_.find(key);

		bool currPressed = (it != keyStates_.end() && it->second);
		bool prevPressed = (prev != prevKeyStates_.end() && prev->second);

		return !currPressed && prevPressed; // released this frame
	}

	bool EnoInputManager::isModifierActive(int mod) const {
		return (modsState_ & mod) != 0;
	}

	int EnoInputManager::currentMods() const {
		return modsState_;
	}

	bool EnoInputManager::isKeyComboPressed(int key, int requiredMods) const {
		return isKeyPressed(key) && (modsState_ & requiredMods) == requiredMods;
	}

	bool EnoInputManager::isMousePressed(int button) const {
		auto it = mouseButtonStates_.find(button);
		return it != mouseButtonStates_.end() && it->second;
	}

	bool EnoInputManager::isMouseDown(int button) const {
		bool curr = mouseButtonStates_.count(button) ? mouseButtonStates_.at(button) : false;
		bool prev = prevMouseButtonStates_.count(button) ? prevMouseButtonStates_.at(button) : false;
		return curr && !prev;
	}

	bool EnoInputManager::isMouseUp(int button) const {
		bool curr = mouseButtonStates_.count(button) ? mouseButtonStates_.at(button) : false;
		bool prev = prevMouseButtonStates_.count(button) ? prevMouseButtonStates_.at(button) : false;
		return !curr && prev;
	}

	bool EnoInputManager::isMouseCombo(int button, int requiredMods) const {
		return isMousePressed(button) && (modsState_ & requiredMods) == requiredMods;
	}

	std::pair<double, double> EnoInputManager::getMousePosition() const {
		return { mouseX_, mouseY_ };
	}

	std::pair<double, double> EnoInputManager::getMouseDelta() const {
		return { mouseX_ - prevMouseX_, mouseY_ - prevMouseY_ };
	}

	std::pair<double, double> EnoInputManager::getScrollOffset() const {
		return { scrollX_, scrollY_ };
	}

	// Handle key inputs
	void EnoInputManager::handleKey(int key, int scancode, int action, int mods)
	{
		modsState_ = mods;

		// When ImGui is consuming text input (e.g. an InputText is active),
		// only process releases so keys do not get stuck; skip all other input.
		//if (ImGui::GetIO().WantTextInput) {
		//	if (GLFW_RELEASE == action) {
		//		keyStates_[key] = false;
		//	}
		//	return;
		//}

		if (GLFW_PRESS == action) {
			keyStates_[key] = true;
		}
		else if (GLFW_RELEASE == action) {
			keyStates_[key] = false;
		}

		auto name = inputMap_.findr(KeyBinding{ key, scancode, action, mods });
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

		fireInput(key, scancode, action, mods);
	}

	void EnoInputManager::handleMouseButton(int button, int action, int mods) {
		if (GLFW_PRESS == action) {
			mouseButtonStates_[button] = true;
		}
		else if (GLFW_RELEASE == action) {
			mouseButtonStates_[button] = false;
		}

		modsState_ = mods;

		// event bus not implemented here yet...
	}

	void EnoInputManager::handleMouseCursor(double xpos, double ypos) {
		mouseX_ = xpos;
		mouseY_ = ypos;
	}

	void EnoInputManager::handleMouseScroll(double xoffset, double yoffset) {
		scrollX_ = xoffset;
		scrollY_ = yoffset;
	}

	void EnoInputManager::fireInput(const int& key, const int& scancode, const int& action, const int& mods)
	{
		bus_.publish<InputSent>(InputSent{key, scancode, action, mods});
	}

	// ============= Gamepad =============

	bool EnoInputManager::isGamepadConnected() const {
		return gamepadConnected_;
	}

	float EnoInputManager::getGamepadAxis(int axis) const {
		if (!gamepadConnected_ || axis < 0 || axis > 5) return 0.0f;
		return gamepadState_.axes[axis];
	}

	bool EnoInputManager::isGamepadButtonHeld(int button) const {
		if (!gamepadConnected_ || button < 0 || button > 14) return false;
		return gamepadState_.buttons[button] == GLFW_PRESS;
	}

	bool EnoInputManager::isGamepadButtonPressed(int button) const {
		if (!gamepadConnected_ || button < 0 || button > 14) return false;
		return gamepadState_.buttons[button] == GLFW_PRESS &&
		       prevGamepadState_.buttons[button] == GLFW_RELEASE;
	}

	bool EnoInputManager::isGamepadButtonReleased(int button) const {
		if (!gamepadConnected_ || button < 0 || button > 14) return false;
		return gamepadState_.buttons[button] == GLFW_RELEASE &&
		       prevGamepadState_.buttons[button] == GLFW_PRESS;
	}
}