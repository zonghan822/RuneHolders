/*============================================================================================
* @file         visibility.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the visibility component
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// Signals
#include "../../event/signals.hpp"

//stds
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Forward declarations
namespace Engine
{
	class EnoEngine;
}

namespace ECS
{
	class Entity;
	class EntityManager;
}

namespace Editor
{
	struct Node;
	class EnoSceneTree;
}

namespace ECSComponents
{
	// Maximum number of layers allowed
	inline constexpr int MAX_LAYERS = 20;

	// Current number of available layers (starts with 4: 0, 1, 2, 3)
	inline int availableLayers = 4;

	/*============================================================================================
	 * \brief       Definition of the visibility component
	*//*========================================================================================*/
	struct VisibilityComponent
	{
	public:
		// Rendering layer (default is 3)
		uint32_t z_index = 3;

		// Visibility
		bool visible = true;

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_; 

		// For serialization purposes
		// Generalized way of using reflection to access and iterate
		// over fields!
		// 
		// The idea is to (in a function call) create a lambda that will be a "visitor",
		// and extract all information as parameters of that lambda. Each call over a
		// field of the component is used to extract metadata on 1 field [name, value, type]
		// 
		// Source: https://stackoverflow.com/questions/41453/how-can-i-add-reflection-to-a-c-application
		// Additional learning: https://refactoring.guru/design-patterns/visitor
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Z Index", z_index, "uint32_t");
			l("Visible", visible, "bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Z Index", z_index, "uint32_t");
			l("Visible", visible, "bool");
		}

		// Getter for z_index
		uint32_t getZIndex() const { return z_index; }

		// Setter for z_index
		void setZIndex(uint32_t index) { z_index = index; }
	};

	// Static storage for sorted layers (inline for single definition across translation units)
	inline std::map<int, std::vector<ECS::Entity*>> sortedObjectLayers;

	/*============================================================================================
	 * \brief				Recursive helper to collect entity and its children by layer
	 * \param[in]			node - pointer to the scene tree node
	 * \param[in]			entityMng - reference to EntityManager
	 * \param[in]			layers - reference to the layer map to populate
	*//*========================================================================================*/
	void collectChildrenRecursive(Editor::Node* node,
	                              ECS::EntityManager& entityMng,
	                              std::map<int, std::vector<ECS::Entity*>>& layers);

	/*============================================================================================
	 * \brief				Collect all objects in the scene and sort them in layers based on z_index
	 * \param[in]			eng - reference to EnoEngine
	 * \param[in]			cachedEntities - reference to vector of cached drawable entities
	*//*========================================================================================*/
	void collectObjectsByLayer(Engine::EnoEngine& eng, std::vector<ECS::Entity>& cachedEntities);

	/*============================================================================================
	 * \brief				Equality operator overload for the visibility component
	 * \param[in]			readonly reference to a VisibilityComponent
	 * \param[in]			readonly reference to a VisibilityComponent
	 * \return				Check and return if all members are equivalent
	*//*========================================================================================*/
	inline bool operator==(const VisibilityComponent& lhs, const VisibilityComponent& rhs)
	{
		return (lhs.z_index == rhs.z_index && lhs.visible == rhs.visible);
	}

	/*============================================================================================
	 * \brief				Stream insertion operator overload for the visibility component
	 * \param[in]			reference to the output stream
	 * \param[in]			readonly reference to a VisibilityComponent to print its members
	 * \return				reference to the output stream
	*//*========================================================================================*/
	inline std::ostream& operator<<(std::ostream& os, const VisibilityComponent& i)
	{
		std::cout << "Z Index: " << i.z_index << std::endl <<
			"Visible: " << std::boolalpha << i.visible;
		return os;
	}
}
