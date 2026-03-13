/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_audio_manager.hpp
* @author       Koe Yun Ji (yunji.koe@digipen.edu)   45%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   45%
* @date         10/3/2025
* @brief        Declaration file for the audio manager All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// FMOD
#include "fmod.hpp"
#include "fmod_errors.h"

// Events
#include "audio_events.hpp"

// Parameter Information
#include "../component/components/audio.hpp"
#include "audio_params.hpp"

// Event Bus
#include "../event/event_bus.hpp"

// Math
#include "../math/math.hpp"

//stds
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>


namespace Audio
{
	class EnoAudioManager 
	{
	public:
		/*============================================================================================
				 * \brief				Constructor
				 * \param[in]			A ref to the event bus to set up future events
		*//*========================================================================================*/
		EnoAudioManager(Event::EnoEventBus&);

		/*============================================================================================
				 * \brief				Destructor for the audio manager
		*//*========================================================================================*/
		~EnoAudioManager();

		/*============================================================================================
				 * \brief				Helper for cleaning up resources stored, such as dsps or sounds
				 * \return				Return type
		*//*========================================================================================*/
		void cleanup();

		/*============================================================================================
				 * \brief				Initializing the FMOD system
		*//*========================================================================================*/
		void init();

		/*============================================================================================
		 * \brief						Getter for the system
		 * \return						Pointer to the FMOD::System
		*//*========================================================================================*/
		FMOD::System* getSystem() const;

		/*============================================================================================
			 * \brief					Getter for the map to all the sounds stored in the manager
			 * \return					an unordered map of string keys and sound values
		*//*========================================================================================*/
		std::unordered_map<std::string, FMOD::Sound*>& getSounds();

		/*============================================================================================
		 * \brief						Update internal FMOD system
		*//*========================================================================================*/
		void process(float);

		/*============================================================================================
			 * \brief					Setup an audio component and its internal audio module
			 * \param					A reference to the audio component to set up
		*//*========================================================================================*/
		void prepareMod(ECSComponents::AudioComponent&);

		/*============================================================================================
			 * \brief					Play a sound from an audio component
		*//*========================================================================================*/
		void play(ECSComponents::AudioComponent&);

		/*============================================================================================
			 * \brief					(Overload) Play a sound from an audio component
		*//*========================================================================================*/
		void play(ECSComponents::AudioComponent&, Math::eVec2<float>);

		/*============================================================================================
			 * \brief					Update volume and pan on a currently-playing channel based on
			 *							distance/offset between the sound's world position and the
			 *							camera's world position. Call every frame for dynamic spatial audio.
			 * \param[in]				Audio component (must have isPlaying_ == true and a valid ch_)
			 * \param[in]				World position of the sound-emitting entity
			 * \param[in]				World position of the active camera
		*//*========================================================================================*/
		void updateSpatialAudio(ECSComponents::AudioComponent&, Math::eVec2<float> soundPos, Math::eVec2<float> camPos);

		/*============================================================================================
		 * \brief						Play all loaded sounds
		 * \param[in]					Set pause state for sounds
		*//*========================================================================================*/
		void playAll(const bool&);

		/*============================================================================================
		 * \brief						Play sounds from a channel group
		 * \param[in]					Pointer to the Sound
		 * \param[in]					Pointer to the Channel Group
		 * \param[in]					Set pause state
		 * \return						pointer to the channel used to play the sound
		*//*========================================================================================*/
		FMOD::Channel* playByGroup(FMOD::Sound*, FMOD::ChannelGroup*, const bool);

		/*============================================================================================
		 * \brief						(Overload) Play sounds from a channel group
		 * \param[in]					Name of sound
		 * \param[in]					Channel group name
		 * \param[in]					Set pause state
		 * \return						pointer to the channel used to play the sound
		*//*========================================================================================*/
		FMOD::Channel* playByGroup(const std::string&, const std::string&, const bool);

		/*============================================================================================
		 * \brief						(Overload) Play sounds from a channel group
		 * \param[in]					Pointer to the Sound
		 * \param[in]					Channel group name
		 * \param[in]					Set pause state
		 * \return						Pointer to the channel used to play the sound
		*//*========================================================================================*/
		FMOD::Channel* playByGroup(FMOD::Sound*, const std::string&, const bool);

		/*============================================================================================
		 * \brief						(Overload) Play sounds from a channel group
		 * \param[in]					Sound name
		 * \param[in]					Pointer to the Channel Group
		 * \param[in]					Set pause state
		 * \return						Pointer to the channel used to play the sound
		*//*========================================================================================*/
		FMOD::Channel* playByGroup(const std::string&, FMOD::ChannelGroup*, const bool);

