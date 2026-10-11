/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_field_value.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// stds
#include <variant>
#include <string>

namespace Script
{
	using Value = std::variant<std::wstring, int, float, double, bool>;
}