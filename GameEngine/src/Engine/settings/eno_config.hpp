#pragma once

// stds
#include <utility>
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_map>

// lib
#include "../math/math.hpp"

#define ENGINE_VER_SIZE 8

namespace Config
{
	inline char engine_version[ENGINE_VER_SIZE];

	class EnoConfigManager 
	{
	private:
		std::string cfgFileName_ = ASSETS_DIR_PATH "settings/config.txt";

		// Map for storing setting strings
		std::unordered_map<std::string, std::string> configMap_ =
		{
			{ "Engine Version", "0"},
			{ "Window Width",	"0"},
			{ "Window Height",	"0"},
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

		// Set engine version
		void setEngineVer();
	};
}