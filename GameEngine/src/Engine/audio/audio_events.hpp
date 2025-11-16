/*============================================================================================
* @file         audio_events.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Events here are used to communicate to and fro the audio manager
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// FMOD
#include "fmod.hpp"

// stds
#include <string>


namespace Audio
{
	/*============================================================================================
	 * \brief				A request from audio manager -> loader for a specific sound
	*//*========================================================================================*/
	struct RequestSound
	{
		std::string sound_;
	};

	/*============================================================================================
		 * \brief				A request from loader -> audio manager to receive a sound
	*//*========================================================================================*/
	struct LoadSound
	{
		std::string soundName_;
		FMOD::Sound* sound_;
	};

	/*============================================================================================
		 * \brief				A request from input -> audio manager to set audio state
	*//*========================================================================================*/
	struct SetAudioState 
	{
		std::string groupName_;
	};

	/*============================================================================================
		 * \brief				A request from input -> audio manager to select a new audio to play
	*//*========================================================================================*/
	struct SetNewAudio
	{
		std::string soundName_;
	};

	/*============================================================================================
	 * \brief				A request from input -> audio manager to play a sound immediately
	*//*========================================================================================*/
	struct PlayInstant
	{
		std::string groupName_;
		std::string soundName_;
	};

	/*============================================================================================
	 * \brief				Behaves as a tag event
	*//*========================================================================================*/
	struct StopBGM {
		
	};
}