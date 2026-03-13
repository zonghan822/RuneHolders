/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         command.hpp
* @author       Caven Koh (caven.koh@digipen.edu)   50%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @date         2025-11-29
* @brief        Base command interface for the undo/redo system using Command Pattern
*//*========================================================================================*/

#pragma once

#include <memory>
#include <string>

namespace Editor
{
	/*============================================================================================
	* \class	ICommand
	* \brief	Interface for all undoable commands
	*
	*			Every action that should be undoable (transform, add, delete)
	*			implements this interface
	*//*========================================================================================*/
	class ICommand
	{
	public:
		virtual ~ICommand() = default;

		/*============================================================================================
		* \brief   Execute the command
		*//*========================================================================================*/
		virtual void execute() = 0;

		/*============================================================================================
		* \brief   Undo the command (restore previous state)
		*//*========================================================================================*/
		virtual void undo() = 0;

		/*============================================================================================
		* \brief   Get command description for debugging
		*//*========================================================================================*/
		virtual std::string getDescription() const = 0;
	};
}
