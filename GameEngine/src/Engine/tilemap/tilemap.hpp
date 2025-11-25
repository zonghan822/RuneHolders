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
        void addLayer(const std::string& name, bool isNavLayer);

        // Remove layer
        void removeLayer(int id);

        void setWalkable(int layerIndex, int x, int y, bool value);

        void paintRectangle(int layerIndex,
            int x0, int y0, int x1, int y1,
            bool walkable);

        // Pathfinding
        bool isWalkable(int x, int y) const;
        bool inBounds(int x, int y) const;

        Math::eVec2<int> worldToTile(const Math::eVec2<float>& pos) const;
        Math::eVec2<float> tileToWorld(int x, int y) const;

        // Serialize
        bool saveToFile(const std::string& filepath) const;
        bool loadFromFile(const std::string& filepath);

        // Accessor
        int width() const;
        int height() const;
        float cellSize() const;
        const std::vector<TileLayer>& layers() const;

        // Modifier
        void resize(int newWidth, int newHeight, float newCellSize = -1.0f, bool preserveData = true);
        void renameLayer(int id, std::string const& newName);
        void setNavLayer(int id, bool);

    private:
        int width_;
        int height_;
        float cellSize_;
        std::vector<TileLayer> layers_;

        Event::EnoEventBus& bus_;
	};

}