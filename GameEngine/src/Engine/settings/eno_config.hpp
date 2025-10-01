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

		std::pair<int, int> getWindowDim()
		{
			auto it = configMap_.find("Window Width");
			auto it2 = configMap_.find("Window Height");
			if (it == configMap_.end() || it2 == configMap_.end())
			{
				throw std::runtime_error("Failed to retrieve window width and/or height value!");
			}

			return { std::stoi(it->second), std::stoi(it2->second) };
		}

		// Read the local Settings file and
		// store configurations
		void retrieveSettings()
		{
			read();

			setEngineVer();
		}

		// Read the local config.txt file
		void read()
		{
			if (std::filesystem::exists(cfgFileName_))
			{
				std::ifstream settingsFile(cfgFileName_);

				if (!settingsFile)
				{
					throw std::runtime_error("Settings file could not be opened!");
				}

				std::string input;
				while (std::getline(settingsFile, input))
				{
					for (auto& pair : configMap_)
					{
						// Find settings strings
						// and store their respective value
						if (input.find(pair.first) != std::string::npos)
						{
							std::size_t pos = input.find(pair.first);
							pair.second = input.substr(pos + std::string(pair.first).size() + COLON_OFFSET);
						}
					}
				}
			}
			else
			{
				throw std::runtime_error("Settings file does not exist!");
			}
		}

		// Set engine version
		void setEngineVer()
		{
			auto it = configMap_.find("Engine Version");
			if (it == configMap_.end())
			{
				throw std::runtime_error("Engine version not found!");
			}

			// Copy into 
			std::strncpy(engine_version, it->second.c_str(), sizeof(engine_version) - 1);
			engine_version[sizeof(engine_version) - 1] = '\0';
		}
	};
}