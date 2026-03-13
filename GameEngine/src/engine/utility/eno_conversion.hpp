/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_conversion.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

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

	// Helper for converting wide strings to standard utf-8
	inline std::string wideToUtf8(const std::wstring& wstr)
	{
		if (wstr.empty()) return {};

		// Get number of bytes needed for UTF-8 conversion
		// Allocate a string of suitable size and convert
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string str(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), str.data(), size, NULL, NULL);

		return str;
	}
}