#pragma once

// stds
#include <utility>
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_map>


namespace EngineConfiguration
{
	class EnoConfigManager
	{
	private:
		std::string cfgFileName_ = "assets/settings/config.txt";

		// Map for storing setting strings
		std::unordered_map<std::string, int> configMap_ =
		{
			{"Window Width", 0},
			{"Window Height", 0},
		};

		static constexpr uint8_t COLON_OFFSET = 2;

	public:

		EnoConfigManager()
		{
			retrieveSettings();
		}

		std::pair<int, int> getWindowDim()
		{
			auto it = configMap_.find("Window Width");
			auto it2 = configMap_.find("Window Height");
			if (it == configMap_.end() || it2 == configMap_.end())
			{
				throw std::runtime_error("Failed to retrieve window width and/or height value!");
			}

			return { it->second, it2->second };
		}

		// Read the local Settings file and
		// store configurations
		void retrieveSettings()
		{
			read();
		}

		// Read and extract from config.txt
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
							pair.second = std::stoi(input.substr(pos + std::string(pair.first).size() + COLON_OFFSET));
						}
					}
				}
			}
			else
			{
				throw std::runtime_error("Settings file does not exist!");
			}
		}
	};
}