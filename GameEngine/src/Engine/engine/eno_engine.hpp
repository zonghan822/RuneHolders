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
#include "../prefab/prefab.hpp"


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
		void createGameInstance();

		// Provide access to engine systems for game code
		ECS::EntityManager& getEntityManager();
		Audio::EnoAudioManager& getAudioManager() { return audioMng_; }
		Graphics::EnoGraphicsManager& getGraphicsManager() { return *graphicsMng_; }
		Event::EnoEventBus& getEventBus() { return bus_; }
		Resources::EnoLoader& getResourceLoader() { return loader_; }
		EngineConfiguration::EnoConfigManager& getConfigManager() { return configMng_; }
		Window::EnoWindow& getWindow() { return window_; }
		Input::EnoInputManager& getInputManager() { return input_; }

		double GetDeltaTime() const { return dt; }
		double GetFPS() const { return fps; }

	private:
		// Functions added for future use**
		void setupResourceLoading();
		void setupEventSystem();
		void setupGraphicsSystem();
		void updateEngineSystems();
		void update_time(double fpsCalcInt = 1.0);

		// Private member shii / was here before i edited
		EngineConfiguration::EnoConfigManager configMng_;
		Window::EnoWindow window_{ configMng_.getWindowDim() };
		Event::EnoEventBus bus_;
		Resources::EnoLoader loader_{ bus_ };
		Input::EnoInputManager input_{ window_.getWindow(), bus_ };
		ECSComponents::EnoComponentRegistry compReg_;
		Tag::EnoTagRegistry tagReg_;
		Audio::EnoAudioManager audioMng_{ bus_ };
		// Game Instance
		std::unique_ptr<Graphics::EnoGraphicsManager> graphicsMng_;
		std::unique_ptr<GameInstance::EnoGameInstance> gameInstance_;


		// TBD Demo for playing sounds
		void soundSetup();
		void soundPlaying();

		// Time
		double dt = 0.0f;
		double fps = 0;
	};
}