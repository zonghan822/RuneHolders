/*============================================================================================
* @file         eno_loader.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author		Muhammad Raziq Bin Mohd Rafig (),
* @author		Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief		Implementation of resource loading system for game assets
*				
*				This file implements the resource loading system that handles loading and
*				management of game assets including textures, shaders, fonts, and audio files.
*				The system provides a unified interface for accessing resources by name and
*				includes comprehensive error handling and debug output. Resources are loaded
*				from predefined directory structures and cached for efficient reuse.
*//*========================================================================================*/

#include "eno_loader.hpp"

namespace Resources {

	EnoLoader::EnoLoader(Event::EnoEventBus& bus) : bus_{ bus } {

		// Build runtime paths
		const auto& base = FileSystem::GetAssetsDir();
		const auto& root = FileSystem::GetRootDir();


		musicPath_     = (base / "sound/music").string();
		sfxPath_       = (base / "sound/sfx").string();
		imagePath_     = (base / "art").string();
		shaderPath_    = (base / "shaders").string();
		fontPath_      = (base / "fonts").string();
		videoPath_     = (base / "video").string();
		rootMusicPath_ = (root / "assets/sound/music").string();
		rootSfxPath_   = (root / "assets/sound/sfx").string();

		// Subscribe audio events
		bus_.subscribe<Audio::RequestSound>([&](const Audio::RequestSound& event) {
			onSoundRequest(event);
			});
	}

	// Sound getters
	const std::string& EnoLoader::getMusicPath() const {
		return musicPath_;
	}

	const std::string& EnoLoader::getSFXPath() const {
		return sfxPath_;
	}

	const std::string& EnoLoader::getRootMusicPath() const {
		return rootMusicPath_;
	}

	const std::string& EnoLoader::getRootSFXPath() const {
		return rootSfxPath_;
	}

	std::array<std::string_view, 2> EnoLoader::getSoundExtensions() const { 
		return soundExtensions_; 
	}

	// Texture getters
	const std::string& EnoLoader::getImagePath() const {
		return imagePath_;
	}

	std::shared_ptr<Graphics::EnoTexture> EnoLoader::getTexture(const std::string& name) {
		auto it = textures_.find(name);
		if (it != textures_.end())
		{
			return it->second;
		}
		std::cerr << "Texture not found: " << name << std::endl;
		return nullptr;
	}

	const std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>>& EnoLoader::getAllTextures() const {
		return textures_;
	}

	const std::unordered_map<std::string, FMOD::Sound*>& EnoLoader::getSounds() const 
	{
		return sounds_;
	}

	//Yun Qi added this for removal of sounds.
    std::unordered_map<std::string, FMOD::Sound*>& EnoLoader::getSoundsMap()
	{
		return sounds_;
	}

	//Yun Qi added this for removal of textures.
	std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>>& EnoLoader::getTexturesMap()
	{
		return textures_;
	}

	// Shader getters
	const std::string& EnoLoader::getShaderPath() const {
		return shaderPath_;
	}

	const std::string& EnoLoader::getVertShader(const std::string& name) const { 
		return vertShaders_.at(name); 
	}

	const std::string& EnoLoader::getFragShader(const std::string& name) const { 
		return fragShaders_.at(name); 
	}

	// Font getters
	std::shared_ptr<Font::EnoFont> EnoLoader::getFont(const std::string& name) {
		return fontManager_.getFont(name); 
	}

	const std::string& EnoLoader::getFontPath() const {
		return fontPath_;
	}

	// Load Images (png | jpg)
	void EnoLoader::loadImages(const std::string& path) {
		if (!std::filesystem::exists(path))
		{
			std::cerr << "Image path does not exist: " << path << std::endl;
			return;
		}

		// Iterate through a folder through the given path
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
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
#ifdef _DEBUG
					std::cout << "Loaded texture: " << entry.path().stem().string() << std::endl;
#endif
				}
				else
				{
					std::cerr << "Failed to load texture: " << entry.path().string() << std::endl;
				}
			}
		}
	}

	// Load shader files
	void EnoLoader::loadShaders(const std::string_view& path) {
		// Iterate through a folder through the given path
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (!entry.is_regular_file())
				continue;

			std::ifstream file(entry.path());

			if (!file.is_open()) {
				throw std::runtime_error("Failed to open a file!");
			}

			// Allocate buffer for storing text data
			// from each shader file
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string source = buffer.str();

			auto name = entry.path().stem().string();

			// Find files with the frag and vert extensions
			if (entry.path().extension().string() == ".vert")
			{
				vertShaders_[name] = std::move(source);
			}
			else if (entry.path().extension().string() == ".frag")
			{
				fragShaders_[name] = std::move(source);
			}

			// When both vert+frag exist, publish event
			if (vertShaders_.count(name) && fragShaders_.count(name)) {
				bus_.publish(Graphics::Events::ShaderLoaded{
					name,
					vertShaders_.at(name),
					fragShaders_.at(name)
					});
			}
		}
	}

	void EnoLoader::loadFonts(const std::string& path, int pixelSize) {
		if (!std::filesystem::exists(path)) {
			std::cerr << "Font path does not exist: " << path << std::endl;
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() == ".ttf" || entry.path().extension() == ".otf") {
				fontManager_.loadFont(entry.path().stem().string(), entry.path().string(), pixelSize);
			}
		}
	}

	void EnoLoader::loadVideos(const std::string& path, Graphics::EnoGraphicsManager& graphicsManager)
	{
		if (!std::filesystem::exists(path))
		{
			std::cout << "[Loader] Video path does not exist, skipping: " << path << "\n";
			return;
		}

		static const std::array<std::string_view, 5> videoExts = { ".mp4", ".avi", ".mov", ".webm", ".mkv" };

		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
		{
			if (!entry.is_regular_file()) continue;

			std::string ext = entry.path().extension().string();
			// Lowercase the extension for case-insensitive matching
			for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

			bool isVideo = false;
			for (auto& ve : videoExts) { if (ext == ve) { isVideo = true; break; } }
			if (!isVideo) continue;

			std::string name = entry.path().stem().string();
			if (graphicsManager.loadVideo(name, entry.path().string()))
			{
				std::cout << "[Loader] Loaded video: " << name << "\n";
			}
			else
			{
				std::cerr << "[Loader] Failed to load video: " << entry.path().string() << "\n";
			}
		}
	}

	void EnoLoader::onSoundRequest(const Audio::RequestSound& event) {
		// Look for sound requested in the map
		auto it = sounds_.find(event.sound_);
		if (it == sounds_.end()) {
			// Instead of throwing, publish an error event
			bus_.publish(Audio::AudioLoadError{
				event.sound_,
				"Unknown location",
				"Unknown",
				"Sound was requested but not found in loaded sounds. The audio file may have failed to load or doesn't exist."
				});
			return;
		}

		bus_.publish(Audio::LoadSound{ event.sound_, it->second });
	}
}