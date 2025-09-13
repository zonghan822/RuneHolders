#pragma once

// FMOD
#include "fmod.hpp"

// Engine Component(s)
#include "../event/eventBus.hpp"

// Events
#include "../audio/audio_events.hpp"

// stds
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <unordered_map>


namespace Resources
{
	class EnoLoader
	{
	private:
		// Sounds
		static constexpr std::string_view musicPath_ = "assets/sound/music/";
		static constexpr std::string_view sfxPath_ = "assets/sound/sfx/";
		static constexpr std::array<std::string_view, 2> soundExtensions_ = { ".wav", ".mp3" };

		std::map<std::string, FMOD::Sound*> sounds_;

		// Images
		static constexpr std::string_view imagePath_ = "assets/art/";

		// Shaders
		static constexpr std::string_view shaderPath_ = "../shaders/";

		std::unordered_map<std::string, std::string> vertShaders_;
		std::unordered_map<std::string, std::string> fragShaders_;

		// ECS Entities
		static constexpr std::string_view entityPath_ = "assets/saves/ecs_save_data.txt";

		// Bus
		Event::EnoEventBus& bus_;

	public:

		EnoLoader(Event::EnoEventBus& bus) : 
			bus_{ bus }
		{}

		// Sound getters
		const std::string& getMusicPath() const
		{
			static const std::string path{ musicPath_.data(), musicPath_.size() };
			return path;
		}
		const std::string& getSFXPath() const
		{
			static const std::string path{ sfxPath_.data(), sfxPath_.size() };
			return path;
		}
		std::array<std::string_view, 2> getSoundExtensions() const { return soundExtensions_; }

		// Image getters

		// Shader getters
		const std::string& getShaderPath() const
		{
			static const std::string path{ shaderPath_.data(), shaderPath_.size() };
			return path;
		}
		const std::string& getVertShader(const std::string& name) const { return vertShaders_.at(name); }
		const std::string& getFragShader(const std::string& name) const { return fragShaders_.at(name); }

		// Engine data getters
		const std::string& getECSSavePath() const 
		{ 
			static const std::string path{ entityPath_.data(), entityPath_.size() };
			return path; 
		}

		// Load Sound files (mp3 | wav)
		// and store it in a container that is passed in
		template <typename Container>
		void loadSound(
			FMOD::System* system, 
			const Container& extensions, 
			const std::string& path)
		{
			if (!std::filesystem::exists(path))
			{
				throw std::runtime_error("Specified sound path does not exist!");
			}

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
						throw std::runtime_error("Failed to create a sound from a loaded mp3/.wav");
					}

					if (result == FMOD_OK)
					{
						// Store the sound obtained
						sounds_[entry.path().filename().stem().string()] = sound;
						std::cout << entry.path().stem().string() << " was retrieved!" << std::endl;
					}
					else
					{
						throw std::runtime_error("Failed to create a sound from a WAV/MP3 file!");
					}
				}
			}
		}

		// Load Images (png | jpg)
		void loadImages(const std::string_view& path)
		{
			// Iterate through a folder through the given path
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (!entry.is_regular_file())
					continue;

				// Find files with the PNG or JPG extensions
				if (entry.path().extension().string() == ".png" ||
					entry.path().extension().string() == ".jpg")
				{
					//sounds_[entry.path().filename().stem().string()] = sound;
				}
			}
		}

		// Load shader files
		void loadShaders(const std::string_view& path)
		{
			// Iterate through a folder through the given path
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (!entry.is_regular_file())
					continue;

				std::ifstream file(entry.path());

				if (!file.is_open())
				{
					throw std::runtime_error("Failed to open a file!");
				}

				// Allocate buffer for storing text data
				// from each shader file
				std::stringstream buffer;
				buffer << file.rdbuf();
				std::string source = buffer.str();

				// Find files with the frag and vert extensions
				if (entry.path().extension().string() == ".vert")
				{
					vertShaders_[entry.path().stem().string()] = std::move(source);
				}
				else if (entry.path().extension().string() == ".frag")
				{
					fragShaders_[entry.path().stem().string()] = std::move(source);
				}
			}
		}

		// ---------------------- Events ----------------------
		
		// Resolve requests for a sound
		void onSoundRequest(const Audio::RequestSound& event)
		{
			std::cout << "Sound requested is: " << event.sound_ << std::endl;

			// Look for sound requested in the map
			auto it = sounds_.find(event.sound_);
			if (it == sounds_.end())
			{
				throw std::runtime_error("Failed to find the requested sound!");
			}

			bus_.publish(Audio::LoadSound{ event.sound_, it->second });
		}
	};
}