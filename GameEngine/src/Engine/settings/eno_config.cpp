#include "eno_config.hpp"

namespace Config {

	EnoConfigManager::EnoConfigManager() {
		retrieveSettings();
	}

	std::pair<int, int> EnoConfigManager::getWindowDim() {
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
	void EnoConfigManager::retrieveSettings()
	{
		read();
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
}