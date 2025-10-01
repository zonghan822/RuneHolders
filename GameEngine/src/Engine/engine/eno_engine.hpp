#pragma once
// Engine component(s)
#include "../graphics/graphics.hpp"
#include "../settings/eno_config.hpp"
#include "../resources/eno_loader.hpp"
#include "../window/eno_window.hpp"
#include "../input/eno_input.hpp"
#include "../ecs/entity.hpp"
#include "../component/componentRegistry.hpp"
#include "../event/signals.hpp"
#include "../audio/eno_audio_manager.hpp"
#include "../math/math.hpp"
#include "../tag/tagRegistry.hpp"
#include "../game_instance/eno_game_instance.hpp"
#include "../collision/collision.hpp"


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
		EngineConfiguration::EnoConfigManager& getConfigManager() { return configMng_; }
		Window::EnoWindow& getWindowManager() { return windowMng_; }
		Input::EnoInputManager& getInputManager() { return inputMng_; }

		double getDeltaTime() const { return dt_; }
		double getFPS() const { return fps_; }

	private:
		// Functions added for future use**
		void setupResourceLoading();
		void setupEventSystem();
		void setupGraphicsSystem();
		void updateEngineSystems();
		void updateTime(double fpsCalcInt = 1.0);

		// Services & Tools
		EngineConfiguration::EnoConfigManager configMng_;
		Window::EnoWindow windowMng_{ configMng_.getWindowDim() };
		Event::EnoEventBus bus_;
		Resources::EnoLoader loader_{ bus_ };
		ECS::EntityManager entityMng_;
		
		// Systems
		Input::EnoInputManager inputMng_{ windowMng_.getWindow(), bus_ };
		Graphics::EnoGraphicsManager graphicsMng_{ bus_, windowMng_.getWindow() };
		Audio::EnoAudioManager audioMng_{ bus_ };
		
		// TBD Demo for playing sounds
		void soundSetup();
		void soundPlaying();

		// Time
		double dt_ = 0.0;
		double fps_ = 0.0;
	};
}