/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_data_types.hpp
* @author       Koe Yun Ji (yunji.koe@digipen.edu)   50%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

#include <string>
#include <variant>

namespace ECSComponents
{
	// Not for storage, used for
	// text-based save file deserialization
	//
	// Refer to loadJson()
	using DataField = std::variant<

		// Primitive types
		int,
		float,
		double,
		bool,
		uint32_t,
		std::string,		// Not actually to store a string, but to store a fixed char array!!!

		// Non-primitive Math types
		Math::eVec2<float>,
		Graphics::Color,

		// Enums
		Color,
		ECSComponents::ColliderType,
		Audio::EnoAudioModule::LoopMode,

		// File imported types
		std::shared_ptr<Graphics::EnoTexture>
	>;
}