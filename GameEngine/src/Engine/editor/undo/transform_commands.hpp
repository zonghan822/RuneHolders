/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         transform_commands.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   80%
* @co-author    Caven Koh (caven.koh@digipen.edu)   20%
* @date         2025-11-29
* @brief        Transform-related commands (move, rotate, scale) for undo/redo
*//*========================================================================================*/



#pragma once

#include "command.hpp"
#include "../../ecs/entity.hpp"
#include "../../component/components/transform.hpp"
#include <sstream>

namespace Editor
{
	/*============================================================================================
	* \class	TransformCommand
	* \brief	Command for undoing/redoing transform changes (position, rotation, scale)
	*//*========================================================================================*/
	class TransformCommand : public ICommand
	{
	public:
		/*============================================================================================
		* \brief   Constructor
		* \param[in] entity The entity being transformed
		* \param[in] entityManager Reference to the entity manager
		* \param[in] oldTransform Previous transform state
		* \param[in] newTransform New transform state
		* \param[in] description Human-readable description
		*//*========================================================================================*/
		TransformCommand(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			const ECSComponents::TransformComponent& oldTransform,
			const ECSComponents::TransformComponent& newTransform,
			const std::string& description = "Transform"
		)
			: entity_(entity)
			, entityManager_(entityManager)
			, oldTransform_(oldTransform)
			, newTransform_(newTransform)
			, description_(description)
		{}

		void execute() override
		{
			if (entity_.id() == UINT32_MAX)
			{
				std::cout << "[TransformCommand] Execute - Entity ID is invalid\n";
				return;
			}

			auto& entityMap = entityManager_.getMap();
			auto it = entityMap.find(entity_.id());
			if (it == entityMap.end())
			{
				std::cout << "[TransformCommand] Execute - Entity " << entity_.id() << " not found in EntityManager\n";
				return;
			}

			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform = newTransform_;
			}
			catch (...) {
				std::cout << "[TransformCommand] Execute - Exception accessing entity " << entity_.id() << "\n";
			}
		}

