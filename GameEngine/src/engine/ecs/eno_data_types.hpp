#pragma once

//stds
#include <string>
#include <variant>


namespace Parser
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

		// File imported types
		std::shared_ptr<Graphics::EnoTexture>
	>;
}