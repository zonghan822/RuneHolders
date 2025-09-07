#pragma once

// fmod
#include "fmod.hpp"

//stds
#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

// tbd
#include <filesystem>


namespace Sound
{
	struct Reverb
	{
		FMOD_DSP_TYPE type;

		float wet;
		float dry;
		float density;
		float diffusion;
	};

	struct EQ
	{
		FMOD_DSP_TYPE type;

		float gain;
		float center;
		float bandwidth;
	};

	struct SoundParameters
	{
		float volume;
		float pitch;
		float pan;

		//Fade fade;

		Reverb reverb;
		EQ eq;
	};

	class EnoAudioManager
	{
	public:
		EnoAudioManager()
		{
			init();
		}

		~EnoAudioManager()
		{
			// Release sounds loaded
			for (auto& pair : sounds)
			{
				if (pair.second)
				{
					pair.second->release();
				}
			}

			system->close();
			system->release();
		}

		void init()
		{
			// System initialization for FMOD
			FMOD::System_Create(&system);
			system->init(512, FMOD_INIT_NORMAL, nullptr);

			load();					//tbd
		}

		// ***** Delegate this function to loader please *****
		// 
		// 
		// Load all sound files located in the
		// folder specified by the path
		void load()
		{
			// Iterate through a folder through the given path
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (!entry.is_regular_file()) 
					continue;

				// Find files with the WAV or MP3 extensions
				if (entry.path().extension().string() == ".wav" ||
					entry.path().extension().string() == ".mp3")
				{
					// Retrieve sound from file
					FMOD::Sound* sound = nullptr;
					FMOD_RESULT result = system->createSound(
						entry.path().string().c_str(), FMOD_DEFAULT, nullptr, &sound);

					if (result != FMOD_OK)
					{
						throw std::runtime_error("Failed to create a sound from a WAV/MP3 file!");
					}
					else
					{
						// Record the sound onto the map, 
						// with the filename extension stripped from the name
						sounds[entry.path().filename().stem().string()] = sound;
					}
				}
			}
		}

		// Play all loaded sounds
		void playAll()
		{
			for (const auto& s : sounds)
			{
				FMOD::Channel* channel{ nullptr };
				system->playSound(s.second, nullptr, false, &channel);
				channels.push_back(channel);
			}

			// Play sound
			system->update();
		}

		// Play sounds from a channel group
		void playByGroup(const std::string& soundName, const std::string& groupName)
		{
			// Fetch the sound and channel group
			auto sound = sounds.find(soundName);
			auto group = channelGroups.find(groupName);

			if (sound == sounds.end() || group == channelGroups.end())
			{
				throw std::runtime_error("Failed to find a sound or group by name!");
			}

			FMOD::Channel* channel{ nullptr };
			system->playSound(sound->second, group->second, false, &channel);
			channels.push_back(channel);

			// Play sound
			system->update();
		}

		// Modify sound parameters
		// for all channels
		void modifyGlobal(const SoundParameters& parameters)
		{
			for (FMOD::Channel* channel : channels)
			{
				modify(channel, parameters);
			}
		}

		// Modify sound parameters
		// for a specific channel group
		void modifyChannelGroup(const std::string& group, const SoundParameters& parameters)
		{
			auto it = channelGroups.find(group);
			if (it == channelGroups.end())
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

			channelGroups[groupName] = group;
		}

		// Get a channel group by name
		FMOD::ChannelGroup* getChannelGroup(const std::string& groupName) const
		{
			auto it = channelGroups.find(groupName);
			if (it == channelGroups.end())
			{
				throw std::runtime_error("Failed to get a channel group by its name!");
			}
			
			return it->second;
		}

	private:
		FMOD::System* system;
		
		// Channels
		std::vector<FMOD::Channel*> channels;

		// Channel Groups
		std::map<std::string, FMOD::ChannelGroup*> channelGroups;

		// Sounds
		std::map<std::string, FMOD::Sound*> sounds;

		// Path to music repo
		const std::string path = "sounds/";


