/*============================================================================================
* @file         entity_commands.hpp
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
			std::cout << "[CreateEntityCommand] Captured " << snapshots_.size() << " entities (including children)\n";
		}

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

			// First pass: create all entities
			for (const auto& snapshot : snapshots_)
			{
				ECS::Entity newEntity = EntitySerializer::restoreEntity(snapshot, entityManager_, sceneTree_, idMapping);

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
				if (snapshot.parentID != UINT32_MAX && nodeMapping.count(snapshot.parentID))
				{
					Node* parent = nodeMapping[snapshot.parentID];
					sceneTree_.nest(node, parent);
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

			std::cout << "[CreateEntityCommand] Undo - deleting " << snapshots_.size() << " entities\n";

			// Delete all entities (children first, then parents)
			for (auto it = snapshots_.rbegin(); it != snapshots_.rend(); ++it)
			{
				const auto& snapshot = *it;

				// Find the current entity with this data
				// Since IDs might have changed, we need to find by matching data
				// For simplicity, we'll use the first entity with matching name
				auto entities = entityManager_.findEntity<ECSComponents::NameComponent>();
				for (auto& entity : entities)
				{
					if (entity.has<ECSComponents::NameComponent>(entityManager_))
					{
						auto& nameComp = entity.get<ECSComponents::NameComponent>(entityManager_);
						if (std::string(nameComp.name) == snapshot.name)
						{
							Node* node = sceneTree_.getNodeByEntityID(entity.id());
							if (node)
							{
								sceneTree_.kill(node);
								entityManager_.destroyEntity(entity.id());
								std::cout << "[CreateEntityCommand] Deleted entity: " << snapshot.name << "\n";
								break;
							}
						}
					}
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
			std::cout << "[DeleteEntityCommand] Captured " << snapshots_.size() << " entities for deletion\n";
		}

		void execute() override
		{
			if (deleted_)
			{
				std::cout << "[DeleteEntityCommand] Entities already deleted\n";
				return;
			}

			std::cout << "[DeleteEntityCommand] Deleting " << snapshots_.size() << " entities\n";

			// Delete all entities (children first, then parents)
			for (auto it = snapshots_.rbegin(); it != snapshots_.rend(); ++it)
			{
				const auto& snapshot = *it;

				// Find entity by matching name
				auto entities = entityManager_.findEntity<ECSComponents::NameComponent>();
				for (auto& entity : entities)
				{
					if (entity.has<ECSComponents::NameComponent>(entityManager_))
					{
						auto& nameComp = entity.get<ECSComponents::NameComponent>(entityManager_);
						if (std::string(nameComp.name) == snapshot.name)
						{
							Node* node = sceneTree_.getNodeByEntityID(entity.id());
							if (node)
							{
								sceneTree_.kill(node);
								entityManager_.destroyEntity(entity.id());
								std::cout << "[DeleteEntityCommand] Deleted entity: " << snapshot.name << "\n";
								break;
							}
						}
					}
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

			// First pass: create all entities
			for (const auto& snapshot : snapshots_)
			{
				ECS::Entity newEntity = EntitySerializer::restoreEntity(snapshot, entityManager_, sceneTree_, idMapping);

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
				if (snapshot.parentID != UINT32_MAX && nodeMapping.count(snapshot.parentID))
				{
					Node* parent = nodeMapping[snapshot.parentID];
					sceneTree_.nest(node, parent);
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
