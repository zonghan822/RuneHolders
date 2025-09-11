#pragma once

// FMOD
#include "fmod.hpp"

// stds
#include <vector>
#include <string>

namespace Resources
{
	class EnoLoader
	{
	public:

		EnoLoader()
		{
			init();
		}

		// Load all asset files
		void init()
		{
			//loadMusic();

			//loadArtAssets();
		}

	private:
		std::string musicPath = "../../assets/sounds/";
	};
}