#include "tile_layer.hpp"

#include <algorithm>

#undef min
#undef max

namespace Map {

    TileLayer::TileLayer(const std::string& name, int w, int h, bool isObstacle)
        : name_(name), isObstacleLayer_(isObstacle), width_(w), height_(h)
    {
        tiles_.resize(h);
        for (auto& row : tiles_)
            row.resize(w);
    }

    bool TileLayer::isWalkable(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return true;
        if (!isObstacleLayer_) return true;
        return !tiles_[y][x].hasTexture;
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
    bool TileLayer::isObstacleLayer() const { return isObstacleLayer_; }
    int TileLayer::zIndex() const { return zIndex_; }
    const std::vector<std::vector<Tile>>& TileLayer::tiles() const { return tiles_; }

    void TileLayer::rename(const std::string& newName) { name_ = newName; }
    void TileLayer::setObstacleLayer(bool obstacle) { isObstacleLayer_ = obstacle; }
    void TileLayer::setZIndex(int z) { zIndex_ = z; }

    void TileLayer::resize(int newWidth, int newHeight, bool preserveData) {
        std::vector<std::vector<Tile>> newTiles(newHeight, std::vector<Tile>(newWidth));

        if (preserveData) {
            int minW = std::min(newWidth, width_);
            // yOffset > 0 when growing: shift existing rows down so new empty rows appear at top
            int yOffset = newHeight - height_;

            for (int srcY = 0; srcY < height_; ++srcY) {
                int dstY = srcY + yOffset;
                if (dstY < 0 || dstY >= newHeight) continue;
                for (int x = 0; x < minW; ++x)
                    newTiles[dstY][x] = tiles_[srcY][x];
            }
        }

        tiles_ = std::move(newTiles);
        width_ = newWidth;
        height_ = newHeight;
    }
}