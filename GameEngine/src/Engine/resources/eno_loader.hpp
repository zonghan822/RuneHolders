/*============================================================================================
* @file         eno_loader.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author       Muhammad Raziq Bin Mohd Rafig (),
* @author       Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief        Resource loading and management system for textures, shaders, fonts, and sounds.
*               Handles loading of image files (PNG/JPG), shader source files (GLSL),
*               audio files (WAV/MP3), and fonts (TTF/OTF). Integrates with an event bus
*               to publish and subscribe to resource-related events.
*===========================================================================================*/

#pragma once
// STB_IMAGE
#include "stb_image.h"

// FMOD
#include "fmod.hpp"

#include "../audio/eno_audio_module.hpp"

// Events
#include "../audio/audio_events.hpp"
#include "../event/event_bus.hpp"

// Graphics
#include "../graphics/graphics.hpp"

// Tile Map
#include "../tilemap/tilemap.hpp"

// stds
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <unordered_map>
#include <memory>
#include <fstream>

namespace Resources
{
	/*============================================================================================
	* \class	EnoLoader
	* \brief	Central resource management system for loading and accessing game assets
	*
	*			Handles loading of textures, shaders, fonts, and audio files from disk and
	*			provides access to loaded resources through a unified interface.
	*//*========================================================================================*/
	class EnoLoader
	{
	private:
		// Sounds
		static constexpr std::string_view musicPath_ = ASSETS_DIR_PATH "sound/music/";
		static constexpr std::string_view sfxPath_ = ASSETS_DIR_PATH "sound/sfx/";
		static constexpr std::array<std::string_view, 2> soundExtensions_ = { ".wav", ".mp3" };
		std::unordered_map<std::string, FMOD::Sound*> sounds_;
		std::vector<std::unique_ptr<Audio::EnoAudioModule>> audioMods_;

		// Images
		static constexpr std::string_view imagePath_ = ASSETS_DIR_PATH "art/";
		std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>> textures_;

		// Shaders
		static constexpr std::string_view shaderPath_ = ASSETS_DIR_PATH "shaders/";
		std::unordered_map<std::string, std::string> vertShaders_;
		std::unordered_map<std::string, std::string> fragShaders_;

		// Fonts
		static constexpr std::string_view fontPath_ = ASSETS_DIR_PATH "fonts/";
		Font::FontManager fontManager_;

		// Event Bus
		Event::EnoEventBus& bus_;

	public:

		/*============================================================================================
		* \brief	Constructor, subscribes to relevant events.
		* \param[in] bus	Reference to the event bus for publishing/subscribing to events.
		*//*========================================================================================*/
		EnoLoader(Event::EnoEventBus& bus);

		/*============================================================================================
		* \brief	Get the music directory path.
		* \return	String reference to the music path.
		*//*========================================================================================*/
		const std::string& getMusicPath() const;

		/*============================================================================================
		* \brief	Get the sound effects (SFX) directory path.
		* \return	String reference to the SFX path.
		*//*========================================================================================*/
		const std::string& getSFXPath() const;

		/*============================================================================================
		* \brief	Get supported sound extensions.
		* \return	Array of supported extensions (.wav, .mp3).
		*//*========================================================================================*/
		std::array<std::string_view, 2> getSoundExtensions() const;

		/*============================================================================================
		* \brief	Get the image directory path.
		* \return	String reference to the image path.
		*//*========================================================================================*/
		const std::string& getImagePath() const;

		/*============================================================================================
		* \brief	Retrieve a texture by name.
		* \param[in] name	Name of the texture (filename stem).
		* \return	Shared pointer to the texture, or nullptr if not found.
		*//*========================================================================================*/
		std::shared_ptr<Graphics::EnoTexture> getTexture(const std::string& name);

		/*============================================================================================
		* \author   Yunqi
		* \brief	Retrieve all textures in read only format.
		* \return	Map of all textures.
		*//*========================================================================================*/
		const std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>>& getAllTextures() const;

		/*============================================================================================
		* \brief	Retrieve all sounds/audio in read only format
		* \return	Map of all audio.
		*//*========================================================================================*/
		const std::unordered_map<std::string, FMOD::Sound*>& getSounds() const;

		/*============================================================================================
		* \brief	Get the shader directory path.
		* \return	String reference to the shader path.
		*//*========================================================================================*/
		const std::string& getShaderPath() const;

		/*============================================================================================
	    * \brief	Retrieve a vertex shader source by name.
	    * \param[in] name	Name of the shader (filename stem).
	    * \return	String reference to the shader source.
	    *//*========================================================================================*/
		const std::string& getVertShader(const std::string& name) const;

		/*============================================================================================
		* \brief	Retrieve a fragment shader source by name.
		* \param[in] name	Name of the shader (filename stem).
		* \return	String reference to the shader source.
		*//*========================================================================================*/
		const std::string& getFragShader(const std::string& name) const;

