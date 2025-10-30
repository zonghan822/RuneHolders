#pragma once

// stds
#include <variant>
#include <string>

namespace Script
{
	using Value = std::variant<std::wstring, int, float, double, bool>;

	enum CSType
	{
		Int32,
		Single,
		Double,
		Boolean,
		String
	};
}