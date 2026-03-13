/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         entity_snapshot.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   42%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   25%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   17%
* @date         2025-11-29
* @brief        Entity snapshot for serialization in undo/redo system
*//*========================================================================================*/



#pragma once

#include "../../ecs/entity.hpp"
#include "../../component/component_list.hpp"
#include <vector>
#include <memory>
#include <any>
#include <unordered_map>
#include <string>
#include <typeindex>

namespace Editor
{
	/*============================================================================================
	* \struct	ComponentSnapshot
	* \brief	Stores a single component's data
	*//*========================================================================================*/
	struct ComponentSnapshot
	{
		std::type_index type;
		std::any data;

		template<typename T>
		ComponentSnapshot(const T& component)
			: type(typeid(T))
			, data(component)
		{}

		template<typename T>
		T& get()
		{
			return std::any_cast<T&>(data);
		}

		template<typename T>
		const T& get() const
		{
			return std::any_cast<const T&>(data);
		}
	};

	/*============================================================================================
	* \struct	EntitySnapshot
	* \brief	Complete snapshot of an entity including all components and hierarchy
	*//*========================================================================================*/
	struct EntitySnapshot
	{
		uint32_t entityID = UINT32_MAX;
		std::string name;
		std::vector<ComponentSnapshot> components;

		// Hierarchy information
		uint32_t parentID = UINT32_MAX;  // UINT32_MAX means no parent (root node)
		std::vector<uint32_t> childIDs;  // IDs of child entities

		// Script information
		std::string scriptPath;  // Path to attached script (if any)

		EntitySnapshot() = default;
		EntitySnapshot(uint32_t id) : entityID(id) {}
	};

	/*============================================================================================
	* \class	EntitySerializer
	* \brief	Serializes and deserializes entities for undo/redo
	*//*========================================================================================*/
	class EntitySerializer
	{
	public:
		/*============================================================================================
		* \brief   Capture all components of an entity as a type_index -> any map.
		*          Used by ComponentChangeCommand for lightweight component-only snapshots.
		*//*========================================================================================*/
		static std::unordered_map<std::type_index, std::any> captureComponentsMap(
			ECS::Entity entity,
			ECS::EntityManager& entityManager
		)
		{
			std::unordered_map<std::type_index, std::any> result;
#define X(CompName) \
			if (entity.has<ECSComponents::CompName>(entityManager)) \
				result[std::type_index(typeid(ECSComponents::CompName))] = \
					entity.get<ECSComponents::CompName>(entityManager);
			COMPONENT_LIST
#undef X
			return result;
		}

		/*============================================================================================
		* \brief   Apply a component map snapshot back onto an existing entity.
		*          Only overwrites components the entity already has.
		*//*========================================================================================*/
		static void applyComponentsMap(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			const std::unordered_map<std::type_index, std::any>& snapshot
		)
		{
#define X(CompName) \
			{ \
				auto _key = std::type_index(typeid(ECSComponents::CompName)); \
				auto _it = snapshot.find(_key); \
				if (_it != snapshot.end() && entity.has<ECSComponents::CompName>(entityManager)) \
					entity.get<ECSComponents::CompName>(entityManager) = \
						std::any_cast<ECSComponents::CompName>(_it->second); \
			}
			COMPONENT_LIST
#undef X
		}

		/*============================================================================================
		* \brief   Create a snapshot of an entity with all its components
		*//*========================================================================================*/
		static EntitySnapshot captureEntity(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			Editor::EnoSceneTree& sceneTree
		)
		{
			EntitySnapshot snapshot(entity.id());

			// Capture name from the scene node
			Node* nameNode = sceneTree.getNodeByEntityID(entity.id());
			if (nameNode)
				snapshot.name = nameNode->name_;

			// Capture all components via COMPONENT_LIST
#define X(CompName) \
			if (entity.has<ECSComponents::CompName>(entityManager)) \
				snapshot.components.emplace_back(entity.get<ECSComponents::CompName>(entityManager));
			COMPONENT_LIST
#undef X

			// Capture hierarchy information from scene tree
			Node* node = sceneTree.getNodeByEntityID(entity.id());
			if (node)
			{
				if (node->parent_)
					snapshot.parentID = node->parent_->entity_.id();

				for (Node* child : node->children_)
					snapshot.childIDs.push_back(child->entity_.id());

				if (node->script_)
					snapshot.scriptPath = node->script_->path();
			}

			std::cout << "[EntitySerializer] Captured entity " << entity.id()
					  << " with " << snapshot.components.size() << " components"
					  << " and " << snapshot.childIDs.size() << " children\n";

			return snapshot;
		}

		/*============================================================================================
		* \brief   Capture an entity and all its descendants (recursive)
		*//*========================================================================================*/
		static std::vector<EntitySnapshot> captureEntityTree(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			Editor::EnoSceneTree& sceneTree
		)
		{
			std::vector<EntitySnapshot> snapshots;

			snapshots.push_back(captureEntity(entity, entityManager, sceneTree));

			Node* node = sceneTree.getNodeByEntityID(entity.id());
			if (node)
			{
				for (Node* child : node->children_)
				{
					auto childSnapshots = captureEntityTree(child->entity_, entityManager, sceneTree);
					snapshots.insert(snapshots.end(), childSnapshots.begin(), childSnapshots.end());
				}
			}

			return snapshots;
		}

		/*============================================================================================
		* \brief   Restore an entity from a snapshot
		* \return  The restored entity
		*//*========================================================================================*/
		static ECS::Entity restoreEntity(
			const EntitySnapshot& snapshot,
			ECS::EntityManager& entityManager,
			Editor::EnoSceneTree& sceneTree,
			std::unordered_map<uint32_t, ECS::Entity>& idMapping
		)
		{
			ECS::Entity newEntity = entityManager.create<>();

			std::cout << "[EntitySerializer] Restoring entity " << snapshot.entityID
					  << " as new entity " << newEntity.id() << "\n";

			idMapping[snapshot.entityID] = newEntity;

			for (const auto& compSnapshot : snapshot.components)
				restoreComponent(newEntity, compSnapshot, entityManager);

			return newEntity;
		}

		/*============================================================================================
		* \brief   Restore component from snapshot onto entity (adds component)
		*//*========================================================================================*/
		static void restoreComponent(
			ECS::Entity& entity,
			const ComponentSnapshot& snapshot,
			ECS::EntityManager& entityManager
		)
		{
#define X(CompName) \
			if (snapshot.type == typeid(ECSComponents::CompName)) { \
				entity.add<ECSComponents::CompName>(entityManager, \
					snapshot.get<ECSComponents::CompName>()); \
				return; \
			}
			COMPONENT_LIST
#undef X
		}
	};
}
