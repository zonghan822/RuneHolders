#pragma once


// Engine components
#include "../window/eno_window.hpp"
#include "../device/eno_device.hpp"

#include "../ecs/entity.hpp"
#include "../component/componentRegistry.hpp"

#include "../event/signals.hpp"

#include "../audio/eno_audio_manager.hpp"

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
		//Window::EnoWindow window_;
		//Graphics::EnoDevice device_{ window_ };

		ECSComponents::ComponentRegistry compReg_;

		Sound::EnoAudioManager audioMng_;
	};
}