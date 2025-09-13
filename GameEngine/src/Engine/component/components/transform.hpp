#pragma once


// GLM
#include <glm/glm.hpp>
#include <iostream>


namespace ECSComponents
{
	struct TransformComponent
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	};

	inline std::ostream& operator<<(std::ostream& os, const TransformComponent& c)
	{
		std::cout << "Position: (" << c.pos.x << ', ' << c.pos.y << ', ' << c.pos.z << ')' <<
					 "Rotation: (" << c.rot.x << ', ' << c.rot.y << ', ' << c.rot.z << ')' <<
					 "Scale: ("    << c.scale.x << ', ' << c.scale.y << ', ' << c.scale.z << ')'
					 << std::endl;

		return os;
	}
}