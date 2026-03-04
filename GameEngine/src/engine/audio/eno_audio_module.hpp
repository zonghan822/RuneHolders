/*============================================================================================
* @file         eno_audio_module.hpp
* @author       Zhe An  (@digipen.edu)
* @date         30-11-2025
* @brief        Audio Module for Audio manager to handle sounds
*//*========================================================================================*/

#pragma once

// FMOD
#include "fmod.hpp"

// stds
#include <string>
#include <cstring>

namespace Audio
{
	struct EnoAudioModule
	{
		enum class LoopMode { Disabled, Forward, PingPong };

		EnoAudioModule();
		EnoAudioModule(const std::string& name, const std::string& path, FMOD::Sound* sound, const std::string& group = "Master");

		// Metadata
		char name_[32];
		char path_[256];

		// Sound
		FMOD::Sound* sound_;
		FMOD::Channel* ch_;
		char chGroup_[32];

		// Properties
		LoopMode loop_;
		float loopStart_;
		float loopEnd_;
	};
}