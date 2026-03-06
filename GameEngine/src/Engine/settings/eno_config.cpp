/*============================================================================================
* @file         eno_config.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Definition file for config manager
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


#include "eno_config.hpp"

namespace Config {

	std::pair<int, int> EnoConfigManager::getWindowDim()
	{
		auto it  = configMap_.find("Window Width");
		auto it2 = configMap_.find("Window Height");
		int w = (it  != configMap_.end() && !it->second.empty())  ? std::stoi(it->second)  : 1280;
		int h = (it2 != configMap_.end() && !it2->second.empty()) ? std::stoi(it2->second) : 720;
		return { w, h };
	}

	std::string EnoConfigManager::getWindowTitle() const
	{
		auto it = configMap_.find("Window Title");
		if (it != configMap_.end() && !it->second.empty())
			return it->second;
		return "RuneHolders";
	}

	std::tuple<int, int, float> EnoConfigManager::getTilemapConfig()
	{
		auto itW = configMap_.find("Tilemap Width");
		auto itH = configMap_.find("Tilemap Height");
		auto itC = configMap_.find("Tilemap CellSize");
		int   w    = (itW != configMap_.end() && !itW->second.empty()) ? std::stoi(itW->second) : 64;
		int   h    = (itH != configMap_.end() && !itH->second.empty()) ? std::stoi(itH->second) : 64;
		float cell = (itC != configMap_.end() && !itC->second.empty()) ? std::stof(itC->second) : 64.f;
		return { w, h, cell };
	}

	bool EnoConfigManager::getFullscreen() const
	{
		auto it = configMap_.find("Fullscreen");
		if (it == configMap_.end()) return false;
		return (it->second == "true" || it->second == "TRUE" || it->second == "True");
	}

	bool EnoConfigManager::getVSync() const
	{
		auto it = configMap_.find("VSync");
		if (it == configMap_.end()) return true;
		return (it->second == "true" || it->second == "TRUE" || it->second == "True");
	}

	void EnoConfigManager::retrieveSettings() {
		read();
		setEngineVer();
	}

	// Reads config.txt and updates configMap_ entries that match.
	// Silently keeps defaults if the file is missing, unreadable, or a field is absent.
	void EnoConfigManager::read() {
		if (!std::filesystem::exists(cfgFileName_))
		{
			std::cout << "[Config] Settings file not found, using defaults: " << cfgFileName_ << "\n";
			return;
		}

		std::ifstream settingsFile(cfgFileName_);
		if (!settingsFile)
		{
			std::cout << "[Config] Settings file could not be opened, using defaults: " << cfgFileName_ << "\n";
			return;
		}

		std::string input;
		while (std::getline(settingsFile, input))
		{
			for (auto& pair : configMap_)
			{
				if (input.find(pair.first) != std::string::npos)
				{
					std::size_t pos = input.find(pair.first);
					pair.second = input.substr(pos + std::string(pair.first).size() + COLON_OFFSET);
				}
			}
		}
	}

	void EnoConfigManager::setEngineVer() {
		auto it = configMap_.find("Engine Version");
		if (it == configMap_.end()) return;
		std::strncpy(engine_version, it->second.c_str(), sizeof(engine_version) - 1);
		engine_version[sizeof(engine_version) - 1] = '\0';
	}
}