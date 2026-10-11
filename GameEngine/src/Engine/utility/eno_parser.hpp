/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_parser.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   80%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   20%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// Value type(s) to parse
#include "../ecs/eno_data_types.hpp"

// stds
#include <algorithm>
#include <stdexcept>
#include <fstream>

namespace Parser
{
	// Helper for creating a DataField that was specified in the save file
	inline ECSComponents::DataField deduceValue(const std::string& value, const std::string& type)
	{
		auto trim = [](const std::string& str) -> std::string
			{
				size_t start = str.find_first_not_of(" \t\n\r");
				if (start == std::string::npos) return "";

				size_t end = str.find_last_not_of(" \t\n\r");
				return str.substr(start, end - start + 1);
			};

		std::string tvalue = trim(value);
		std::string ttype = trim(type);

		// Convert to lowercase for consistency
		std::transform(std::begin(ttype), std::end(ttype), std::begin(ttype), ::tolower);

		// int
		if (ttype == "int")
			return std::stoi(tvalue);

		// uint32_t
		if (ttype == "uint32_t")
			return static_cast<uint32_t>(std::stoul(tvalue));

		// float
		if (ttype == "float")
			return std::stof(tvalue);

		// double
		if (ttype == "double")
			return std::stod(tvalue);

		// bool
		if (ttype == "bool")
		{
			if (tvalue == "true" || tvalue == "1") return true;
			if (tvalue == "false" || tvalue == "0") return false;

			throw std::runtime_error("Unknown string set as bool value in save!");
		}

		// char[32]
		if (ttype == "char32")
		{
			return tvalue;
		}

		// char[256]
		if (ttype == "char256")
		{
			return tvalue;
		}

		// char[64]
		if (ttype == "char64")
		{
			return tvalue;
		}

		// ---------------- Enums ----------------

		// Color
		if (ttype == "color")
		{
			const std::unordered_map<std::string, Color> colorMap =
			{
				{ "Green", Color::Green },
				{ "Red", Color::Red }
			};

			auto it = colorMap.find(tvalue);
			if (it != colorMap.end())
			{
				return it->second;
			}
		}

		// Collider Type
		if (ttype == "collidertype")
		{
			const std::unordered_map<std::string, ECSComponents::ColliderType> colliderMap =
			{
				{ "Circle",			ECSComponents::ColliderType::CIRCLE },
				{ "Line",			ECSComponents::ColliderType::LINE },
				{ "Point",			ECSComponents::ColliderType::POINT },
				{ "Rectangle",		ECSComponents::ColliderType::RECTANGLE },
			};

			auto it = colliderMap.find(tvalue);
			if (it != colliderMap.end())
			{
				return it->second;
			}
		}

		// Loop Mode
		if (ttype == "loopmode")
		{
			const std::unordered_map<std::string, Audio::EnoAudioModule::LoopMode> loopMap =
			{
				{ "Disabled",	Audio::EnoAudioModule::LoopMode::Disabled },
				{ "Forward",	Audio::EnoAudioModule::LoopMode::Forward },
				{ "PingPong",	Audio::EnoAudioModule::LoopMode::PingPong },
			};

			auto it = loopMap.find(tvalue);
			if (it != loopMap.end())
			{
				return it->second;
			}
		}

		// ------------- Custom Math -------------
		// eVec2<float>
		if (ttype == "evec2(float)")
		{
			// First value
			float fv{}, sv{};
			size_t vstart = tvalue.find_first_of(':');
			size_t vend{};
			if (vstart != std::string::npos)
			{
				++vstart;

				// skip whitespaces to value
				while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;

				vend = tvalue.find_first_of('|', vstart);
				if (vend != std::string::npos)
				{
					fv = std::stof(tvalue.substr(vstart, vend - vstart));
				}
			}

			// Second value
			vstart = tvalue.find(':', vstart);
			if (vstart != std::string::npos)
			{
				++vstart;

				// skip whitespaces to value
				while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;

				vend = tvalue.find_first_of('|', vend + 1);
				if (vend != std::string::npos)
				{
					sv = std::stof(tvalue.substr(vstart, vend - vstart));
				}
				else if (vend == std::string::npos)
				{
					sv = std::stof(tvalue.substr(vstart));
				}
			}

			return Math::eVec2{ fv, sv };
		}

		// Graphics::Color (RGBA struct)
		if (ttype == "color(rgba)")
		{
			float r{}, g{}, b{}, a{};
			size_t vstart{}, vend{};

			// Parse r value
			vstart = tvalue.find_first_of(':');
			if (vstart != std::string::npos)
			{
				++vstart;
				while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
				vend = tvalue.find_first_of('|', vstart);
				if (vend != std::string::npos)
					r = std::stof(tvalue.substr(vstart, vend - vstart));
			}

			// Parse g value
			vstart = tvalue.find(':', vend);
			if (vstart != std::string::npos)
			{
				++vstart;
				while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
				vend = tvalue.find_first_of('|', vstart);
				if (vend != std::string::npos)
					g = std::stof(tvalue.substr(vstart, vend - vstart));
			}

			// Parse b value
			vstart = tvalue.find(':', vend);
			if (vstart != std::string::npos)
			{
				++vstart;
				while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
				vend = tvalue.find_first_of('|', vstart);
				if (vend != std::string::npos)
					b = std::stof(tvalue.substr(vstart, vend - vstart));
			}

			// Parse a value
			vstart = tvalue.find(':', vend);
			if (vstart != std::string::npos)
			{
				++vstart;
				while (vstart < tvalue.size() && isspace(tvalue[vstart])) ++vstart;
				vend = tvalue.find_first_of('|', vstart);
				if (vend != std::string::npos)
					a = std::stof(tvalue.substr(vstart, vend - vstart));
				else
					a = std::stof(tvalue.substr(vstart));
			}

			return Graphics::Color{ r, g, b, a };
		}

		// File path types
		if (ttype == "texchar240")
		{
			return tvalue;
		}

		throw std::runtime_error("Unknown type { " + ttype + " } has been set in the save file!");
	}

