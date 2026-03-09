#pragma once
#include <vector>
#include <string>

#include "../math/math.hpp"

namespace Map {

    struct Tile {
        bool walkable = true;

        std::string textureName = "";
        Math::eVec2<float> uv0 = { 0, 0 };   // top-left UV
        Math::eVec2<float> uv1 = { 0, 0 };   // bottom-right UV
        bool hasTexture = false;
        int rotation = 0;  // clockwise degrees: 0, 90, 180, 270
    };

    class TileLayer {
    public:

        TileLayer(const std::string& name, int w, int h, bool nav);

        bool isWalkable(int x, int y) const;
        void setWalkable(int x, int y, bool value);

        bool hasTileTexture(int x, int y) const;
        const Tile& getTile(int x, int y) const;
        Tile& getTile(int x, int y);
        void setTile(int x, int y, const std::string& textureName, Math::eVec2<float> uv0, Math::eVec2<float> uv1, int rotation = 0);
        void clearTile(int x, int y);

        // Accessor
        std::string name() const;
        int width() const;
        int height() const;
        bool isNavLayer() const;
        const std::vector<std::vector<Tile>>& tiles() const;

        // Modifier
        void rename(const std::string& newName);
        void setNav(bool isNavLayer);
        void resize(int newWidth, int newHeight, bool preserveData);

    private:
        std::string name_;
        bool contributesToNavigation_ = false;
        int width_ = 0;
        int height_ = 0;
        std::vector<std::vector<Tile>> tiles_;
    };
}