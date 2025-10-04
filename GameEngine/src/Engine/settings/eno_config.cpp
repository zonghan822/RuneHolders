/*============================================================================================
* @file         eno_config.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Definition file for config manager
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


#include "eno_config.hpp"

namespace Config {

	EnoConfigManager::EnoConfigManager() {
		retrieveSettings();
	}

	std::pair<int, int> EnoConfigManager::getWindowDim() 
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
	void EnoConfigManager::retrieveSettings() {
		read();
		setEngineVer();
	}

	// Read the local config.txt file
	void EnoConfigManager::read() {
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

	void EnoConfigManager::setEngineVer() {

		auto it = configMap_.find("Engine Version");
		if (it == configMap_.end())
		{
			throw std::runtime_error("Engine version not found!");
		}

		// Copy into 
		std::strncpy(engine_version, it->second.c_str(), sizeof(engine_version) - 1);
		engine_version[sizeof(engine_version) - 1] = '\0';
	}
}