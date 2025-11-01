/*============================================================================================
* @file         player_config.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         1-11-2025
* @brief        Configuration loader for player properties. Reads player stats from text files
*               instead of hardcoding values. This allows easy tweaking without recompilation    
*//*========================================================================================*/

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Player 
{
	/*============================================================================================
	* \brief       Structure to hold all player configuration values loaded from file
	*//*========================================================================================*/
	struct PlayerConfig
	{
		// Movement properties
		Math::eVec2<float> velocity;        // Current velocity in pixels per second
		float acceleration;                 // Acceleration when moving (pixels/sec^2)
		float friction;                     // Friction coefficient (0-1, higher = more friction)
		float maxSpeed;                     // Maximum movement speed (pixels/sec)

		bool canMove;				// Initial move state (usually true)

		// Sprite and collider dimensions
		float spriteWidth;			// Visual sprite width in pixels
		float spriteHeight;			// Visual sprite height in pixels
		float colliderWidth;		// Collision box width in pixels
		float colliderHeight;		// Collision box height in pixels

		// Dash properties
		float dashDuration;			// How long dash lasts in seconds
		float dashDistance;			// How far to dash in pixels
		float dashCooldown;			// Cooldown between dashes in seconds

		/*============================================================================================
		* \brief       Default constructor with fallback values if file loading fails
		*//*========================================================================================*/
		PlayerConfig()
			: velocity{ 0.0f, 0.0f }
			, acceleration(800.0f)          // Acceleration rate (pixels/sec^2)
			, friction(8.0f)                // Friction coefficient
			, canMove(true)
			, spriteWidth(48.0f)
			, spriteHeight(48.0f)
			, colliderWidth(48.0f)
			, colliderHeight(48.0f)
			, dashDuration(0.3f)
			, dashDistance(150.0f)
			, dashCooldown(0.5f)
		{
		}

		/*============================================================================================
		* \brief       Load configuration from text file
		* \param[in]   std::string const& filepath - Path to configuration file
		* \return      bool - True if loaded successfully, false if file not found/invalid
		*//*========================================================================================*/
		bool LoadFromFile(const std::string& filepath)
		{
			std::ifstream file(filepath);
			if (!file.is_open())
			{
				std::cerr << "[PlayerConfig] Failed to open config file: " << filepath << std::endl;
				std::cerr << "[PlayerConfig] Using default values instead." << std::endl;
				return false;
			}

			std::string line;
			int lineNum = 0;

			while (std::getline(file, line))
			{
				lineNum++;

				// Skip empty lines and comments (lins starting with # or //)
				if (line.empty() || line[0] == '#' || line.substr(0, 2) == "//") continue;

				// Parse key-value pairs (format: key=value or key = value)
				std::istringstream iss(line);
				std::string key;
				char equals;

				if (!(iss >> key >> equals)) continue; // Skip malformed lines

				if (equals != '=')
				{
					std::cerr << "[PlayerConfig] Line" << lineNum << ": Expected '=' after key '" << key << "'" << std::endl;
					continue;
				}

				// Parse the value based on the key
				if (key == "acceleration")
				{
					if (!(iss >> acceleration))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for acceleration" << std::endl;
				}
				else if (key == "friction") {
					if (!(iss >> friction))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for friction" << std::endl;
				}
				else if (key == "maxSpeed") {
					if (!(iss >> maxSpeed))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for maxSpeed" << std::endl;
				}

				else if (key == "canMove") {
					std::string value;
					if (iss >> value) {
						// Convert string to bool (accepts "true"/"false", "1"/"0", "yes"/"no")
						canMove = (value == "true" || value == "1" || value == "yes" || value == "True" || value == "YES");
					}
				}
				else if (key == "spriteWidth") {
					if (!(iss >> spriteWidth))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for spriteWidth" << std::endl;
				}
				else if (key == "spriteHeight")
				{
					if (!(iss >> spriteHeight))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for spriteHeight" << std::endl;
				}
				else if (key == "colliderWidth")
				{
					if (!(iss >> colliderWidth))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for colliderWidth" << std::endl;
				}
				else if (key == "colliderHeight")
				{
					if (!(iss >> colliderHeight))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for colliderHeight" << std::endl;
				}
				else if (key == "dashDuration")
				{
					if (!(iss >> dashDuration))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for dashDuration" << std::endl;
				}
				else if (key == "dashDistance")
				{
					if (!(iss >> dashDistance))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for dashDistance" << std::endl;
				}
				else if (key == "dashCooldown")
				{
					if (!(iss >> dashCooldown))
						std::cerr << "[PlayerConfig] Line " << lineNum << ": Invalid value for dashCooldown" << std::endl;
				}
				else
				{
					std::cerr << "[PlayerConfig] Line " << lineNum << ": Unknown key '" << key << "'" << std::endl;
				}
			}

			file.close();
			std::cout << "[PlayerConfig] Successfully loaded configuration from: " << filepath << std::endl;
			return true;
		}

		/*============================================================================================
		* \brief       Print current configuration values to console for debugging
		*//*========================================================================================*/
		void PrintConfig() const {
			std::cout << "\n=== Player Configuration ===" << std::endl;
			std::cout << "Can Move: " << (canMove ? "true" : "false") << std::endl;
			std::cout << "Sprite: " << spriteWidth << "x" << spriteHeight << " px" << std::endl;
			std::cout << "Collider: " << colliderWidth << "x" << colliderHeight << " px" << std::endl;
			std::cout << "Dash Duration: " << dashDuration << " s" << std::endl;
			std::cout << "Dash Distance: " << dashDistance << " px" << std::endl;
			std::cout << "Dash Cooldown: " << dashCooldown << " s" << std::endl;
			std::cout << "===========================\n" << std::endl;
		}

	};

} // namespace Player