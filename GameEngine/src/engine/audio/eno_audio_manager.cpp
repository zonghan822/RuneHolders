/*============================================================================================
* @file         eno_audio_manager.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Definition file for the audio manager
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_audio_manager.hpp"


namespace Audio
{
	EnoAudioManager::EnoAudioManager(Event::EnoEventBus& bus)
		:
		bus_{ bus },
		selectedSound_{ nullptr },
		soundPlaying_{ false }
	{}

	EnoAudioManager::~EnoAudioManager()
	{
		cleanup();

		system->close();
		system->release();
	}

	void EnoAudioManager::cleanup()
	{
		// Detaching potential dsps
		for (const auto& ch : channels_)
		{
			// Get number of dsps to iterate through
			int dspCount{};
			ch->getNumDSPs(&dspCount);

			// Get and detach each DSP from each channel
			for (size_t i = 0; i < dspCount; i++)
			{
				FMOD::DSP* dsp{ nullptr };
				ch->getDSP(i, &dsp);
				ch->removeDSP(dsp);
			}
		}

		for (const auto& group : channelGroups_)
		{
			// Get number of dsps to iterate through
			int dspCount{};
			group.second->getNumDSPs(&dspCount);

			// Get and detach each DSP from each channel group
			for (size_t i = 0; i < dspCount; i++)
			{
				FMOD::DSP* dsp{ nullptr };
				group.second->getDSP(i, &dsp);
				group.second->removeDSP(dsp);
			}
		}

		// DSP cleanup
		for (const auto& dsp : dsps_)
		{
			dsp->release();
		}

		// Sound cleanup
		for (const auto& sound : sounds_)
		{
			sound.second->release();
		}
	}

	void EnoAudioManager::init()
	{
		// System initialization for FMOD
		FMOD::System_Create(&system);
		system->init(512, FMOD_INIT_NORMAL, nullptr);

		bus_.subscribe<LoadSound>([&](const LoadSound& event) {
			onSoundReceived(event);
		});

		bus_.subscribe<SetAudioState>([&](const SetAudioState& event) {
			onTogglePlay(event);
		});

		bus_.subscribe<SetNewAudio>([&](const SetNewAudio& event){
			onAudioChange(event);
		});
	}

	FMOD::System* EnoAudioManager::getSystem() const 
		{ return system; }

	std::unordered_map<std::string, FMOD::Sound*>& EnoAudioManager::getSounds() 
		{ return sounds_; }

	/*============================================================================================
	 * \brief				Update internal FMOD system
	*//*========================================================================================*/
	void EnoAudioManager::process()
	{
		system->update();
	}

	/*============================================================================================
	 * \brief				Play all loaded sounds
	 * \param[in]			Set pause state for sounds
	*//*========================================================================================*/
	void EnoAudioManager::playAll(const bool& paused)
	{
		for (const auto& s : sounds_)
		{
			FMOD::Channel* channel{ nullptr };
			system->playSound(s.second, nullptr, paused, &channel);
			channels_.push_back(channel);
		}
	}

	/*============================================================================================
	 * \brief				Play sounds from a channel group
	 * \param[in]			Name of sound
	 * \param[in]			Channel group name
	 * \param[in]			Set pause state
	 * \return				pointer to the channel used to play the sound
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByGroup(
		const std::string& soundName, const std::string& groupName, const bool& paused)
	{
		// Fetch the sound and channel group
		auto sound = sounds_.find(soundName);
		auto group = channelGroups_.find(groupName);

		if (sound == sounds_.end() || group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find a sound or group by name!");
		}

		FMOD::Channel* channel{ nullptr };
		system->playSound(sound->second, group->second, paused, &channel);
		channels_.push_back(channel);

		return channel;
	}

	/*============================================================================================
	 * \brief				(Overload) Play sounds from a channel group
	 * \param[in]			Pointer to the Sound
	 * \param[in]			Channel group name
	 * \param[in]			Set pause state
	 * \return				pointer to the channel used to play the sound
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByGroup(
		FMOD::Sound* sound, const std::string& groupName, const bool& paused)
	{
		// Fetch the sound and channel group
		auto group = channelGroups_.find(groupName);

		if (group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find a sound or group by name!");
		}

		FMOD::Channel* channel{ nullptr };
		system->playSound(sound, group->second, paused, &channel);
		channels_.push_back(channel);

		return channel;
	}

	/*============================================================================================
	 * \brief				Set up fade to a channel group
	 * \param[in]			Channel group name
	 * \param[in]			Fade in time
	 * \param[in]			Fade out time
	 * \param[in]			Duration in seconds
	*//*========================================================================================*/
	void EnoAudioManager::scheduleGroupFade(
		const std::string& groupName,
		const float& fadeInTime, const float& fadeOutTime, const float& duration)
	{
		auto group = channelGroups_.find(groupName);

		if (group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find a sound or group by name!");
		}

		group->second->setVolumeRamp(true);

		// DSP Clock
		unsigned long long dspClock{};
		group->second->getDSPClock(nullptr, &dspClock);

		// System sample rate
		int sampleRate;
		system->getSoftwareFormat(&sampleRate, nullptr, nullptr);

		// Convert seconds to sample count
		unsigned long long fadeInSamples = static_cast<unsigned long long>(fadeInTime * sampleRate);
		unsigned long long fadeOutSamples = static_cast<unsigned long long>(fadeOutTime * sampleRate);
		unsigned long long totalSamples = static_cast<unsigned long long>(duration * sampleRate);

		// Schedule fade in
		group->second->addFadePoint(dspClock, 0.0f);
		group->second->addFadePoint(dspClock + fadeInSamples, 1.0f);

		// Schedule fade out
		group->second->addFadePoint(dspClock + totalSamples - fadeOutSamples, 1.0f);
		group->second->addFadePoint(dspClock + totalSamples, 0.0f);
	}

	/*============================================================================================
	 * \brief				Set up fade to a channel group
	 * \param[in]			Group name
	 * \param[in]			Fade in time
	 * \param[in]			Fade out time
	 * \param[in]			Duration in seconds
	*//*========================================================================================*/
	void EnoAudioManager::modifyGlobal(const SoundParameters& parameters)
	{
		for (FMOD::Channel* channel : channels_)
		{
			modify(channel, parameters);
		}
	}

	/*============================================================================================
	 * \brief				Modify sound parameters for a specific channel group
	 * \param[in]			Group name
	 * \param[in]			Parameters to modify with
	*//*========================================================================================*/
	void EnoAudioManager::modifyChannelGroup(const std::string& group, const SoundParameters& parameters)
	{
		auto it = channelGroups_.find(group);
		if (it == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find a channel group by name!");
		}

		modify(it->second, parameters);
	}
	/*============================================================================================
	 * \brief				Set channel group
	 * \param[in]			Channel group name
	*//*========================================================================================*/
	void EnoAudioManager::setChannelGroup(const std::string& groupName)
	{
		FMOD::ChannelGroup* group{ nullptr };
		system->createChannelGroup(groupName.c_str(), &group);

		channelGroups_[groupName] = group;
	}

	/*============================================================================================
	 * \brief				Get a channel group by name
	 * \param[in]			Channel group name
	*//*========================================================================================*/
	FMOD::ChannelGroup* EnoAudioManager::getChannelGroup(const std::string& group) const
	{
		auto it = channelGroups_.find(group);
		if (it == channelGroups_.end())
		{
			throw std::runtime_error("Failed to get a channel group by its name!");
		}

		return it->second;
	}

	/*============================================================================================
	 * \brief				Get the length of a sound sample (in seconds) by sound name
	 * \param[in]			Name of sound
	 * \return				Length of sound sample in seconds
	*//*========================================================================================*/
	float EnoAudioManager::getSoundDurSec(const std::string& name)
	{
		unsigned int soundLength{};
		auto it = sounds_.find(name);
		if (it == sounds_.end())
		{
			throw std::runtime_error("Failed to find sound by name!");
		}

		// System Sample Rate
		int sampleRate;
		system->getSoftwareFormat(&sampleRate, nullptr, nullptr);

		it->second->getLength(&soundLength, FMOD_TIMEUNIT_PCM);

		return static_cast<float>(soundLength) / static_cast<float>(sampleRate);
	}

	/*============================================================================================
	 * \brief				(Overload) Get the length of a sound sample by sound
	 * \param[in]			Pointer to a sound
	 * \return				Length of sound sample in seconds
	*//*========================================================================================*/
	float EnoAudioManager::getSoundDurSec(FMOD::Sound* sound)
	{
		unsigned int soundLength{};

		// System Sample Rate
		int sampleRate;
		system->getSoftwareFormat(&sampleRate, nullptr, nullptr);

		sound->getLength(&soundLength, FMOD_TIMEUNIT_PCM);

		return static_cast<float>(soundLength) / static_cast<float>(sampleRate);
	}

	// ---------- Pause ---------- 

	/*============================================================================================
		* \brief				Set pause state of all groups
		* \param[in]			Pause state to set
	*//*========================================================================================*/
	void EnoAudioManager::togglePauseAllGroups(const bool& paused)
	{
		for (auto& group : channelGroups_)
		{
			group.second->setPaused(paused);
		}
	}

	/*============================================================================================
	 * \brief				Set pause state of a group by name
	 * \param[in]			Channel group name
	 * \param[in]			Pause state to set
	*//*========================================================================================*/
	void EnoAudioManager::setPauseGroup(const std::string& name, const bool& paused)
	{
		auto group = channelGroups_.find(name);
		if (group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find group by name!");
		}

		group->second->setPaused(paused);
	}

	/*============================================================================================
	 * \brief				Set pause state of a channel
	 * \param[in]			Pointer to a Channel
	 * \param[in]			Pause state to set
	*//*========================================================================================*/
	void EnoAudioManager::setPauseChannel(FMOD::Channel* channel, const bool& paused)
	{
		channel->setPaused(paused);
	}

	// ---------- Stop ---------- 

	/*============================================================================================
		* \brief				Stop a channel group
		* \param[in]			Channel group name
	*//*========================================================================================*/
	void EnoAudioManager::stopChannelGroup(const std::string& name)
	{
		auto group = channelGroups_.find(name);
		if (group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find group by name!");
		}

		group->second->stop();
	}

	/*============================================================================================
	 * \brief				Stop a channel
	 * \param[in]			Pointer to a Channel
	*//*========================================================================================*/
	void EnoAudioManager::stopChannel(FMOD::Channel* channel)
	{
		channel->stop();
	}

	/*========================================= Events ===========================================

	/*============================================================================================
	 * \brief				Publish request for a sound by name
	 * \param[in]			Name of sound to request for
	*//*========================================================================================*/
	void EnoAudioManager::publishSoundRequest(const std::string& soundName)
	{
		bus_.publish(RequestSound{ soundName });
	}

	/*============================================================================================
	 * \brief				Received sound post-request
	 * \param[in]			LoadSound struct (sound and sound name to store)
	*//*========================================================================================*/
	void EnoAudioManager::onSoundReceived(const LoadSound& event)
	{
		// Storing sound
		sounds_[event.soundName_] = event.sound_;
	}

	/*============================================================================================
	 * \brief				Received request to play or pause a sound
	 * \param[in]			AudioState struct (Name of group to play)
	*//*========================================================================================*/
	void EnoAudioManager::onTogglePlay(const SetAudioState& event)
	{
		// Verify that a sound is selected
		if (!selectedSound_)
		{
			std::cout << "No sound selected!" << std::endl;
			return;
		}

		soundPlaying_ ^= 1;

		auto group = channelGroups_.find(event.groupName_);
		if (group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find the group" + event.groupName_);
		}

		// Toggle playing state of specified group
		if (soundPlaying_)
		{
			// Play
			FMOD::Channel* channel = playByGroup(selectedSound_, event.groupName_, true);
			scheduleGroupFade("BGM", FADE_IN, FADE_OUT, getSoundDurSec(selectedSound_));

			setPauseChannel(channel, false);
		}
		else
		{
			// Stop playing
			for (const auto& ch : channels_)
			{
				ch->stop();
			}

			channels_.clear();
		}
	}

	/*============================================================================================
	 * \brief				Received request to change the selected sound
	 * \param[in]			SetNewAudio struct (Name of audio to select)
	*//*========================================================================================*/
	void EnoAudioManager::onAudioChange(const Audio::SetNewAudio& event)
	{
		if (soundPlaying_ && selectedSound_)
		{
			std::cout << "Audio is currently playing, please wait or stop the current audio to change!" << std::endl;
			return;
		}

		auto sound = sounds_.find(event.soundName_);
		if (sound == sounds_.end())
		{
			throw std::runtime_error("Failed to find the sound" + event.soundName_);
		}

		selectedSound_ = sound->second;
	}

	/*============================================================================================
	 * \brief			Helper for modifying a reverb DSP
	 * \param[in]		SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
	 * \return			pointer to a DSP to use to modify channel/channel group
	*//*========================================================================================*/
	FMOD::DSP* EnoAudioManager::modifyReverb(const SoundParameters& parameters)
	{
		if (!parameters.reverb) return nullptr;

		FMOD::DSP* dsp{ nullptr };
		system->createDSPByType(parameters.reverb->type, &dsp);

		switch (parameters.reverb->type)
		{
		case FMOD_DSP_TYPE_SFXREVERB:
			if (parameters.reverb->sfx)
			{
				auto& reverb = parameters.reverb->sfx.value();

				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, reverb.wet);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, reverb.dry);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, reverb.density);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, reverb.diffusion);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, reverb.highcut);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, reverb.decayTime);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, reverb.earlyDelay);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, reverb.lateDelay);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, reverb.earlyLateMix);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, reverb.hfDecayRatio);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFREFERENCE, reverb.hfRef);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFGAIN, reverb.lowShelfGain);
				dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFFREQUENCY, reverb.lowShelfFreq);
			}

			break;

		case FMOD_DSP_TYPE_CONVOLUTIONREVERB:
			if (parameters.reverb->convolution)
			{
				auto& reverb = parameters.reverb->convolution.value();

				dsp->setParameterFloat(FMOD_DSP_CONVOLUTION_REVERB_PARAM_DRY, reverb.dry);
				dsp->setParameterFloat(FMOD_DSP_CONVOLUTION_REVERB_PARAM_WET, reverb.wet);
				dsp->setParameterFloat(FMOD_DSP_CONVOLUTION_REVERB_PARAM_IR, reverb.ir);
				dsp->setParameterFloat(FMOD_DSP_CONVOLUTION_REVERB_PARAM_LINKED, reverb.linked);
			}
			break;

		default:
			std::cout << "Reverb Type does not exist!" << std::endl;
			break;
		}

		return dsp;
	}

	/*============================================================================================
	 * \brief			Helper for modifying an EQ DSP
	 * \param[in]		SoundParameters struct (Reverb/EQ/Volume/Pitch/Pan)
	 * \return			Pointer to a DSP to use to modify channel/channel group
	*//*========================================================================================*/
	FMOD::DSP* EnoAudioManager::modifyEQ(const SoundParameters& parameters)
	{
		if (!parameters.eq) return nullptr;

		FMOD::DSP* dsp{ nullptr };
		system->createDSPByType(parameters.eq->type, &dsp);

		switch (parameters.eq->type)
		{
		case FMOD_DSP_TYPE_PARAMEQ:
			if (parameters.eq->paramEQ)
			{
				auto& paramEQ = parameters.eq->paramEQ.value();

				dsp->setParameterFloat(FMOD_DSP_PARAMEQ_GAIN, paramEQ.gain);
				dsp->setParameterFloat(FMOD_DSP_PARAMEQ_CENTER, paramEQ.center);
				dsp->setParameterFloat(FMOD_DSP_PARAMEQ_BANDWIDTH, paramEQ.bandwidth);
			}

			break;
		case FMOD_DSP_TYPE_THREE_EQ:
			if (parameters.eq->threeEQ)
			{
				auto& threeEQ = parameters.eq->threeEQ.value();

				dsp->setParameterFloat(FMOD_DSP_THREE_EQ_LOWGAIN, threeEQ.lowGain);
				dsp->setParameterFloat(FMOD_DSP_THREE_EQ_MIDGAIN, threeEQ.midGain);
				dsp->setParameterFloat(FMOD_DSP_THREE_EQ_HIGHGAIN, threeEQ.highGain);
				dsp->setParameterFloat(FMOD_DSP_THREE_EQ_LOWCROSSOVER, threeEQ.lowCrossOver);
				dsp->setParameterFloat(FMOD_DSP_THREE_EQ_HIGHCROSSOVER, threeEQ.highCrossOver);
			}

			break;

		default:
			std::cout << "EQ Type does not exist!" << std::endl;
			break;
		}

		return dsp;
	}
}