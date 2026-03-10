#pragma once
#include <vector>
#include <string>
#include "tile_layer.hpp"
#include "../event/event_bus.hpp"
#include "../math/math.hpp"

namespace Map {
	
	// Basic tile map system for runtime editing and integration with pathfinding.
	// Each tile can be walkable or obstacle; you can also paint tiles in editor.
	class TileMapSystem {
	public:

        TileMapSystem(Event::EnoEventBus& bus, int width, int height, float cellSize);

        // Create new layer
        void addLayer(const std::string& name, bool isObsLayer);

        // Remove layer
        void removeLayer(int id);

        void setTile(int layerIndex, int x, int y, const std::string& textureName, Math::eVec2<float> uv0, Math::eVec2<float> uv1, int rotation = 0);
        void clearTile(int layerIndex, int x, int y);

        // Pathfinding
        bool isWalkable(int x, int y) const;
        bool inBounds(int x, int y) const;

        Math::eVec2<int> worldToTile(const Math::eVec2<float>& pos) const;
        Math::eVec2<float> tileToWorld(int x, int y) const;

        // Accessor
        int width() const;
        int height() const;
        float cellSize() const;
        const std::vector<TileLayer>& layers() const;

        Math::eVec2<float> getWorldSize() const;
        float worldOriginY() const;

        // Modifier
        void resize(int newWidth, int newHeight, float newCellSize = -1.0f, bool preserveData = true);
        void setWorldOriginY(float y);
        void renameLayer(int id, std::string const& newName);
        void setObstacleLayer(int id, bool);
        void setLayerZIndex(int id, int z);

        // Serialize
        bool saveToFile(const std::string& filepath) const;
        bool loadFromFile(const std::string& filepath);

    private:
        int width_;
        int height_;
        float cellSize_;
        float worldOriginY_ = 0.0f;
        std::vector<TileLayer> layers_;

        Event::EnoEventBus& bus_;
	};

}