#include "map_editor.hpp"
#include "../editor_logic.hpp"

void MapEditor::displayMenu() {
	if (!editMode_) return;

	ImGui::Begin("Tilemap Editor", nullptr, ImGuiWindowFlags_NoCollapse);

	ImGui::Text("=== TILEMAP EDITOR ===");
	ImGui::Separator();

	// --- Layer selection ---
	ImGui::Text("Active Layer:");

	auto& tilemapLayers = tilemap_.layers();
	int layerCount = static_cast<int>(tilemapLayers.size());

	// Ensure selectedLayer_ is valid
	if (selectedLayer_ >= layerCount) {
		selectedLayer_ = layerCount > 0 ? 0 : 0;
	}

	std::vector<std::string> layerNames;
	for (auto const& layer : tilemapLayers)
		layerNames.push_back(layer.name());

	// Only show combo if there are layers
	const char* comboPreview = (layerCount > 0 && selectedLayer_ < layerCount)
		? layerNames[selectedLayer_].c_str()
		: "No layers";

	if (ImGui::BeginCombo("##TilemapLayerCombo", comboPreview)) {
		for (int i = 0; i < layerCount; ++i) {
			bool isSelected = (i == selectedLayer_);
			if (ImGui::Selectable(layerNames[i].c_str(), isSelected))
				selectedLayer_ = i;
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	if (ImGui::Button("Edit Layer")) {
		ImGui::OpenPopup("layer_edit");
	}
	ImGui::SameLine();
	if (ImGui::Button("Add New Layer")) {
		ImGui::OpenPopup("layer_add");
	}

	if (ImGui::BeginPopup("layer_edit")) {
		static std::string warningBuf;
		static char renameBuf[64];
		ImGui::Text("Change layer name");
		ImGui::InputText("##Rename_Layer", renameBuf, sizeof(renameBuf));

		if (!warningBuf.empty())
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), warningBuf.c_str());

		if (ImGui::Button("Change new name")) {

			if (!renameBuf || renameBuf[0] == '\0') {
				warningBuf = "Empty name not allowed!";
			}
			else {
				tilemap_.renameLayer(selectedLayer_, std::string(renameBuf));
				warningBuf.clear();
				ImGui::CloseCurrentPopup();
				memset(renameBuf, 0, sizeof(renameBuf));
			}
		}

		if (layerCount > 1) {
			if (ImGui::Button("Delete layer")) {
				tilemap_.removeLayer(selectedLayer_);
				selectedLayer_ = 0;
				warningBuf.clear();
				ImGui::CloseCurrentPopup();
			}
		}

		// Cancel edits
		if (ImGui::Button("Cancel")) {
			warningBuf.clear();
			memset(renameBuf, 0, sizeof(renameBuf));
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("layer_add")) {
		static std::string warningBuf;
		static char newLayerBuf[64];
		ImGui::Text("Layer name");
		ImGui::InputText("##New_Layer_Name", newLayerBuf, sizeof(newLayerBuf));

		if (!warningBuf.empty())
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), warningBuf.c_str());

		static bool newIsNav = false;
		ImGui::Checkbox("Navigation Layer", &newIsNav);

		if (ImGui::Button("Add New Layer")) {

			if (!newLayerBuf || newLayerBuf[0] == '\0') {
				warningBuf = "Empty name not allowed!";
			}
			else {
				tilemap_.addLayer(std::string(newLayerBuf), newIsNav);
				selectedLayer_ = layerCount;
				ImGui::CloseCurrentPopup();
				warningBuf.clear();
				memset(newLayerBuf, 0, sizeof(newLayerBuf));
			}
		}

		// Cancel add
		if (ImGui::Button("Cancel")) {
			warningBuf.clear();
			memset(newLayerBuf, 0, sizeof(newLayerBuf));
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::Separator();

	// --- Resize tilemap ---
	static int newW = tilemap_.width();
	static int newH = tilemap_.height();
	static float newCellSize = tilemap_.cellSize();

	ImGui::InputInt("Width", &newW);
	ImGui::InputInt("Height", &newH);
	ImGui::InputFloat("Cell Size", &newCellSize);

	if (ImGui::Button("Resize Tilemap")) {
		tilemap_.resize(newW, newH, newCellSize);
	}

	ImGui::Separator();

	// --- Set Navigation Checkbox ---
	if (layerCount > 0 && selectedLayer_ < layerCount) {
		bool isNav = tilemapLayers[selectedLayer_].isNavLayer();
		if (ImGui::Checkbox("Navigation Layer", &isNav)) {
			tilemap_.setNavLayer(selectedLayer_, isNav);
		}
	}
	else {
		ImGui::TextDisabled("Navigation Layer (no layer selected)");
	}

	ImGui::Separator();

	// --- Save / Load ---
	ImGui::Text("-- Save / Load Tilemap --");

	if (!currentFile_.empty()) {
		if (ImGui::Button("Save")) {
			tilemap_.saveToFile(currentFile_);
		}
		ImGui::SameLine();
	}
	
	if (ImGui::Button("Save As")) {
		ImGui::OpenPopup("map_save");
	}

	if (ImGui::Button("Load Map")) {
		ImGui::OpenPopup("map_load");
	}

	if (ImGui::BeginPopup("map_save")) {
		static std::string warningBuf;
		static char mapSaveBuf[64];
		ImGui::Text("Enter file name:");
		ImGui::InputText("##Map_Name", mapSaveBuf, sizeof(mapSaveBuf));

		if (!warningBuf.empty())
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), warningBuf.c_str());

		if (ImGui::Button("Save")) {

			if (!mapSaveBuf || mapSaveBuf[0] == '\0') {
				warningBuf = "Empty name not allowed!";
			}
			else {
				std::string newMapName = mapSaveBuf;
				if (newMapName.find(".json") == std::string::npos) newMapName += ".json";

				std::string mapsDir = FileSystem::GetAssetsDir().string() + "/saves/maps/";
				if (!std::filesystem::exists(mapsDir)) {
					std::filesystem::create_directories(mapsDir);
				}

				std::string fullpath = mapsDir + newMapName;

				tilemap_.saveToFile(fullpath);
				currentFile_ = fullpath;
				warningBuf.clear();
				ImGui::CloseCurrentPopup();
				memset(mapSaveBuf, 0, sizeof(mapSaveBuf));
			}
		}

		// Cancel save
		if (ImGui::Button("Cancel")) {
			memset(mapSaveBuf, 0, sizeof(mapSaveBuf));
			warningBuf.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("map_load")) {
		ImGui::Text("Select map to load:");
		static int selected = -1;

		std::vector<std::string> files;
		std::string mapsDir = FileSystem::GetAssetsDir().string() + "/saves/maps/";

		if (!std::filesystem::exists(mapsDir)) {
			std::filesystem::create_directories(mapsDir);
		}

		for (auto& entry : std::filesystem::directory_iterator(mapsDir)) {
			if (entry.path().extension() == ".json")
				files.push_back(entry.path().filename().string());
		}

		if (files.empty()) {
			ImGui::Text("No map files found!");
		}
		else {
			for (int i = 0; i < (int)files.size(); i++) {
				bool isSelected = (selected == i);

				if (ImGui::Selectable(files[i].c_str(), isSelected)) {
					selected = i;
					std::string full = mapsDir + files[selected];
					tilemap_.loadFromFile(full);
					currentFile_ = full;
					selected = -1;
					selectedLayer_ = 0;
				}
			}
		}

		// Cancel load
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
			selected = -1;
		}

		ImGui::EndPopup();
	}

	ImGui::Separator();

	// --- Brush selector ---
	ImGui::RadioButton("Walkable", &brush_, 1); ImGui::SameLine();
	ImGui::RadioButton("Obstacle", &brush_, 0);

	ImGui::Separator();

	// Instructions
	ImGui::Text("Load and Select Tile from Tileset");
	ImGui::Text("Left-click: Paint");

	ImGui::End();
}