		/*============================================================================================
		* \brief	Retrieve a loaded font by name.
		* \param[in] name	Name of the font (filename stem).
		* \return	Shared pointer to the font, or nullptr if not found.
		*//*========================================================================================*/
		std::shared_ptr<Font::EnoFont> getFont(const std::string& name);

		/*============================================================================================
		* \brief	Get the font directory path.
		* \return	String reference to the font path.
		*//*========================================================================================*/
		const std::string& getFontPath() const;

		/*============================================================================================
		* \brief	Load sound files (WAV/MP3) into FMOD.
		* \tparam	Container Type of container holding supported file extensions.
		* \param[in] system		FMOD system pointer.
		* \param[in] extensions	List of supported file extensions.
		* \param[in] path		Directory to search for sounds.
		* \throw	std::runtime_error If path does not exist or sound creation fails.
		*//*========================================================================================*/
		template <typename Container>
		void loadSound(FMOD::System* system, const Container& extensions, const std::string& path) {
			if (!std::filesystem::exists(path))
			{
				throw std::runtime_error("Specified path does not exist!");
			}

			// Iterate through a folder through the given path
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (!entry.is_regular_file())
					continue;

				std::string fileExtension = entry.path().extension().string();

				// Check if this is an audio-like file but not supported
				bool isAudioFile = (fileExtension == ".wav" || fileExtension == ".mp3" ||
					fileExtension == ".ogg" || fileExtension == ".flac" ||
					fileExtension == ".aac" || fileExtension == ".wma" ||
					fileExtension == ".m4a" || fileExtension == ".aiff");

				// Find files with the WAV or MP3 extensions
				if (fileExtension == ".wav" || fileExtension == ".mp3")
				{
					// Retrieve sound from file
					FMOD::Sound* sound = nullptr;
					FMOD_RESULT result = system->createSound(
						entry.path().string().c_str(), FMOD_DEFAULT, nullptr, &sound);

					if (result == FMOD_OK)
					{
						// Store the sound obtained
						sounds_[entry.path().filename().stem().string()] = sound;
#ifdef _DEBUG
						std::cout << entry.path().filename().stem().string() << " was retrieved!" << std::endl;
#endif
					}
					else
					{
						// FMOD failed to load a supported format - publish error event
						bus_.publish(Audio::AudioLoadError{
							entry.path().filename().string(),
							entry.path().string(),
							fileExtension,
							"FMOD failed to load the file. The file may be corrupted or in an unsupported codec."
							});
					}
				}
				else if (isAudioFile)
				{
					// Unsupported audio format detected - publish error event
					bus_.publish(Audio::AudioLoadError{
						entry.path().filename().string(),
						entry.path().string(),
						fileExtension,
						"File format is not supported. Only .wav and .mp3 files are supported."
						});

#ifdef _DEBUG
					std::cout << "Unsupported audio format detected: " << entry.path().filename().string() << std::endl;
#endif
				}
			}
		}

		/*============================================================================================
		* \brief	checks if there is texture in a vector. 
		* \author   YunQi
		*//*========================================================================================*/
		bool hasTexture(const std::string& name) const {
			return textures_.find(name) != textures_.end();
		}

		/*============================================================================================
		* \brief	Adds image textures (PNG/JPG) from a directory.
		* \param[in] path	Directory path for textures.
		*//*========================================================================================*/
		void addTexture(const std::string& name, std::shared_ptr<Graphics::EnoTexture> tex) {
			textures_[name] = tex;
		}

		/*============================================================================================
		* \brief			Store Audio modules
		* \param[in] path	Directory path for textures.
		*//*========================================================================================*/
		void storeAudioMod(std::unique_ptr<Audio::EnoAudioModule> mod)
		{
			audioMods_.push_back(std::move(mod));
		}

		/*============================================================================================
		* \brief			Add a loaded sound to the sound map
		* \param[in] name	Name/key for the sound (usually filename without extension)
		* \param[in] sound	Pointer to the FMOD sound object
		*//*========================================================================================*/
		void addSound(const std::string& name, FMOD::Sound* sound)
		{
			sounds_[name] = sound;
		}

		/*============================================================================================
		* \brief	Load image textures (PNG/JPG) from a directory.
		* \param[in] path	Directory path for textures.
		*//*========================================================================================*/
		void loadImages(const std::string& path);

		/*============================================================================================
		* \brief	Load GLSL shader files (.vert/.frag).
		* \param[in] path	Directory path for shader files.
		*//*========================================================================================*/
		void loadShaders(const std::string_view& path);

		/*============================================================================================
		* \brief	Load font files (.ttf/.otf).
		* \param[in] path		Directory path for font files.
		* \param[in] pixelSize	Size of the font in pixels.
		*//*========================================================================================*/
		void loadFonts(const std::string& path, int pixelSize);

		/*============================================================================================
		* \brief	Handle sound request events from the event bus.
		* \param[in] event	RequestSound event containing sound name.
		*//*========================================================================================*/
		void onSoundRequest(const Audio::RequestSound& event);

	};
}