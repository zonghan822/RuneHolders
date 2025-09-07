#pragma once

// stds
#include <string> 
#include <iostream>


namespace ECSComponents
{
	struct TagComponent
	{
		TagComponent(const char* t)
		{
			strncpy_s(tag, t, sizeof(tag) - 1);
			tag[sizeof(tag) - 1] = '\0';
		}

		TagComponent(const std::string& t)
		{
			strncpy_s(tag, t.c_str(), sizeof(tag) - 1);
			tag[sizeof(tag) - 1] = '\0';
		}

		char tag[16] = "NULL";
	};

	inline std::ostream& operator<<(std::ostream& os, const TagComponent& n)
	{
		std::cout << n.tag << std::endl;
		return os;
	}
}