void MapEditor::displayTilesetMenu(Resources::EnoLoader& loader) {
	if (!editMode_) return;

	ImGui::Begin("Tileset", nullptr, ImGuiWindowFlags_NoCollapse);

	// Drop target for texture from Assets Browser
	ImGui::Button("Drop Texture Here", ImVec2(200, 40));

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME")) {
			tilesetName_ = (const char*)payload->Data;

			// Get texture from resource loader
			tilesetTexture_ = loader.getTexture(tilesetName_.c_str());

			if (tilesetTexture_) {
				int texW = tilesetTexture_->getWidth();
				int texH = tilesetTexture_->getHeight();

				// auto-generate rows/cols using tilemap cell size
				float cs = tilemap_.cellSize();
				tilesetCols_ = static_cast<int>(texW / cs);
				tilesetRows_ = static_cast<int>(texH / cs);

				tilesetReady_ = true;
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (!tilesetReady_) {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "No tileset loaded.");
		ImGui::End();
		return;
	}

	ImGui::Separator();

	// ================================
	// AUTO-SCALED TILESET PREVIEW
	// ================================

	float windowWidth = ImGui::GetContentRegionAvail().x;
	float maxPreviewSize = std::min(windowWidth, 512.0f);    // safety cap
	float aspect = (float)tilesetTexture_->getWidth() / (float)tilesetTexture_->getHeight();
	float previewW = maxPreviewSize;
	float previewH = maxPreviewSize / aspect;

	ImTextureID texID = (ImTextureID)(intptr_t)tilesetTexture_->getID();
	ImGui::Image(texID, ImVec2(previewW, previewH));   // draw scaled image

	ImVec2 imgMin = ImGui::GetItemRectMin();
	ImVec2 imgMax = ImGui::GetItemRectMax();

	// Tile size on screen
	float tileW = previewW / tilesetCols_;
	float tileH = previewH / tilesetRows_;

	// ================================
	// CLICK TO PICK TILE
	// ================================
	ImVec2 mouse = ImGui::GetMousePos();

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
		int tx = static_cast<int>((mouse.x - imgMin.x) / tileW);
		int ty = static_cast<int>((mouse.y - imgMin.y) / tileH);

		if (tx >= 0 && tx < tilesetCols_ && ty >= 0 && ty < tilesetRows_) {
			selectedTileX_ = tx;
			selectedTileY_ = ty;
		}
	}

	// ================================
	// DRAW TILE GRID LINES
	// ================================
	ImDrawList* dl = ImGui::GetWindowDrawList();

	for (int x = 0; x <= tilesetCols_; ++x) {
		float px = imgMin.x + x * tileW;
		dl->AddLine(ImVec2(px, imgMin.y), ImVec2(px, imgMax.y), IM_COL32(255, 255, 255, 80));
	}

	for (int y = 0; y <= tilesetRows_; ++y) {
		float py = imgMin.y + y * tileH;
		dl->AddLine(ImVec2(imgMin.x, py), ImVec2(imgMax.x, py), IM_COL32(255, 255, 255, 80));
	}

	// ================================
	// HIGHLIGHT SELECTED TILE
	// ================================
	if (selectedTileX_ >= 0) {
		ImVec2 p1(
			imgMin.x + selectedTileX_ * tileW,
			imgMin.y + selectedTileY_ * tileH
		);
		ImVec2 p2(
			imgMin.x + (selectedTileX_ + 1) * tileW,
			imgMin.y + (selectedTileY_ + 1) * tileH
		);

		dl->AddRect(p1, p2, IM_COL32(255, 255, 0, 255), 0, 0, 3.0f);       // yellow border
		dl->AddRect(p1, p2, IM_COL32(0, 0, 0, 120), 0, 0, 1.5f);           // subtle outline
	}

	ImGui::Separator();
	ImGui::Text("Selected Tile: (%d, %d)", selectedTileX_, selectedTileY_);

	ImGui::End();
}