		/*============================================================================================
		 * \brief						Play sounds from with no channel group
		 * \param[in]					Channel group name
		 * \param[in]					Fade in time
		 * \param[in]					Fade out time
		 * \param[in]					Duration in seconds
		*//*========================================================================================*/
		FMOD::Channel* playByChannel(FMOD::Sound*, const bool);

		/*============================================================================================
		 * \brief						Set up fade to a channel group
		 * \param[in]					Channel group name
		 * \param[in]					Fade in time
		 * \param[in]					Fade out time
		 * \param[in]					Duration in seconds
		*//*========================================================================================*/
		void scheduleGroupFade(const std::string&, float, float, float);

		/*============================================================================================
		 * \brief				Based on the audio component passed, create and return an
		 *						appropriate SoundParameters struct
		 * \param[in]			Audio Component
		*//*========================================================================================*/
		SoundParameters createSP(ECSComponents::AudioComponent&);

		/*============================================================================================
		 * \brief						Set up fade to a channel group
		 * \param[in]					Group name
		 * \param[in]					Fade in time
		 * \param[in]					Fade out time
		 * \param[in]					Duration in seconds
		*//*========================================================================================*/
		void modifyGlobal(const SoundParameters&);

		/*============================================================================================
		 * \brief						Modify sound parameters for a specific channel group
		 * \param[in]					Group name
		 * \param[in]					Parameters to modify with
		*//*========================================================================================*/
		void modifyChannelGroup(const std::string&, const SoundParameters&);

		/*============================================================================================
		 * \brief						Set channel group
		 * \param[in]					Channel group name
		*//*========================================================================================*/
		void setChannelGroup(const std::string&);

		/*============================================================================================
		 * \brief						Get a channel group by name
		 * \param[in]					Channel group name
		*//*========================================================================================*/
		FMOD::ChannelGroup* getChannelGroup(const std::string&) const;

		/*============================================================================================
		 * \brief						Get the length of a sound sample (in seconds) by sound name
		 * \param[in]					Name of sound
		 * \return						Length of sound sample in seconds
		*//*========================================================================================*/
		float getSoundDurSec(const std::string&);

		/*============================================================================================
		 * \brief						(Overload) Get the length of a sound sample by sound
		 * \param[in]					Pointer to a sound
		 * \return						Length of sound sample in seconds
		*//*========================================================================================*/
		float getSoundDurSec(FMOD::Sound*);

		// ---------- Pause ---------- 

		/*============================================================================================
		 * \brief						Set pause state of all groups
		 * \param[in]					Pause state to set
		*//*========================================================================================*/
		void togglePauseAllGroups(const bool&);

		/*============================================================================================
		 * \brief						Set pause state of a group by name
		 * \param[in]					Channel group name
		 * \param[in]					Pause state to set
		*//*========================================================================================*/
		void setPauseGroup(const std::string&, const bool&);

		/*============================================================================================
		 * \brief						Set pause state of a channel
		 * \param[in]					Pointer to a Channel
		 * \param[in]					Pause state to set
		*//*========================================================================================*/
		void setPauseChannel(FMOD::Channel*, const bool&);

		// ---------- Stop ----------

		/*============================================================================================
		 * \brief						Immediately stop all channels and clear the channel list.
		 *								Called on scene stop to prevent orphaned channels from the
		 *								previous play session from bleeding into the next one.
		*//*========================================================================================*/
		void stopAllImmediate();

		/*============================================================================================
		 * \brief						Stop a channel group
		 * \param[in]					Channel group name
		*//*========================================================================================*/
		void stopChannelGroup(const std::string&);

		/*============================================================================================
		* \brief				Stop all channels within a channel group after designated time
		* \param[in]			Channel group name
		* \param[in]			Duration
		*//*========================================================================================*/
		void stopAllChannels(FMOD::ChannelGroup*);

		/*============================================================================================
		 * \brief						Stop a channel
		 * \param[in]					Pointer to a Channel
		*//*========================================================================================*/
		void stopChannel(FMOD::Channel*);

		/*======================================== Events ==========================================*/

		/*============================================================================================
		 * \brief						Publish request for a sound by name
		 * \param[in]					Name of sound to request for
		*//*========================================================================================*/
		void publishSoundRequest(const std::string&);

		/*============================================================================================
		 * \brief						Received sound post-request
		 * \param[in]					LoadSound struct 
										sound and sound name to store
		*//*========================================================================================*/
		void onSoundReceived(const LoadSound&);

		/*============================================================================================
		 * \brief						Received request to play or pause a sound
		 * \param[in]					AudioState struct 
										Name of group to play
		*//*========================================================================================*/
		void onTogglePlay(const SetAudioState&);

