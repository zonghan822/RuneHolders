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
		// string represents the tag name
		// bool represents the scope (true : global | false : scene)
		std::unordered_map<std::string, bool> tagMap_;

	public:

		// Print all tags and their scope
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

		// ----------------------- Events -----------------------
		// Register a tag and its corresponding scope
		void registerTag(const RecordTag& event)
		{
			tagMap_[event.tag_] = event.scope_;
		}

	private:

		std::string printPadding(const std::string& str, int width)
		{

		}
	};
}