void MapEditor::paint(EditorApplication* g) {
	if (!editMode_) return;

	// Safety check: ensure we have layers
	if (tilemap_.layers().empty()) return;
	if (selectedLayer_ >= static_cast<int>(tilemap_.layers().size())) {
		selectedLayer_ = 0;
	}

	Math::eVec2<float> mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
	float cameraWidth = static_cast<float>(g->getEditorWidth());
	float cameraHeight = static_cast<float>(g->getEditorHeight());
	Math::eVec2<float> editorMin = g->getEditorViewMin();
	Math::eVec2<float> editorMax = g->getEditorViewMax();
	float camX = g->getEditorCameraX();
	float camY = g->getEditorCameraY();

	bool left = ImGui::IsItemHovered() && ImGui::IsMouseDown(0);

	if (!tilesetReady_) {
		// default walkability mode
		if (left) {
			auto world = Graphics::screenToWorld(mousePos, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);
			auto tile = tilemap_.worldToTile(world);
			if (tilemap_.inBounds(tile.x, tile.y)) {
				tilemap_.setWalkable(selectedLayer_, tile.x, tile.y, (brush_ == 1));
				g->markEntitiesDirty();
			}
		}
		return;
	}

	// TILESET PAINT MODE ----------------------------------
	if (left && selectedTileX_ >= 0) {
		auto world = Graphics::screenToWorld(mousePos, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);
		auto tile = tilemap_.worldToTile(world);

		if (tilemap_.inBounds(tile.x, tile.y)) {
			float u0 = (float)selectedTileX_ / tilesetCols_;
			float v0 = (float)selectedTileY_ / tilesetRows_;
			float u1 = (float)(selectedTileX_ + 1) / tilesetCols_;
			float v1 = (float)(selectedTileY_ + 1) / tilesetRows_;

			tilemap_.setWalkable(selectedLayer_, tile.x, tile.y, (brush_ == 1));
			tilemap_.setTile(
				selectedLayer_,
				tile.x, tile.y,
				tilesetName_,
				{ u0, v0 },
				{ u1, v1 }
			);
			
			g->markEntitiesDirty();
		}
	}
}	

