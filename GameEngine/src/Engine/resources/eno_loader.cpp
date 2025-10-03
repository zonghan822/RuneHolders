/*============================================================================================
* @file         eno_loader.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu), Muhammad Raziq Bin Mohd Rafig(
* @date         10/3/2025
* @brief        Definition file for the resource loader
*//*========================================================================================*/


#include "eno_loader.hpp"


namespace Resources
{
	EnoLoader::EnoLoader(Event::EnoEventBus& bus) :
		bus_{ bus }
	{
	}

	EnoLoader::~EnoLoader()
	{
		for (const auto& sound : sounds_)
		{
			if (sound.second) sound.second->release();
		}
	}

	// Sound getters
	const std::string& EnoLoader::getMusicPath() const
	{
		static const std::string path{ musicPath_.data(), musicPath_.size() };
		return path;
	}

	const std::string& EnoLoader::getSFXPath() const
	{
		static const std::string path{ sfxPath_.data(), sfxPath_.size() };
		return path;
	}
	std::array<std::string_view, 2> EnoLoader::getSoundExtensions() const { return soundExtensions_; }

	// Texture getters
	const std::string& EnoLoader::getImagePath() const
	{
		static const std::string path{ imagePath_.data(), imagePath_.size() };
		return path;
	}

	std::shared_ptr<Graphics::EnoTexture> EnoLoader::getTexture(const std::string& name)
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
	const std::string& EnoLoader::getShaderPath() const
	{
		static const std::string path{ shaderPath_.data(), shaderPath_.size() };
		return path;
	}
	const std::string& EnoLoader::getVertShader(const std::string& name) const { return vertShaders_.at(name); }
	const std::string& EnoLoader::getFragShader(const std::string& name) const { return fragShaders_.at(name); }

	// Engine data getters
	const std::string& EnoLoader::getECSSavePath() const
	{
		static const std::string path{ entityPath_.data(), entityPath_.size() };
		return path;
	}

	void EnoLoader::loadSound(FMOD::System* system, const std::string& path)
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
			auto extensions = entry.path().extension().string();
			std::string_view view = extensions;

			// Iterate through the extensions of the files 
			// while checking against the specified sound extensions 
			if (std::find(soundExtensions_.begin(), soundExtensions_.end(), view) != soundExtensions_.end())
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
	void EnoLoader::loadImages(const std::string& path)
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
	void EnoLoader::loadShaders(const std::string_view& path)
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

	void EnoLoader::onSoundRequest(const Audio::RequestSound& event)
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
}