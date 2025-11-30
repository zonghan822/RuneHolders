/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         entity_commands.
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         2025-11-29
* @brief        Entity creation/deletion commands for undo/redo
*//*========================================================================================*/

#pragma once

#include "command.hpp"
#include "entity_snapshot.hpp"
#include "../eno_scene_tree.hpp"
#include "../../ecs/entity.hpp"
#include <sstream>
#include <vector>
#include <unordered_map>

// Forward declaration to avoid circular dependency
class GameApplication;

// Forward declarations for global symbols defined in imGui.cpp
extern GameApplication* g_game;
extern void markGameEntitiesDirty();

namespace Editor
{
	// Helper function to mark entities dirty (implementation in imGui.cpp)
	inline void markEntitiesDirtyIfNeeded()
	{
		if (::g_game)
		{
			::markGameEntitiesDirty();
		}
	}

	/*============================================================================================
	* \class	CreateEntityCommand
	* \brief	Command for creating entities with full snapshot support
	*
	*			Captures entity state including all components and children for proper redo
	*//*========================================================================================*/
	class CreateEntityCommand : public ICommand
	{
	public:
		CreateEntityCommand(
			EnoSceneTree& sceneTree,
			ECS::EntityManager& entityManager,
			ECS::Entity entity
		)
			: sceneTree_(sceneTree)
			, entityManager_(entityManager)
			, entityExists_(true)
		{
			// Capture the entity and all its children
			snapshots_ = EntitySerializer::captureEntityTree(entity, entityManager, sceneTree);

			// Store the current entity IDs for deletion during undo
			currentEntityIDs_.clear();
			collectEntityIDsFromTree(entity, entityManager, sceneTree);

			std::cout << "[CreateEntityCommand] Captured " << snapshots_.size() << " entities (including children)\n";
		}

	private:
		// Helper to collect current entity IDs
		void collectEntityIDsFromTree(ECS::Entity entity, ECS::EntityManager& entityManager, EnoSceneTree& sceneTree)
		{
			currentEntityIDs_.push_back(entity.id());

			// Collect children
			Node* node = sceneTree.getNodeByEntityID(entity.id());
			if (node)
			{
				for (Node* child : node->children_)
				{
					collectEntityIDsFromTree(child->entity_, entityManager, sceneTree);
				}
			}
		}

	public:

		void execute() override
		{
			if (entityExists_)
			{
				// First execution: entity already exists
				std::cout << "[CreateEntityCommand] Execute - entities already exist\n";
				return;
			}

			// Redo: recreate entities from snapshots
			std::cout << "[CreateEntityCommand] Redo - recreating " << snapshots_.size() << " entities\n";

			std::unordered_map<uint32_t, ECS::Entity> idMapping;
			std::unordered_map<uint32_t, Node*> nodeMapping;

			// Clear previous entity IDs
			currentEntityIDs_.clear();

			// First pass: create all entities
			for (const auto& snapshot : snapshots_)
			{
				ECS::Entity newEntity = EntitySerializer::restoreEntity(snapshot, entityManager_, sceneTree_, idMapping);

				// Store the new entity ID
				currentEntityIDs_.push_back(newEntity.id());

				// Add to scene tree (temporarily as root, we'll fix hierarchy next)
				Node* node = sceneTree_.createFromEntity(newEntity, nullptr);

				// Restore the node name from snapshot
				if (!snapshot.name.empty())
				{
					node->name_ = snapshot.name;
				}

				nodeMapping[snapshot.entityID] = node;
			}

			// Second pass: restore hierarchy
			for (const auto& snapshot : snapshots_)
			{
				Node* node = nodeMapping[snapshot.entityID];
				if (!node) continue;

				// Set parent if exists
				if (snapshot.parentID != UINT32_MAX)
				{
					Node* parent = nullptr;

					// First try to find parent in this command's nodeMapping
					if (nodeMapping.count(snapshot.parentID))
					{
						parent = nodeMapping[snapshot.parentID];
					}
					else
					{
						// Parent not in this command - search existing scene tree by entity ID
						parent = sceneTree_.getNodeByEntityID(snapshot.parentID);

						if (parent)
						{
							std::cout << "[CreateEntityCommand] Found parent entity " << snapshot.parentID << " in existing scene tree\n";
						}
					}

					if (parent)
					{
						sceneTree_.nest(node, parent);
					}
					else
					{
						std::cout << "[CreateEntityCommand] Warning: Parent (entity ID " << snapshot.parentID << ") not found for child '" << snapshot.name << "'\n";
					}
				}

				// TODO: Restore script if needed
			}

			// Mark entities dirty so render cache updates
			markEntitiesDirtyIfNeeded();

			entityExists_ = true;
			std::cout << "[CreateEntityCommand] Successfully recreated entity tree\n";
		}

