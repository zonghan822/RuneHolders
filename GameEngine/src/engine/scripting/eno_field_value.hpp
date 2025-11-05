#pragma once

// stds
#include <variant>
#include <string>

namespace Script
{
	using Value = std::variant<std::wstring, int, float, double, bool>;
}