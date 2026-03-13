/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_field_value.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)
* @brief        Field value types and helpers used by the C# scripting reflection system.
*//*========================================================================================*/

#pragma once

// stds
#include <variant>
#include <string>

namespace Script
{
	using Value = std::variant<std::wstring, int, float, double, bool>;
}