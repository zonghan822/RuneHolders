#pragma once


// GLM
#include <glm/glm.hpp>
#include <iostream>
#include "../../math/math.hpp"


namespace ECSComponents
{
	struct TransformComponent
	{
		// Spatial properties only - position, rotation, scale
		Math::eVec2<float> pos{};
		Math::eVec2<float> rot{};
		Math::eVec2<float> scale{ 1.0f, 1.0f };  // Default to unit scale

		// Reset for play/stop state (editor functionality)
		Math::eVec2<float> resetPos{};

		// Default constructor
		TransformComponent() = default;

		// Constructor
		TransformComponent(Math::eVec2<float> _pos, Math::eVec2<float> _rot, Math::eVec2<float> _scale)
			: pos(_pos), rot(_rot), scale(_scale) {}

	};

	// print overload
	inline std::ostream& operator<<(std::ostream& os, const TransformComponent& c)
	{
		std::cout << "Position: (" << c.pos.x	<< ', ' << c.pos.y	 << ')' <<
					 "Rotation: (" << c.rot.x	<< ', ' << c.rot.y	 << ')' <<
					 "Scale: ("    << c.scale.x << ', ' << c.scale.y << ')' << 
					  std::endl;

		return os;
	}

}