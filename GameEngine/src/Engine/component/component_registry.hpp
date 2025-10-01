#pragma once

// Engine Component(s)
#include "../ecs/archetype_presets.hpp"
#include "../component/component_id.hpp"

// stds
#include <cstdint>
#include <unordered_map>
#include <typeindex>
#include <algorithm>


namespace ECSComponents
{
	class EnoComponentRegistry
	{
	public:
		using regID = RegistryInfo::ComponentID;

		struct ComponentInfo
		{
			uint32_t id;
			size_t size;
		};

		static std::unordered_map<std::type_index, ComponentInfo>& ComponentMap()
		{
			static std::unordered_map<std::type_index, ComponentInfo> map =
			{
				{ std::type_index(typeid(NameComponent)),			{ regID::NAME,			sizeof(NameComponent)		}},
				{ std::type_index(typeid(VisibilityComponent)),		{ regID::VISBILITY,		sizeof(VisibilityComponent)	}},
				{ std::type_index(typeid(TransformComponent)),		{ regID::TRANSFORM,		sizeof(TransformComponent)	}},
				{ std::type_index(typeid(LayerComponent)),			{ regID::LAYER,			sizeof(LayerComponent)		}},
				{ std::type_index(typeid(ModulationComponent)),		{ regID::MODULATION,	sizeof(ModulationComponent) }},
				{ std::type_index(typeid(DrawInfoComponent)),		{ regID::DRAWINFO,		sizeof(DrawInfoComponent)   }}
			};

			return map;
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
			ComponentMap().emplace(
				index, 
				ComponentInfo{ static_cast<uint32_t>(id), sizeof(Component) });
		}

		//  Sort and print each component and their corresponding id within the map
		static void PrintIDs()
		{
			std::vector<std::pair<std::type_index, ComponentInfo>> cont{
				ComponentMap().begin(), ComponentMap().end() };

			// Sort components based on their id (ascending order)
			std::sort(cont.begin(), cont.end(), [](auto& a, auto& b) 
				{ return a.second.id < b.second.id; });

			// Print
			for (const auto& pair : cont)
			{
				std::cout << "Component: " << pair.first.name() << '\n'
					<< "ID: " << pair.second.id << '\n';
			}
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

		// Get a component based on the id passed
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

		// Get a component's size
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