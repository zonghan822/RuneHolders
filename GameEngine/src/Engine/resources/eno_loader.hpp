/*============================================================================================
* @file         eno_loader.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu), Muhammad Raziq Bin Mohd Rafig(
* @date         10/3/2025
* @brief        Declaration file for the resource loader
*//*========================================================================================*/

#pragma once

// STB_IMAGE
#include "stb_image.h"

// FMOD
#include "fmod.hpp"

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
#include <fstream>


namespace Resources
{
	class EnoLoader
	{
	private:
		/*============================================================================================
		 * \brief				Sound paths and accepted extensions
		*//*========================================================================================*/
		static constexpr std::string_view musicPath_ = ASSETS_DIR_PATH "sound/music/";
		static constexpr std::string_view sfxPath_ = ASSETS_DIR_PATH "sound/sfx/";
		static constexpr std::array<std::string_view, 2> soundExtensions_ = { ".wav", ".mp3" };

		/*============================================================================================
		 * \brief				Sound storage
		*//*========================================================================================*/
		std::map<std::string, FMOD::Sound*> sounds_;

		/*============================================================================================
		 * \brief				Image path and storage
		*//*========================================================================================*/
		static constexpr std::string_view imagePath_ = ASSETS_DIR_PATH "art/";
		std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>> textures_;

		/*============================================================================================
		 * \brief				Shader path and storage
		*//*========================================================================================*/
		static constexpr std::string_view shaderPath_ = ASSETS_DIR_PATH "shaders/";
		std::unordered_map<std::string, std::string> vertShaders_;
		std::unordered_map<std::string, std::string> fragShaders_;

		// Fonts
		static constexpr std::string_view fontPath_ = ASSETS_DIR_PATH "fonts/";
		Font::FontManager fontManager_;

		// ECS Entities
		/*============================================================================================
		 * \brief				Entity save path
		*//*========================================================================================*/
		static constexpr std::string_view entityPath_ = ASSETS_DIR_PATH "saves/ecs_save_data.txt";

		/*============================================================================================
		 * \brief				Evrenet bus reference
		*//*========================================================================================*/
		Event::EnoEventBus& bus_;

	public:

		/*============================================================================================
		 * \brief				Eno Loader Constructor
		 * \param[in]			A reference to the event bus is taken to facilitate events for the loader
		*//*========================================================================================*/
		EnoLoader(Event::EnoEventBus&);

		/*============================================================================================
		 * \brief				Sound getters (Paths and extensions)
		*//*========================================================================================*/
		const std::string& getMusicPath() const;

		const std::string& getSFXPath() const;
		std::array<std::string_view, 2> getSoundExtensions() const;

		/*============================================================================================
		 * \brief				Texture getters (Path and textures)
		*//*========================================================================================*/
		const std::string& getImagePath() const;

		std::shared_ptr<Graphics::EnoTexture> getTexture(const std::string&);

		/*============================================================================================
		 * \brief				Shader getters (Path and shaders)
		*//*========================================================================================*/
		const std::string& getShaderPath() const;
		const std::string& getVertShader(const std::string& name) const;
		const std::string& getFragShader(const std::string& name) const;

		/*============================================================================================
		 * \brief				Eno Loader Constructor
		 * \param[in]			A reference to the event bus is taken to facilitate events for the loader
		*//*========================================================================================*/
		EnoLoader(Event::EnoEventBus&);

		/*============================================================================================
		 * \brief				Engine data getter
		*//*========================================================================================*/
		const std::string& getECSSavePath() const;

		/*============================================================================================
		 * \brief				Load Sound files (mp3 | wav)
		 * \param[in]			FMOD system
		 * \param[in]			Path to extract sounds from
		*//*========================================================================================*/
		void loadSound(FMOD::System* system, const std::string& path);

		/*============================================================================================
		 * \brief				Load png and jpg images
		 * \param[in]			Path to extract images from
		*//*========================================================================================*/
		void loadImages(const std::string& path);

		// Load shader files
		/*============================================================================================
		 * \brief				Load shader files
		 * \param[in]			Path to extract shaders from
		*//*========================================================================================*/
		void loadShaders(const std::string_view& path);

		/*========================================= Events ===========================================

		/*============================================================================================
		 * \brief				Load shader files
		 * \param[in]			Path to extract shaders from
		*//*========================================================================================*/
		void onSoundRequest(const Audio::RequestSound&);
	};
}