/*============================================================================================
* @file         transform_commands.hpp
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
}
