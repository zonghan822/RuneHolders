#include "tilemap.hpp"

// rapidjson
#include <document.h>
#include <stringbuffer.h>
#include <prettywriter.h>
#include <filewritestream.h>

#include <fstream>

namespace Map {

    TileMapSystem::TileMapSystem(Event::EnoEventBus& bus, int width, int height, float cellSize)
        : width_(width), height_(height), cellSize_(cellSize), bus_(bus)
    { 
        addLayer("Tile Layer 1", false);
    }

    // Create new layer
    void TileMapSystem::addLayer(const std::string& name, bool isNavLayer) {
        layers_.emplace_back(name, width_, height_, isNavLayer);
    }

    void TileMapSystem::removeLayer(int id) {
        if (id < layers_.size())
            layers_.erase(layers_.begin() + id);
    }

    // Painting
    void TileMapSystem::setWalkable(int layerIndex, int x, int y, bool value) {
        layers_[layerIndex].setWalkable(x, y, value);
    }

    void TileMapSystem::setTile(int layerIndex, int x, int y, const std::string& textureName, Math::eVec2<float> uv0, Math::eVec2<float> uv1) {
        layers_[layerIndex].setTile(x, y, textureName, uv0, uv1);
    }

    void TileMapSystem::paintRectangle(int layerIndex,
        int x0, int y0, int x1, int y1,
        bool walkable)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);

        for (int y = y0; y <= y1; ++y)
            for (int x = x0; x <= x1; ++x)
                layers_[layerIndex].setWalkable(x, y, walkable);
    }

    // Combined navigation result
    bool TileMapSystem::isWalkable(int x, int y) const {
        for (auto& layer : layers_) {
            if (!layer.isNavLayer()) continue;

            if (!layer.isWalkable(x, y))
                return false;  // Any blocking layer blocks
        }
        return true;
    }

    bool TileMapSystem::inBounds(int x, int y) const {
        return x >= 0 && y >= 0 &&
            x < width_ && y < height_;
    }

    Math::eVec2<int> TileMapSystem::worldToTile(const Math::eVec2<float>& pos) const {
        return { int(pos.x / cellSize_), int(pos.y / cellSize_) };
    }

    Math::eVec2<float> TileMapSystem::tileToWorld(int x, int y) const {
        return { x * cellSize_ + 0.5f * cellSize_,
                 y * cellSize_ + 0.5f * cellSize_ };
    }

    int TileMapSystem::width() const { return width_; }
    int TileMapSystem::height() const { return height_; }
    float TileMapSystem::cellSize() const { return cellSize_; }
    const std::vector<TileLayer>& TileMapSystem::layers() const { return layers_; }

    Math::eVec2<float> TileMapSystem::getWorldSize() const {
        return {
            static_cast<float>(width_) * cellSize_,
            static_cast<float>(height_) * cellSize_,
        };
    }

    bool TileMapSystem::saveToFile(const std::string& filepath) const {

        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();

        // Header
        doc.AddMember("width", width_, alloc);
        doc.AddMember("height", height_, alloc);
        doc.AddMember("cellSize", cellSize_, alloc);

        // Layers
        rapidjson::Value layers(rapidjson::kArrayType);

        for (const auto& layer : layers_) {
            rapidjson::Value jl(rapidjson::kObjectType);

            jl.AddMember("name",
                rapidjson::Value(layer.name().c_str(), alloc),
                alloc);

            jl.AddMember("nav", layer.isNavLayer(), alloc);

            // Tiles
            rapidjson::Value tileRows(rapidjson::kArrayType);

            for (int y = 0; y < height_; ++y) {
                rapidjson::Value row(rapidjson::kArrayType);
                row.Reserve(width_, alloc);

                for (int x = 0; x < width_; ++x) {
                    const auto& t = layer.tiles()[y][x];

                    rapidjson::Value tile(rapidjson::kObjectType);
                    tile.AddMember("walk", t.walkable, alloc);
                    tile.AddMember("tex", rapidjson::Value(t.textureName.c_str(), alloc), alloc);

                    rapidjson::Value uv0(rapidjson::kArrayType);
                    uv0.PushBack(t.uv0.x, alloc);
                    uv0.PushBack(t.uv0.y, alloc);
                    tile.AddMember("uv0", uv0, alloc);

                    rapidjson::Value uv1(rapidjson::kArrayType);
                    uv1.PushBack(t.uv1.x, alloc);
                    uv1.PushBack(t.uv1.y, alloc);
                    tile.AddMember("uv1", uv1, alloc);

                    tile.AddMember("hasTex", t.hasTexture, alloc);

                    row.PushBack(tile, alloc);
                }

                tileRows.PushBack(row, alloc);
            }

            jl.AddMember("tiles", tileRows, alloc);
            layers.PushBack(jl, alloc);
        }

        doc.AddMember("layers", layers, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        writer.SetIndent(' ', 2);
        doc.Accept(writer);

        std::ofstream ofs(filepath);
        if (!ofs.is_open())
            return false;

        ofs << buffer.GetString();
        return true;
    }

    bool TileMapSystem::loadFromFile(const std::string& filepath) {

        std::ifstream ifs(filepath);
        if (!ifs.is_open())
            return false;

        // Read entire file into std::string
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        rapidjson::Document doc;
        if (doc.Parse(data.c_str()).HasParseError())
            return false;

        width_ = doc["width"].GetInt();
        height_ = doc["height"].GetInt();
        cellSize_ = doc["cellSize"].GetFloat();

        layers_.clear();

        const auto& jlayers = doc["layers"];
        for (rapidjson::SizeType i = 0; i < jlayers.Size(); ++i) {
            const auto& jl = jlayers[i];

            TileLayer layer(
                jl["name"].GetString(),
                width_,
                height_,
                jl["nav"].GetBool()
            );

            const auto& jtiles = jl["tiles"];

            for (int y = 0; y < height_; ++y) {
                const auto& jrow = jtiles[y];
                for (int x = 0; x < width_; ++x) {
                    const auto& jt = jrow[x];
                    layer.setWalkable(x, y, jt["walk"].GetBool());

                    Tile& t = layer.getTile(x, y);

                    t.hasTexture = jt["hasTex"].GetBool();

                    if (t.hasTexture) {
                        t.textureName = jt["tex"].GetString();
                        t.uv0 = { jt["uv0"][0].GetFloat(), jt["uv0"][1].GetFloat() };
                        t.uv1 = { jt["uv1"][0].GetFloat(), jt["uv1"][1].GetFloat() };
                    }
                }
            }

            layers_.push_back(std::move(layer));
        }

        return true;
    }

    void TileMapSystem::resize(int newWidth, int newHeight, float newCellSize, bool preserveData)
    {
        if (newWidth <= 0 || newHeight <= 0)
            return;

        // Optional cell size change
        if (newCellSize > 0.0f)
            cellSize_ = newCellSize;

        // Resize each layer
        for (auto& layer : layers_) {
            layer.resize(newWidth, newHeight, preserveData);
        }

        width_ = newWidth;
        height_ = newHeight;
    }

    void TileMapSystem::renameLayer(int id, std::string const& newName) {
        layers_[id].rename(newName);
    }

    void TileMapSystem::setNavLayer(int id, bool isNav) {
        layers_[id].setNav(isNav);
    }
}