		/*============================================================================================
		 * \brief						Received request to play or pause a sound
		 * \param[in]					SetNewAudio struct 
										Name of audio to select
		*//*========================================================================================*/
		void onAudioChange(const Audio::SetNewAudio&);

		/*============================================================================================
		 * \brief						Received request to play or pause a sound immediately
		 * \param[in]					PlayInstant struct 
										Name of group and audio to play in group
		*//*========================================================================================*/
		void onAudioInstantPlay(const Audio::PlayInstant&);

	private:

		struct PendingStop
		{
			FMOD::ChannelGroup* group_;
			float timeRemaining_;
		};

		// Params
		static constexpr float FADE_IN = 5.f;
		static constexpr float FADE_OUT = 5.f;

		// States
		bool soundPlaying_;

		// FMOD System
		FMOD::System* system;

		// Channels
		std::vector<FMOD::Channel*> channels_;

		// Channel Groups
		std::unordered_map<std::string, FMOD::ChannelGroup*> channelGroups_;

		// Pending Channel Groups
		std::vector<PendingStop> pendingStops_;

		// Sounds
		std::unordered_map<std::string, FMOD::Sound*> sounds_;
		FMOD::Sound* selectedSound_;

		// DSPs
		std::vector<FMOD::DSP*> dsps_;

		// Event Bus
		Event::EnoEventBus& bus_;

		/*============================================================================================
		 * \brief						Received request to play or pause a sound
		 * \param[in]					SetNewAudio struct (Name of audio to select)
		*//*========================================================================================*/
		template <typename CH>
		void modify(CH* ch, const SoundParameters& parameters)
		{
			// Basic
			modifyBasic(ch, parameters);

			// Reverb DSP
			FMOD::DSP* reverbDSP{ nullptr };
			if (parameters.reverb)
				reverbDSP = modifyReverb(parameters);

			// EQ DSP
			FMOD::DSP* eqDSP{ nullptr };
			if (parameters.eq)
				eqDSP = modifyEQ(parameters);

			// Low Pass DSP
			FMOD::DSP* lpDSP{ nullptr };
			if (parameters.lowPass)
				lpDSP = modifyLowPass(parameters);

			// High Pass DSP
			FMOD::DSP* hpDSP{ nullptr };
			if (parameters.highPass)
				hpDSP = modifyHighPass(parameters);

			// Append DSPs (only non-null to avoid FMOD errors and null dereference on cleanup)
			if (reverbDSP) { ch->addDSP(0, reverbDSP); dsps_.push_back(reverbDSP); }
			if (eqDSP)     { ch->addDSP(0, eqDSP);     dsps_.push_back(eqDSP); }
			if (lpDSP)     { ch->addDSP(0, lpDSP);     dsps_.push_back(lpDSP); }
			if (hpDSP)     { ch->addDSP(0, hpDSP);     dsps_.push_back(hpDSP); }
		}

		/*============================================================================================
		 * \brief						Helper for modifying a channel or channel group's volume/pitch/pan
		 * \param[in]					Channel/Channel group to be modified
		 * \param[in]					SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
		*//*========================================================================================*/
		template <typename CH>
		void modifyBasic(CH* ch, const SoundParameters& parameters)
		{
			ch->setVolume(parameters.volume);
			ch->setPitch(parameters.pitch);
			ch->setPan(parameters.pan);
		}

		/*============================================================================================
		 * \brief						Apply loop settings from AudioComponent to FMOD sound
		 * \param[in]					Pointer to FMOD::Sound
		 * \param[in]					AudioComponent with loop settings
		*//*========================================================================================*/
		void applyLoopSettings(FMOD::Sound* sound, ECSComponents::AudioComponent& audioComp);

		/*============================================================================================
		 * \brief						Helper for modifying a reverb DSP
		 * \param[in]					SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
		 * \return						Pointer to a DSP to use to modify channel/channel group
		*//*========================================================================================*/
		FMOD::DSP* modifyReverb(const SoundParameters&);

		/*============================================================================================
		 * \brief						Helper for modifying an EQ DSP
		 * \param[in]					SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
		 * \return						Pointer to a DSP to use to modify channel/channel group
		*//*========================================================================================*/
		FMOD::DSP* modifyEQ(const SoundParameters&);

		/*============================================================================================
		* \brief						Modifyng the lowpass of audio
		* \param[in]					SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
		* \return						Pointer to a DSP to use to modify channel/channel group
		*//*========================================================================================*/
		FMOD::DSP* modifyHighPass(const SoundParameters&);

		/*============================================================================================
		 * \brief						Modifyng the lowpass of audio
		 * \param[in]					SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
		 * \return						Pointer to a DSP to use to modify channel/channel group
		*//*========================================================================================*/
		FMOD::DSP* modifyLowPass(const SoundParameters&);
	};



}