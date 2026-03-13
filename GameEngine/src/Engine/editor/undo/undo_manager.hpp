/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         undo_manager.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   60%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   20%
* @co-author    Caven Koh (caven.koh@digipen.edu)   20%
* @date         2025-11-29
* @brief        Central undo/redo manager that integrates with the editor
*//*========================================================================================*/

#pragma once

#include "command_history.hpp"
#include "transform_commands.hpp"
#include "entity_commands.hpp"
#include "../editor_events.hpp"
#include "../../event/event_bus.hpp"
#include "../../ecs/entity.hpp"
#include "../../graphics/graphics.hpp"
#include <iostream>
#include <unordered_map>

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
			Graphics::EnoGraphicsManager& graphicsManager,
			size_t maxHistorySize = 30
		)
			: bus_(bus)
			, entityManager_(entityManager)
			, graphicsMng_(graphicsManager)
			, maxHistorySize_(maxHistorySize)
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
		* \brief   Capture all components of an entity as a type->any map (for inspector undo)
		*//*========================================================================================*/
		std::unordered_map<std::type_index, std::any> captureComponentsMap(ECS::Entity entity)
		{
			return EntitySerializer::captureComponentsMap(entity, entityManager_);
		}

		/*============================================================================================
		* \brief   Record a change to any component(s) on an entity.
		*          Call captureComponentsMap() before and after the edit, then pass both here.
		*//*========================================================================================*/
		void recordComponentChange(
			ECS::Entity entity,
			std::unordered_map<std::type_index, std::any> before,
			std::unordered_map<std::type_index, std::any> after,
			const std::string& description = "Edit Component"
		)
		{
			executeCommand(std::make_unique<ComponentChangeCommand>(
				entity, entityManager_, graphicsMng_, std::move(before), std::move(after), description));
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
		* \brief   Switch the active scene, saving the current history and restoring (or
		*          creating) the history for the new scene. Call this whenever a scene loads.
		* \param[in] oldScene  Name of the scene being left (empty string = no previous scene)
		* \param[in] newScene  Name of the scene being entered
		*//*========================================================================================*/
		void switchScene(const std::string& oldScene, const std::string& newScene)
		{
			// Save current history under the old scene name (only if we had one)
			if (!oldScene.empty())
			{
				sceneHistories_.emplace(oldScene, std::move(history_));
			}

			// Restore or create history for the new scene
			auto it = sceneHistories_.find(newScene);
			if (it != sceneHistories_.end())
			{
				history_ = std::move(it->second);
				sceneHistories_.erase(it);
				std::cout << "[UndoManager] Restored history for scene '" << newScene << "'\n";
			}
			else
			{
				history_ = CommandHistory(maxHistorySize_);
				std::cout << "[UndoManager] Fresh history for scene '" << newScene << "'\n";
			}

			currentScene_ = newScene;
		}

		/*============================================================================================
		* \brief   Clear all history (current scene + all stored scenes)
		*//*========================================================================================*/
		void clear()
		{
			history_.clear();
			sceneHistories_.clear();
		}

		/*============================================================================================
		* \brief   Get the command history (for debugging or UI display)
		*//*========================================================================================*/
		const CommandHistory& getHistory() const
		{
			return history_;
		}

		/*============================================================================================
		* \brief   Get a reference to the entity manager
		*//*========================================================================================*/
		ECS::EntityManager& getEntityManager()
		{
			return entityManager_;
		}

	private:
		Event::EnoEventBus& bus_;
		ECS::EntityManager& entityManager_;
		Graphics::EnoGraphicsManager& graphicsMng_;
		size_t maxHistorySize_;
		CommandHistory history_;
		std::string currentScene_;
		std::unordered_map<std::string, CommandHistory> sceneHistories_;
	};
}
