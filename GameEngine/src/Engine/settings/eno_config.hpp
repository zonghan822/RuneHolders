/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_config.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   78%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   17%
* @date         10/3/2025
* @brief        Declaration file for config manager
*//*========================================================================================*/



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
#include "../file/file_system.hpp"

#define ENGINE_VER_SIZE 8

namespace Config
{
	inline char engine_version[ENGINE_VER_SIZE];

	class EnoConfigManager 
	{
	private:
		std::string cfgFileName_ = FileSystem::GetWritableDir().string() + "/settings/config.txt";

		/*============================================================================================
		 * \brief				Map for storing setting strings
		*//*========================================================================================*/
		std::unordered_map<std::string, std::string> configMap_ =
		{
			{ "Engine Version",   "0"},
			{ "Window Title",      "RuneHolders"},
			{ "Window Width",	    "1280"},
			{ "Window Height",	    "720"},
			{ "Fullscreen",        "false"},
			{ "VSync",             "true"},
			{ "Tilemap Width",     "64"},
			{ "Tilemap Height",    "64"},
			{ "Tilemap CellSize",  "64"},
		};

		static constexpr uint8_t COLON_OFFSET = 2;

	public:

		/*============================================================================================
		 * \brief				Finds the read window dimensions and casts them to int before
		 *						returning them
		 * \return				Window width and height respectively
		*//*========================================================================================*/
		std::pair<int, int> getWindowDim();

		std::string getWindowTitle() const;

		std::tuple<int, int, float> getTilemapConfig();

		bool getFullscreen() const;

		bool getVSync() const;

		/*============================================================================================
		 * \brief				Read the local Settings file and store configurations
		*//*========================================================================================*/
		void retrieveSettings();

		/*============================================================================================
		 * \brief				Read the local config.txt file and store configurations
		*//*========================================================================================*/
		void read();

		/*============================================================================================
		 * \brief				Set engine version
		*//*========================================================================================*/
		void setEngineVer();

		/*============================================================================================
		 * \brief				Update a single key in the in-memory config map
		*//*========================================================================================*/
		void set(const std::string& key, const std::string& value) { configMap_[key] = value; }

		/*============================================================================================
		 * \brief				Read a single key from the in-memory config map
		 * \return				Value string, or defaultVal if the key is absent
		*//*========================================================================================*/
		std::string get(const std::string& key, const std::string& defaultVal = "") const
		{
			auto it = configMap_.find(key);
			return (it != configMap_.end()) ? it->second : defaultVal;
		}

		/*============================================================================================
		 * \brief				Write the current config map back to config.txt,
		 *						preserving line order and any unrecognised lines.
		*//*========================================================================================*/
		void write();
	};
}