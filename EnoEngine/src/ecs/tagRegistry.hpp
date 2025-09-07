#pragma once


//stds
#include <vector>
#include <unordered_map>
#include <string>

namespace ECS
{
	class TagRegistry
	{
	public:

		// Create a tag
		static void RegisterTag(const std::string& tag)
		{
			tags().push_back(tag);
		}

		// Insert a map in
		static void Insert()
		{

		}

		// Print all tags and their associated entities
		static void Print()
		{

		}

	private:
		static std::vector<std::string>& tags()
		{
			static std::vector<std::string> t;
			return t;
		}

		static std::unordered_map<uint32_t, std::string>& tagMap()
		{
			static std::unordered_map<uint32_t, std::string> map;
			return map;
		}
	};
}