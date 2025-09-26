#pragma once

#include <iostream>
#include <string>
//would i be able to add the math library here? -YunQi


namespace ECSComponents
{
	struct DrawInfoComponent
	{
		DrawInfoComponent() = default;

		DrawInfoComponent(const char* n)
		{
			strncpy_s(shape, n, sizeof(shape) - 1);
			shape[sizeof(shape) - 1] = '\0';
		}

		char shape[32] = "Rectangle";
		
	};

	inline std::ostream& operator<<(std::ostream& os, const DrawInfoComponent& draw)
	{
		std::cout << "Shape to draw: (" << draw.shape << ')' << std::endl;

		return os;
	}
}