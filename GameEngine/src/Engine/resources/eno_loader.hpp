/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_loader.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   62%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   18%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   18%
* @date         03-10-2025
* @brief        Resource loading and management system for textures, shaders, fonts, and sounds.
*//*========================================================================================*/



#pragma once
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

#include "../file/file_system.hpp"

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
		std::string musicPath_;
		std::string sfxPath_;
		std::string rootMusicPath_;
		std::string rootSfxPath_;

		// Videos
		std::string videoPath_;
		static constexpr std::array<std::string_view, 2> soundExtensions_ = { ".wav", ".mp3" };
		std::unordered_map<std::string, FMOD::Sound*> sounds_;
		std::vector<std::unique_ptr<Audio::EnoAudioModule>> audioMods_;

		// Images
		std::string imagePath_;
		std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>> textures_;

		// Shaders
		std::string shaderPath_;
		std::unordered_map<std::string, std::string> vertShaders_;
		std::unordered_map<std::string, std::string> fragShaders_;

		// Fonts
		std::string fontPath_;
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
		* \brief	Get the music directory path.
		* \return	String reference to the music path.
		*//*========================================================================================*/
		const std::string& getRootMusicPath() const;

		/*============================================================================================
		* \brief	Get the sound effects (SFX) directory path.
		* \return	String reference to the SFX path.
		*//*========================================================================================*/
		const std::string& getRootSFXPath() const;

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
		* \author   Yun Qi
		* \brief	Retrieve all sounds/audio. Specially made for Deletion purposes
		* \return	Map of all audio.
		*//*========================================================================================*/
		std::unordered_map<std::string, FMOD::Sound*>& getSoundsMap();

		/*============================================================================================
		* \author   Yun Qi
		* \brief	Retrieve all textures. Specially made for Deletion purposes
		* \return	Map of all textures.
		*//*========================================================================================*/
		std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>>& getTexturesMap();

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
		* \brief	Retrieve names of all loaded fonts.
		* \return	Sorted vector of font name strings.
		*//*========================================================================================*/
		std::vector<std::string> getFontNames() const {
			return fontManager_.getFontNames();
		}

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
				std::cout << "[Loader] Sound path does not exist, skipping: " << path << "\n";
				return;
			}

			// Iterate through a folder through the given path
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
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
		* \brief	Re-read all path members from the current FileSystem::GetAssetsDir().
		*           Call this after FileSystem::Initialize() to keep paths in sync.
		*//*========================================================================================*/
		void reloadPaths() {
			const auto& base = FileSystem::GetAssetsDir();
			musicPath_  = (base / "sound/music").string();
			sfxPath_    = (base / "sound/sfx").string();
			imagePath_  = (base / "art").string();
			shaderPath_ = (base / "shaders").string();
			fontPath_   = (base / "fonts").string();
			videoPath_  = (base / "video").string();
		}

		/*============================================================================================
		* \brief	Get the video directory path.
		* \return	String reference to the video path.
		*//*========================================================================================*/
		const std::string& getVideoPath() const { return videoPath_; }

		/*============================================================================================
		* \brief	Scan the video directory and load all recognised video files into the graphics
		*           manager (mp4, avi, mov, webm, mkv).
		* \param[in] path				Directory to scan.
		* \param[in] graphicsManager	Graphics manager that will own the VideoPlayer objects.
		*//*========================================================================================*/
		void loadVideos(const std::string& path, Graphics::EnoGraphicsManager& graphicsManager);

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