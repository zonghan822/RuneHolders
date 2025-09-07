#pragma once


// stds
#include <cstdint>
#include <unordered_map>
#include <typeindex>


namespace ECSComponents
{
	class ComponentRegistry
	{
	public:

		///
		/// Register a new component into the componentMap
		///
		template <typename T>
		static void RegisterComponent()
		{
			auto index = std::type_index(typeid(T));
			auto it = componentMap().find(index);

			size_t id = componentMap().size();
			componentMap()[index] = static_cast<uint32_t>(id);
		}

		///
		///  Print each component and their corresponding id within the map
		///
		static void printIDs()
		{
			for (const auto& pair : componentMap())
			{
				std::cout << "Component: " << pair.first.name() << '\n'
						  << "ID: " << pair.second << '\n';
			}
		}

		static std::unordered_map<std::type_index, uint32_t>& componentMap()
		{
			static std::unordered_map<std::type_index, uint32_t> map;
			return map;
		}
	};
}