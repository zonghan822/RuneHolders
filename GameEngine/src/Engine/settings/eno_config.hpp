/*============================================================================================
* @file         eno_config.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration file for config manager
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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

#define ENGINE_VER_SIZE 8


namespace EngineConfiguration
{
	inline char engine_version[ENGINE_VER_SIZE];

	class EnoConfigManager : public Sys::ISystem
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

		/*============================================================================================
		 * \brief				Eno Config Constructor
		*//*========================================================================================*/
		EnoConfigManager();

		/*============================================================================================
		 * \brief				Finds the read window dimensions and casts them to int before
		 *						returning them
		 * \return				Window width and height respectively
		*//*========================================================================================*/
		std::pair<int, int> getWindowDim();

		/*============================================================================================
		 * \brief				Read the local Settings file and store configurations
		*//*========================================================================================*/
		void retrieveSettings();

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