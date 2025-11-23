#pragma once
#include <vector>
#include <string>

namespace Map {

    struct Tile {
        bool walkable = true;
        uint32_t tileID = 0;  // Texture ID for rendering
    };

    class TileLayer {
    public:

        TileLayer(const std::string& name, int w, int h, bool nav);

        bool isWalkable(int x, int y) const;
        void setWalkable(int x, int y, bool value);

        uint32_t getTexture(int x, int y) const;
        void setTexture(int x, int y, uint32_t textureID);

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