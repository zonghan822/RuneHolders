/*============================================================================================
* @file         eno_config.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration file for config manager
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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

namespace Config
{
	inline char engine_version[ENGINE_VER_SIZE];

	class EnoConfigManager 
	{
	private:
		std::string cfgFileName_ = ASSETS_DIR_PATH "settings/config.txt";

		/*============================================================================================
		 * \brief				Map for storing setting strings
		*//*========================================================================================*/
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

		/*============================================================================================
		 * \brief				Read the local config.txt file and store configurations
		*//*========================================================================================*/
		void read();

		/*============================================================================================
		 * \brief				Set engine version
		*//*========================================================================================*/
		void setEngineVer();
	};
}