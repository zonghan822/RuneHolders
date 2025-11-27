#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>

#include "engine/eno_engine.hpp"

class GameApplication;

class MapEditor {
public:
	MapEditor(Map::TileMapSystem& tilemap) : tilemap_{ tilemap } {}

	bool isEditMode() const { return editMode_; }
	int getBrush() const  { return brush_; }
	int getSelectedLayer() const { return selectedLayer_; }

	void setEditMode(bool mode) { editMode_ = mode; }
	void setBrush(int brush) { brush_ = brush; }
	void selectLayer(int layerId) { selectedLayer_ = layerId; }

	void displayMenu();
	void displayTilesetMenu(Resources::EnoLoader& loader);
	void paint(GameApplication* g);
	void draw(GameApplication* g, ImDrawList* drawList);


private:
	Map::TileMapSystem& tilemap_;

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

	// --- Serializing ---
	std::string currentFile_;
};