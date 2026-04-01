/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_config.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   73%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   13%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   13%
* @date         10/3/2025
* @brief        Definition file for config manager
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
			// First launch: copy config from install directory if available
			std::string installCfg = FileSystem::GetAssetsDir().string() + "/settings/config.txt";
			if (cfgFileName_ != installCfg && std::filesystem::exists(installCfg))
			{
				std::filesystem::create_directories(
					std::filesystem::path(cfgFileName_).parent_path());
				std::filesystem::copy_file(installCfg, cfgFileName_);
				std::cout << "[Config] Copied config from install dir to: " << cfgFileName_ << "\n";
			}
			else
			{
				std::cout << "[Config] Settings file not found, using defaults: " << cfgFileName_ << "\n";
				return;
			}
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
			// Parse every "Key: Value" line and upsert into the map.
			// This ensures keys not present in the default map (e.g. "Audio Master")
			// are also loaded, so saved audio volumes survive program restarts.
			auto colon = input.find(": ");
			if (colon == std::string::npos) continue;
			std::string key = input.substr(0, colon);
			std::string val = input.substr(colon + COLON_OFFSET);
			configMap_[key] = val;
		}
	}

	void EnoConfigManager::write() {
		// Read existing lines so we preserve ordering and unknown entries
		std::vector<std::string> lines;
		{
			std::ifstream in(cfgFileName_);
			std::string line;
			while (std::getline(in, line))
				lines.push_back(line);
		}

		// Replace lines whose key prefix matches a known config entry
		for (auto& line : lines) {
			for (auto& [key, val] : configMap_) {
				if (line.rfind(key + ":", 0) == 0) {
					line = key + ": " + val;
					break;
				}
			}
		}

		// Append any config keys not yet present in the file
		for (auto& [key, val] : configMap_) {
			bool found = false;
			for (const auto& line : lines)
				if (line.rfind(key + ":", 0) == 0) { found = true; break; }
			if (!found)
				lines.push_back(key + ": " + val);
		}

		std::filesystem::create_directories(
			std::filesystem::path(cfgFileName_).parent_path());
		std::ofstream out(cfgFileName_);
		for (auto& l : lines)
			out << l << "\n";
	}

	void EnoConfigManager::setEngineVer() {
		auto it = configMap_.find("Engine Version");
		if (it == configMap_.end()) return;
		std::strncpy(engine_version, it->second.c_str(), sizeof(engine_version) - 1);
		engine_version[sizeof(engine_version) - 1] = '\0';
	}
}