	// Look for the matching closing brace and return its pos
	inline size_t findMatchingBrace(const std::string& data, const size_t& openBrace)
	{
		if (data[openBrace] != '{') return std::string::npos;

		size_t pos = openBrace + 1;
		int depth = 1;

		// Look through data and when }
		while (pos < data.size() && depth > 0)
		{
			if (data[pos] == '{')
				++depth;

			// Found closing
			else if (data[pos] == '}')
				--depth;

			++pos;
		}

		if (depth == 0)
			return pos - 1;

		return std::string::npos;
	}

	// Appends indentation(s) to save file for json format
	inline void appendIndent(std::ofstream& save, const uint32_t& level)
	{
		for (size_t i = 0; i < level; i++)
		{
			save << "\t";
		}
	}

	// Filter all special characters from the str passed
	inline std::string trim(const std::string& str)
	{
		const auto start = str.find_first_not_of(" \t\n\r");
		if (start == std::string::npos)
			return "";

		const auto end = str.find_last_not_of(" \t\n\r");
		return str.substr(start, end - start + 1);
	}

	// Extract file name from the path specified
	inline std::string extractFname(const std::string& path)
	{
		size_t pos = path.find_last_of("\\/");
		if (pos == std::string::npos)
			return path;

		return path.substr(pos + 1);
	}

	// Removes the specified prefix from the passed string
	inline std::string stripPrefix(const std::string& str)
	{
		std::string prefix = "struct ECSComponents::";

		if (str.rfind(prefix, 0) == 0)
			return str.substr(prefix.length());

		std::cerr << "Prefix to strip cannot be found!" << std::endl;
		return str;
	}

	// Determine the script language to write
	//inline void wrtScriptLanguage(std::ofstream& save, const Script::ScriptLanguage& lng)
	//{
	//	switch (lng)
	//	{
	//	case Script::ScriptLanguage::CSharp:
	//		save << "\"Script Language\": { CSharp }\n";
	//		break;
	//	case Script::ScriptLanguage::Lua:
	//		save << "\"Script Language\": { Lua }\n";
	//		break;
	//	}
	//}
}
