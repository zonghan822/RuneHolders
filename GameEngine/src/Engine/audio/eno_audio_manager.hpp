#pragma once

// FMOD
#include "fmod.hpp"

// Events
#include "audio_events.hpp"

// Parameter Information
#include "audio_params.hpp"

// Event Bus
#include "../event/event_bus.hpp"

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
		EnoAudioManager(Event::EnoEventBus&);

		~EnoAudioManager();

		void cleanup();

		void init();

		// Getters
		FMOD::System* getSystem() const;
		std::unordered_map<std::string, FMOD::Sound*>& getSounds();

		// Update system
		void process();

		// Play all loaded sounds
		void playAll(const bool&);

		// Play sounds from a channel group by passing the name of the sound
		FMOD::Channel* playByGroup(const std::string&, const std::string&, const bool&);

		// (Overload) Play sounds from a channel group by passing the sound
		FMOD::Channel* playByGroup(FMOD::Sound*, const std::string&, const bool&);

		// Set up fade to a channel group
		void scheduleGroupFade(
			const std::string&,
			const float&, const float&, const float&);

		// Modify sound parameters
		// for all channels
		void modifyGlobal(const SoundParameters&);

		// Modify sound parameters
		// for a specific channel group
		void modifyChannelGroup(const std::string&, const SoundParameters&);

		// Set channel group
		void setChannelGroup(const std::string&);

		// Get a channel group by name
		FMOD::ChannelGroup* getChannelGroup(const std::string&) const;

		// Get the length of a sound sample by sound name
		float getSoundDurSec(const std::string&);

		// (Overload) Get the length of a sound sample by sound
		float getSoundDurSec(FMOD::Sound*);

		// ---------- Pause ---------- 

		// Set pause state of all groups
		void togglePauseAllGroups(const bool&);

		// Set pause state of a group by name
		void setPauseGroup(const std::string&, const bool&);

		// Set pause state of a channel
		void setPauseChannel(FMOD::Channel*, const bool&);

		// ---------- Stop ---------- 

		// Stop a channel group
		void stopChannelGroup(const std::string&);

		// Stop a channel
		void stopChannel(FMOD::Channel*);

		// ----------------------- Events -----------------------

		// Publish request for a sound by name
		void publishSoundRequest(const std::string&);

		// Received sound post-request
		void onSoundReceived(const LoadSound&);

		// Received request to play or pause a sound
		void onTogglePlay(const SetAudioState&);

		// Received request to change sound
		void onAudioChange(const Audio::SetNewAudio&);

	private:

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

		// Sounds
		std::unordered_map<std::string, FMOD::Sound*> sounds_;
		FMOD::Sound* selectedSound_;

		// DSPs
		std::vector<FMOD::DSP*> dsps_;

		// Event Bus
		Event::EnoEventBus& bus_;


		template <typename CH>
		// Helper for modifying a channel or channel group
		void modify(CH* ch, const SoundParameters& parameters)
		{
			// Basic
			modifyBasic(ch, parameters);

			// Reverb DSP
			FMOD::DSP* reverbDSP{ nullptr };
			if (parameters.reverb)
			{
				reverbDSP = modifyReverb(parameters);
			}

			// EQ DSP
			FMOD::DSP* eqDSP{ nullptr };
			if (parameters.eq)
			{
				eqDSP = modifyEQ(parameters);
			}

			// Append DSPs
			ch->addDSP(0, reverbDSP);
			ch->addDSP(0, eqDSP);

			dsps_.push_back(reverbDSP);
			dsps_.push_back(eqDSP);
		}

		template <typename CH>
		void modifyBasic(CH* ch, const SoundParameters& parameters)
		{
			ch->setVolume(parameters.volume);
			ch->setPitch(parameters.pitch);
			ch->setPan(parameters.pan);
		}

		// Helper for modifying a reverb DSP
		FMOD::DSP* modifyReverb(const SoundParameters&);

		// Helper for modifying an EQ DSP
		FMOD::DSP* modifyEQ(const SoundParameters&);
	};

}