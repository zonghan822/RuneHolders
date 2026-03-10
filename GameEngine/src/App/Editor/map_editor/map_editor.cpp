#include "map_editor.hpp"
#include "../editor_logic.hpp"

void MapEditor::displayMenu() {
	if (!hasTilemap_ || !editMode_) return;

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

		int zIdx = tilemap_.layers()[selectedLayer_].zIndex();
		ImGui::Text("Z Index");
		ImGui::SameLine();
		if (ImGui::InputInt("##LayerZIndex", &zIdx)) {
			if (zIdx < 0) zIdx = 0;
			tilemap_.setLayerZIndex(selectedLayer_, zIdx);
		}

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

		static bool newIsObs = false;
		ImGui::Checkbox("Obstacle Layer", &newIsObs);

		if (ImGui::Button("Add New Layer")) {

			if (!newLayerBuf || newLayerBuf[0] == '\0') {
				warningBuf = "Empty name not allowed!";
			}
			else {
				tilemap_.addLayer(std::string(newLayerBuf), newIsObs);
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
		clearAllTileColliders();
		tilemap_.resize(newW, newH, newCellSize);
		rebuildAllTileColliders();
	}

	ImGui::Separator();

	// --- Set Navigation Checkbox ---
	if (layerCount > 0 && selectedLayer_ < layerCount) {
		bool isObstacle = tilemapLayers[selectedLayer_].isObstacleLayer();
		if (ImGui::Checkbox("Obstacle Layer", &isObstacle)) {
			tilemap_.setObstacleLayer(selectedLayer_, isObstacle);
			if (isObstacle)
				rebuildAllTileColliders();
			else {
				clearAllTileColliders();
				rebuildAllTileColliders();
			}
		}
	}
	else {
		ImGui::TextDisabled("Obstacle Layer (no layer selected)");
	}

	ImGui::Separator();

	// Instructions
	ImGui::Text("Load and Select Tile from Tileset");
	ImGui::Text("Left-click: Paint  |  Right-click: Erase");

	ImGui::Separator();

	// --- Tile rotation ---
	ImGui::Text("Tile Rotation: %d deg", tileRotation_);
	if (ImGui::Button("Rotate 90 CW")) {
		tileRotation_ = (tileRotation_ + 90) % 360;
	}
	ImGui::SameLine();
	if (ImGui::Button("Rotate 90 CCW")) {
		tileRotation_ = (tileRotation_ + 270) % 360;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset Rotation")) {
		tileRotation_ = 0;
	}

	ImGui::End();
}

void MapEditor::tryRestoreTileset(Resources::EnoLoader& loader) {
	if (tilesetReady_) return;

	// Scan all layers for any tile with a saved texture name
	for (const auto& layer : tilemap_.layers()) {
		for (const auto& row : layer.tiles()) {
			for (const auto& tile : row) {
				if (!tile.hasTexture || tile.textureName.empty()) continue;

				auto tex = loader.getTexture(tile.textureName.c_str());
				if (!tex) continue;

				tilesetName_    = tile.textureName;
				tilesetTexture_ = tex;
				float cs        = tilemap_.cellSize();
				tilesetCols_    = static_cast<int>(tex->getWidth()  / cs);
				tilesetRows_    = static_cast<int>(tex->getHeight() / cs);
				tilesetReady_   = true;
				return;
			}
		}
	}
}

void MapEditor::displayTilesetMenu(Resources::EnoLoader& loader) {
	if (!hasTilemap_ || !editMode_) return;

	tryRestoreTileset(loader);

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

	// ================================
	// ROTATED TILE PREVIEW
	// ================================
	if (selectedTileX_ >= 0) {
		ImGui::Separator();
		ImGui::Text("Preview (rotation: %d deg)", tileRotation_);

		const float previewSize = 96.0f;
		ImVec2 cursor = ImGui::GetCursorScreenPos();

		// Reserve space
		ImGui::Dummy(ImVec2(previewSize, previewSize));

		// UV corners for the selected tile
		float u0 = (float)selectedTileX_       / tilesetCols_;
		float v0 = (float)selectedTileY_       / tilesetRows_;
		float u1 = (float)(selectedTileX_ + 1) / tilesetCols_;
		float v1 = (float)(selectedTileY_ + 1) / tilesetRows_;

		// Screen corners (TL, TR, BR, BL)
		ImVec2 sTL(cursor.x,               cursor.y);
		ImVec2 sTR(cursor.x + previewSize,  cursor.y);
		ImVec2 sBR(cursor.x + previewSize,  cursor.y + previewSize);
		ImVec2 sBL(cursor.x,               cursor.y + previewSize);

		// Remap UV corners by rotation
		ImVec2 uvTL, uvTR, uvBR, uvBL;
		switch (tileRotation_) {
		default:
		case 0:
			uvTL = { u0, v0 }; uvTR = { u1, v0 };
			uvBR = { u1, v1 }; uvBL = { u0, v1 };
			break;
		case 90:
			uvTL = { u0, v1 }; uvTR = { u0, v0 };
			uvBR = { u1, v0 }; uvBL = { u1, v1 };
			break;
		case 180:
			uvTL = { u1, v1 }; uvTR = { u0, v1 };
			uvBR = { u0, v0 }; uvBL = { u1, v0 };
			break;
		case 270:
			uvTL = { u1, v0 }; uvTR = { u1, v1 };
			uvBR = { u0, v1 }; uvBL = { u0, v0 };
			break;
		}

		ImDrawList* pdl = ImGui::GetWindowDrawList();
		pdl->AddImageQuad(
			(ImTextureID)(intptr_t)tilesetTexture_->getID(),
			sTL, sTR, sBR, sBL,
			uvTL, uvTR, uvBR, uvBL
		);

		// Border around preview
		pdl->AddRect(sTL, sBR, IM_COL32(255, 255, 255, 120));
	}

	ImGui::End();
}

void MapEditor::paint(EditorApplication* g) {
	if (!hasTilemap_ || !editMode_) return;

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

	bool left  = ImGui::IsItemHovered() && ImGui::IsMouseDown(0);
	bool right = ImGui::IsItemHovered() && ImGui::IsMouseDown(1);

	if (!tilesetReady_) return;

	// TILESET PAINT MODE ----------------------------------
	if (left && selectedTileX_ >= 0) {
		auto world = Graphics::screenToWorld(mousePos, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);
		auto tile = tilemap_.worldToTile(world);

		if (tilemap_.inBounds(tile.x, tile.y)) {
			float u0 = (float)selectedTileX_ / tilesetCols_;
			float v0 = (float)selectedTileY_ / tilesetRows_;
			float u1 = (float)(selectedTileX_ + 1) / tilesetCols_;
			float v1 = (float)(selectedTileY_ + 1) / tilesetRows_;

			syncTileCollider(selectedLayer_, tile.x, tile.y);
			tilemap_.setTile(
				selectedLayer_,
				tile.x, tile.y,
				tilesetName_,
				{ u0, v0 },
				{ u1, v1 },
				tileRotation_
			);

			g->markEntitiesDirty();
		}
	}

	// ERASE MODE — right-click clears tile texture
	if (right) {
		auto world = Graphics::screenToWorld(mousePos, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);
		auto tile = tilemap_.worldToTile(world);

		if (tilemap_.inBounds(tile.x, tile.y)) {
			tilemap_.clearTile(selectedLayer_, tile.x, tile.y);
			syncTileCollider(selectedLayer_, tile.x, tile.y);
			g->markEntitiesDirty();
		}
	}
}	

void MapEditor::syncTileCollider(int layer, int x, int y)
{
	float cs = tilemap_.cellSize();
	bool isObstacle = tilemap_.layers()[layer].isObstacleLayer() && tilemap_.layers()[layer].hasTileTexture(x, y);
	uint64_t key = tileKey(layer, x, y);

	auto it = tileColliders_.find(key);

	if (isObstacle)
	{
		if (it == tileColliders_.end())
		{
			float cx = (x + 0.5f) * cs;
			float cy = tilemap_.worldOriginY() + (y + 0.5f) * cs;
			ECS::Entity e = entMng_.create<
				ECSComponents::TransformComponent,
				ECSComponents::CollisionComponent>(
					ECSComponents::TransformComponent{
						Math::eVec2<float>{ cx, cy },
						Math::eVec2<float>{ 0.f, 0.f },
						Math::eVec2<float>{ 1.f, 1.f }
					},
					ECSComponents::CollisionComponent{ cs, cs }
				);
			tileColliders_[key] = e.id();
		}
	}
	else
	{
		if (it != tileColliders_.end())
		{
			ECS::Entity(it->second).destroy(entMng_);
			tileColliders_.erase(it);
		}
	}
}

void MapEditor::clearAllTileColliders()
{
	for (auto& [key, entityID] : tileColliders_)
	{
		ECS::Entity(entityID).destroy(entMng_);
	}
	tileColliders_.clear();
}

void MapEditor::rebuildAllTileColliders()
{
	float cs = tilemap_.cellSize();
	int layerIdx = 0;
	for (auto& layer : tilemap_.layers())
	{
		for (int y = 0; y < tilemap_.height(); ++y)
		{
			for (int x = 0; x < tilemap_.width(); ++x)
			{
				if (layer.isObstacleLayer() && layer.hasTileTexture(x, y))
				{
					uint64_t key = tileKey(layerIdx, x, y);
					float cx = (x + 0.5f) * cs;
					float cy = tilemap_.worldOriginY() + (y + 0.5f) * cs;
					ECS::Entity e = entMng_.create<
						ECSComponents::TransformComponent,
						ECSComponents::CollisionComponent>(
							ECSComponents::TransformComponent{
								Math::eVec2<float>{ cx, cy },
								Math::eVec2<float>{ 0.f, 0.f },
								Math::eVec2<float>{ 1.f, 1.f }
							},
							ECSComponents::CollisionComponent{ cs, cs }
						);
					tileColliders_[key] = e.id();
				}
			}
		}
		++layerIdx;
	}
}

void MapEditor::draw(EditorApplication* g, ImDrawList* drawList) {
	if (!hasTilemap_ || !editMode_) return;

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

	// Cover the framebuffer (which shows all layers) so only the selected layer is visible
	drawList->AddRectFilled(
		ImVec2(editorMin.x, editorMin.y),
		ImVec2(editorMax.x, editorMax.y),
		IM_COL32(0, 0, 0, 255)
	);

	auto& layer = tilemap_.layers()[selectedLayer_];

	// Draw tileset-based tiles
	if (tilesetReady_) {
		float cs = tilemap_.cellSize();
		float originY = tilemap_.worldOriginY();
		for (int y = 0; y < tilemap_.height(); ++y) {
			for (int x = 0; x < tilemap_.width(); ++x) {
				if (!layer.hasTileTexture(x, y)) continue;
				const auto& tile = layer.getTile(x, y);

				Math::eVec2<float> sp1 = Graphics::worldToScreen(
					{ x * cs, originY + y * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);
				Math::eVec2<float> sp2 = Graphics::worldToScreen(
					{ (x + 1) * cs, originY + (y + 1) * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY);

				// Screen corners: TL, TR, BR, BL
				ImVec2 sTL(sp1.x, sp1.y);
				ImVec2 sTR(sp2.x, sp1.y);
				ImVec2 sBR(sp2.x, sp2.y);
				ImVec2 sBL(sp1.x, sp2.y);

				// UV corners remapped by rotation (clockwise)
				float u0 = tile.uv0.x, v0 = tile.uv0.y;
				float u1 = tile.uv1.x, v1 = tile.uv1.y;

				ImVec2 uvTL, uvTR, uvBR, uvBL;
				switch (tile.rotation) {
				default:
				case 0:
					uvTL = { u0, v0 }; uvTR = { u1, v0 };
					uvBR = { u1, v1 }; uvBL = { u0, v1 };
					break;
				case 90:
					uvTL = { u0, v1 }; uvTR = { u0, v0 };
					uvBR = { u1, v0 }; uvBL = { u1, v1 };
					break;
				case 180:
					uvTL = { u1, v1 }; uvTR = { u0, v1 };
					uvBR = { u0, v0 }; uvBL = { u1, v0 };
					break;
				case 270:
					uvTL = { u1, v0 }; uvTR = { u1, v1 };
					uvBR = { u0, v1 }; uvBL = { u0, v0 };
					break;
				}

				ImGui::GetWindowDrawList()->AddImageQuad(
					(ImTextureID)(intptr_t)tilesetTexture_->getID(),
					sTL, sTR, sBR, sBL,
					uvTL, uvTR, uvBR, uvBL
				);
			}
		}
	}

	{
		float cs = tilemap_.cellSize();
		float originY = tilemap_.worldOriginY();
	for (int y = 0; y < tilemap_.height(); ++y) {
		for (int x = 0; x < tilemap_.width(); ++x) {
			Math::eVec2<float> pt1 = Graphics::worldToScreen(
				{ x * cs, originY + y * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY
			);
			Math::eVec2<float> pt2 = Graphics::worldToScreen(
				{ (x + 1) * cs, originY + (y + 1) * cs }, editorMin, editorMax, cameraWidth, cameraHeight, camX, camY
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
	}
	drawList->PopClipRect();
}