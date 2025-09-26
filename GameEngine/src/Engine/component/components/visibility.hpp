#pragma once

//stds
#include <iostream>
#include <string>

namespace ECSComponents
{
	struct VisibilityComponent
	{
		uint32_t z_index = 3;
		bool visible = false;
	};

	inline std::ostream& operator<<(std::ostream& os, const VisibilityComponent& i)
	{
		std::cout << i.z_index << std::endl;
		return os;
	}
}