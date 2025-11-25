#include "map_editor.hpp"
#include "../game_logic.hpp"

void MapEditor::displayMenu() {
	if (!editMode_) return;

	ImGui::Begin("Tilemap Editor", nullptr, ImGuiWindowFlags_NoCollapse);

	ImGui::Text("=== TILEMAP EDITOR ===");
	ImGui::Separator();

	// --- Layer selection ---
	ImGui::Text("Active Layer:");

	auto& tilemapLayers = tilemap_.layers();
	int layerCount = static_cast<int>(tilemapLayers.size());
	std::vector<std::string> layerNames;
	for (auto const& layer : tilemapLayers)
		layerNames.push_back(layer.name());

	if (ImGui::BeginCombo("##TilemapLayerCombo", layerNames[selectedLayer_].c_str())) {
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
	bool isNav = tilemapLayers[selectedLayer_].isNavLayer();
	if (ImGui::Checkbox("Navigation Layer", &isNav)) {
		tilemap_.setNavLayer(selectedLayer_, isNav);
	}

	ImGui::Separator();

	// --- Save / Load ---
	if (ImGui::Button("Save Tilemap")) {
		std::cout << "Saving Tilemap..." << std::endl;
		// tilemap.saveToFile();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Tilemap")) {
		std::cout << "Loading Tilemap..." << std::endl;
		// tilemap.loadFromFile();
	}

	ImGui::Separator();

	// --- Brush selector ---
	ImGui::RadioButton("Walkable", &brush_, 1); ImGui::SameLine();
	ImGui::RadioButton("Obstacle", &brush_, 0);

	ImGui::Separator();

	// Instructions
	ImGui::Text("Left-click: Paint");
	ImGui::Text("Right-click: Erase");

	ImGui::End();
}

// Helper function to convert screen coordinates to world coordinates (accounting for camera)
Math::eVec2<float> screenToWorldCoords(ImVec2 screenPos, float editorWidth, float editorHeight) {
	ImVec2 imageMin = ImGui::GetItemRectMin();
	ImVec2 imageMax = ImGui::GetItemRectMax();

	// Get mouse position relative to the image (0.0 to 1.0)
	float relX = (screenPos.x - imageMin.x) / (imageMax.x - imageMin.x);
	float relY = (screenPos.y - imageMin.y) / (imageMax.y - imageMin.y);

	relX = std::clamp(relX, 0.0f, 1.0f);
	relY = std::clamp(relY, 0.0f, 1.0f);

	// Convert to screen space coordinates (within the editor viewport)
	float screenX = relX * editorWidth;
	float screenY = relY * editorHeight;

	// Add camera offset to convert screen space to world space
	float worldX = screenX + g_game->getEditorCameraX();
	float worldY = screenY + g_game->getEditorCameraY();

	return Math::eVec2(worldX, worldY);
};

// Helper function to convert world coordinates to screen coordinates (for drawing gizmos)
Math::eVec2<float> worldToScreenCoords (ImVec2 worldPos, float editorWidth, float editorHeight) {
	ImVec2 imageMin = ImGui::GetItemRectMin();
	ImVec2 imageMax = ImGui::GetItemRectMax();

	// Subtract camera offset to convert world space to screen space
	float screenX = worldPos.x - g_game->getEditorCameraX();
	float screenY = worldPos.y - g_game->getEditorCameraY();

	// Convert to normalized coordinates (0.0 to 1.0)
	float relX = screenX / editorWidth;
	float relY = screenY / editorHeight;

	// Convert to actual screen position
	float finalX = imageMin.x + relX * (imageMax.x - imageMin.x);
	float finalY = imageMin.y + relY * (imageMax.y - imageMin.y);

	return Math::eVec2(finalX, finalY);
};

void MapEditor::paint(GameApplication* g) {
	if (!editMode_) return;

	float editorWidth = static_cast<float>(g->getEditorWidth());
	float editorHeight = static_cast<float>(g->getEditorHeight());

	// Left Mouse Click
	if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0)) {
		Math::eVec2<float> worldPos = screenToWorldCoords(ImGui::GetMousePos(), editorWidth, editorHeight);
		Math::eVec2<int> tilePos = tilemap_.worldToTile(worldPos);

		if (tilemap_.inBounds(tilePos.x, tilePos.y)) {
			// paint using brush
			bool isWalkable = (brush_ == 1);
			tilemap_.setWalkable(selectedLayer_, tilePos.x, tilePos.y, isWalkable);

			// mark render dirty
			g->markEntitiesDirty();
		}
	}

	// Right Mouse Click
	if (ImGui::IsItemHovered() && ImGui::IsMouseDown(1)) {
		Math::eVec2<float> worldPos = screenToWorldCoords(ImGui::GetMousePos(), editorWidth, editorHeight);
		Math::eVec2<int> tilePos = tilemap_.worldToTile(worldPos);

		if (tilemap_.inBounds(tilePos.x, tilePos.y)) {
			tilemap_.setWalkable(selectedLayer_, tilePos.x, tilePos.y, true);

			g->markEntitiesDirty();
		}
	}
}	

void MapEditor::draw(GameApplication* g, ImDrawList* drawList) {
	if (!editMode_) return;

	float editorWidth = static_cast<float>(g->getEditorWidth());
	float editorHeight = static_cast<float>(g->getEditorHeight());

	auto& layer = tilemap_.layers()[selectedLayer_];
	for (int y = 0; y < tilemap_.height(); ++y) {
		for (int x = 0; x < tilemap_.width(); ++x) {
			float cs = tilemap_.cellSize();
			Math::eVec2<float> pt1 = worldToScreenCoords({ x * cs, y * cs }, editorWidth, editorHeight);
			Math::eVec2<float> pt2 = worldToScreenCoords({ (x + 1) * cs, (y + 1) * cs }, editorWidth, editorHeight);

			bool walk = layer.isWalkable(x, y);

			ImU32 col = walk ? IM_COL32(0, 255, 0, 80) : IM_COL32(255, 0, 0, 80);
			ImVec2 p1{ pt1.x, pt1.y }, p2{ pt2.x, pt2.y };
			drawList->AddRectFilled(p1, p2, col);
			drawList->AddRect(p1, p2, IM_COL32(255, 255, 255, 20));
		}
	}
}