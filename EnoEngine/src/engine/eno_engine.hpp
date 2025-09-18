#pragma once

// Engine component(s)
#include "../settings/eno_config.hpp"
#include "../resources/eno_loader.hpp"

#include "../window/eno_window.hpp"
#include "../device/eno_device.hpp"

#include "../ecs/entity.hpp"
#include "../component/eno_component_reg.hpp"

#include "../tag/eno_tag_reg.hpp"

#include "../game_instance/eno_game_instance.hpp"

#include "../input/eno_input.hpp"

#include "../event/signals.hpp"

#include "../audio/eno_audio_manager.hpp"

// Utility
#include "../math/math.hpp"

// stds
#include <chrono>
#include <algorithm>


namespace Engine
{
	class EnoEngine
	{
	public:

		void setup();
		void run();

		void createGameInstance();

	private:

		// Config
		EngineConfiguration::EnoConfigManager configMng_; // maybe interface

		// Window & Device
		Window::EnoWindow window_{ configMng_.getWindowDim()};	// interface likely
		//Graphics::EnoDevice device_{ window_ };

		// Event System(s)
		Event::EnoEventBus bus_;

		// Resource Loader
		Resources::EnoLoader loader_{ bus_ };	// interface

		// Input Manager
		Input::EnoInputManager input_{ window_.getWindow(), bus_ };		// interface

		// Component Registry
		ECSComponents::EnoComponentRegistry compReg_;

		// Tag Registry
		Tag::EnoTagRegistry tagReg_;

		// Audio Manager
		Audio::EnoAudioManager audioMng_{ bus_ };		// interface

		// Game Instance
		std::unique_ptr<GameInstance::EnoGameInstance> gameInstance_;
	};
}