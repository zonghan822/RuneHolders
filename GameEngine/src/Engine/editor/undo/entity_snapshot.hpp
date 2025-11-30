/*============================================================================================
* @file         entity_snapshot.hpp
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
		* \brief   Create a snapshot of an entity with all its components
		*//*========================================================================================*/
		static EntitySnapshot captureEntity(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			Editor::EnoSceneTree& sceneTree
		)
		{
			EntitySnapshot snapshot(entity.id());

			// Capture name if it exists
			if (entity.has<ECSComponents::NameComponent>(entityManager))
			{
				auto& nameComp = entity.get<ECSComponents::NameComponent>(entityManager);
				snapshot.name = nameComp.name;
			}

			// Capture all components
			// Transform
			if (entity.has<ECSComponents::TransformComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::TransformComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// DrawInfo
			if (entity.has<ECSComponents::DrawInfoComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::DrawInfoComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}
			// DrawInfo
			if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Collision
			if (entity.has<ECSComponents::CollisionComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::CollisionComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Physics
			if (entity.has<ECSComponents::PhysicsComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::PhysicsComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Visibility
			if (entity.has<ECSComponents::VisibilityComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::VisibilityComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Layer
			if (entity.has<ECSComponents::LayerComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::LayerComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Button
			if (entity.has<ECSComponents::ButtonComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::ButtonComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Audio
			if (entity.has<ECSComponents::AudioComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::AudioComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// AI
			if (entity.has<ECSComponents::AIComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::AIComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Player
			if (entity.has<ECSComponents::PlayerComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::PlayerComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Box (Pushable)
			if (entity.has<ECSComponents::BoxComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::BoxComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// PrefabInstance
			if (entity.has<ECSComponents::PrefabInstanceComponent>(entityManager))
			{
				auto& comp = entity.get<ECSComponents::PrefabInstanceComponent>(entityManager);
				snapshot.components.emplace_back(comp);
			}

			// Capture hierarchy information from scene tree
			Node* node = sceneTree.getNodeByEntityID(entity.id());
			if (node)
			{
				// Capture parent
				if (node->parent_)
				{
					snapshot.parentID = node->parent_->entity_.id();
				}

				// Capture children
				for (Node* child : node->children_)
				{
					snapshot.childIDs.push_back(child->entity_.id());
				}

				// Capture script path if exists
				if (node->script_)
				{
					snapshot.scriptPath = node->script_->path();
				}
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

			// Capture this entity
			snapshots.push_back(captureEntity(entity, entityManager, sceneTree));

			// Capture all children recursively
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
			std::unordered_map<uint32_t, ECS::Entity>& idMapping  // Map old ID to new ID
		)
		{
			// Create new entity (will get a new ID)
			ECS::Entity newEntity = entityManager.create<>();

			std::cout << "[EntitySerializer] Restoring entity " << snapshot.entityID
					  << " as new entity " << newEntity.id() << "\n";

			// Store mapping from old ID to new entity
			idMapping[snapshot.entityID] = newEntity;

			// Restore all components
			for (const auto& compSnapshot : snapshot.components)
			{
				restoreComponent(newEntity, compSnapshot, entityManager);
			}

			// Restore name - ensure NameComponent exists
			if (!snapshot.name.empty())
			{
				if (!newEntity.has<ECSComponents::NameComponent>(entityManager))
				{
					// Add NameComponent if it wasn't in the snapshot
					newEntity.add<ECSComponents::NameComponent>(entityManager);
				}

				auto& nameComp = newEntity.get<ECSComponents::NameComponent>(entityManager);
				strncpy(nameComp.name, snapshot.name.c_str(), sizeof(nameComp.name) - 1);
				nameComp.name[sizeof(nameComp.name) - 1] = '\0';
			}

			return newEntity;
		}

		/*============================================================================================
		* \brief   Restore component from snapshot
		*//*========================================================================================*/
		static void restoreComponent(
			ECS::Entity& entity,
			const ComponentSnapshot& snapshot,
			ECS::EntityManager& entityManager
		)
		{
			// Check component type and restore accordingly
			if (snapshot.type == typeid(ECSComponents::TransformComponent))
			{
				auto& comp = snapshot.get<ECSComponents::TransformComponent>();
				entity.add<ECSComponents::TransformComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::DrawInfoComponent))
			{
				auto& comp = snapshot.get<ECSComponents::DrawInfoComponent>();
				entity.add<ECSComponents::DrawInfoComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::AnimationControllerComponent))
			{
				auto& comp = snapshot.get<ECSComponents::AnimationControllerComponent>();
				entity.add<ECSComponents::AnimationControllerComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::NameComponent))
			{
				auto& comp = snapshot.get<ECSComponents::NameComponent>();
				entity.add<ECSComponents::NameComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::CollisionComponent))
			{
				auto& comp = snapshot.get<ECSComponents::CollisionComponent>();
				entity.add<ECSComponents::CollisionComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::PhysicsComponent))
			{
				auto& comp = snapshot.get<ECSComponents::PhysicsComponent>();
				entity.add<ECSComponents::PhysicsComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::VisibilityComponent))
			{
				auto& comp = snapshot.get<ECSComponents::VisibilityComponent>();
				entity.add<ECSComponents::VisibilityComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::LayerComponent))
			{
				auto& comp = snapshot.get<ECSComponents::LayerComponent>();
				entity.add<ECSComponents::LayerComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::ButtonComponent))
			{
				auto& comp = snapshot.get<ECSComponents::ButtonComponent>();
				entity.add<ECSComponents::ButtonComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::AudioComponent))
			{
				auto& comp = snapshot.get<ECSComponents::AudioComponent>();
				entity.add<ECSComponents::AudioComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::AIComponent))
			{
				auto& comp = snapshot.get<ECSComponents::AIComponent>();
				entity.add<ECSComponents::AIComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::PlayerComponent))
			{
				auto& comp = snapshot.get<ECSComponents::PlayerComponent>();
				entity.add<ECSComponents::PlayerComponent>(entityManager, comp);
			}
			else if (snapshot.type == typeid(ECSComponents::PrefabInstanceComponent))
			{
				auto& comp = snapshot.get<ECSComponents::PrefabInstanceComponent>();
				entity.add<ECSComponents::PrefabInstanceComponent>(entityManager, comp);
			}
		}
	};
}
