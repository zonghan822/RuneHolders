#pragma once

// stds
#include <utility>
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_map>
#include "../math/math.hpp"

namespace Config
{
	class EnoConfigManager 
	{
	private:
		std::string cfgFileName_ = ASSETS_DIR_PATH "settings/config.txt";

		// Map for storing setting strings
		std::unordered_map<std::string, int> configMap_ {
			{"Window Width", 0},
			{"Window Height", 0},
		};

		static constexpr uint8_t COLON_OFFSET = 2;

	public:

		EnoConfigManager();

		std::pair<int, int> getWindowDim();

		// Read the local Settings file and
		// store configurations
		void retrieveSettings();

		// Read the local config.txt file
		void read();
	};
}