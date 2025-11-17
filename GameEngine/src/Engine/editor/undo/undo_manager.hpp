/*============================================================================================
* @file         undo_manager.hpp
* @brief        Central undo/redo manager that integrates with the editor
*//*========================================================================================*/

#pragma once

#include "command_history.hpp"
#include "transform_commands.hpp"
#include "entity_commands.hpp"
#include "../editor_events.hpp"
#include "../../event/event_bus.hpp"
#include "../../ecs/entity.hpp"
#include <iostream>

namespace Editor
{
	/*============================================================================================
	* \class	UndoManager
	* \brief	Manages undo/redo operations and integrates with the event system
	*//*========================================================================================*/
	class UndoManager
	{
	public:
		/*============================================================================================
		* \brief   Constructor
		* \param[in] bus Event bus for subscribing to undo/redo events
		* \param[in] entityManager Reference to the entity manager
		* \param[in] maxHistorySize Maximum undo history (default: 20)
		*//*========================================================================================*/
		UndoManager(
			Event::EnoEventBus& bus,
			ECS::EntityManager& entityManager,
			size_t maxHistorySize = 20
		)
			: bus_(bus)
			, entityManager_(entityManager)
			, history_(maxHistorySize)
		{
			// Subscribe to undo/redo events
			bus_.subscribe<UndoAction>([this](const UndoAction&) {
				if (canUndo()) {
					undo();
				}
			});

			bus_.subscribe<RedoAction>([this](const RedoAction&) {
				if (canRedo()) {
					redo();
				}
			});
		}

		/*============================================================================================
		* \brief   Execute a command and add it to history
		*//*========================================================================================*/
		void executeCommand(std::unique_ptr<ICommand> command)
		{
			std::cout << "[UndoManager] Executing command: " << command->getDescription() << std::endl;
			history_.executeCommand(std::move(command));
			std::cout << "[UndoManager] Command added to history. History size: " << history_.size() << std::endl;
		}

		/*============================================================================================
		* \brief   Undo the last command
		*//*========================================================================================*/
		void undo()
		{
			std::cout << "[UndoManager] Undo requested. History size: " << history_.size() << ", Can undo: " << history_.canUndo() << std::endl;

			if (!canUndo())
			{
				std::cout << "[Undo] Nothing to undo" << std::endl;
				return;
			}

			try {
				if (history_.undo())
				{
					std::cout << "[Undo] " << history_.getRedoDescription() << std::endl;
				}
			}
			catch (const std::exception& e) {
				std::cout << "[UndoManager] Exception during undo: " << e.what() << std::endl;
				// Clear history to prevent further corruption
				clear();
			}
			catch (...) {
				std::cout << "[UndoManager] Unknown exception during undo - clearing history" << std::endl;
				clear();
			}
		}

		/*============================================================================================
		* \brief   Redo the last undone command
		*//*========================================================================================*/
		void redo()
		{
			if (history_.redo())
			{
				std::cout << "[Redo] " << history_.getUndoDescription() << std::endl;
			}
			else
			{
				std::cout << "[Redo] Nothing to redo" << std::endl;
			}
		}

		/*============================================================================================
		* \brief   Clear undo history (used when entity IDs might be reused)
		*//*========================================================================================*/
		void clearHistory()
		{
			std::cout << "[UndoManager] Clearing all undo history\n";
			clear();
		}

		/*============================================================================================
		* \brief   Record a transform change
		* \param[in] entity The entity being transformed
		* \param[in] oldTransform Previous transform state
		* \param[in] newTransform New transform state
		* \param[in] description Description of the transformation
		*//*========================================================================================*/
		void recordTransform(
			ECS::Entity entity,
			const ECSComponents::TransformComponent& oldTransform,
			const ECSComponents::TransformComponent& newTransform,
			const std::string& description = "Transform"
		)
		{
			auto command = std::make_unique<TransformCommand>(
				entity, entityManager_, oldTransform, newTransform, description
			);
			executeCommand(std::move(command));
		}

		/*============================================================================================
		* \brief   Record a position change
		*//*========================================================================================*/
		void recordMove(
			ECS::Entity entity,
			const Math::eVec2<float>& oldPos,
			const Math::eVec2<float>& newPos
		)
		{
			auto command = std::make_unique<MoveCommand>(
				entity, entityManager_, oldPos, newPos
			);
			executeCommand(std::move(command));
		}

		/*============================================================================================
		* \brief   Record a rotation change
		*//*========================================================================================*/
		void recordRotation(
			ECS::Entity entity,
			const Math::eVec2<float>& oldRot,
			const Math::eVec2<float>& newRot
		)
		{
			auto command = std::make_unique<RotateCommand>(
				entity, entityManager_, oldRot, newRot
			);
			executeCommand(std::move(command));
		}

		/*============================================================================================
		* \brief   Record a scale change
		*//*========================================================================================*/
		void recordScale(
			ECS::Entity entity,
			const Math::eVec2<float>& oldScale,
			const Math::eVec2<float>& newScale
		)
		{
			auto command = std::make_unique<ScaleCommand>(
				entity, entityManager_, oldScale, newScale
			);
			executeCommand(std::move(command));
		}

		/*============================================================================================
		* \brief   Check if undo is available
		*//*========================================================================================*/
		bool canUndo() const
		{
			return history_.canUndo();
		}

		/*============================================================================================
		* \brief   Check if redo is available
		*//*========================================================================================*/
		bool canRedo() const
		{
			return history_.canRedo();
		}

		/*============================================================================================
		* \brief   Clear all history
		*//*========================================================================================*/
		void clear()
		{
			history_.clear();
		}

		/*============================================================================================
		* \brief   Get the command history (for debugging or UI display)
		*//*========================================================================================*/
		const CommandHistory& getHistory() const
		{
			return history_;
		}

	private:
		Event::EnoEventBus& bus_;
		ECS::EntityManager& entityManager_;
		CommandHistory history_;
	};
}
