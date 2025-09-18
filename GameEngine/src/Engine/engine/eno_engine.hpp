#pragma once
// Engine component(s)
#include "../settings/eno_config.hpp"
#include "../resources/eno_loader.hpp"
#include "../window/eno_window.hpp"
#include "../input/eno_input.hpp"
#include "../ecs/entity.hpp"
#include "../component/componentRegistry.hpp"
#include "../event/signals.hpp"
#include "../audio/eno_audio_manager.hpp"
#include "../math/math.hpp"

// stds
#include <functional>
#include <chrono>
#include <algorithm>

namespace Engine
{
	class EnoEngine
	{
	public:
		void setup();
		void run();
		void run(std::function<void()> gameUpdateCallback); // Overloaded version

		// Provide access to engine systems for game code
		ECS::EntityManager& getEntityManager();
		Audio::EnoAudioManager& getAudioManager() { return audioMng_; }
		Event::EnoEventBus& getEventBus() { return bus_; }
		Resources::EnoLoader& getResourceLoader() { return loader_; }
		EngineConfiguration::EnoConfigManager& getConfigManager() { return configMng_; }
		Window::EnoWindow& getWindow() { return window_; }

	private:
		// Functions added for future use**
		void setupResourceLoading();
		void setupEventSystem();
		void updateEngineSystems();

		// Private member shii / was here before i edited
		EngineConfiguration::EnoConfigManager configMng_;
		Window::EnoWindow window_{ configMng_.getWindowDim() };
		Input::EnoInputManager input_{ window_.getWindow() };
		Event::EnoEventBus bus_;
		Resources::EnoLoader loader_{ bus_ };
		ECSComponents::ComponentRegistry compReg_;
		Audio::EnoAudioManager audioMng_{ bus_ };
	};
}