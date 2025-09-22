 #pragma once

// FMOD
#include "fmod.hpp"

// stds
#include <string>


namespace Audio
{
	struct RequestSound
	{
		std::string sound_;
	};

	struct LoadSound
	{
		std::string soundName_;
		FMOD::Sound* sound_;
	};
}