void MapEditor::draw(EditorApplication* g, ImDrawList* drawList) {
	if (!editMode_) return;

	// Safety check: ensure we have layers
	if (tilemap_.layers().empty()) return;
	if (selectedLayer_ >= static_cast<int>(tilemap_.layers().size())) {
		selectedLayer_ = 0;
	}

	float cameraWidth = static_cast<float>(g->getEditorWidth());
	float cameraHeight = static_cast<float>(g->getEditorHeight());
	Math::eVec2<float> editorMin = g->getEditorViewMin();
	Math::eVec2<float> editorMax = g->getEditorViewMax();
	float camX = g->getEditorCameraX();
	float camY = g->getEditorCameraY();

	drawList->PushClipRect(ImVec2(editorMin.x, editorMin.y), ImVec2(editorMax.x, editorMax.y), true);

	auto& layer = tilemap_.layers()[selectedLayer_];

	// Draw tileset-based tiles
	if (tilesetReady_) {
		float cs = tilemap_.cellSize();
		for (int y = 0; y < tilemap_.height(); ++y) {
			for (int x = 0; x < tilemap_.width(); ++x) {
				if (!layer.hasTileTexture(x, y)) continue;
				auto tile = layer.getTile(x, y);

				Math::eVec2<float> p1 = Graphics::worldToScreen(
					{ x * cs, y * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);

				Math::eVec2<float> p2 = Graphics::worldToScreen(
					{ (x + 1) * cs, (y + 1) * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);

				ImGui::GetWindowDrawList()->AddImage(
					(ImTextureID)(intptr_t)tilesetTexture_->getID(),
					ImVec2(p1.x, p1.y),
					ImVec2(p2.x, p2.y),
					ImVec2(tile.uv0.x, tile.uv0.y),
					ImVec2(tile.uv1.x, tile.uv1.y)
				);
			}
		}
	}

	for (int y = 0; y < tilemap_.height(); ++y) {
		for (int x = 0; x < tilemap_.width(); ++x) {
			float cs = tilemap_.cellSize();
			Math::eVec2<float> pt1 = Graphics::worldToScreen(
				{ x * cs, y * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY
			);
			Math::eVec2<float> pt2 = Graphics::worldToScreen(
				{ (x + 1) * cs, (y + 1) * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY
			);

			ImVec2 p1{ pt1.x, pt1.y }, p2{ pt2.x, pt2.y };

			// ignore if outside image viewport
			if (p2.x < editorMin.x || p1.x > editorMax.x) continue;
			if (p2.y < editorMin.y || p1.y > editorMax.y) continue;

			bool walk = layer.isWalkable(x, y);

			ImU32 col = walk ? IM_COL32(0, 255, 0, 20) : IM_COL32(255, 0, 0, 20);
			drawList->AddRectFilled(p1, p2, col);
			drawList->AddRect(p1, p2, IM_COL32(255, 255, 255, 20));
		}
	}
	drawList->PopClipRect();
}