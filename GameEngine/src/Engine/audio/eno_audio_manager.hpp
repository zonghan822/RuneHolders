#pragma once

// FMOD
#include "fmod.hpp"

// Events
#include "audio_events.hpp"

// Parameter Information
#include "audio_params.hpp"

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
		EnoAudioManager(Event::EnoEventBus& bus) : bus_{ bus }
		{
			init();
		}

		~EnoAudioManager()
		{
			cleanup();

			system->close();
			system->release();
		}

		void cleanup()
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

		void init()
		{
			// System initialization for FMOD
			FMOD::System_Create(&system);
			system->init(512, FMOD_INIT_NORMAL, nullptr);
		}

		FMOD::System* getSystem() const { return system; }
		std::map<std::string, FMOD::Sound*>& getSounds() { return sounds_; }

		// Update system
		void process()
		{
			system->update();
		}

		// Play all loaded sounds
		void playAll(const bool& paused)
		{
			for (const auto& s : sounds_)
			{
				FMOD::Channel* channel{ nullptr };
				system->playSound(s.second, nullptr, paused, &channel);
				channels_.push_back(channel);
			}
		}

		// Play sounds from a channel group
		FMOD::Channel* playByGroup(const std::string& soundName, const std::string& groupName, const bool& paused)
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

		// Set up fade to a channel group
		void scheduleGroupFade(
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

		// Modify sound parameters
		// for all channels
		void modifyGlobal(const SoundParameters& parameters)
		{
			for (FMOD::Channel* channel : channels_)
			{
				modify(channel, parameters);
			}
		}

		// Modify sound parameters
		// for a specific channel group
		void modifyChannelGroup(const std::string& group, const SoundParameters& parameters)
		{
			auto it = channelGroups_.find(group);
			if (it == channelGroups_.end())
			{
				throw std::runtime_error("Failed to find a channel group by name!");
			}

			modify(it->second, parameters);
		}

		// Set channel group
		void setChannelGroup(const std::string& groupName)
		{
			FMOD::ChannelGroup* group{ nullptr };
			system->createChannelGroup(groupName.c_str(), &group);

			channelGroups_[groupName] = group;
		}

		// Get a channel group by name
		FMOD::ChannelGroup* getChannelGroup(const std::string& group) const
		{
			auto it = channelGroups_.find(group);
			if (it == channelGroups_.end())
			{
				throw std::runtime_error("Failed to get a channel group by its name!");
			}

			return it->second;
		}

		// Get the length of a sound sample
		float getSoundDurSec(const std::string& name)
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

		// ---------- Pause ---------- 

		// Set pause state of all groups
		void togglePauseAllGroups(const bool& paused)
		{
			for (auto& group : channelGroups_)
			{
				group.second->setPaused(paused);
			}
		}

		// Set pause state of a group by name
		void setPauseGroup(const std::string& name, const bool& paused)
		{
			auto group = channelGroups_.find(name);
			if (group == channelGroups_.end())
			{
				throw std::runtime_error("Failed to find group by name!");
			}

			group->second->setPaused(paused);
		}

		// Set pause state of a channel
		void setPauseChannel(FMOD::Channel* channel, const bool& paused)
		{
			channel->setPaused(paused);
		}

		// ---------- Stop ---------- 

		// Stop a channel group
		void stopChannelGroup(const std::string& name)
		{
			auto group = channelGroups_.find(name);
			if (group == channelGroups_.end())
			{
				throw std::runtime_error("Failed to find group by name!");
			}

			group->second->stop();
		}

		// Stop a channel
		void stopChannel(FMOD::Channel* channel)
		{
			channel->stop();
		}

		// ----------------------- Events -----------------------

		// Publish request for a sound by name
		void publishSoundRequest(const std::string& soundName)
		{
			bus_.publish(RequestSound{ soundName });
		}

		// Receive sound post-request
		void onSoundReceived(const LoadSound& event)
		{
			// Storing sound
			sounds_[event.soundName_] = event.sound_;
		}

	private:

		// FMOD System
		FMOD::System* system;

		// Channels
		std::vector<FMOD::Channel*> channels_;

		// Channel Groups
		std::map<std::string, FMOD::ChannelGroup*> channelGroups_;

		// Sounds
		std::map<std::string, FMOD::Sound*> sounds_;

		// DSPs
		std::vector<FMOD::DSP*> dsps_;

		// Event bus
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
		FMOD::DSP* modifyReverb(const SoundParameters& parameters)
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

		// Helper for modifying an EQ DSP
		FMOD::DSP* modifyEQ(const SoundParameters& parameters)
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
	};

}