		void undo() override
		{
			if (!entityExists_)
			{
				std::cout << "[CreateEntityCommand] Entities already deleted\n";
				return;
			}

			std::cout << "[CreateEntityCommand] Undo - deleting " << currentEntityIDs_.size() << " entities\n";

			// Delete all entities using stored IDs (reverse order - children first)
			for (auto it = currentEntityIDs_.rbegin(); it != currentEntityIDs_.rend(); ++it)
			{
				uint32_t entityID = *it;

				// Check if entity still exists
				auto& entityMap = entityManager_.getMap();
				if (entityMap.find(entityID) != entityMap.end())
				{
					// Remove from scene tree first
					Node* node = sceneTree_.getNodeByEntityID(entityID);
					if (node)
					{
						sceneTree_.kill(node);
					}

					// Then destroy the entity
					entityManager_.destroyEntity(entityID);
					std::cout << "[CreateEntityCommand] Deleted entity: " << entityID << "\n";
				}
			}

			// Mark entities dirty so render cache updates
			markEntitiesDirtyIfNeeded();

			entityExists_ = false;
		}

		std::string getDescription() const override
		{
			if (!snapshots_.empty())
			{
				std::ostringstream oss;
				oss << "Create Entity '" << snapshots_[0].name << "'";
				if (snapshots_.size() > 1)
				{
					oss << " (+ " << (snapshots_.size() - 1) << " children)";
				}
				return oss.str();
			}
			return "Create Entity";
		}

	private:
		EnoSceneTree& sceneTree_;
		ECS::EntityManager& entityManager_;
		std::vector<EntitySnapshot> snapshots_;  // Snapshots of entity and all children
		std::vector<uint32_t> currentEntityIDs_;  // Current entity IDs (for deletion during undo)
		bool entityExists_;
	};

	/*============================================================================================
	* \class	DeleteEntityCommand
	* \brief	Command for deleting entities with full snapshot support
	*
	*			Captures entity state before deletion for proper undo
	*//*========================================================================================*/
	class DeleteEntityCommand : public ICommand
	{
	public:
		DeleteEntityCommand(
			EnoSceneTree& sceneTree,
			ECS::EntityManager& entityManager,
			ECS::Entity entity
		)
			: sceneTree_(sceneTree)
			, entityManager_(entityManager)
			, deleted_(false)
		{
			// Capture the entity and all its children BEFORE deletion
			snapshots_ = EntitySerializer::captureEntityTree(entity, entityManager, sceneTree);

			// Store the entity IDs that will be deleted
			entitiesToDelete_.clear();
			collectEntityIDsFromTree(entity, entityManager, sceneTree);

			std::cout << "[DeleteEntityCommand] Captured " << snapshots_.size() << " entities for deletion\n";
		}

	private:
		// Helper to collect entity IDs before deletion
		void collectEntityIDsFromTree(ECS::Entity entity, ECS::EntityManager& entityManager, EnoSceneTree& sceneTree)
		{
			entitiesToDelete_.push_back(entity.id());

			// Collect children
			Node* node = sceneTree.getNodeByEntityID(entity.id());
			if (node)
			{
				for (Node* child : node->children_)
				{
					collectEntityIDsFromTree(child->entity_, entityManager, sceneTree);
				}
			}
		}

	public:

		void execute() override
		{
			if (deleted_)
			{
				std::cout << "[DeleteEntityCommand] Entities already deleted\n";
				return;
			}

			std::cout << "[DeleteEntityCommand] Deleting " << entitiesToDelete_.size() << " entities\n";

			// Delete all entities using stored IDs (reverse order - children first)
			for (auto it = entitiesToDelete_.rbegin(); it != entitiesToDelete_.rend(); ++it)
			{
				uint32_t entityID = *it;

				// Check if entity still exists
				auto& entityMap = entityManager_.getMap();
				if (entityMap.find(entityID) != entityMap.end())
				{
					// Remove from scene tree first
					Node* node = sceneTree_.getNodeByEntityID(entityID);
					if (node)
					{
						sceneTree_.kill(node);
					}

					// Then destroy the entity
					entityManager_.destroyEntity(entityID);
					std::cout << "[DeleteEntityCommand] Deleted entity: " << entityID << "\n";
				}
			}

			// Mark entities dirty so render cache updates
			markEntitiesDirtyIfNeeded();

			deleted_ = true;
		}

