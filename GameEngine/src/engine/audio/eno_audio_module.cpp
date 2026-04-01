/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_audio_module.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#include "eno_audio_module.hpp"

namespace Audio
{
	EnoAudioModule::EnoAudioModule()
		:
		sound_{ nullptr },
		ch_{ nullptr },
		loop_{ LoopMode::Disabled },
		loopStart_{ 0.f },
		loopEnd_{ 0.f }
	{
		name_[0] = '\0';
		path_[0] = '\0';

		std::strncpy(chGroup_, "Master", sizeof(chGroup_) - 1);
		chGroup_[sizeof(chGroup_ - 1)] = '\0';
	}

	EnoAudioModule::EnoAudioModule(
		const std::string& name, const std::string& path, FMOD::Sound* sound, const std::string& group)
		:
		sound_{ sound },
		ch_{ nullptr },
		loop_ { LoopMode::Disabled },
		loopStart_{ 0.f },
		loopEnd_{ 0.f }
	{
		std::strncpy(name_, name.c_str(), sizeof(name_) - 1);
		name_[sizeof(name_ - 1)] = '\0';
		
		std::strncpy(path_, path.c_str(), sizeof(path_) - 1);
		path_[sizeof(path_ - 1)] = '\0';

		std::strncpy(chGroup_, group.c_str(), sizeof(chGroup_) - 1);
		chGroup_[sizeof(chGroup_ - 1)] = '\0';
	}
}