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

	private:

		// Config
		EngineConfiguration::EnoConfigManager configMng_;

		// Window & Device
		Window::EnoWindow window_{ configMng_.getWindowDim()};
		//Graphics::EnoDevice device_{ window_ };

		// Input Manager
		Input::EnoInputManager input_{ window_.getWindow() };

		// Event system(s)
		Event::EnoEventBus bus_;

		// Resource Loader
		Resources::EnoLoader loader_{ bus_ };

		// Component Registry
		ECSComponents::ComponentRegistry compReg_;

		// Audio Manager
		Audio::EnoAudioManager audioMng_{ bus_ };
	};
}