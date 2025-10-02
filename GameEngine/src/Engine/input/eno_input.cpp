#include "eno_input.hpp"

namespace Input {

	EnoInputManager::EnoInputManager(GLFWwindow* window, Event::EnoEventBus& bus) : bus_{ bus }
	{
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwSetCursorPosCallback(window, MousePosCallback);
		glfwSetScrollCallback(window, MouseScrollCallback);
	}

	void EnoInputManager::endFrame() {
		prevKeyStates_ = keyStates_;
		prevMouseButtonStates_ = mouseButtonStates_;
		prevMouseX_ = mouseX_;
		prevMouseY_ = mouseY_;

		// Reset scroll for next frame
		scrollX_ = 0.0;
		scrollY_ = 0.0;
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

	void EnoInputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		// Delegate to a non-static function to handle input
		auto mgr = static_cast<EnoInputManager*>(glfwGetWindowUserPointer(window));
		mgr->handleKey(key, scancode, action, mods);
	}

	void EnoInputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto mgr = static_cast<EnoInputManager*>(glfwGetWindowUserPointer(window));
		mgr->handleMouseButton(button, action, mods);
	}

	void EnoInputManager::MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
		auto mgr = static_cast<EnoInputManager*>(glfwGetWindowUserPointer(window));
		mgr->handleMouseCursor(xpos, ypos);
	}

	void EnoInputManager::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		auto mgr = static_cast<EnoInputManager*>(glfwGetWindowUserPointer(window));
		mgr->handleMouseScroll(xoffset, yoffset);
	}

	// Handle key inputs
	void EnoInputManager::handleKey(int key, int scancode, int action, int mods)
	{
		if (GLFW_PRESS == action) {
			keyStates_[key] = true;
		}
		else if (GLFW_RELEASE == action) {
			keyStates_[key] = false;
		}

		modsState_ = mods;

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
}