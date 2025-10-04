/*============================================================================================
* @file         component_registry.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the component registry
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


#pragma once

// Engine Component(s)
#include "../ecs/archetype_presets.hpp"
#include "../component/component_id.hpp"

// stds
#include <cstdint>
#include <unordered_map>
#include <typeindex>
#include <algorithm>
#include <iomanip>


namespace ECSComponents
{
	class EnoComponentRegistry
	{
	public:
		struct ComponentInfo
		{
			uint32_t id;
			size_t size;
		};

		/*============================================================================================
		 * \brief		Static component map, using macro expansion to populate components
		 * \return		unordered map of components
		*//*========================================================================================*/
		static std::unordered_map<std::type_index, ComponentInfo>& ComponentMap()
		{
			static std::unordered_map<std::type_index, ComponentInfo> map =
			{
#define X(TYPE) { std::type_index(typeid(TYPE)), { __COUNTER__, sizeof(TYPE) } },
				COMPONENT_LIST
#undef X
			};

			return map;
		}

		// Print all registered components and their respective ID and size
		static void DisplayReg()
		{
			for (const auto& component : ComponentMap())
			{
				std::cout << "Component: " << component.first.name() << std::endl
					<< "ID: " << component.second.id << std::endl
					<< "Size: " << component.second.size << std::endl
					<< std::setw(10) << std::setfill('_') << std::endl;
			}

			// Reset fill char
			std::cout << std::setfill(' ');
		}

		// Register a new component into the componentMap
		template <typename Component>
		static void RegisterComponent()
		{
			// Verify that component has not already been added
			auto index = std::type_index(typeid(Component));
			auto it = ComponentMap().find(index);

			if (it != ComponentMap().end())
			{
				throw std::runtime_error(std::string(index.name()) + " has already been registered!");
			}

			size_t id = ComponentMap().size();
			ComponentMap().emplace(index, ComponentInfo{ static_cast<uint32_t>(id), sizeof(Component) });
		}

		// Get the ID of a component
		static uint32_t GetComponentID(const std::type_index& type)
		{
			auto it = ComponentMap().find(type);
			if (it == ComponentMap().end())
			{
				throw std::runtime_error("Component searched has not been registered in the Component Registry map!");
			}

			return it->second.id;
		}

		/*============================================================================================
		 * \brief			Using an id to find a component stored
		 * \param[in]       ID used to search the component
		 * \return			Found component
		*//*========================================================================================*/
		static std::type_index FindComponent(const std::size_t& id)
		{
			auto it = std::find_if(
				ComponentMap().begin(),
				ComponentMap().end(),
				[id](const std::pair<std::type_index, ComponentInfo>& pair)
				{
					return pair.second.id == id;
				});

			if (it == ComponentMap().end())
			{
				throw std::runtime_error("Failed to find a component in the registry based on id!");
			}

			return it->first;
		}

		/*============================================================================================
		 * \brief			Get a component's size
		 * \param[in]       Component type
		 * \return			Component size
		*//*========================================================================================*/
		static std::size_t GetComponentSize(const std::type_index& type)
		{
			auto it = ComponentMap().find(type);
			if (it == ComponentMap().end())
			{
				throw std::runtime_error("Component searched has not been registered in the Component Registry map!");
			}

			return it->second.size;
		}
	};
}