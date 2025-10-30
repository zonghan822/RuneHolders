#pragma once

// stds
//#include <Windows.h>
#include <variant>
#include <string>
#include <unordered_map>

namespace Script
{
	class EnoScript
	{
	public:
		using Value = std::variant<std::string, int, float>;

		EnoScript(const std::wstring&, const std::wstring&);

		// Accessor(s)
		//std::string path();

	private:
		std::wstring name_;
		std::wstring path_;
		std::unordered_map<std::string, Value> variables_;		// var name | value

		// Helper for converting wide strings to standard utf-8
		//std::string wideToUtf8(const std::wstring&);
	};
}