/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         editor_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   83%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   17%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

namespace Editor
{
	/*============================================================================================
	 * \brief				A request from input -> game scene for to play the scene
	*//*========================================================================================*/
	struct PlayScene {};

	/*============================================================================================
	 * \brief				A request from input -> game scene for to stop the scene
	*//*========================================================================================*/
	struct StopScene {};

	/*============================================================================================
	 * \brief				A request from input -> game scene for to pause the scene
	*//*========================================================================================*/
	struct PauseScene {};

	/*============================================================================================
	 * \brief				A request from game scene -> window manager to create a game window
	*//*========================================================================================*/
	struct CreateGameWindow {};

	/*============================================================================================
	 * \brief				A request from game scene -> window manager to destroy a game window
	*//*========================================================================================*/
	struct DestroyGameWindow {};

	/** @brief Event published when the game finishes initialization. */
	struct GameInitialized {};
	/** @brief Event published when the game has fully stopped. */
	struct GameStopped {};


	/*============================================================================================
	 * \brief				A request to delete the currently selected entity
	*//*========================================================================================*/
	struct DeleteSelectedEntity {};

	/*============================================================================================
	 * \brief				Request to undo the last action
	*//*========================================================================================*/
	struct UndoAction {};

	/*============================================================================================
	 * \brief				Request to redo the last undone action
	*//*========================================================================================*/
	struct RedoAction {};

	/*============================================================================================
	 * \brief				Event published when a scene is cleared/loaded to reset UI state
	*//*========================================================================================*/
	struct SceneCleared {};
}