#include "tile_layer.hpp"

#include <algorithm>

#undef min
#undef max

namespace Map {

    TileLayer::TileLayer(const std::string& name, int w, int h, bool nav)
        : name_(name), contributesToNavigation_(nav), width_(w), height_(h) 
    {
        tiles_.resize(h);
        for (auto& row : tiles_)
            row.resize(w);
    }

    bool TileLayer::isWalkable(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return false;
        return tiles_[y][x].walkable;
    }

    void TileLayer::setWalkable(int x, int y, bool value) {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        tiles_[y][x].walkable = value;
    }
    
    // Returns whether tile has a visual texture
    bool TileLayer::hasTileTexture(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return false;
        return tiles_[y][x].hasTexture;
    }

    const Tile& TileLayer::getTile(int x, int y) const {
        return tiles_[y][x];
    }

    Tile& TileLayer::getTile(int x, int y) {
        return tiles_[y][x];
    }

    void TileLayer::setTile(
        int x, int y,
        const std::string& textureName,
        Math::eVec2<float> uv0,
        Math::eVec2<float> uv1,
        int rotation)
    {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

        Tile& t = tiles_[y][x];
        t.textureName = textureName;
        t.uv0 = uv0;
        t.uv1 = uv1;
        t.hasTexture = true;
        t.rotation = rotation;
    }

    void TileLayer::clearTile(int x, int y)
    {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

        Tile& t = tiles_[y][x];
        t.textureName.clear();
        t.uv0 = { 0, 0 };
        t.uv1 = { 0, 0 };
        t.hasTexture = false;
        t.rotation = 0;
    }

    std::string TileLayer::name() const { return name_; }
    int TileLayer::width() const { return width_; }
    int TileLayer::height() const { return height_; }
    bool TileLayer::isNavLayer() const { return contributesToNavigation_; }
    const std::vector<std::vector<Tile>>& TileLayer::tiles() const { return tiles_; }

    void TileLayer::rename(const std::string& newName) { name_ = newName; }
    void TileLayer::setNav(bool isNavLayer) { contributesToNavigation_ = isNavLayer; }

    void TileLayer::resize(int newWidth, int newHeight, bool preserveData) {
        std::vector<std::vector<Tile>> newTiles(newHeight, std::vector<Tile>(newWidth));

        if (preserveData) {
            int minH = std::min(newHeight, height_);
            int minW = std::min(newWidth, width_);

            for (int y = 0; y < minH; ++y)
                for (int x = 0; x < minW; ++x)
                    newTiles[y][x] = tiles_[y][x];
        }

        tiles_ = std::move(newTiles);
        width_ = newWidth;
        height_ = newHeight;
    }
}