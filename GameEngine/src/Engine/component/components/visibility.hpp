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

	inline bool operator==(const VisibilityComponent& lhs, const VisibilityComponent& rhs)
	{
		return (lhs.z_index == rhs.z_index && lhs.visible == rhs.visible);
	}

	inline std::ostream& operator<<(std::ostream& os, const VisibilityComponent& i)
	{
		std::cout << i.z_index << std::endl;
		return os;
	}
}