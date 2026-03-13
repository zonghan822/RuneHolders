/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         font.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   75%
* @co-author    Caven Koh (caven.koh@digipen.edu)   25%
* @date         03-10-2025
* @brief        Font system built on FreeType and OpenGL. Provides EnoFont for glyph atlas creation and FontManager for font resource management.
*//*========================================================================================*/

#pragma once
// Freetype api
#include <ft2build.h>
#include FT_FREETYPE_H

// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// stds
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>

// Engine components
#include "../../math/math.hpp"

//Forward declarations
namespace Graphics {
    class EnoTexture;
}

namespace Font {

    /*============================================================================================
    * @struct   Character
    * @brief    Stores glyph metrics and texture coordinates within the font atlas.
    *//*========================================================================================*/
    struct Character {
        Math::eVec2<float> size;     // glyph size
        Math::eVec2<float> bearing;  // offset from baseline
        unsigned int advance;        // horizontal advance
        Math::eVec2<float> uvStart;  // bottom-left UV
        Math::eVec2<float> uvEnd;    // top-right UV
    };

    /*============================================================================================
    * @class    EnoFont
    * @brief    Loads and manages glyphs for a single font using a texture atlas.
    *//*========================================================================================*/
    class EnoFont {
    public:

        EnoFont() = default;

        /*============================================================================================
        * \brief   Load font data from file and build glyph atlas.
        * \param[in] ft         Reference to FreeType library instance.
        * \param[in] path       Path to the font file (.ttf or .otf).
        * \param[in] pixelSize  Target glyph size in pixels.
        * \return   True if font loaded successfully, false otherwise.
        *//*========================================================================================*/
        bool loadFromFile(FT_Library ft, const std::string& path, int pixelSize);

        /*============================================================================================
        * \brief   Get the OpenGL texture atlas for this font.
        * \return  Shared pointer to the atlas texture.
        *//*========================================================================================*/
        std::shared_ptr<Graphics::EnoTexture> getAtlasTexture() const;

        /*============================================================================================
        * \brief   Retrieve glyph character data.
        * \param[in] c  ASCII character.
        * \return   Reference to the corresponding Character struct.
        *//*========================================================================================*/
        const Character& getChar(char c) const;

    private:
        std::shared_ptr<Graphics::EnoTexture> atlasTexture_;
        std::unordered_map<char, Character> characters_;
    };

    /*============================================================================================
    * @class FontManager
    * @brief Centralized font resource manager for multiple fonts.
    *//*========================================================================================*/
    class FontManager {
    public:
        /*============================================================================================
        * \brief   Initialize the FreeType library for font handling.
        * \throw   std::runtime_error if initialization fails.
        *//*========================================================================================*/
        FontManager();

        /*============================================================================================
        * \brief   Destroy the FreeType library instance on shutdown.
        *//*========================================================================================*/
        ~FontManager();

        /*============================================================================================
        * \brief   Load a font and store it in the manager.
        * \param[in] name  Identifier for the font (e.g., "Arial").
        * \param[in] path  Path to the font file (.ttf or .otf).
        * \param[in] size  Glyph pixel size.
        * \return   True if the font was loaded successfully, false otherwise.
        *//*========================================================================================*/
        bool loadFont(const std::string& name, const std::string& path, int size);

        /*============================================================================================
        * \brief   Retrieve a previously loaded font by name.
        * \param[in] name  Identifier for the font.
        * \return   Shared pointer to the EnoFont. Returns nullptr if not found.
        *//*========================================================================================*/
        std::shared_ptr<EnoFont> getFont(const std::string& name);

        /*============================================================================================
        * \brief   Retrieve names of all loaded fonts.
        * \return  Sorted vector of font name strings.
        *//*========================================================================================*/
        std::vector<std::string> getFontNames() const {
            std::vector<std::string> names;
            names.reserve(fonts_.size());
            for (const auto& [name, font] : fonts_) {
                names.emplace_back(name);
            }
            std::sort(names.begin(), names.end());
            return names;
        }

    private:
        FT_Library ft_;
        std::unordered_map<std::string, std::shared_ptr<EnoFont>> fonts_;
    };
}