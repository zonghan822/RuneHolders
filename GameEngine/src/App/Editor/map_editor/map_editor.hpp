#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include "engine/eno_engine.hpp"

class EditorApplication;

class MapEditor {
public:
	MapEditor(Map::TileMapSystem& tilemap, ECS::EntityManager& entMng)
		: tilemap_{ tilemap }, entMng_{ entMng } {}

	bool isEditMode() const { return editMode_; }
	int getBrush() const  { return brush_; }
	int getSelectedLayer() const { return selectedLayer_; }

	void setEditMode(bool mode) { editMode_ = mode; }
	void setBrush(int brush) { brush_ = brush; }
	void selectLayer(int layerId) { selectedLayer_ = layerId; }

	void displayMenu();
	void displayTilesetMenu(Resources::EnoLoader& loader);
	void paint(EditorApplication* g);
	void draw(EditorApplication* g, ImDrawList* drawList);

	// Clears the collider ID cache without destroying entities (call after scene reload)
	void clearColliderCache() { tileColliders_.clear(); }

	// Destroy all tracked tile collider entities and clear the cache
	void clearAllTileColliders();

	// Rebuild colliders for all currently non-walkable tiles across all nav layers
	void rebuildAllTileColliders();

private:
	Map::TileMapSystem& tilemap_;
	ECS::EntityManager& entMng_;

	bool editMode_{false};
	int brush_{0};
	int selectedLayer_{0};

	// --- Tileset Feature ---
	std::string tilesetName_ = "";
	std::shared_ptr<Graphics::EnoTexture> tilesetTexture_;
	int tilesetRows_ = 1;
	int tilesetCols_ = 1;
	int selectedTileX_ = -1;
	int selectedTileY_ = -1;
	bool tilesetReady_ = false;
	int tileRotation_ = 0;  // current brush rotation: 0, 90, 180, 270

	// --- Serializing ---
	std::string currentFile_;

	// --- Tile Colliders ---
	// Key: packed (layer << 32 | y << 16 | x), Value: entity ID
	std::unordered_map<uint64_t, uint32_t> tileColliders_;

	static uint64_t tileKey(int layer, int x, int y)
	{
		return (static_cast<uint64_t>(layer) << 32) |
		       (static_cast<uint64_t>(static_cast<uint16_t>(y)) << 16) |
		        static_cast<uint64_t>(static_cast<uint16_t>(x));
	}

	// Create or destroy the collider entity for tile (layer, x, y) based on walkability
	void syncTileCollider(int layer, int x, int y);
};