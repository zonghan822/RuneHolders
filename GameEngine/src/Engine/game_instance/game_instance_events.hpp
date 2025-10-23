/*============================================================================================
* @file         game_instance_events.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Events to interface with the game scene
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


#pragma once

namespace GameInstance
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
}