		void undo() override
		{
			if (entity_.id() == UINT32_MAX)
			{
				std::cout << "[TransformCommand] Undo - Entity ID is invalid\n";
				return;
			}

			auto& entityMap = entityManager_.getMap();
			auto it = entityMap.find(entity_.id());
			if (it == entityMap.end())
			{
				std::cout << "[TransformCommand] Undo - Entity " << entity_.id() << " not found in EntityManager\n";
				return;
			}

			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform = oldTransform_;
			}
			catch (...) {
				std::cout << "[TransformCommand] Undo - Exception accessing entity " << entity_.id() << "\n";
			}
		}

		std::string getDescription() const override
		{
			return description_;
		}

	private:
		ECS::Entity entity_;
		ECS::EntityManager& entityManager_;
		ECSComponents::TransformComponent oldTransform_;
		ECSComponents::TransformComponent newTransform_;
		std::string description_;
	};

	/*============================================================================================
	* \class	MoveCommand
	* \brief	Specialized command for position changes
	*//*========================================================================================*/
	class MoveCommand : public ICommand
	{
	public:
		MoveCommand(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			const Math::eVec2<float>& oldPos,
			const Math::eVec2<float>& newPos
		)
			: entity_(entity)
			, entityManager_(entityManager)
			, oldPos_(oldPos)
			, newPos_(newPos)
		{}

		void execute() override
		{
			if (entity_.id() == UINT32_MAX || entityManager_.getMap().find(entity_.id()) == entityManager_.getMap().end())
			{
				return;
			}
			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform.pos = newPos_;
			}
			catch (...) { }
		}

		void undo() override
		{
			if (entity_.id() == UINT32_MAX || entityManager_.getMap().find(entity_.id()) == entityManager_.getMap().end())
			{
				return;
			}
			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform.pos = oldPos_;
			}
			catch (...) { }
		}

		std::string getDescription() const override
		{
			std::ostringstream oss;
			oss << "Move Entity " << entity_.id();
			return oss.str();
		}

	private:
		ECS::Entity entity_;
		ECS::EntityManager& entityManager_;
		Math::eVec2<float> oldPos_;
		Math::eVec2<float> newPos_;
	};

	/*============================================================================================
	* \class	RotateCommand
	* \brief	Specialized command for rotation changes
	*//*========================================================================================*/
	class RotateCommand : public ICommand
	{
	public:
		RotateCommand(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			const Math::eVec2<float>& oldRot,
			const Math::eVec2<float>& newRot
		)
			: entity_(entity)
			, entityManager_(entityManager)
			, oldRot_(oldRot)
			, newRot_(newRot)
		{}

		void execute() override
		{
			if (entity_.id() == UINT32_MAX || entityManager_.getMap().find(entity_.id()) == entityManager_.getMap().end())
			{
				return;
			}
			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform.rot = newRot_;
			}
			catch (...) { }
		}

		void undo() override
		{
			if (entity_.id() == UINT32_MAX || entityManager_.getMap().find(entity_.id()) == entityManager_.getMap().end())
			{
				return;
			}
			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform.rot = oldRot_;
			}
			catch (...) { }
		}

		std::string getDescription() const override
		{
			std::ostringstream oss;
			oss << "Rotate Entity " << entity_.id();
			return oss.str();
		}

	private:
		ECS::Entity entity_;
		ECS::EntityManager& entityManager_;
		Math::eVec2<float> oldRot_;
		Math::eVec2<float> newRot_;
	};

	/*============================================================================================
	* \class	ScaleCommand
	* \brief	Specialized command for scale changes
	*//*========================================================================================*/
	class ScaleCommand : public ICommand
	{
	public:
		ScaleCommand(
			ECS::Entity entity,
			ECS::EntityManager& entityManager,
			const Math::eVec2<float>& oldScale,
			const Math::eVec2<float>& newScale
		)
			: entity_(entity)
			, entityManager_(entityManager)
			, oldScale_(oldScale)
			, newScale_(newScale)
		{}

		void execute() override
		{
			if (entity_.id() == UINT32_MAX || entityManager_.getMap().find(entity_.id()) == entityManager_.getMap().end())
			{
				return;
			}
			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform.scale = newScale_;
			}
			catch (...) { }
		}

		void undo() override
		{
			if (entity_.id() == UINT32_MAX || entityManager_.getMap().find(entity_.id()) == entityManager_.getMap().end())
			{
				return;
			}
			try {
				auto& transform = entity_.get<ECSComponents::TransformComponent>(entityManager_);
				transform.scale = oldScale_;
			}
			catch (...) { }
		}

		std::string getDescription() const override
		{
			std::ostringstream oss;
			oss << "Scale Entity " << entity_.id();
			return oss.str();
		}

	private:
		ECS::Entity entity_;
		ECS::EntityManager& entityManager_;
		Math::eVec2<float> oldScale_;
		Math::eVec2<float> newScale_;
	};

	/*============================================================================================
	* \struct	EntityTransformSnapshot
	* \brief	Stores transform state for a single entity
	*//*========================================================================================*/
	struct EntityTransformSnapshot
	{
		uint32_t entityID;
		ECSComponents::TransformComponent transform;

		EntityTransformSnapshot() : entityID(UINT32_MAX) {}
		EntityTransformSnapshot(uint32_t id, const ECSComponents::TransformComponent& t)
			: entityID(id), transform(t) {}
	};

	/*============================================================================================
	* \class	MultiTransformCommand
	* \brief	Command for transforming a parent and all its children
	*
	*			Captures the transform state of a parent entity and all its children before
	*			and after a transformation, allowing proper undo/redo of hierarchical transforms
	*//*========================================================================================*/
	class MultiTransformCommand : public ICommand
	{
	public:
		MultiTransformCommand(
			ECS::EntityManager& entityManager,
			const std::string& description = "Transform Hierarchy"
		)
			: entityManager_(entityManager)
			, description_(description)
		{}

		// Capture transform state before modification
		void captureOldState(const std::vector<uint32_t>& entityIDs)
		{
			oldTransforms_.clear();
			for (uint32_t id : entityIDs)
			{
				auto& entityMap = entityManager_.getMap();
				if (entityMap.find(id) != entityMap.end())
				{
					ECS::Entity entity(id);
					if (entity.has<ECSComponents::TransformComponent>(entityManager_))
					{
						auto& tc = entity.get<ECSComponents::TransformComponent>(entityManager_);
						oldTransforms_.emplace_back(id, tc);
					}
				}
			}
		}

		// Capture transform state after modification
		void captureNewState(const std::vector<uint32_t>& entityIDs)
		{
			newTransforms_.clear();
			for (uint32_t id : entityIDs)
			{
				auto& entityMap = entityManager_.getMap();
				if (entityMap.find(id) != entityMap.end())
				{
					ECS::Entity entity(id);
					if (entity.has<ECSComponents::TransformComponent>(entityManager_))
					{
						auto& tc = entity.get<ECSComponents::TransformComponent>(entityManager_);
						newTransforms_.emplace_back(id, tc);
					}
				}
			}
		}

		void execute() override
		{
			// Apply new transforms
			for (const auto& snapshot : newTransforms_)
			{
				auto& entityMap = entityManager_.getMap();
				if (entityMap.find(snapshot.entityID) != entityMap.end())
				{
					ECS::Entity entity(snapshot.entityID);
					if (entity.has<ECSComponents::TransformComponent>(entityManager_))
					{
						auto& tc = entity.get<ECSComponents::TransformComponent>(entityManager_);
						tc = snapshot.transform;
					}
				}
			}
		}

		void undo() override
		{
			// Restore old transforms
			for (const auto& snapshot : oldTransforms_)
			{
				auto& entityMap = entityManager_.getMap();
				if (entityMap.find(snapshot.entityID) != entityMap.end())
				{
					ECS::Entity entity(snapshot.entityID);
					if (entity.has<ECSComponents::TransformComponent>(entityManager_))
					{
						auto& tc = entity.get<ECSComponents::TransformComponent>(entityManager_);
						tc = snapshot.transform;
					}
				}
			}
		}

		std::string getDescription() const override
		{
			std::ostringstream oss;
			oss << description_;
			if (oldTransforms_.size() > 1)
			{
				oss << " (" << oldTransforms_.size() << " entities)";
			}
			return oss.str();
		}

		bool isEmpty() const
		{
			return oldTransforms_.empty() && newTransforms_.empty();
		}

	private:
		ECS::EntityManager& entityManager_;
		std::string description_;
		std::vector<EntityTransformSnapshot> oldTransforms_;
		std::vector<EntityTransformSnapshot> newTransforms_;
	};
}
