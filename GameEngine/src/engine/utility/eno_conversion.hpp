#pragma once

// stds
#include <algorithm>
#include <cctype>
#include <string>


namespace Conversion
{
	inline void strToLower(std::string& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](char c)
			{
				return std::tolower(c);
			});
	}
}