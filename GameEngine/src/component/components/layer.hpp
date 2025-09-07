#pragma once

//stds
#include <string>
#include <iostream>


namespace ECSComponents
{
	struct LayerComponent
	{
		char layer[16] = "NULL";
	};

	inline std::ostream& operator<<(std::ostream& os, const LayerComponent& n)
	{
		std::cout << n.layer << std::endl;
		return os;
	}
}