#pragma once


// GLM
#include <glm/glm.hpp>
#include <iostream>
#include "../../math/math.hpp"


namespace ECSComponents
{
	struct TransformComponent
	{
		
		/*glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
		glm::vec2 vel;*/

		// Zero initialized values
		Math::eVec2<float> pos{};
		Math::eVec2<float> rot{};
		Math::eVec2<float> scale{};
		
		Math::eVec2<float> vel{};	// remove this after rewriting movementtest

		// Reset for pause play feature
		Math::eVec2<float> resetPos{};

		// Default constructor
		TransformComponent() = default;

		// Constructor
		TransformComponent(Math::eVec2<float> _pos, Math::eVec2<float> _rot, Math::eVec2<float> _scale) : pos(_pos), rot(_rot), scale(_scale) {};

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