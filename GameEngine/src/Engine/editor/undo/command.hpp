/*============================================================================================
* @file         command.hpp
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