		void undo() override
		{
			if (!deleted_)
			{
				std::cout << "[DeleteEntityCommand] Entities not deleted yet\n";
				return;
			}

			// Recreate entities from snapshots
			std::cout << "[DeleteEntityCommand] Undo - recreating " << snapshots_.size() << " entities\n";

			std::unordered_map<uint32_t, ECS::Entity> idMapping;
			std::unordered_map<uint32_t, Node*> nodeMapping;

			// Clear and rebuild entity IDs list with new IDs
			entitiesToDelete_.clear();

			// First pass: create all entities
			for (const auto& snapshot : snapshots_)
			{
				ECS::Entity newEntity = EntitySerializer::restoreEntity(snapshot, entityManager_, sceneTree_, idMapping);

				// Store the new entity ID
				entitiesToDelete_.push_back(newEntity.id());

				// Add to scene tree (temporarily as root)
				Node* node = sceneTree_.createFromEntity(newEntity, nullptr);

				// Restore the node name from snapshot
				if (!snapshot.name.empty())
				{
					node->name_ = snapshot.name;
				}

				nodeMapping[snapshot.entityID] = node;
			}

			// Second pass: restore hierarchy
			for (const auto& snapshot : snapshots_)
			{
				Node* node = nodeMapping[snapshot.entityID];
				if (!node) continue;

				// Set parent if exists
				if (snapshot.parentID != UINT32_MAX)
				{
					Node* parent = nullptr;

					// First try to find parent in this command's nodeMapping
					if (nodeMapping.count(snapshot.parentID))
					{
						parent = nodeMapping[snapshot.parentID];
					}
					else
					{
						// Parent not in this command - search existing scene tree by entity ID
						parent = sceneTree_.getNodeByEntityID(snapshot.parentID);

						if (parent)
						{
							std::cout << "[DeleteEntityCommand] Found parent entity " << snapshot.parentID << " in existing scene tree\n";
						}
					}

					if (parent)
					{
						sceneTree_.nest(node, parent);
					}
					else
					{
						std::cout << "[DeleteEntityCommand] Warning: Parent (entity ID " << snapshot.parentID << ") not found for child '" << snapshot.name << "'\n";
					}
				}

				// TODO: Restore script if needed
			}

			// Mark entities dirty so render cache updates
			markEntitiesDirtyIfNeeded();

			deleted_ = false;
			std::cout << "[DeleteEntityCommand] Successfully restored entity tree\n";
		}

		std::string getDescription() const override
		{
			if (!snapshots_.empty())
			{
				std::ostringstream oss;
				oss << "Delete Entity '" << snapshots_[0].name << "'";
				if (snapshots_.size() > 1)
				{
					oss << " (+ " << (snapshots_.size() - 1) << " children)";
				}
				return oss.str();
			}
			return "Delete Entity";
		}

	private:
		EnoSceneTree& sceneTree_;
		ECS::EntityManager& entityManager_;
		std::vector<EntitySnapshot> snapshots_;  // Snapshots of entity and all children
		std::vector<uint32_t> entitiesToDelete_;  // Entity IDs to delete (updated after undo recreates them)
		bool deleted_;
	};

	/*============================================================================================
	* \class	MultiEntityCommand
	* \brief	Batch command for applying the same operation to multiple entities
	*
	*			Useful for group transformations
	*//*========================================================================================*/
	class MultiEntityCommand : public ICommand
	{
	public:
		MultiEntityCommand(const std::string& description = "Multi-entity operation")
			: description_(description)
		{}

		void addCommand(std::unique_ptr<ICommand> command)
		{
			commands_.push_back(std::move(command));
		}

		void execute() override
		{
			for (auto& cmd : commands_)
			{
				cmd->execute();
			}
		}

		void undo() override
		{
			// Undo in reverse order
			for (auto it = commands_.rbegin(); it != commands_.rend(); ++it)
			{
				(*it)->undo();
			}
		}

		std::string getDescription() const override
		{
			return description_;
		}

		bool isEmpty() const
		{
			return commands_.empty();
		}

	private:
		std::vector<std::unique_ptr<ICommand>> commands_;
		std::string description_;
	};
}
