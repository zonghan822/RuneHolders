#pragma once

// GLM
#include <glm/mat4x4.hpp>
#include <iostream>


namespace ECSComponents
{
	struct ModulationComponent
	{
		glm::vec4 color;
	};

	inline std::ostream& operator<<(std::ostream& os, const ModulationComponent& mod)
	{
		std::cout << "Modulation: (" << mod.color.x << ", " <<
										mod.color.y << ", " <<
										mod.color.z << ", " <<
										mod.color.w << ')' << std::endl;

		return os;
	}
}