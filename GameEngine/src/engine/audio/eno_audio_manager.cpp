/*============================================================================================
* @file         eno_audio_manager.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Definition file for the audio manager
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_audio_manager.hpp"


namespace Audio
{
	EnoAudioManager::EnoAudioManager(Event::EnoEventBus& bus)
		:
		bus_{ bus },
		selectedSound_{ nullptr },
		soundPlaying_{ false },
		system{ nullptr }
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
			for (int i = 0; i < dspCount; i++)
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
			for (int i = 0; i < dspCount; i++)
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

		bus_.subscribe<LoadSound>([&](const LoadSound& event) 
			{
				onSoundReceived(event);
			});

		bus_.subscribe<SetAudioState>([&](const SetAudioState& event) 
			{
				onTogglePlay(event);
			});

		bus_.subscribe<SetNewAudio>([&](const SetNewAudio& event)
			{
				onAudioChange(event);
			});

		bus_.subscribe<PlayInstant>([&](const PlayInstant& event)
			{
				onAudioInstantPlay(event);
			});

		bus_.subscribe<StopBGM>([&](const StopBGM&) {
			stopChannelGroup("BGM");
			soundPlaying_ = false; // reset toggle state
			});

		setChannelGroup("BGM");
		setChannelGroup("SFX");

		Audio::SoundParameters sound;
		Audio::ParamEQ eq;
		Audio::SFXReverb reverb;
		eq = Audio::ParamEQ{ 3.f, 1000.f, 1.f };
		sound.volume = 0.1f;
		sound.eq = Audio::EQ{};
		sound.eq->type = FMOD_DSP_TYPE_PARAMEQ;
		sound.eq->paramEQ = eq;
		sound.reverb = Audio::Reverb{};
		sound.reverb->type = FMOD_DSP_TYPE_SFXREVERB;
		sound.reverb->sfx = reverb;
		modifyChannelGroup("BGM", sound);
	}

	FMOD::System* EnoAudioManager::getSystem() const 
		{ return system; }

	std::unordered_map<std::string, FMOD::Sound*>& EnoAudioManager::getSounds() 
		{ return sounds_; }

	/*============================================================================================
	 * \brief				Update internal FMOD system
	*//*========================================================================================*/
	void EnoAudioManager::process(float dt)
	{
		for (auto it = pendingStops_.begin(); it != pendingStops_.end();)
		{
			it->timeRemaining_ -= dt;

			if (it->timeRemaining_ <= 0.f)
			{
				int count{};
				it->group_->getNumChannels(&count);
				for (size_t i = 0; i < count; i++)
				{
					FMOD::Channel* ch{};
					it->group_->getChannel(i, &ch);
					if (ch)
						ch->stop();
				}
				it = pendingStops_.erase(it);
			}
			else
			{
				++it;
			}
		}

		system->update();
	}

	/*============================================================================================
				 * \brief					Setup an audio component and its internal audio module
				 * \param					A reference to the audio component to set up
	*//*========================================================================================*/
	void EnoAudioManager::prepareMod(ECSComponents::AudioComponent& audioComp)
	{
		// Retrieve and store the sound in the audio module
		auto it = sounds_.find(audioComp.audioMod_.name_);
		if (it == sounds_.end())
		{
			// Instead of throwing, publish an error event and log to console
			std::cerr << "[Audio Error] Unknown sound requested: " << audioComp.audioMod_.name_ << std::endl;
			bus_.publish(AudioLoadError{
				audioComp.audioMod_.name_,
				"Component audio module",
				"Unknown",
				"Sound requested by audio component was not found in loaded sounds."
				});
			return;
		}

		audioComp.audioMod_.sound_ = it->second;
	}

	/*============================================================================================
	 * \brief				Sets up and plays sound based on params provided by an audio component
	 * \param[in]			Audio component
	*//*========================================================================================*/
	void EnoAudioManager::play(ECSComponents::AudioComponent& audioComp)
	{
		// CRITICAL FIX: Don't restart audio that's already playing
		// This prevents the game loop from constantly restarting audio every frame
		if (audioComp.isPlaying_)
		{
			return; // Audio already playing, nothing to do
		}

		// Since isPlaying_ is false, the stored channel pointer may be stale
		// (FMOD invalidates channels on game stop but doesn't null the pointer).
		// Clear it now so the ch->stop() below doesn't touch freed memory.
		audioComp.audioMod_.ch_ = nullptr;

		// Auto-prepare if sound pointer is null but name is set
		// This handles cases where audio was deserialized from a save file
		if (audioComp.audioMod_.sound_ == nullptr && audioComp.audioMod_.name_[0] != '\0')
		{
			std::cout << "[Audio] Auto-preparing audio module for: " << audioComp.audioMod_.name_ << std::endl;
			prepareMod(audioComp);
		}

		FMOD::Sound* sound{ audioComp.audioMod_.sound_ };
		std::string group{ audioComp.audioMod_.chGroup_ };
		FMOD::Channel* ch{ audioComp.audioMod_.ch_ };

		FMOD::ChannelGroup* chGroup{ nullptr };

		auto it = channelGroups_.find(group);
		if (it == channelGroups_.end())
		{
			// Create and store a new channel group for the audio component
			system->createChannelGroup(audioComp.audioMod_.chGroup_, &chGroup);
			channelGroups_[group] = chGroup;
		}
		else
		{
			// Channel group exists and will be used
			chGroup = it->second;

			chGroup->removeFadePoints(0, ULLONG_MAX);
			chGroup->setDelay(0, 0, false);
			chGroup->setVolume(1.f);
		}

		if (ch)
			ch->stop();

		// Apply loop settings to the sound before playing
		applyLoopSettings(sound, audioComp);

		// Play the sound (channel is created here)
		system->playSound(
			audioComp.audioMod_.sound_,
			chGroup,
			true,  // Start paused so we can apply settings first
			&ch);

		audioComp.audioMod_.ch_ = ch;

		// Now that channel exists, apply volume/pitch/pan
		if (ch)
		{
			modifyBasic(ch, createSP(audioComp));

			// CRITICAL FIX: Always unpause when play() is called
			// The play() function is meant to START playback
			// isAutoplay_ should only control scene-start behavior, not manual play
			ch->setPaused(false);

			// Store the channel for later use
			channels_.push_back(ch);

			// Mark as playing AFTER successfully starting playback
			audioComp.isPlaying_ = true;
		}

		// Emit signals
		auto signalIt = audioComp.signals_.find("started");
		if (signalIt != audioComp.signals_.end())
		{
			signalIt->second.emit({ audioComp.volume_ });
		}
	}

	/*============================================================================================
	 * \brief				(Overload) Sets up and plays sound based on params provided by an audio component
	 * \param[in]			Audio component
	*//*========================================================================================*/
	void EnoAudioManager::play(ECSComponents::AudioComponent& audioComp, Math::eVec2<float> camPos)
	{
		// CRITICAL FIX: Don't restart audio that's already playing
		// This prevents the game loop from constantly restarting audio every frame
		if (audioComp.isPlaying_)
		{
			return; // Audio already playing, nothing to do
		}

		// Auto-prepare if sound pointer is null but name is set
		// This handles cases where audio was deserialized from a save file
		if (audioComp.audioMod_.sound_ == nullptr && audioComp.audioMod_.name_[0] != '\0')
		{
			std::cout << "[Audio] Auto-preparing audio module for: " << audioComp.audioMod_.name_ << std::endl;
			prepareMod(audioComp);
		}

		FMOD::Sound* sound{ audioComp.audioMod_.sound_ };
		std::string group{ audioComp.audioMod_.chGroup_ };
		FMOD::Channel* ch{ audioComp.audioMod_.ch_ };

		FMOD::ChannelGroup* chGroup{ nullptr };

		auto it = channelGroups_.find(group);
		if (it == channelGroups_.end())
		{
			// Create and store a new channel group for the audio component
			system->createChannelGroup(audioComp.audioMod_.chGroup_, &chGroup);
			channelGroups_[group] = chGroup;
		}
		else
		{
			// Channel group exists and will be used
			chGroup = it->second;
			
			// Clear previous fades and scheduled stops
			chGroup->removeFadePoints(0, ULLONG_MAX);
			chGroup->setDelay(0, 0, false);
			chGroup->setVolume(1.f);
		}

		if (ch)
			ch->stop();

		// Apply loop settings to the sound before playing
		applyLoopSettings(sound, audioComp);

		// Play the sound (channel is created here)
		FMOD_RESULT result;
		result = system->playSound(
			audioComp.audioMod_.sound_,
			chGroup,
			true,  // Start paused so we can apply settings first
			&ch);

		channels_.push_back(ch);

		// Now that channel exists, apply volume/pitch/pan
		if (ch)
		{
			// Modify volume/pan based on cam pos
			audioComp.volume_ -= 0.125 * camPos.x;
			audioComp.panScale_ -= 0.125 * camPos.x;

			modifyBasic(ch, createSP(audioComp));

			// CRITICAL FIX: Always unpause when play() is called
			// The play() function is meant to START playback
			// isAutoplay_ should only control scene-start behavior, not manual play
			ch->setPaused(false);

			// Store the channel for later use
			channels_.push_back(ch);

			// Mark as playing AFTER successfully starting playback
			audioComp.isPlaying_ = true;
		}

		// Emit signals
		auto signalIt = audioComp.signals_.find("started");
		if (signalIt != audioComp.signals_.end())
		{
			signalIt->second.emit({ audioComp.volume_ });
		}
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
	 * \brief				(Overload) Play sounds from a channel group
	 * \param[in]			Pointer to Sound
	 * \param[in]			Pointer to Channel group
	 * \param[in]			Set pause state
	 * \return				pointer to the channel used to play the sound
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByGroup(
		FMOD::Sound* sound, FMOD::ChannelGroup* group, const bool paused)
	{
		FMOD::Channel* channel{ nullptr };
		system->playSound(sound, group, paused, &channel);
		channels_.push_back(channel);

		return channel;
	}

	/*============================================================================================
	 * \brief				Play sounds from a channel group
	 * \param[in]			Name of sound
	 * \param[in]			Channel group name
	 * \param[in]			Set pause state
	 * \return				pointer to the channel used to play the sound
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByGroup(
		const std::string& soundName, const std::string& groupName, const bool paused)
	{
		// Fetch the sound and channel group
		auto sound = sounds_.find(soundName);
		auto group = channelGroups_.find(groupName);

		if (sound == sounds_.end() || group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find a sound or group by name!");
		}

		return playByGroup(sound->second, group->second, paused);
	}

	/*============================================================================================
	 * \brief				(Overload) Play sounds from a channel group
	 * \param[in]			Pointer to the Sound
	 * \param[in]			Channel group name
	 * \param[in]			Set pause state
	 * \return				pointer to the channel used to play the sound
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByGroup(
		FMOD::Sound* sound, const std::string& groupName, const bool paused)
	{
		// Fetch the channel group
		auto group = channelGroups_.find(groupName);

		if (group == channelGroups_.end())
		{
			throw std::runtime_error("Failed to find a sound or group by name!");
		}

		return playByGroup(sound, group->second, paused);
	}

	/*============================================================================================
	 * \brief				(Overload) Play sounds from a channel group
	 * \param[in]			Pointer to the Sound
	 * \param[in]			Channel group name
	 * \param[in]			Set pause state
	 * \return				pointer to the channel used to play the sound
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByGroup(
		const std::string& soundName, FMOD::ChannelGroup* group, const bool paused)
	{
		// Fetch the sound
		auto sound = sounds_.find(soundName);

		if (sound == sounds_.end())
		{
			throw std::runtime_error("Failed to find a sound or group by name!");
		}

		return playByGroup(sound->second, group, paused);
	}

	/*============================================================================================
	 * \brief				Set up fade to a channel group
	 * \param[in]			Channel group name
	 * \param[in]			Fade in time
	 * \param[in]			Fade out time
	 * \param[in]			Duration in seconds
	*//*========================================================================================*/
	FMOD::Channel* EnoAudioManager::playByChannel(FMOD::Sound* sound, const bool paused)
	{
		FMOD::Channel* channel{ nullptr };
		system->playSound(sound, nullptr, paused, &channel);
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
		float fadeInTime, float fadeOutTime, float duration)
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
		//unsigned long long fadeInSamples = static_cast<unsigned long long>(fadeInTime * sampleRate);
		unsigned long long fadeOutSamples = static_cast<unsigned long long>(fadeOutTime * sampleRate);
		unsigned long long totalSamples = static_cast<unsigned long long>(duration * sampleRate);

		unsigned long long endDSP = dspClock + totalSamples;

		// Schedule fade out
		group->second->addFadePoint(dspClock + totalSamples - fadeOutSamples, 1.0f);
		group->second->addFadePoint(dspClock + totalSamples, 0.0f);

		group->second->setDelay(0, endDSP, true);
	}

	/*============================================================================================
	 * \brief				Based on the audio component passed, create and return an
	 *						appropriate SoundParameters struct
	 * \param[in]			Audio Component
	*//*========================================================================================*/
	SoundParameters EnoAudioManager::createSP(ECSComponents::AudioComponent& ac)
	{
		SoundParameters sp;
		// Convert volume from 0-100 range to 0.0-1.0 range for FMOD
		sp.volume = static_cast<float>(ac.volume_ / 100.0);
		// Clamp volume to valid range
		if (sp.volume < 0.0f) sp.volume = 0.0f;
		if (sp.volume > 1.0f) sp.volume = 1.0f;

		// Pitch scale - ensure it's at least 0.1 to avoid silent/broken audio
		sp.pitch = static_cast<float>(ac.pitchScale_);
		if (sp.pitch < 0.1f) sp.pitch = 0.1f;
		if (sp.pitch > 5.0f) sp.pitch = 5.0f;

		// Pan - clamp to -1.0 to 1.0
		sp.pan = static_cast<float>(ac.panScale_);
		if (sp.pan < -1.0f) sp.pan = -1.0f;
		if (sp.pan > 1.0f) sp.pan = 1.0f;

		return sp;
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
		constexpr float FADE_IN_TIME = 0.f;
		constexpr float FADE_OUT_TIME = 2.f;
		constexpr float DURATION = 1.f;

		auto group = channelGroups_.find(name);
		if (group == channelGroups_.end())
		{
			// Group doesn't exist - nothing to stop, return silently
			return;
		}

		// Fade out sound
		scheduleGroupFade(name, FADE_IN_TIME, FADE_OUT_TIME, DURATION);
		pendingStops_.push_back({ group->second, FADE_OUT_TIME });
	}

	/*============================================================================================
	* \brief				Stop all channels within a channel group
	* \param[in]			Channel group name
	*//*========================================================================================*/
	void EnoAudioManager::stopAllChannels(FMOD::ChannelGroup* group)
	{
		int channelCount{};
		group->getNumChannels(&channelCount);

		for (size_t i = 0; i < channelCount; i++)
		{
			FMOD::Channel* channel{};
			group->getChannel(i, &channel);

			if (channel)
				channel->stop();
		}
	}

	/*============================================================================================
	 * \brief				Stop a channel
	 * \param[in]			Pointer to a Channel
	*//*========================================================================================*/
	void EnoAudioManager::stopChannel(FMOD::Channel* channel)
	{
		channel->stop();
	}

	/*======================================== Events ===========================================*/

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
			std::cerr << "[Audio Error] Failed to find the channel group: " << event.groupName_ << std::endl;
			bus_.publish(AudioLoadError{
				"N/A",
				"Channel group: " + event.groupName_,
				"Unknown",
				"Channel group '" + event.groupName_ + "' was not found. The channel group may not have been created."
				});
			soundPlaying_ ^= 1; // Restore the state since we're not playing
			return;
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

		for (const auto& sound : sounds_)
		{
			std::cout << "Sound: " << sound.first << std::endl;
		}

		auto sound = sounds_.find(event.soundName_);
		if (sound == sounds_.end())
		{
			std::cerr << "[Audio Error] Failed to find the sound: " << event.soundName_ << std::endl;
			bus_.publish(AudioLoadError{
				event.soundName_,
				"Unknown location",
				"Unknown",
				"Sound '" + event.soundName_ + "' was requested but not found in loaded sounds."
				});
			return;
		}

		selectedSound_ = sound->second;
	}

	/*============================================================================================
	 * \brief				Received request to change the selected sound
	 * \param[in]			SetNewAudio struct (Name of audio to select)
	*//*========================================================================================*/
	void EnoAudioManager::onAudioInstantPlay(const Audio::PlayInstant& event)
	{
		auto group = channelGroups_.find(event.groupName_);
		if (group == channelGroups_.end())
		{
			std::cerr << "[Audio Error] Failed to find the channel group: " << event.groupName_ << std::endl;
			bus_.publish(AudioLoadError{
				event.soundName_,
				"Channel group: " + event.groupName_,
				"Unknown",
				"Channel group '" + event.groupName_ + "' was not found. The channel group may not have been created."
				});
			return;
		}

		auto sound = sounds_.find(event.soundName_);
		if (sound == sounds_.end())
		{
			std::cerr << "[Audio Error] Failed to find the sound: " << event.soundName_ << std::endl;
			bus_.publish(AudioLoadError{
				event.soundName_,
				"Unknown location",
				"Unknown",
				"Sound '" + event.soundName_ + "' was requested but not found in loaded sounds."
				});
			return;
		}

		FMOD::Channel* channel = playByChannel(sound->second, true);
		//scheduleGroupFade("SFX", FADE_IN, FADE_OUT, getSoundDurSec(sound->second));
		setPauseChannel(channel, false);
	}

	/*============================================================================================
	 * \brief			Apply loop settings from AudioComponent to FMOD sound
	 * \param[in]		Pointer to FMOD::Sound
	 * \param[in]		AudioComponent with loop settings
	*//*========================================================================================*/
	void EnoAudioManager::applyLoopSettings(FMOD::Sound* sound, ECSComponents::AudioComponent& audioComp)
	{
		if (!sound) return;

		// Set loop mode based on the enum
		switch (audioComp.audioMod_.loop_)
		{
		case Audio::EnoAudioModule::LoopMode::Disabled:
			sound->setMode(FMOD_LOOP_OFF);
			break;

		case Audio::EnoAudioModule::LoopMode::Forward:
			sound->setMode(FMOD_LOOP_NORMAL);
			// Set loop points if specified (convert seconds to PCM samples)
			if (audioComp.audioMod_.loopStart_ > 0.0f || audioComp.audioMod_.loopEnd_ > 0.0f)
			{
				// Get sound format info
				float frequency{};
				sound->getDefaults(&frequency, nullptr);

				unsigned int startPCM = static_cast<unsigned int>(audioComp.audioMod_.loopStart_ * frequency);
				unsigned int endPCM = 0;

				if (audioComp.audioMod_.loopEnd_ > 0.0f)
				{
					endPCM = static_cast<unsigned int>(audioComp.audioMod_.loopEnd_ * frequency);
				}
				else
				{
					// If no end specified, loop to the end of the sound
					sound->getLength(&endPCM, FMOD_TIMEUNIT_PCM);
				}

				sound->setLoopPoints(startPCM, FMOD_TIMEUNIT_PCM, endPCM, FMOD_TIMEUNIT_PCM);
			}
			break;

		case Audio::EnoAudioModule::LoopMode::PingPong:
			sound->setMode(FMOD_LOOP_BIDI); // Bidirectional loop (ping-pong)
			// Set loop points if specified
			if (audioComp.audioMod_.loopStart_ > 0.0f || audioComp.audioMod_.loopEnd_ > 0.0f)
			{
				float frequency{};
				sound->getDefaults(&frequency, nullptr);

				unsigned int startPCM = static_cast<unsigned int>(audioComp.audioMod_.loopStart_ * frequency);
				unsigned int endPCM = 0;

				if (audioComp.audioMod_.loopEnd_ > 0.0f)
				{
					endPCM = static_cast<unsigned int>(audioComp.audioMod_.loopEnd_ * frequency);
				}
				else
				{
					sound->getLength(&endPCM, FMOD_TIMEUNIT_PCM);
				}

				sound->setLoopPoints(startPCM, FMOD_TIMEUNIT_PCM, endPCM, FMOD_TIMEUNIT_PCM);
			}
			break;
		}
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