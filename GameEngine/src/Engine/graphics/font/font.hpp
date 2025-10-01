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

// Engine components
#include "../../math/math.hpp"

//Forward declarations
namespace Graphics {
    class EnoTexture;
}

namespace Font {

    struct Character {
        Math::eVec2<float> Size;     // glyph size
        Math::eVec2<float> Bearing;  // offset from baseline
        unsigned int Advance;        // horizontal advance
        Math::eVec2<float> UVStart;  // bottom-left UV
        Math::eVec2<float> UVEnd;    // top-right UV
    };

    class EnoFont {
    public:
        EnoFont() = default;
        bool loadFromFile(FT_Library ft, const std::string& path, int pixelSize);
        std::shared_ptr<Graphics::EnoTexture> getAtlasTexture() const;
        const Character& getChar(char c) const;

    private:
        std::shared_ptr<Graphics::EnoTexture> atlasTexture_;
        std::unordered_map<char, Character> characters_;
    };

    class FontManager {
    public:
        FontManager();
        ~FontManager();
        bool loadFont(const std::string& name, const std::string& path, int size);
        std::shared_ptr<EnoFont> getFont(const std::string& name);

    private:
        FT_Library ft_;
        std::unordered_map<std::string, std::shared_ptr<EnoFont>> fonts_;
    };
}