		template <typename CH>
		// Helper for modifying a channel or channel group
		void modify(CH* ch, const SoundParameters& parameters)
		{
			// Basic
			ch->setVolume(parameters.volume);
			ch->setPitch(parameters.pitch);
			ch->setPan(parameters.pan);

			// Reverb DSP
			FMOD::DSP* reverbDSP{ nullptr };
			system->createDSPByType(parameters.reverb.type, &reverbDSP);
			reverbDSP->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, parameters.reverb.wet);
			reverbDSP->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, parameters.reverb.dry);
			reverbDSP->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, parameters.reverb.density);
			reverbDSP->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, parameters.reverb.diffusion);

			// EQ DSP
			FMOD::DSP* eqDSP{ nullptr };
			system->createDSPByType(parameters.eq.type, &eqDSP);
			eqDSP->setParameterFloat(FMOD_DSP_PARAMEQ_GAIN, parameters.eq.gain);
			eqDSP->setParameterFloat(FMOD_DSP_PARAMEQ_CENTER, parameters.eq.center);
			eqDSP->setParameterFloat(FMOD_DSP_PARAMEQ_BANDWIDTH, parameters.eq.bandwidth);

			// Append DSPs
			ch->addDSP(0, reverbDSP);
			ch->addDSP(0, eqDSP);
		}
	};


	//struct WavHeader
	//{
	//	// Subchunk information
	//	uint16_t audioFormat;
	//	uint16_t numChannels;
	//	uint32_t sampleRate;
	//	uint32_t byteRate;
	//	uint16_t blockAlign;
	//	uint16_t bitsPerSample;
	//	uint32_t dataSize;
	//};


	// ********* Might not be necessary anymore but I am keeping it around *********
	// 
	// Read a wav file 
	// and store its information
	//void readWAV(const std::string& filename, EnoAudioManager::WavHeader& header, std::vector<uint8_t>& data)
	//{
	//	std::ifstream file(filename, std::ios::binary);

	//	// Verify that the file is both a valid file and a WAV file
	//	if (!file)
	//	{
	//		throw std::runtime_error("Failed to find the file!");
	//	}

	//	// Identify file as a RIFF file, 
	//	// first 4 bytes read should be RIFF
	//	char riff[4];
	//	file.read(riff, 4);
	//	if (std::string(riff, 4) != "RIFF")
	//	{
	//		throw std::runtime_error("File found is not a RIFF file!");
	//	}

	//	// Identify file as a WAV file,
	//	// found under format
	//	// Starts on the 8th byte offset of the file
	//	file.seekg(8, std::ios::beg);
	//	char wavFormat[4];
	//	file.read(wavFormat, 4);
	//	if (std::string(wavFormat, 4) != "WAVE")
	//	{
	//		throw std::runtime_error("File found does not have the WAVE format!");
	//	}

	//	// Read in fmt chunk
	//	// for audio information
	//	bool fmtFound{ false };
	//	bool dataFound{ false };
	//	while (!fmtFound && !dataFound)
	//	{
	//		fmtFound = true;

	//		char chunkID[4];
	//		uint32_t chunkSize;

	//		// Not able to find chunk id at
	//		// expected position
	//		if (!file.read(chunkID, 4))
	//			break;

	//		// Read in the size of the fmt data chunk
	//		file.read(reinterpret_cast<char*>(&chunkSize), 4);

	//		// Chunk ID found, id should be fmt
	//		file.read(reinterpret_cast<char*>(chunkID), 4);
	//		if (std::string(chunkID, 4) == "fmt ")
	//		{
	//			// Declared in the order of the
	//			// canonical WAVE format
	//			uint16_t audioFormat{}, numChannels{};
	//			uint32_t sampleRate{}, byteRate{};
	//			uint16_t blockAlign{}, bitsPerSample{};

	//			// fmt found, extracting information
	//			// from the sub chunk
	//			file.read(reinterpret_cast<char*>(&audioFormat), 2);
	//			file.read(reinterpret_cast<char*>(&numChannels), 2);
	//			file.read(reinterpret_cast<char*>(&sampleRate), 2);
	//			file.read(reinterpret_cast<char*>(&byteRate), 2);
	//			file.read(reinterpret_cast<char*>(&blockAlign), 2);
	//			file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

	//			// Append information
	//			header.audioFormat = audioFormat;
	//			header.numChannels = numChannels;
	//			header.sampleRate = sampleRate;
	//			header.byteRate = byteRate;
	//			header.blockAlign = blockAlign;
	//			header.bitsPerSample = bitsPerSample;
	//		}
	//		else if (std::string(chunkID, 4) == "data")
	//		{
	//			dataFound = true;
	//			header.dataSize = chunkSize;

	//			data.resize(chunkSize);
	//			file.read(reinterpret_cast<char*>(data.data()), chunkSize);
	//		}
	//		else
	//		{
	//			// Skip unknown chunk
	//			file.seekg(chunkSize, std::ios::cur);
	//		}
	//	}
	//}

}