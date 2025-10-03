#pragma once

// stds
#include <string>
#include <iostream>


namespace ECSComponents
{
	struct NameComponent
	{
		NameComponent() = default;

		NameComponent(const char* n)
		{
			strncpy_s(name, n, sizeof(name) - 1);
			name[sizeof(name) - 1] = '\0';
		}

		char name[32] = "DefaultName";
	};

	inline bool operator==(const NameComponent& lhs, const NameComponent& rhs)
	{
		return strcmp(lhs.name, rhs.name) == 0;
	}

	inline std::ostream& operator<<(std::ostream& os, const NameComponent& n)
	{
		std::cout << n.name << std::endl;
		return os;
	}
}