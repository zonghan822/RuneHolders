#pragma once

//stds
#include <string>
#include <iostream>


namespace ECSComponents
{
	struct LayerComponent
	{
		LayerComponent() = default;

		LayerComponent(const char* l)
		{
			strncpy_s(layer, l, sizeof(layer) - 1);
			layer[sizeof(layer) - 1] = '\0';
		}

		char layer[32] = "NULL";
	};

	inline std::ostream& operator<<(std::ostream& os, const LayerComponent& n)
	{
		std::cout << n.layer << std::endl;
		return os;
	}
}