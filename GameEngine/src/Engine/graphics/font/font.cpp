/*============================================================================================
* @file         font.cpp
* @author       Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief        Implementation of the EnoFont and FontManager classes using FreeType and OpenGL.
*               
*                This file implements the font rendering system using FreeType for font loading
*               and glyph processing. It creates texture atlases containing all printable characters
*               for efficient text rendering. The system handles font metrics, character spacing,
*               and UV coordinate calculation for proper text rendering in the graphics system.
*//*========================================================================================*/

#include "font.hpp"
#include "../graphics.hpp"

namespace Font {

    // =============================================================================
    // Font Implementation - Wrapper class to hold glyph textures
    // =============================================================================

    bool EnoFont::loadFromFile(FT_Library ft, const std::string& path, int pixelSize) {
        FT_Face face;
        if (FT_New_Face(ft, path.c_str(), 0, &face)) {
            std::cerr << "Failed to load font: " << path << std::endl;
            return false;
        }
        FT_Set_Pixel_Sizes(face, 0, pixelSize);

        // ---- Pass 1: measure atlas size ----
        int atlasWidth = 0, atlasHeight = 0;
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            atlasWidth += face->glyph->bitmap.width;
            atlasHeight = std::max(atlasHeight, (int)face->glyph->bitmap.rows);
        }

        std::vector<unsigned char> atlasPixels(atlasWidth * atlasHeight, 0);

        // ---- Pass 2: blit glyphs into atlas ----
        int xOffset = 0;
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            FT_Bitmap& bmp = face->glyph->bitmap;

            for (int row = 0; row < bmp.rows; ++row) {
                for (int col = 0; col < bmp.width; ++col) {
                    int atlasIndex = (row * atlasWidth) + (xOffset + col);
                    atlasPixels[atlasIndex] = bmp.buffer[row * bmp.pitch + col];
                }
            }

            Character ch;
            ch.size = { static_cast<float>(bmp.width), static_cast<float>(bmp.rows) };
            ch.bearing = { static_cast<float>(face->glyph->bitmap_left), 
                static_cast<float>(face->glyph->bitmap_top) };
            ch.advance = static_cast<unsigned int>(face->glyph->advance.x);

            ch.uvStart = { static_cast<float>(xOffset) / atlasWidth, 0.0f };
            ch.uvEnd = { static_cast<float>(xOffset + bmp.width) / atlasWidth, 
                static_cast<float>(bmp.rows) / atlasHeight };

            characters_.insert({ c, ch });
            xOffset += bmp.width;
        }

        // ---- Upload to OpenGL via EnoTexture ----
        atlasTexture_ = std::make_shared<Graphics::EnoTexture>();
        atlasTexture_->loadFromMemory(atlasWidth, atlasHeight, 1, atlasPixels.data());

        FT_Done_Face(face);
        return true;
    }

    std::shared_ptr<Graphics::EnoTexture> EnoFont::getAtlasTexture() const {
        return atlasTexture_;
    }

    const Character& EnoFont::getChar(char c) const {
        return characters_.at(c); 
    }


    // =============================================================================
    // FontManager Implementation
    // =============================================================================

    FontManager::FontManager() {
        if (FT_Init_FreeType(&ft_)) {
            throw std::runtime_error("Failed to init FreeType");
        }
    }

    FontManager::~FontManager() {
        FT_Done_FreeType(ft_);
    }

    bool FontManager::loadFont(const std::string& name, const std::string& path, int size) {
        auto font = std::make_shared<EnoFont>();
        if (!font->loadFromFile(ft_, path, size)) {
            return false;
        }
        fonts_[name] = font;
        return true;
    }

    std::shared_ptr<EnoFont> FontManager::getFont(const std::string& name) {
        auto it = fonts_.find(name);
        if (it != fonts_.end()) return it->second;
        std::cerr << "Font not found: " << name << std::endl;
        return nullptr;
    }
}