#pragma once


// Engine Component(s)
#include "../../audio/eno_audio_module.hpp"

// Event
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
			signals["started"] = Events::EnoSignal();
			signals["finished"] = Events::EnoSignal();
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals;
		Events::EnoSignal& signal(const std::string& name) { return signals.at(name); }

		Audio::EnoAudioModule audioMod_;

		double volume_{ 50.0 };
		double pitchScale_{ 0.0 };
		double panScale_{ 0.0 };
		
		// Audio position resets when false
		bool isPlaying_{ false };

		// Keeps playback position, as stream is paused
		bool isAutoplay_{ false };

		template <typename Visitor>
		void reflect(Visitor&& v)
		{
			// Audio Module
			//v("Sound Name",			audioMod_.name_,		"char32");
			//v("Sound Path",			audioMod_.path_,		"char256");
			//v("Channel Group",		audioMod_.chGroup_,		"char32");
			//v("Loop Mod",			audioMod_.loop_,		"char32");
			//v("Loop Start",			audioMod_.loopStart_,	"float");
			//v("Loop End",			audioMod_.loopEnd_,		"float");

			// Component Properties
			v("Volume",				volume_,				"double");
			v("Pitch Scale",		pitchScale_,			"double");
			v("Pan Scale",			panScale_,				"double");

			v("Is Playing",			isPlaying_,				"bool");
			v("Is Autoplay",		isAutoplay_,			"bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& v) const
		{
			// Audio Module
			//v("Sound Name", audioMod_.name_, "char32");
			//v("Sound Path", audioMod_.path_, "char256");
			//v("Channel Group", audioMod_.chGroup_, "char32");
			//v("Loop Mod", audioMod_.loop_, "char32");
			//v("Loop Start", audioMod_.loopStart_, "float");
			//v("Loop End", audioMod_.loopEnd_, "float");

			// Component Properties
			v("Volume", volume_, "double");
			v("Pitch Scale", pitchScale_, "double");
			v("Pan Scale", panScale_, "double");

			v("Is Playing", isPlaying_, "bool");
			v("Is Autoplay", isAutoplay_, "bool");
		}
	};
}