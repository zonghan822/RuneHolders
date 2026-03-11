#include "tilemap.hpp"
#include "../ecs/entity.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/collision_component.hpp"

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
    void TileMapSystem::addLayer(const std::string& name, bool isObsLayer) {
        layers_.emplace_back(name, width_, height_, isObsLayer);
    }

    void TileMapSystem::removeLayer(int id) {
        if (id < layers_.size())
            layers_.erase(layers_.begin() + id);
    }

    // Painting
    void TileMapSystem::setTile(int layerIndex, int x, int y, const std::string& textureName, Math::eVec2<float> uv0, Math::eVec2<float> uv1, int rotation) {
        layers_[layerIndex].setTile(x, y, textureName, uv0, uv1, rotation);
    }

    void TileMapSystem::clearTile(int layerIndex, int x, int y) {
        layers_[layerIndex].clearTile(x, y);
    }


    // Combined navigation result
    bool TileMapSystem::isWalkable(int x, int y) const {
        for (auto& layer : layers_) {
            if (!layer.isObstacleLayer()) continue;
            if (layer.hasTileTexture(x, y))
                return false;
        }
        return true;
    }

    bool TileMapSystem::inBounds(int x, int y) const {
        return x >= 0 && y >= 0 &&
            x < width_ && y < height_;
    }

    Math::eVec2<int> TileMapSystem::worldToTile(const Math::eVec2<float>& pos) const {
        return { int(pos.x / cellSize_), int((pos.y - worldOriginY_) / cellSize_) };
    }

    Math::eVec2<float> TileMapSystem::tileToWorld(int x, int y) const {
        return { x * cellSize_ + 0.5f * cellSize_,
                 worldOriginY_ + y * cellSize_ + 0.5f * cellSize_ };
    }

    int TileMapSystem::width() const { return width_; }
    int TileMapSystem::height() const { return height_; }
    float TileMapSystem::cellSize() const { return cellSize_; }
    float TileMapSystem::worldOriginY() const { return worldOriginY_; }
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
        doc.AddMember("worldOriginY", worldOriginY_, alloc);

        // Layers
        rapidjson::Value layers(rapidjson::kArrayType);

        for (const auto& layer : layers_) {
            rapidjson::Value jl(rapidjson::kObjectType);

            jl.AddMember("name",
                rapidjson::Value(layer.name().c_str(), alloc),
                alloc);

            jl.AddMember("obstacle", layer.isObstacleLayer(), alloc);
            jl.AddMember("zIndex", layer.zIndex(), alloc);

            // Tiles
            rapidjson::Value tileRows(rapidjson::kArrayType);

            for (int y = 0; y < height_; ++y) {
                rapidjson::Value row(rapidjson::kArrayType);
                row.Reserve(width_, alloc);

                for (int x = 0; x < width_; ++x) {
                    const auto& t = layer.tiles()[y][x];

                    rapidjson::Value tile(rapidjson::kObjectType);
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
                    tile.AddMember("rot", t.rotation, alloc);

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
        worldOriginY_ = doc.HasMember("worldOriginY") ? doc["worldOriginY"].GetFloat() : 0.0f;

        layers_.clear();

        const auto& jlayers = doc["layers"];
        for (rapidjson::SizeType i = 0; i < jlayers.Size(); ++i) {
            const auto& jl = jlayers[i];

            bool isObstacle = jl.HasMember("obstacle") ? jl["obstacle"].GetBool()
                             : (jl.HasMember("nav") ? jl["nav"].GetBool() : false);
            TileLayer layer(
                jl["name"].GetString(),
                width_,
                height_,
                isObstacle
            );
            if (jl.HasMember("zIndex"))
                layer.setZIndex(jl["zIndex"].GetUint());

            const auto& jtiles = jl["tiles"];

            for (int y = 0; y < height_; ++y) {
                const auto& jrow = jtiles[y];
                for (int x = 0; x < width_; ++x) {
                    const auto& jt = jrow[x];
                    Tile& t = layer.getTile(x, y);

                    t.hasTexture = jt["hasTex"].GetBool();

                    if (t.hasTexture) {
                        t.textureName = jt["tex"].GetString();
                        t.uv0 = { jt["uv0"][0].GetFloat(), jt["uv0"][1].GetFloat() };
                        t.uv1 = { jt["uv1"][0].GetFloat(), jt["uv1"][1].GetFloat() };
                    }
                    t.rotation = jt.HasMember("rot") ? jt["rot"].GetInt() : 0;
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

        // When height grows, shift origin up so existing tiles keep their world positions
        if (preserveData && newHeight > height_)
            worldOriginY_ -= (newHeight - height_) * cellSize_;

        // Resize each layer
        for (auto& layer : layers_) {
            layer.resize(newWidth, newHeight, preserveData);
        }

        width_ = newWidth;
        height_ = newHeight;
    }

    void TileMapSystem::setWorldOriginY(float y) { worldOriginY_ = y; }

    void TileMapSystem::renameLayer(int id, std::string const& newName) {
        layers_[id].rename(newName);
    }

    void TileMapSystem::setObstacleLayer(int id, bool obstacle) {
        layers_[id].setObstacleLayer(obstacle);
    }

    void TileMapSystem::setLayerZIndex(int id, int z) {
        layers_[id].setZIndex(z);
    }

    void TileMapSystem::buildTileColliders(ECS::EntityManager& entMng)
    {
        for (uint32_t id : tileColliderEntities_)
            ECS::Entity(id).destroy(entMng);
        tileColliderEntities_.clear();

        for (const auto& layer : layers_)
        {
            if (!layer.isObstacleLayer()) continue;
            for (int y = 0; y < height_; ++y)
            {
                for (int x = 0; x < width_; ++x)
                {
                    if (!layer.hasTileTexture(x, y)) continue;
                    float cx = (x + 0.5f) * cellSize_;
                    float cy = worldOriginY_ + (y + 0.5f) * cellSize_;
                    ECS::Entity e = entMng.create<
                        ECSComponents::TransformComponent,
                        ECSComponents::CollisionComponent>(
                            ECSComponents::TransformComponent{
                                Math::eVec2<float>{ cx, cy },
                                Math::eVec2<float>{ 0.f, 0.f },
                                Math::eVec2<float>{ 1.f, 1.f }
                            },
                            ECSComponents::CollisionComponent{ cellSize_, cellSize_ }
                        );
                    tileColliderEntities_.push_back(e.id());
                }
            }
        }
    }
}