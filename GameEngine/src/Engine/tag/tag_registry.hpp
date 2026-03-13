/*============================================================================================
* @file         tag_events.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the tag registry
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// Events
#include "tag_events.hpp"

//stds
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>


namespace Tag
{
	class EnoTagRegistry
	{
	private:
		/*============================================================================================
		 * \brief				String represents the tag name
		 *						bool represents the scope (true : global | false : scene)
		*//*========================================================================================*/
		std::unordered_map<std::string, bool> tagMap_;

	public:

		/*============================================================================================
		 * \brief				Print all tags and their scope
		*//*========================================================================================*/
		void print()
		{
			const int tagsPadding = 10;
			const int globalPadding = 10;

			std::cout <<
				std::setw(tagsPadding)
				<< "Tags" << std::setw(globalPadding)
				<< "Global" << std::endl
				<< "___________________________________" << std::endl;
			for (const auto& [key, value] : tagMap_)
			{
				// Center key and values by calculating the
				// size of each text string

				std::cout <<
					std::boolalpha <<
					std::setw(3) << key <<
					std::setw(19) << value <<
					std::endl;
			}
		}

		/*========================================= Events ===========================================

		/*============================================================================================
		 * \brief				Register a tag received
		 * \param				RecordTag event to get the name of the new tag
		*//*========================================================================================*/
		void registerTag(const RecordTag& event)
		{
			tagMap_[event.tag_] = event.scope_;
		}
	};
}