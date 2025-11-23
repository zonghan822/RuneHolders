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

    uint32_t TileLayer::getTexture(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        return tiles_[y][x].tileID;
    }

    void TileLayer::setTexture(int x, int y, uint32_t textureID) {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        tiles_[y][x].tileID = textureID;
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