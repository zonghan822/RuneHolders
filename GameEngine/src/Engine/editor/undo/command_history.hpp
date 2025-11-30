/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         command_history.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         2025-11-29
* @brief        Command history manager for undo/redo functionality
*//*========================================================================================*/

#pragma once

#include "command.hpp"
#include <vector>
#include <memory>
#include <algorithm>

namespace Editor
{
	/*============================================================================================
	* \class	CommandHistory
	* \brief	Manages the undo/redo stack with a configurable history size
	*//*========================================================================================*/
	class CommandHistory
	{
	public:
		/*============================================================================================
		* \brief   Constructor
		* \param[in] maxHistorySize Maximum number of commands to keep (default: 20)
		*//*========================================================================================*/
		explicit CommandHistory(size_t maxHistorySize = 20)
			: maxHistorySize_(maxHistorySize), currentIndex_(-1) {}

		/*============================================================================================
		* \brief   Execute and add a command to history
		* \param[in] command The command to execute
		*//*========================================================================================*/
		void executeCommand(std::unique_ptr<ICommand> command)
		{
			std::cout << "[CommandHistory] executeCommand - Before: currentIndex=" << currentIndex_ << ", size=" << commands_.size() << "\n";

			// Execute the command
			command->execute();

			// Clear any commands after current index (redo history is invalidated)
			// Only erase if currentIndex is valid and there are commands after it
			if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(commands_.size()) - 1)
			{
				std::cout << "[CommandHistory] Erasing redo history from index " << (currentIndex_ + 1) << " to end\n";
				commands_.erase(commands_.begin() + currentIndex_ + 1, commands_.end());
			}
			else if (currentIndex_ == -1 && !commands_.empty())
			{
				// All commands were undone, clear everything
				std::cout << "[CommandHistory] Clearing all commands (all were undone)\n";
				commands_.clear();
			}

			// Add the new command
			commands_.push_back(std::move(command));
			std::cout << "[CommandHistory] Added command, size now=" << commands_.size() << "\n";

			// Enforce max history size (remove oldest commands)
			if (commands_.size() > maxHistorySize_)
			{
				std::cout << "[CommandHistory] Max size exceeded, removing oldest\n";
				commands_.erase(commands_.begin());
			}
			else
			{
				++currentIndex_;
			}

			// Update current index to point to the latest command
			currentIndex_ = static_cast<int>(commands_.size()) - 1;
			std::cout << "[CommandHistory] executeCommand - After: currentIndex=" << currentIndex_ << ", size=" << commands_.size() << "\n";
		}

		/*============================================================================================
		* \brief   Undo the last command
		* \return  True if undo was successful, false if nothing to undo
		*//*========================================================================================*/
		bool undo()
		{
			std::cout << "[CommandHistory] undo - Before: currentIndex=" << currentIndex_ << ", size=" << commands_.size() << "\n";

			if (!canUndo())
			{
				std::cout << "[CommandHistory] Cannot undo (currentIndex < 0)\n";
				return false;
			}

			if (currentIndex_ >= static_cast<int>(commands_.size()))
			{
				std::cout << "[CommandHistory] ERROR: currentIndex out of bounds!\n";
				return false;
			}

			commands_[currentIndex_]->undo();
			--currentIndex_;
			std::cout << "[CommandHistory] undo - After: currentIndex=" << currentIndex_ << "\n";
			return true;
		}

		/*============================================================================================
		* \brief   Redo the next command
		* \return  True if redo was successful, false if nothing to redo
		*//*========================================================================================*/
		bool redo()
		{
			if (!canRedo())
				return false;

			++currentIndex_;
			commands_[currentIndex_]->execute();
			return true;
		}

		/*============================================================================================
		* \brief   Check if undo is available
		*//*========================================================================================*/
		bool canUndo() const
		{
			return currentIndex_ >= 0;
		}

		/*============================================================================================
		* \brief   Check if redo is available
		*//*========================================================================================*/
		bool canRedo() const
		{
			return currentIndex_ < static_cast<int>(commands_.size()) - 1;
		}

		/*============================================================================================
		* \brief   Clear all command history
		*//*========================================================================================*/
		void clear()
		{
			commands_.clear();
			currentIndex_ = -1;
		}

		/*============================================================================================
		* \brief   Get the current number of commands in history
		*//*========================================================================================*/
		size_t size() const
		{
			return commands_.size();
		}

		/*============================================================================================
		* \brief   Get description of the command that would be undone
		*//*========================================================================================*/
		std::string getUndoDescription() const
		{
			if (canUndo())
				return commands_[currentIndex_]->getDescription();
			return "";
		}

		/*============================================================================================
		* \brief   Get description of the command that would be redone
		*//*========================================================================================*/
		std::string getRedoDescription() const
		{
			if (canRedo())
				return commands_[currentIndex_ + 1]->getDescription();
			return "";
		}

	private:
		std::vector<std::unique_ptr<ICommand>> commands_;
		int currentIndex_;  // Index of the last executed command (-1 if empty)
		size_t maxHistorySize_;
	};
}
