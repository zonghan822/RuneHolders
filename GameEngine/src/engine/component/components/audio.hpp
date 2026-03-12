/*============================================================================================
* @file         audio.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         30-11-2025
* @brief        Audio component for serialising and functionality
*//*========================================================================================*/

#pragma once


// Engine Component(s)
#include "../../audio/eno_audio_module.hpp"

// Signals
#include "../../event/signals.hpp"

// stds
#include <string>
#include <iostream>


namespace ECSComponents
{
	struct AudioComponent
	{
		// Predefine signal(s) for the user to utilize
		AudioComponent()
		{
			signals_["started"] = Events::EnoSignal();
			signals_["finished"] = Events::EnoSignal();
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		Audio::EnoAudioModule audioMod_;

		double volume_{ 50.0 };
		double pitchScale_{ 1.0 };  // 1.0 = normal pitch (was 0.0 which would silence audio!)
		double panScale_{ 0.0 };

		// Audio position resets when false
		bool isPlaying_{ false };

		// Keeps playback position, as stream is paused
		bool isAutoplay_{ false };

		// Spatial audio: attenuates volume and pans based on distance/offset from camera
		bool spatialAudio_{ false };
		float maxAudibleDistance_{ 500.0f };

		// Reverb settings (per entity)
		bool enableReverb_{ false };
		float reverbWet_{ 0.0f };
		float reverbDry_{ 1.0f };
		float reverbDecay_{ 1500.0f };

		// EQ settings (per entity)
		bool enableEQ_{ false };
		float eqLowGain_{ 0.0f };
		float eqMidGain_{ 0.0f };
		float eqHighGain_{ 0.0f };

		template <typename Visitor>
		void reflect(Visitor&& v)
		{
			// Audio Module
			v("Sound Name",			audioMod_.name_,		"char32");
			v("Sound Path",			audioMod_.path_,		"char256");
			v("Channel Group",		audioMod_.chGroup_,		"char32");
			v("Loop Mode",			audioMod_.loop_,		"loopmode");
			v("Loop Start",			audioMod_.loopStart_,	"float");
			v("Loop End",			audioMod_.loopEnd_,		"float");

			// Component Properties
			v("Volume",				volume_,				"double");
			v("Pitch Scale",		pitchScale_,			"double");
			v("Pan Scale",			panScale_,				"double");

			v("Is Playing",			isPlaying_,				"bool");
			v("Is Autoplay",		isAutoplay_,			"bool");

			// Spatial audio settings
			v("Spatial Audio",			spatialAudio_,				"bool");
			v("Max Audible Distance",	maxAudibleDistance_,		"float");

			// Reverb settings
			v("Enable Reverb",		enableReverb_,			"bool");
			v("Reverb Wet",			reverbWet_,				"float");
			v("Reverb Dry",			reverbDry_,				"float");
			v("Reverb Decay",		reverbDecay_,			"float");

			// EQ settings
			v("Enable EQ",			enableEQ_,				"bool");
			v("EQ Low Gain",		eqLowGain_,				"float");
			v("EQ Mid Gain",		eqMidGain_,				"float");
			v("EQ High Gain",		eqHighGain_,			"float");
		}

		template <typename Visitor>
		void reflect(Visitor&& v) const
		{
			 //Audio Module
			v("Sound Name", audioMod_.name_, "char32");
			v("Sound Path", audioMod_.path_, "char256");
			v("Channel Group", audioMod_.chGroup_, "char32");
			v("Loop Mode", audioMod_.loop_, "loopmode");
			v("Loop Start", audioMod_.loopStart_, "float");
			v("Loop End", audioMod_.loopEnd_, "float");

			// Component Properties
			v("Volume", volume_, "double");
			v("Pitch Scale", pitchScale_, "double");
			v("Pan Scale", panScale_, "double");

			v("Is Playing", isPlaying_, "bool");
			v("Is Autoplay", isAutoplay_, "bool");

			// Spatial audio settings
			v("Spatial Audio",			spatialAudio_,				"bool");
			v("Max Audible Distance",	maxAudibleDistance_,		"float");

			// Reverb settings
			v("Enable Reverb", enableReverb_, "bool");
			v("Reverb Wet", reverbWet_, "float");
			v("Reverb Dry", reverbDry_, "float");
			v("Reverb Decay", reverbDecay_, "float");

			// EQ settings
			v("Enable EQ", enableEQ_, "bool");
			v("EQ Low Gain", eqLowGain_, "float");
			v("EQ Mid Gain", eqMidGain_, "float");
			v("EQ High Gain", eqHighGain_, "float");
		}
	};
}