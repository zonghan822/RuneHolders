/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         audio_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   38%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   38%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   25%
* @date         10/3/2025
* @brief        Events here are used to communicate to and fro the audio manager
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

	/*============================================================================================
	 * \brief				Event published when an audio file fails to load
	 * \details				Contains information about the failed file for error reporting
	*//*========================================================================================*/
	struct AudioLoadError
	{
		std::string filename_;		// Name of the file that failed to load
		std::string filepath_;		// Full path to the file
		std::string extension_;		// File extension
		std::string errorReason_;	// Detailed error message
	};
}