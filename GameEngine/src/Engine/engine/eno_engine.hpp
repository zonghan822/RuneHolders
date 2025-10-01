#pragma once
// Engine component(s)
#include "../graphics/graphics.hpp"
#include "../settings/eno_config.hpp"
#include "../resources/eno_loader.hpp"
#include "../window/eno_window.hpp"
#include "../input/eno_input.hpp"
#include "../ecs/entity.hpp"
#include "../component/component_registry.hpp"
#include "../event/signals.hpp"
#include "../audio/eno_audio_manager.hpp"
#include "../math/math.hpp"
#include "../tag/tag_registry.hpp"
#include "../game_instance/eno_game_instance.hpp"
#include "../collision/collision.hpp"
#include "../prefab/prefab.hpp"
#include "../crashlog/crashlogger.hpp"


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
		void run(std::function<void()> gameUpdateCallback);
		void shutDown();

		// Provide access to engine systems for game code
		ECS::EntityManager& getEntityManager() { return entityMng_; }
		Audio::EnoAudioManager& getAudioManager() { return audioMng_; }
		Graphics::EnoGraphicsManager& getGraphicsManager() { return graphicsMng_; }
		Event::EnoEventBus& getEventBus() { return bus_; }
		Resources::EnoLoader& getResourceLoader() { return loader_; }
		Config::EnoConfigManager& getConfigManager() { return configMng_; }
		Window::EnoWindow& getWindowManager() { return window_; }
		Input::EnoInputManager& getInputManager() { return inputMng_; }

		double getDeltaTime() const { return dt_; }
		double getFPS() const { return fps_; }

	private:
		// Functions added for future use**
		void setupResourceLoading();
		void setupEventSystem();
		void updateEngineSystems();
		void updateTime(double fpsCalcInt = 1.0);

		// Services & Tools
		Config::EnoConfigManager configMng_;
		Event::EnoEventBus bus_;
		Window::EnoWindow window_{ bus_ };
		Resources::EnoLoader loader_{ bus_ };
		ECS::EntityManager entityMng_;
		
		// Systems
		Input::EnoInputManager inputMng_{ bus_ };
		Graphics::EnoGraphicsManager graphicsMng_{ bus_ };
		Audio::EnoAudioManager audioMng_{ bus_ };
		
		// TBD Demo for playing sounds
		void soundSetup();
		void soundPlaying();

		// Time
		double dt_ = 0.0;
		double fps_ = 0.0;
	};
}