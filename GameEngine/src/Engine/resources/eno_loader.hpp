#pragma once
// STB_IMAGE
#include "stb_image.h"

// FMOD
#include "fmod.hpp"

// Interface
#include "../interface/system.hpp"

// Events
#include "../audio/audio_events.hpp"
#include "../event/event_bus.hpp"

// Graphics
#include "../graphics/graphics.hpp"

// stds
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <unordered_map>
#include <memory>


namespace Resources
{
	class EnoLoader : public Sys::ISystem
	{
	private:
		// Sounds
		static constexpr std::string_view musicPath_ = ASSETS_DIR_PATH "sound/music/";
		static constexpr std::string_view sfxPath_ = ASSETS_DIR_PATH "sound/sfx/";
		static constexpr std::array<std::string_view, 2> soundExtensions_ = { ".wav", ".mp3" };

		std::map<std::string, FMOD::Sound*> sounds_;

		// Images
		static constexpr std::string_view imagePath_ = ASSETS_DIR_PATH "art/";
		std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>> textures_;

		// Shaders
		static constexpr std::string_view shaderPath_ = ASSETS_DIR_PATH "shaders/";

		std::unordered_map<std::string, std::string> vertShaders_;
		std::unordered_map<std::string, std::string> fragShaders_;

		// ECS Entities
		static constexpr std::string_view entityPath_ = ASSETS_DIR_PATH "saves/ecs_save_data.txt";

		// Event Bus
		Event::EnoEventBus& bus_;

	public:

		EnoLoader(Event::EnoEventBus& bus) :
			bus_{ bus }
		{}

		~EnoLoader()
		{
			for (const auto& sound : sounds_)
			{
				if (sound.second) sound.second->release();
			}
		}

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

		// Texture getters
		const std::string& getImagePath() const
		{
			static const std::string path{ imagePath_.data(), imagePath_.size() };
			return path;
		}

		std::shared_ptr<Graphics::EnoTexture> getTexture(const std::string& name)
		{
			auto it = textures_.find(name);
			if (it != textures_.end())
			{
				return it->second;
			}
			std::cerr << "Texture not found: " << name << std::endl;
			return nullptr;
		}

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

		void Update() {}

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
				throw std::runtime_error("Specified path does not exist!");
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

					if (result == FMOD_OK)
					{
						// Store the sound obtained
						sounds_[entry.path().filename().stem().string()] = sound;
						std::cout << entry.path().filename().stem().string() << " was retrieved!" << std::endl;
					}
					else
					{
						throw std::runtime_error("Failed to create a sound from a WAV/MP3 file!");
					}
				}
			}
		}

		// Load Images (png | jpg)
		void loadImages(const std::string& path)
		{
			if (!std::filesystem::exists(path))
			{
				std::cerr << "Image path does not exist: " << path << std::endl;
				return;
			}

			// Iterate through a folder through the given path
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (!entry.is_regular_file())
					continue;

				// Find files with the PNG or JPG extensions
				if (entry.path().extension().string() == ".png" ||
					entry.path().extension().string() == ".jpg")
				{
					auto texture = std::make_shared<Graphics::EnoTexture>();
					if (texture->loadFromFile(entry.path().string()))
					{
						textures_[entry.path().stem().string()] = texture;
						std::cout << "Loaded texture: " << entry.path().stem().string() << std::endl;
					}
					else
					{
						std::cerr << "Failed to load texture: " << entry.path().string() << std::endl;
					}
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

		void onSoundRequest(const Audio::RequestSound& event)
		{
			std::cout << "Sound Requested: " << event.sound_ << std::endl;

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