/*============================================================================================
* @file         audio_params.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Params for a user to define, which includes reverb, eq, volume, pitch and pan.
*//*========================================================================================*/


#pragma once

// fmod
#include "fmod.hpp"

// stds
#include <optional>


namespace Audio
{
	// --------------------- Reverbs ---------------------
	struct SFXReverb
	{
		float wet							= 0.0f;
		float dry							= 0.0f;
		float density						= 100.f;
		float diffusion						= 100.f;
		float highcut						= 20000.f;
		float decayTime						= 1500.f;
		float earlyDelay					= 20.f;
		float lateDelay						= 40.f;
		float earlyLateMix					= 50.f;
		float hfDecayRatio					= 50.f;
		float hfRef							= 5000.f;
		float lowShelfGain					= 0.f;
		float lowShelfFreq					= 250.f;
	};

	struct ConvolutionReverb
	{
		float wet							= 0.f;
		float dry							= 0.f;
		float ir							= 0.f;
		float linked						= 0.f;
	};

	struct Reverb
	{
		FMOD_DSP_TYPE type;

		std::optional<SFXReverb> sfx;
		std::optional<ConvolutionReverb> convolution;
	};

	// ----------------------- EQ ------------------------

	struct ParamEQ
	{
		float gain							= 0.f;
		float center						= 1000.f;
		float bandwidth						= 1.f;
	};

	struct ThreeEQ
	{
		float lowGain						= 0.f;
		float midGain						= 0.f;
		float highGain						= 0.f;
		float lowCrossOver					= 360.f;
		float highCrossOver					= 3600.f;
	};

	struct EQ
	{
		FMOD_DSP_TYPE type;

		std::optional<ParamEQ> paramEQ;
		std::optional<ThreeEQ> threeEQ;
	};

	// -------------------- Parameters --------------------
	struct SoundParameters
	{
		float volume = 0.01f;
		float pitch = 1.f;
		float pan = 0.f;

		std::optional<Reverb> reverb;
		std::optional<EQ> eq;
	};
}