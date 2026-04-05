/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         map_editor.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   82%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   18%
* @brief        See file name.
*//*========================================================================================*/

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
	bool hasTilemap() const { return hasTilemap_; }
	int getSelectedLayer() const { return selectedLayer_; }

	void setEditMode(bool mode) { editMode_ = mode; }
	void setHasTilemap(bool v) { hasTilemap_ = v; if (!v) editMode_ = false; }
	void selectLayer(int layerId) { selectedLayer_ = layerId; }

	/** @brief Renders the map editor ImGui menu (layer selection, tilemap options). */
	void displayMenu();
	/** @brief Renders the tileset selection and preview panel. */
	void displayTilesetMenu(Resources::EnoLoader& loader);
	/** @brief Handles mouse input to paint tiles onto the active layer. */
	void paint(EditorApplication* g);
	/** @brief Draws the tilemap grid overlay onto the given ImGui draw list. */
	void draw(EditorApplication* g, ImDrawList* drawList);


private:
	Map::TileMapSystem& tilemap_;
	ECS::EntityManager& entMng_;

	bool editMode_{false};
	bool hasTilemap_{false};
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

	// Rebuild all tile colliders after a paint operation
	void syncTileCollider(int layer, int x, int y);

	// Restore tileset state from saved tile data after scene load/restart
	void tryRestoreTileset(Resources::EnoLoader& loader);
};