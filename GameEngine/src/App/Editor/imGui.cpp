/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         imGui.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        This file contains the functions that are required for Dear ImGui displays,
*				which includes the init, update and terminate functions.
*     			There are also functions for spawning and counting entities.
*//*========================================================================================*/
#include <filesystem>
#include <random>
#include "imGui.hpp"
#include "game_logic.hpp"
#include "player/player_config.hpp"
#include "player/player_archetype_config.hpp"
#include "editor/undo/undo_manager.hpp"

#undef min
#undef max

// Forward declaration for Editor namespace
namespace Editor {
	struct Node;
}

using Editor::Node;

#pragma region Game Variables

extern GameApplication game;

bool showColliders_ = true;

GameApplication* g_game = nullptr;

// Player entity ID (to track the player)
uint32_t playerEntityID_ = 0;
bool playerSpawned_ = false;

// Containers for time display
std::unordered_map<std::string, double> systemTimes_; //actual system time
std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)

// Hierarchy tree removed - now passed as parameter


// Option Menu
const char* shapes[] = { "Circle", "Rectangle" };
const char* colors[] = { "Green"  ,     "Red" };
const char* colliderShapes[] = { "None", "Circle", "Rectangle" };

// Values to set for entities
static float           eWidth = 10;
static float           eHeight = 10;
static int             eColor = Color::Green;
static int             eShapeIndex = Shapes::Rect;
static int             selectedCollider = 0;         // for the newly spawned entity
static float           colliderWidth = 10.0f;
static float           colliderHeight = 10.0f;
static float           colliderRadius = 5.0f;

// Player Scale Variables
float pScaleX = 1.0f;
float pScaleY = 1.0f;

// Keeping track of animations
static std::vector<AnimationPreset> presets;
static int selectedPresetIndex = 0; // for ImGui dropdown
static Graphics::TextureData idleTexData;
static Graphics::TextureData walkTexData;
static Graphics::TextureData attackTexData;


// Time
static float shown_fps = 0.0f;
static double next_update = 0.0;         // next time we refresh the text

// Entity counter
size_t entity_count{};
static uint32_t selectedEntityID = UINT32_MAX;  // Use max value to indicate "no selection" since 0 is valid

// Drag-Drop Gizmos
static bool isDragging = false;
static uint32_t draggedEntityID = 0;
static ImVec2 dragOffset;
static ImVec2 lastDragPos;

// Transform Gizmos
enum GizmoMode { GIZMO_TRANSLATE, GIZMO_ROTATE, GIZMO_SCALE };
static GizmoMode currentGizmoMode = GIZMO_TRANSLATE;
static bool isRotating = false;
static bool isScaling = false;
static int activeTransformAxis = 0; // 0=none, 1=X-axis, 2=Y-axis, 3=uniform scale, 4=X-scale, 5=Y-scale
static float initialAngle = 0.0f;
static float initialRotation = 0.0f;
static ImVec2 initialScale = ImVec2(1.0f, 1.0f);
static ImVec2 lastScale = ImVec2(1.0f, 1.0f);  // Track last frame scale for delta calculation
static ImVec2 scaleStartPos = ImVec2(0.0f, 0.0f);
static ImVec2 dragStartWorldPos = ImVec2(0.0f, 0.0f);

// Undo system - store transform state before gizmo operations
static Math::eVec2<float> transformBeforeDrag;
static Math::eVec2<float> rotationBeforeDrag;
static Math::eVec2<float> scaleBeforeDrag;
static std::vector<uint32_t> affectedEntityIDs;  // Track all entities affected by transformation (parent + children)
static std::unique_ptr<Editor::MultiTransformCommand> pendingTransformCommand;  // Command being built during transformation

// Prefab placement
static std::string g_selectedPrefabForPlacement = "";

// Entity deletion by click
static bool g_waitingForEntityDeletion = false;

// Deferred scene loading to prevent crashes during ImGui rendering
static bool g_pendingNewScene = false;
static bool g_pendingSceneLoad = false;
static std::string g_sceneToLoad = "";
static bool g_refreshSceneList = false;  // Flag to refresh scene list after operations

// Audio filesystems
static std::vector<std::string> audioAssets;

#pragma endregion

/*============================================================================================
* \\brief   Helper function for undo/redo system to mark entities dirty
*
*           This function is called from entity_commands.hpp to update the render cache
*           after entity creation, deletion, or restoration operations.
*//*========================================================================================*/
void markGameEntitiesDirty()
{
	if (g_game)
	{
		g_game->markEntitiesDirty();
	}
}

float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

double percentage(double target, double max) {
	if (max == 0.0) return 0.0;
	return (target / max) * 100.0;
}

void imGuiInit(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.FontGlobalScale = 1.5f;
	//ImGui::GetStyle().ScaleAllSizes(1.5f);
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingNoSplit = false;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void imGuiDisplay(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager) {
	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// get engine systems
	auto& entityManager = engine_.getEntityManager();

	// BEGIN DOCKSPACE
	imGuiBeginDockspace();

	// Apply default layout once after dockspace is created
	static bool layoutApplied = false;
	if (!layoutApplied) {
		imGuiSetupDefaultLayout();
		layoutApplied = true;
	}

	// Load player config once
	static Player::PlayerConfig playerConfig;
	static bool configLoaded = false;
	if (!configLoaded) {
		configLoaded = playerConfig.LoadFromFile(APP_SRC_PATH "player/player_config.txt");
		if (configLoaded) {
			std::cout << "[ImGui] Player config loaded successfully" << std::endl;
		}
		else {
			std::cout << "[ImGui] Using defauly player config values" << std::endl;
		}
	}

	// Get ImGui IO for FPS display
	ImGuiIO& io = ImGui::GetIO();

	// Assuming that there will only be 1 entity with player component
	bool playerFound = false;
	for (ECS::Entity player : entityManager.findEntity<ECSComponents::PlayerComponent>())
	{
		playerEntityID_ = player.id();
		playerFound = true;
	}
	// Sync playerSpawned_ flag with actual player existence in ECS
	playerSpawned_ = playerFound;

	// FPS display throttling
	double now = ImGui::GetTime();
	if (now >= next_update) {
		shown_fps = io.Framerate;
		throttledTimes_ = systemTimes_;
		next_update = now + 0.5;
	}

	// Hierarchy
	ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);
	displayHierarchyTab(engine_, sceneTree, undoManager);
	ImGui::End();

	// Scene select
	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse);
	displaySceneTab(engine_.getEnoEditor());
	ImGui::End();

	// Audio select
	ImGui::Begin("Audio Browser", nullptr, ImGuiWindowFlags_NoCollapse);
	displayAudioTab(engine_);
	ImGui::End();


	// Edit tab
	ImGui::Begin("Edit", nullptr, ImGuiWindowFlags_NoCollapse);
	displayEditTab(engine_, sceneTree);
	displayAllEntitiesTab(engine_, sceneTree, undoManager);
	ImGui::End();

	// Prefabs
	ImGui::Begin("Prefabs", nullptr, ImGuiWindowFlags_NoCollapse);
	displayPrefabsTab(engine_, sceneTree, undoManager);
	ImGui::End();

	// Game View (full scene, bottom-right)
	//ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	//displayGameView();
	//ImGui::End();

	// Editor View (zoomed in, center - replaces old Game View behavior)
	ImGui::Begin("Editor View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayEditorView(engine_, sceneTree, undoManager);
	ImGui::End();

	// Assets Browser
	ImGui::Begin("Assets Browser", nullptr, ImGuiWindowFlags_NoCollapse);
	displayAssetTab(engine_);
	ImGui::End();

	// FPS
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoCollapse);
	displayFPSTab(engine_);
	ImGui::End();


	// END DOCKSPACE
	imGuiEndDockspace();

	// Process deferred scene operations AFTER all ImGui rendering is complete
	// This prevents crashes from destroying entities while ImGui is iterating over them
	if (g_pendingNewScene)
	{
		clearSelectedEntity();
		engine_.getEnoEditor().createEmptyScene(ASSET_SCENE_PATH);
		g_game->markEntitiesDirty();  // Force entity cache refresh
		g_pendingNewScene = false;
		std::cout << "[ImGui] Deferred: Created new scene" << std::endl;
	}
	else if (g_pendingSceneLoad)
	{
		clearSelectedEntity();
		engine_.getEnoEditor().selectScene(g_sceneToLoad);
		g_game->markEntitiesDirty();  // Force entity cache refresh
		g_pendingSceneLoad = false;
		g_sceneToLoad = "";
		std::cout << "[ImGui] Deferred: Loaded scene " << g_sceneToLoad << std::endl;
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void displayFPSTab(Engine::EnoEngine& engine_) {
	ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
	ImGui::Text("Frame (%.1f FPS)", shown_fps);
	ImGui::Text("=== PER SYSTEM TIMING IN MS ===");
	for (auto& time : throttledTimes_) {

		if (time.first != "totalTimePerFrame") {
			ImGui::Text("%s: %.3f ms", time.first.c_str(), time.second / 1000.0);// divide by 1000 to show microseconds as milliseconds
		}

	}
	ImGui::Text("=== PER SYSTEM TIMING IN PERCENTAGE ===");
	for (auto& time : throttledTimes_)
	{
		if (time.first != "totalTimePerFrame") {
			ImGui::Text("%s: %.0f%%", time.first.c_str(), percentage(time.second, throttledTimes_["totalTimePerFrame"]));
		}
	}

	if (ImGui::Button("Show Entity Count")) {
		displayEntityCount(engine_);
	}

	if (ImGui::Button("Show Entity Characteristics")) {
		displayEntityCharacteristics(engine_);
	}

}

void displayEntityCount(Engine::EnoEngine& engine_) {
	auto& entityManager = engine_.getEntityManager();
	size_t counter = 0;

	std::vector<ECS::Entity> EntityList = entityManager.findEntity<>();
	for (ECS::Entity const& e : EntityList) {
		counter++;
		(void)e; // silence unused warning
	}
	std::cout << "Number of entities: " << counter << "\n";
}


void displayEntityCharacteristics(Engine::EnoEngine& engine_) {
	engine_.getEntityManager().viewMap();
}

void displayEditTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree) {

	auto& entityManager = engine_.getEntityManager();
	const char* filters[] = { "Archetype" , "Last Spawned", "Mouse Select", "All" };
	static int FilterType = Filters::Architype;

	std::vector<ECS::Entity> toDelete;

	ImGui::Combo("Filters", &FilterType, filters, IM_ARRAYSIZE(filters));


	if (FilterType == Filters::Architype) {
		if (ImGui::Button("DEBUG: List Named Entities")) {
			for (ECS::Entity& e : engine_.getEntityManager().findEntity<ECSComponents::NameComponent>()) {
				auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
				std::cout << "Entity " << e.id() << " name: '" << nameComp.name << "'\n";
			}
		}


		static char searchInput[32] = ""; // to enter archetype name
		ImGui::InputTextWithHint("##Search", "Enter Name to delete...", searchInput, sizeof(searchInput));


		if (ImGui::Button("Delete Entities")) {
			std::string target = searchInput;
			bool deletedSelected = false;

			// Iterate only once through all entities that have NameComponent
			for (ECS::Entity& e : engine_.getEntityManager().findEntity<ECSComponents::NameComponent>()) {
				ECSComponents::NameComponent& nameComp = e.get<ECSComponents::NameComponent>(entityManager);

				if (!deletedSelected && e.id() == selectedEntityID) {
					if (playerEntityID_ == e.id()) playerSpawned_ = false;

					// Remove from scene hierarchy and destroy children
					Node* node = sceneTree.getNodeByEntityID(e.id());
					if (node) {
						destroyChildrenRecursive(node, entityManager);
						sceneTree.kill(node);
					}
					entityManager.destroyEntity(e.id());

					selectedEntityID = UINT32_MAX;
					deletedSelected = true;
					std::cout << "Deleted selected entity and its children\n";
					continue;
				}

				if (nameComp.name == target) {
					if (playerEntityID_ == e.id()) playerSpawned_ = false;
					toDelete.push_back(e);
				}

			}

			if (g_game)
				g_game->markEntitiesDirty();
		}
	}
	else if (FilterType == Filters::Last) {
		if (ImGui::Button("Delete Last Spawned")) {
			if (!ECS::spawnHistory.empty()) {
				uint32_t lastID = ECS::spawnHistory.back();
				ECS::spawnHistory.pop_back();  // remove from history

				bool found = false;
				for (auto& e : entityManager.findEntity<>()) {
					if (e.id() == lastID) {
						if (playerEntityID_ == lastID) playerSpawned_ = false;
						toDelete.push_back(e);
						found = true;
						if (g_game) g_game->markEntitiesDirty();
						std::cout << "Deleted last spawned entity (ID: " << lastID << ")\n";
						break;
					}
				}
				if (!found)
					std::cout << "Entity ID " << lastID << " not found in registry.\n";
			}
			else {
				std::cout << "No spawn history left to delete.\n";
			}
		}
	}
	else if (FilterType == Filters::Select) {
		if (!g_waitingForEntityDeletion) {
			if (ImGui::Button("Delete an Entity by Click")) {
				g_waitingForEntityDeletion = true;
			}
		}
		else {
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Click in Game View to delete an entity");
			// Cancel option
			if (ImGui::Button("Cancel")) {
				g_waitingForEntityDeletion = false;
			}
		}
	}
	else if (FilterType == Filters::All) {
		if (ImGui::Button("Delete ALL Entities")) {
			for (auto& e : entityManager.findEntity<>()) {
				toDelete.push_back(e);
			}
			playerSpawned_ = false;
		}

	}

	if (!toDelete.empty()) {
		for (auto& a : toDelete) {
			// Remove from scene hierarchy and destroy children
			Node* node = sceneTree.getNodeByEntityID(a.id());
			if (node) {
				destroyChildrenRecursive(node, entityManager);
				sceneTree.kill(node);
			}
			entityManager.destroyEntity(a.id());
		}

		// Mark cache as dirty after deletion
		if (g_game) {
			g_game->markEntitiesDirty();
		}
	}

}

std::optional<ECS::Entity> getEntityUnderMouse(Engine::EnoEngine& engine_, Math::eVec2<float> mouse)
{
	auto& entityManager = engine_.getEntityManager();

	// Collect all entities that contain the mouse position
	std::vector<ECS::Entity> matchingEntities;

	for (ECS::Entity& e : engine_.getEntityManager().findEntity<
		ECSComponents::TransformComponent,
		ECSComponents::DrawInfoComponent>())
	{
		auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);
		auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);

		// TransformComponent.pos is the CENTER of the entity
		float halfW = dc.width * 0.5f;
		float halfH = dc.height * 0.5f;
		float left = tc.pos.x - halfW;
		float right = tc.pos.x + halfW;
		float top = tc.pos.y - halfH;
		float bottom = tc.pos.y + halfH;

		if (mouse.x >= left && mouse.x <= right &&
			mouse.y >= top && mouse.y <= bottom)
		{
			matchingEntities.push_back(e);
		}
	}

	// Return the LAST matching entity (most recently created, drawn on top)
	if (!matchingEntities.empty()) {
		return matchingEntities.back();
	}

	return std::nullopt;
}

void moveChildrenRecursive(Editor::Node* node, float deltaX, float deltaY, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Move all children of this node
	for (Editor::Node* child : node->children_)
	{
		// Get child's transform and move it
		if (child->entity_.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& tc = child->entity_.get<ECSComponents::TransformComponent>(entityManager);
			tc.pos.x += deltaX;
			tc.pos.y += deltaY;
		}

		// Recursively move this child's children
		moveChildrenRecursive(child, deltaX, deltaY, entityManager);
	}
}

void rotateChildrenRecursive(Editor::Node* node, float deltaRotation, Math::eVec2<float> parentPos, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Rotate all children of this node around the parent's position
	for (Editor::Node* child : node->children_)
	{
		if (child->entity_.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& tc = child->entity_.get<ECSComponents::TransformComponent>(entityManager);

			// 1. Rotate the child's own rotation
			tc.rot.x += deltaRotation;

			// 2. Rotate the child's position around the parent position
			// Get relative position from parent
			float relX = tc.pos.x - parentPos.x;
			float relY = tc.pos.y - parentPos.y;

			// Rotate the relative position
			float cosRot = cosf(deltaRotation);
			float sinRot = sinf(deltaRotation);
			float newRelX = relX * cosRot - relY * sinRot;
			float newRelY = relX * sinRot + relY * cosRot;

			// Update child position
			tc.pos.x = parentPos.x + newRelX;
			tc.pos.y = parentPos.y + newRelY;
		}

		// Recursively rotate this child's children
		rotateChildrenRecursive(child, deltaRotation, parentPos, entityManager);
	}
}

void scaleChildrenRecursive(Editor::Node* node, float scaleFactorX, float scaleFactorY, Math::eVec2<float> parentPos, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Scale all children of this node from the parent's position
	for (Editor::Node* child : node->children_)
	{
		if (child->entity_.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& tc = child->entity_.get<ECSComponents::TransformComponent>(entityManager);

			// 1. Scale the child's own scale
			tc.scale.x *= scaleFactorX;
			tc.scale.y *= scaleFactorY;

			// 2. Scale the child's position relative to parent
			// Get relative position from parent
			float relX = tc.pos.x - parentPos.x;
			float relY = tc.pos.y - parentPos.y;

			// Scale the relative position
			relX *= scaleFactorX;
			relY *= scaleFactorY;

			// Update child position
			tc.pos.x = parentPos.x + relX;
			tc.pos.y = parentPos.y + relY;
		}

		// Recursively scale this child's children
		scaleChildrenRecursive(child, scaleFactorX, scaleFactorY, parentPos, entityManager);
	}
}

void destroyChildrenRecursive(Editor::Node* node, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Recursively destroy all children first (depth-first)
	for (Editor::Node* child : node->children_)
	{
		destroyChildrenRecursive(child, entityManager);

		// Destroy the child entity
		entityManager.destroyEntity(child->entity_.id());
		std::cout << "[DEBUG] Destroyed child entity " << child->entity_.id() << "\n";
	}
}

// Helper function to collect entity IDs for a node and all its children
void collectEntityIDsRecursive(Editor::Node* node, std::vector<uint32_t>& entityIDs)
{
	if (!node) return;

	// Add this entity's ID
	entityIDs.push_back(node->entity_.id());

	// Recursively add all children
	for (Editor::Node* child : node->children_)
	{
		collectEntityIDsRecursive(child, entityIDs);
	}
}

uint32_t getSelectedEntityID()
{
	return selectedEntityID;
}

void clearSelectedEntity()
{
	selectedEntityID = UINT32_MAX;
}

// Helper to collect all entity IDs in a node hierarchy
void collectEntityIDs(Editor::Node* node, std::vector<uint32_t>& entityIDs)
{
	if (!node) return;

	// Add this node's entity ID
	entityIDs.push_back(node->entity_.id());

	// Recursively collect children's entity IDs
	for (Editor::Node* child : node->children_)
	{
		collectEntityIDs(child, entityIDs);
	}
}

void deleteSelectedEntity(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager, Editor::UndoManager& undoManager)
{
	if (selectedEntityID == UINT32_MAX) {
		std::cout << "[DEBUG] No entity selected for deletion\n";
		return;
	}

	std::cout << "[DEBUG] Attempting to delete entity with ID: " << selectedEntityID << "\n";

	// Debug: List all nodes in scene tree
	auto allNodes = sceneTree.getAllNodes();
	std::cout << "[DEBUG] Scene tree contains " << allNodes.size() << " nodes:\n";
	for (auto* n : allNodes) {
		std::cout << "  - Node ID: " << n->sceneObjectID_ << ", Entity ID: " << n->entity_.id() << ", Name: " << n->name_ << "\n";
	}

	// Get the node for the selected entity
	Editor::Node* node = sceneTree.getNodeByEntityID(selectedEntityID);

	if (node) {
		std::cout << "[DEBUG] Found node! Deleting node " << selectedEntityID << " (\"" << node->name_ << "\") and all children from hierarchy\n";

		// Get entity to capture snapshot
		ECS::Entity entity = node->entity_;

		// Create delete command (captures snapshot before deletion)
		auto deleteCmd = std::make_unique<Editor::DeleteEntityCommand>(
			sceneTree,
			entityManager,
			entity
		);

		// Execute the deletion (also adds to undo history)
		undoManager.executeCommand(std::move(deleteCmd));

		// Clear selection
		selectedEntityID = UINT32_MAX;

		std::cout << "[DEBUG] Entity deletion completed with undo support\n";
	}
	else {
		std::cout << "[ERROR] Selected entity " << selectedEntityID << " not found in scene tree!\n";
		std::cout << "[ERROR] This entity exists in ECS but has no corresponding node in the scene tree.\n";
	}
}

ECS::Entity spawnPrefabAt(float x, float y, std::string name, Engine::EnoEngine& engine, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager)
{
	auto& entityManager = engine.getEntityManager();

	// Use the new snapshot-based prefab system
	ECS::Entity e = instantiatePrefabFromSnapshot(name, entityManager, sceneTree);

	// Override the position to where the user clicked (treat x,y as CENTER)
	if (e.has<ECSComponents::TransformComponent>(entityManager))
	{
		auto& transform = e.get<ECSComponents::TransformComponent>(entityManager);
		transform.pos.x = x;
		transform.pos.y = y;
	}

	// Add entity to scene hierarchy as root node (must be done before CreateEntityCommand)
	Node* node = sceneTree.createFromEntity(e, nullptr);

	// Update node name from entity's NameComponent if it exists
	if (e.has<ECSComponents::NameComponent>(entityManager))
	{
		auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
		node->name_ = nameComp.name;
	}

	// Record creation for undo/redo with full snapshot
	auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
		sceneTree,
		entityManager,
		e
	);
	undoManager.executeCommand(std::move(createCmd));

	// update render cache
	if (g_game) g_game->markEntitiesDirty();

	std::cout << "Spawned prefab '" << name
		<< "' at (" << x << ", " << y << ")\n";

	return e;
}

void displayPrefabsTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager) {

	auto& entityManager = engine_.getEntityManager();

	static PrefabData currentPrefab{};  // local editable prefab
	static char prefabName[64] = "";
	static bool hasCollider = false;
	static int shapeIndex = 1;  // 1 = Rectangle (default), 0 = Circle
	static int colorIndex = Color::Green;

	// New implementation for dropdown scrollable UI for prefabs
	static std::string statusMessage{};
	static std::string selectedPrefabName{};



	ImGui::Text("==========PREFABS TAB==========");

	ImGui::Text("Prefab name:");
	ImGui::InputText("##Prefab Name", prefabName, sizeof(prefabName));
	currentPrefab.name = prefabName;

	ImGui::Separator();
	ImGui::Text("Transform / Size");
	ImGui::DragFloat("Position X", &currentPrefab.x, 1.0f);
	ImGui::DragFloat("Position Y", &currentPrefab.y, 1.0f);
	ImGui::DragFloat("Width", &currentPrefab.width, 0.5f, 1.0f, 200.0f);
	ImGui::DragFloat("Height", &currentPrefab.height, 0.5f, 1.0f, 200.0f);


	ImGui::Combo("Shape", &shapeIndex, shapes, IM_ARRAYSIZE(shapes));
	currentPrefab.shape = shapeIndex;

	ImGui::Combo("Color", &colorIndex, colors, IM_ARRAYSIZE(colors));
	currentPrefab.color = colorIndex;

	ImGui::Checkbox("Has Collider", &hasCollider);
	currentPrefab.hasCollider = hasCollider;


	// Save a new prefab or edit existing value
	if (ImGui::Button("Save New or Existing Prefabs:")) {
		savePrefab(currentPrefab);
		prefabRegistry[currentPrefab.name] = currentPrefab; //puts the prefab's data into a registry
		prefabRegistry[currentPrefab.name] = currentPrefab; //puts the prefab's data into a registry
	}

	//ImGui::Separator();

	// Load performs a "read all file in directory" now
	if (ImGui::Button("Load Existing Prefab")) {
		try {
			for (auto& entry : std::filesystem::directory_iterator(ASSETS_DIR_PATH"saves/prefabs/")) {
				if (entry.path().extension() == ".txt") {
					try {
						PrefabData loaded = loadPrefab(entry.path().string());
						if (!loaded.name.empty()) {
							prefabRegistry[loaded.name] = loaded;
							std::cout << "[UI] Loaded prefab: " << loaded.name << "\n";
						}
					}
					catch (const std::exception& e) {
						std::cerr << "[UI] Failed to load prefab from " << entry.path() << ": " << e.what() << "\n";
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "[UI] Failed to read prefab directory: " << e.what() << "\n";
		}
	}

	//Drop down and selectable list of loaded prefabs
	ImGui::Text("Available Prefabs:");
	if (ImGui::BeginListBox("##PrefabList", ImVec2(200, 150))) {
		for (auto& [name, data] : prefabRegistry) {
			bool selected = (selectedPrefabName == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selectedPrefabName = name;
				currentPrefab = data;  // load into editor fields
				statusMessage = "Loaded prefab: " + name;
				strncpy(prefabName, name.c_str(), sizeof(prefabName));
				prefabName[sizeof(prefabName) - 1] = '\0';
				g_selectedPrefabForPlacement = name;
			}
		}
		ImGui::EndListBox();
	}

	// Show selected prefab status
	if (!selectedPrefabName.empty()) {
		ImGui::Text("Selected: %s", selectedPrefabName.c_str());
		if (!g_selectedPrefabForPlacement.empty()) {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Click in Game View to place");
			if (ImGui::Button("Cancel Placement")) {
				g_selectedPrefabForPlacement.clear();
			}
		}
	}

	//Spawn desired prefab
	ImGui::Separator();
	if (ImGui::Button("Spawn Prefab")) {
		if (prefabRegistry.count(selectedPrefabName)) {
			ECS::Entity e = instantiatePrefabFromSnapshot(selectedPrefabName, entityManager, sceneTree);

			// Add entity to scene hierarchy as root node
			sceneTree.createFromEntity(e, nullptr);

			// Record creation for undo/redo with full snapshot
			auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
				sceneTree,
				entityManager,
				e
			);
			undoManager.executeCommand(std::move(createCmd));

			if (g_game) g_game->markEntitiesDirty();
			std::cout << "Spawned prefab: " << selectedPrefabName << "\n";

		}
	}

	//std::vector<ECS::Entity> NamedEntityList = entityManager.findEntity<ECSComponents::NameComponent>();

	//ImGui::Text("Total Entities: %d", static_cast<int>(NamedEntityList.size()));
	//ImGui::Separator();

	//// Iterate through each entity
	//for (ECS::Entity& e : NamedEntityList)
	//{
	//	const auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
	//	std::string name = nameComp.name;
	//	bool selected = (selectedEntityID == e.id());
	//	std::string label = std::to_string(e.id()) + ": " + name;
	//	if (ImGui::Selectable(label.c_str(), selected))
	//	{
	//		selectedEntityID = e.id();
	//	}
	//}
}

void displayAllEntitiesTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager)
{
	auto& entityManager = engine_.getEntityManager();

	// Get all entities that have a NameComponent
	std::vector<ECS::Entity> NamedEntityList = entityManager.findEntity<ECSComponents::NameComponent>();

	ImGui::Text("Total Entities: %d", static_cast<int>(NamedEntityList.size()));
	ImGui::Separator();

	// --- Entity selection list ---
	for (ECS::Entity& e : NamedEntityList)
	{
		const auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
		std::string name = nameComp.name;
		bool selected = (selectedEntityID == e.id());

		std::string label = std::to_string(e.id()) + ": " + name;
		if (ImGui::Selectable(label.c_str(), selected))
		{
			selectedEntityID = e.id();
		}
	}

	// --- Display inspector if selected entity exists ---
	// Note: Entity ID 0 is valid, so we check if any entity was selected
	if (selectedEntityID != UINT32_MAX)
	{
		// Find and display the selected entity
		bool entityFound = false;
		for (ECS::Entity& e : NamedEntityList)
		{
			if (e.id() == selectedEntityID)
			{
				// Display inspector for all entities (including root nodes)
				displayEntityInspector(engine_, e, undoManager, sceneTree);
				assetPresetsTab(engine_, e);
				entityFound = true;
				break;
			}
		}

		// If entity was not found, clear selection
		if (!entityFound)
		{
			selectedEntityID = UINT32_MAX;
		}
	}
}

void displayHierarchyTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager)
{
	ImGui::Text("=== Scene Hierarchy ===");
	ImGui::Separator();

	// ==================== SPAWN PLAYER ====================
	ImGui::Text("=== PLAYER CONTROLS ===");
	ImGui::Text("WASD to move player");

	if (!playerSpawned_) {
		if (ImGui::Button("Spawn Player")) {

			// Load player configuration from file
			Player::PlayerConfig playerConfig;
			playerConfig.LoadFromFile(APP_SRC_PATH "player/player_config.txt");

			// Spawn player at center of virtual world (1280x720 / 2 = 640, 360)
			constexpr float centerX = 640.0f;
			constexpr float centerY = 360.0f;

			// Create complete player entity using the config
			ECS::Entity player = Player::CreatePlayerEntityFromConfig(
				engine_.getEntityManager(),
				Math::eVec2<float>{centerX, centerY},
				playerConfig
			);

			// Store player ID for reference
			playerEntityID_ = player.id();
			playerSpawned_ = true;

			// Add player entity to scene tree as root node
			sceneTree.createFromEntity(player, nullptr);

			std::cout << "Player spawned at center (" << centerX << ", " << centerY << ")\n";
		}
	}
	else {
		ImGui::Text("Player Active (ID: %u)", playerEntityID_);
		if (ImGui::Button("Destroy Player")) {
			if (playerSpawned_ == true) {
				// Remove from scene hierarchy and destroy children
				Editor::Node* node = sceneTree.getNodeByEntityID(playerEntityID_);
				if (node) {
					destroyChildrenRecursive(node, engine_.getEntityManager());
					sceneTree.kill(node);
				}
				engine_.getEntityManager().destroyEntity(playerEntityID_);
			}
			playerSpawned_ = false;
			if (g_game) g_game->markEntitiesDirty();
			std::cout << "Player and all children destroyed\n";
		}
	}
	ImGui::Separator();

	// ==================== SPAWN ANIMATED ENTITIES ====================
	ImGui::Text("=== ANIMATED SPAWNER ===");
	if (ImGui::Button("Spawn 1000 Animated Entities")) {
		auto& entityManager = engine_.getEntityManager();
		auto& loader = engine_.getResourceLoader();

		// Load the walking animation spritesheet
		auto walkTexture = loader.getTexture("Protagonist-Walk-Sheet");
		if (!walkTexture) {
			std::cout << "[ERROR] Failed to load Protagonist-Walk-Sheet texture!\n";
		}
		else {
			std::cout << "[INFO] Spawning 1000 animated rotating entities...\n";

			// Random number generation setup
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> posX(0.0f, 1280.0f);  // Screen width
			std::uniform_real_distribution<float> posY(0.0f, 720.0f);   // Screen height
			std::uniform_real_distribution<float> rotSpeed(-3.14f, 3.14f); // Rotation speed (radians/sec)

			for (int i = 0; i < 10; ++i) {
				// Create transform with random position and rotation speed
				// rot.x = current rotation angle, rot.y = rotation speed
				ECSComponents::TransformComponent transform{
					{posX(gen), posY(gen)},      // Random position
					{0.0f, rotSpeed(gen)},       // rot.x = angle (0), rot.y = rotation speed
					{1.0f, 1.0f}                 // Scale
				};

				// Create render component with walk animation texture
				ECSComponents::RenderComponent renderComp;
				renderComp.width = 96.0f;   // Frame size
				renderComp.height = 96.0f;
				strncpy_s(renderComp.shape, "Sprite", sizeof(renderComp.shape) - 1);
				renderComp.setTexture(walkTexture, "Protagonist-Walk-Sheet");
				renderComp.color = Graphics::Color::White;
				renderComp.texID = 0;  // Start at first frame, will animate

				// Create name component
				ECSComponents::NameComponent nameComp;
				std::string entityName = "AnimatedEntity " + std::to_string(i + 1);
				strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
				nameComp.name[sizeof(nameComp.name) - 1] = '\0';

				// Create entity with all components
				ECS::Entity entity = entityManager.create<ECSComponents::TransformComponent>(transform);
				entity.add<ECSComponents::DrawInfoComponent>(entityManager, renderComp);
				entity.add<ECSComponents::NameComponent>(entityManager, nameComp);
				


				// Add to scene tree as root node
				sceneTree.createFromEntity(entity, nullptr);
			}

			if (g_game) g_game->markEntitiesDirty();
			std::cout << "[SUCCESS] Spawned 1000 animated rotating entities!\n";
		}
	}
	ImGui::Separator();

	// ==================== HIERARCHY CONTROLS ====================
	static int entityCounter = 1;  // Track entity numbering
	static bool attachScript = false; // Checkbox state

	static int selectedScriptIndex = -1;	// Script dropdown
	static std::vector<std::wstring> availableScripts; // List of available scripts
	static bool scriptsLoaded = false;

	Script::EnoScriptManager& scriptMng = engine_.getEnoEditor().getScriptManager();

	ImGui::Checkbox("Attach Script to New Entity", &attachScript);

	// Here i prepare the list for showcase of the scripts
	if (attachScript) {
		// Load scripts only once or when needed
		if (!scriptsLoaded) {
			availableScripts = scriptMng.getAvailableScripts();
			scriptsLoaded = true;
		}

		if (!availableScripts.empty()) {
			std::vector<std::string> scriptNames;
			for (const auto& ws : availableScripts)
				scriptNames.push_back(std::string(ws.begin(), ws.end()));

			// Convert vector to array for ImGui Combo
			std::vector<const char*> scriptItems;
			for (auto& name : scriptNames)
				scriptItems.push_back(name.c_str());

			ImGui::Text("Select Script:");
			ImGui::Combo("##ScriptCombo", &selectedScriptIndex, scriptItems.data(), (int)scriptItems.size());
		}
		else {
			ImGui::Text("No scripts found.");
		}
	}



	if (ImGui::Button("Add Root Node")) {
		auto& entityManager = engine_.getEntityManager();

		// Create transform with default position at center of screen
		ECSComponents::TransformComponent transform{
			{600.0f, 450.0f},  // Center position
			{0.0f, 0.0f},      // Velocity
			{1.0f, 1.0f}       // Scale
		};

		// Create render component with default size and color
		ECSComponents::RenderComponent renderComp{
			50.0f,        // Width
			50.0f,        // Height
			"Rectangle",  // Shape
			Color::Green  // Color
		};

		ECS::Entity newEntity = entityManager.create<ECSComponents::TransformComponent>(transform);
		newEntity.add<ECSComponents::DrawInfoComponent>(entityManager, renderComp);
		newEntity.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent());
		newEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, engine_.getAnimationRegistry().createController());
		


		// Add collision component for physics interactions
		ECSComponents::CollisionComponent collisionComp;
		if (strcmp(renderComp.shape, "Circle") == 0) {
			// Auto-calculate circle collider from render dimensions
			collisionComp = ECSComponents::CreateCircleFromRender(renderComp);
		}
		else {
			// Auto-calculate rectangle collider from render dimensions
			collisionComp.isActive = true;
			collisionComp.isTrigger = false;
			collisionComp.type = ECSComponents::ColliderType::RECTANGLE;
			collisionComp.data.rectangle.width = renderComp.width;
			collisionComp.data.rectangle.height = renderComp.height;
		}
		newEntity.add<ECSComponents::CollisionComponent>(entityManager,
			collisionComp);

		// Add physics component to enable pushing
		ECSComponents::PhysicsComponent physicsComp;
		physicsComp.mass = 2.0f;          // Mass (heavier = harder to push)
		physicsComp.drag = 8.0f;          // Higher drag for quick stopping
		physicsComp.isKinematic = false;  // NOT kinematic (can be pushed)
		physicsComp.useGravity = false;   // No gravity for top-down game
		newEntity.add<ECSComponents::PhysicsComponent>(entityManager,
			physicsComp);

		
		
		newEntity.add<ECSComponents::BoxComponent>(entityManager, ECSComponents::BoxComponent());


		// Name root nodes as "Entity 1", "Entity 2", etc.
		auto& nameComp = newEntity.get<ECSComponents::NameComponent>(entityManager);
		std::string entityName = "Entity " + std::to_string(entityCounter++);
		strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
		nameComp.name[sizeof(nameComp.name) - 1] = '\0';

		Node* createdNode = sceneTree.createFromEntity(newEntity, nullptr);

		// Record creation for undo/redo with full snapshot
		auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
			sceneTree,
			entityManager,
			newEntity
		);
		undoManager.executeCommand(std::move(createCmd));

		std::cout << "[DEBUG] Add Root Node: Entity " << entityName << " created\n";
		if (attachScript && selectedScriptIndex >= 0) {
			std::wstring selectedScript = availableScripts[selectedScriptIndex];
			std::wcout << L"[DEBUG] Selected Script: " << selectedScript << std::endl;
			auto script = scriptMng.loadScript(selectedScript); // Replace selectScript() with this new call
			if (script)
				sceneTree.attachScript(createdNode, std::move(script));
			else
				std::cerr << "No script is attached!" << std::endl;
		}
		if (g_game) g_game->markEntitiesDirty();
	}

	ImGui::SameLine();

	if (ImGui::Button("Add Child Node")) {
		if (selectedEntityID != UINT32_MAX) {
			// Verify the parent node exists in the scene tree
			if (!sceneTree.getNodeByEntityID(selectedEntityID)) {
				std::cout << "[ERROR] Selected entity " << selectedEntityID << " not found in scene tree. Cannot add child.\n";
			}
			else {
				auto& entityManager = engine_.getEntityManager();

				// Create transform with default position at center of screen
				ECSComponents::TransformComponent transform{
					{600.0f, 450.0f},  // Center position
					{0.0f, 0.0f},      // Velocity
					{1.0f, 1.0f}       // Scale
				};

				// Create render component with default size and color
				ECSComponents::RenderComponent renderComp{
					50.0f,        // Width
					50.0f,        // Height
					"Rectangle",  // Shape
					Color::Red    // Color (different from root nodes)
				};

				ECS::Entity childEntity = entityManager.create<ECSComponents::TransformComponent>(transform);
				childEntity.add<ECSComponents::DrawInfoComponent>(entityManager, renderComp);
				childEntity.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent());
				childEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, engine_.getAnimationRegistry().createController());
				
				
				// Add collision component for physics interactions
				ECSComponents::CollisionComponent childCollisionComp;
				childCollisionComp.isActive = true;
				childCollisionComp.isTrigger = false;
				childCollisionComp.type = ECSComponents::ColliderType::RECTANGLE;
				childCollisionComp.data.rectangle.width = 50.0f;
				childCollisionComp.data.rectangle.height = 50.0f;
				childEntity.add<ECSComponents::CollisionComponent>(entityManager,
					childCollisionComp);

				// Add physics component to enable pushing
				ECSComponents::PhysicsComponent childPhysicsComp;
				childPhysicsComp.mass = 2.0f;
				childPhysicsComp.drag = 8.0f;
				childPhysicsComp.isKinematic = false;
				childPhysicsComp.useGravity = false;
				childEntity.add<ECSComponents::PhysicsComponent>(entityManager,
					childPhysicsComp);

				childEntity.add<ECSComponents::BoxComponent>(entityManager,
					ECSComponents::BoxComponent());
				// Name child nodes as "Entity #" using the same counter
				auto& nameComp = childEntity.get<ECSComponents::NameComponent>(entityManager);
				std::string entityName = "Entity " + std::to_string(entityCounter++);
				strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
				nameComp.name[sizeof(nameComp.name) - 1] = '\0';

				// Get parent node and add child entity to scene tree
				Editor::Node* parentNode = sceneTree.getNodeByEntityID(selectedEntityID);
				Editor::Node* childNode = sceneTree.createFromEntity(childEntity, parentNode);

				// Record creation for undo/redo
				auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
					sceneTree,
					entityManager,
					childEntity
				);
				undoManager.executeCommand(std::move(createCmd));

				//if (attachScript) {
				//	auto script = scriptMng.selectScript(); // file picker
				//	if (script) {
				//		sceneTree.attachScript(childNode, std::move(script));
				//	}
				//}
				if (attachScript && selectedScriptIndex >= 0) {
					std::wstring selectedScript = availableScripts[selectedScriptIndex];
					std::wcout << L"[DEBUG] Selected Script: " << selectedScript << std::endl;
					auto script = scriptMng.loadScript(selectedScript); // Replace selectScript() with this new call
					if (script)
						sceneTree.attachScript(childNode, std::move(script));
				}

				if (g_game) g_game->markEntitiesDirty();

				std::cout << "Added child node " << childEntity.id() << " ('" << nameComp.name
					<< "') to parent " << selectedEntityID << " with undo support\n";

				// Debug: verify the relationship was set
				if (childNode) {
					std::cout << "  -> Child's parent: " << (childNode->parent_ ? std::to_string(childNode->parent_->sceneObjectID_) : "none") << "\n";
				}
			}
		}
		else {
			std::cout << "No node selected. Please select a node first.\n";
		}
	}




	ImGui::Separator();

	// --- Delete Node Button ---
	if (ImGui::Button("Delete Selected Node")) {
		auto& em = engine_.getEntityManager();
		Editor::Node* node = sceneTree.getNodeByEntityID(selectedEntityID);
		if (node) {
			std::cout << "[DEBUG] Deleting node " << selectedEntityID << " and all children from hierarchy\n";

			// First, destroy all child entities recursively
			destroyChildrenRecursive(node, em);

			// Then kill the scene tree nodes (removes from tree structure)
			sceneTree.kill(node);

			// Finally, destroy the parent entity
			em.destroyEntity(selectedEntityID);
			selectedEntityID = UINT32_MAX;  // Reset to "no selection"

			if (g_game)
				g_game->markEntitiesDirty();
		}
		else {
			std::cout << "[WARN] Selected node " << selectedEntityID << " not found in scene tree\n";
		}
	}


	ImGui::Separator();

	auto& entityManager = engine_.getEntityManager();
	auto allNodes = sceneTree.getAllNodes();
	bool anyNode = false;
	for (Editor::Node* node : allNodes) {
		if (node->parent_ == nullptr) {  // nullptr means no parent (root node)
			drawNodeRecursive(sceneTree, node->entity_.id(), entityManager);
			anyNode = true;
		}
	}

	if (!anyNode)
		ImGui::TextDisabled("(no nodes to display)");

	ImGui::Separator();

	if (ImGui::Button("Print Tree to Terminal")) {
		auto all = sceneTree.getAllNodes();
		for (Editor::Node* node : all) {
			std::cout << "Entity " << node->entity_.id()
				<< " | Parent: " << (node->parent_ ? std::to_string(node->parent_->entity_.id()) : "none")
				<< " | Children: ";
			for (Editor::Node* c : node->children_) std::cout << c->entity_.id() << " ";
			std::cout << "\n";
		}
	}
}

void displaySceneTab(Editor::EnoEditor& editor_)
{
	// Heading
	ImGui::Text("=== Scene Select===");
	ImGui::Text("Current Scene:");

	// Get current scene
	std::string currScene = editor_.getCurrentScene().getSceneName();

	// Create a container for all scene names inside my scene folder
	static std::vector<std::filesystem::path> sceneNames;
	// A toggle flag to refresh the scene list
	static bool refreshSceneName = true;
	// Buffer to change scene name
	static char scene_name[64] = "";

	// Check global flag for scene list refresh
	if (g_refreshSceneList)
	{
		refreshSceneName = true;
		g_refreshSceneList = false;
	}

	if (refreshSceneName)
	{
		// Clear the current container
		sceneNames.clear();
		// A string path to the assets folder
		std::string sceneSavePath = (ASSET_SCENE_PATH);
		// Check that the file is a .txt file
		try {
			for (const auto& fileEntry : std::filesystem::directory_iterator(sceneSavePath))
			{
				if (fileEntry.is_regular_file() && fileEntry.path().extension() == ".txt" && fileEntry.path().string() != (sceneSavePath + "LastSavedScene.txt"))
				{
					// Add file to container
					sceneNames.push_back(fileEntry.path());
					//std::cout << "Filename: " << fileEntry.path() << std::endl;
				}
			}

			// Check that scene list contains at least one scene
			if (!sceneNames.empty())
			{
				// Check that no scene was recently deleted
				auto it = std::find_if(sceneNames.begin(), sceneNames.end(),
					[&](const std::filesystem::path& fileName)
					{
						return fileName.filename() == currScene;
					});

				if (it == sceneNames.end())
				{
					std::cout << "[ImGui] file does not exist: " << currScene << std::endl;
					// Scene deleted, load the first available scene instead
					editor_.selectScene(sceneNames.front().filename().string());
					// Update last saved scene file
					editor_.updateLastSavedScene();
					g_game->markEntitiesDirty();
				}
			} 
			// Set refresh to false to prevent reopening
			refreshSceneName = false;
		}
		// In case of invalid file
		catch (const std::filesystem::filesystem_error& e)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
			return;
		}
	}

	// No scenes created
	if (sceneNames.empty()) currScene = "No scenes available.";

	// Display active scene name
	ImGui::Text(currScene.c_str());
	
	ImGui::Separator();

	// Display scene names
	for (const auto& filePath : sceneNames)
	{
		std::string filename = filePath.filename().string();

		// Give it an id
		ImGui::PushID(filename.c_str());

		// Create an invisible button over the text
		// Size of invisible button
		ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
		ImGui::InvisibleButton("##scene", textSize);

		// Left click loads scene
		// IsItemClicked checks the click on the most recent pushed imgui item aka invisible button
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			// Load scene
			std::cout << "current scene: " << editor_.getCurrentScene().getSceneName() << ", filename: " << filename << std::endl;
			// Check that scene is not currently loaded
			if (editor_.getCurrentScene().getSceneName() != filename)
			{
				std::cout << "Loading file: " << filename << std::endl;
				// Defer scene loading until after ImGui rendering completes
				g_pendingSceneLoad = true;
				g_sceneToLoad = filename;
			}
			else
			{
				std::cout << "Scene is already loaded!";
			}

		}

		// Right click opens a panel to allow you to change and delete scene name
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			// Open popup
			ImGui::OpenPopup("scene_menu");
		}

		// Create menu popup
		if (ImGui::BeginPopup("scene_menu"))
		{
			ImGui::Text("%s", filename.c_str());
			ImGui::Separator();

			// Input box to change name
			ImGui::Text("Change scene name");
			ImGui::InputText("##Change_scene_name", scene_name, sizeof(scene_name));

			if (ImGui::Button("Change new name"))
			{
				if (!scene_name || scene_name[0] == '\0')
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: no input detected!");
				}
				else
				{
					// Rename scene file
					try
					{
						// Get current filepath
						std::filesystem::path currentPath = filePath;

						// Check for .txt extension in new file
						std::string newSceneName = scene_name;
						if (newSceneName.find(".txt") == std::string::npos) newSceneName += ".txt";

						std::filesystem::path newPath = ASSET_SCENE_PATH + newSceneName;
						std::filesystem::rename(currentPath, newPath);

						// Check if we're renaming the currently active scene
						bool isCurrentScene = (filename == editor_.getCurrentScene().getSceneName());

						// If we renamed the active scene, update the current scene name
						if (isCurrentScene)
						{
							editor_.getCurrentScene().setSceneName(newSceneName);

							// Update LastSavedScene.txt
							editor_.updateLastSavedScene();
						}

						// Close popup after renaming
						ImGui::CloseCurrentPopup();
						// Clear buffer
						memset(scene_name, 0, sizeof(scene_name));

						// If file rename is successful
						refreshSceneName = true;
					}
					catch (const std::filesystem::filesystem_error& e)
					{
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
						return;
					}
				}

			}
			// Delete scene
			if (ImGui::Button("Delete scene"))
			{
				try
				{
					// Remove file
					std::filesystem::remove(filePath);
					// Remove active scene name
					//editor_.getCurrentScene().setSceneName("");
					// Close current popup
					ImGui::CloseCurrentPopup();
					// Refresh list
					refreshSceneName = true;
				}
				catch (const std::filesystem::filesystem_error& e)
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
				}
			}

			// Cancel edits
			if (ImGui::Button("Close menu"))
			{
				// Close popup
				ImGui::CloseCurrentPopup();
				// In case theres some random stuff inside the input box, clear the buffer
				memset(scene_name, 0, sizeof(scene_name));
			}

			ImGui::EndPopup();

		}

		// Aligning text to invisible button
		ImGui::SameLine(0, 0);
		// Minus button size offset
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - textSize.x);
		// Display file name as text
		ImGui::Text("%s", filename.c_str());

		if (ImGui::IsItemHovered())
		{
			// Hover feedback
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		// Prevents any duplicate of ID
		ImGui::PopID();

	}

	// Save scene
	if (ImGui::Button("Save scene"))
	{
		std::string sceneName = editor_.getCurrentScene().getSceneName();
		// Save scene
		editor_.getSceneTree().save(ASSET_SCENE_PATH + sceneName, sceneName);
		// Update last saved scene
		editor_.updateLastSavedScene();
	}

	if (ImGui::Button("Create new scene"))
	{
		// Defer scene creation until after ImGui rendering completes
		g_pendingNewScene = true;
		g_refreshSceneList = true;
	}


}

static void LoadAudioAssets(Engine::EnoEngine& engine_)
{
	audioAssets.clear();

	std::vector<std::string> folders = {
		engine_.getResourceLoader().getMusicPath().c_str(),
		engine_.getResourceLoader().getSFXPath().c_str(),
	};

	for (const auto& folder : folders)
	{
		if (!std::filesystem::exists(folder))
			continue;

		for (const auto& entry : std::filesystem::directory_iterator(folder))
		{
			if (!entry.is_regular_file()) continue;

			std::string ext = entry.path().extension().string();
			if (ext == ".wav" || ext == ".ogg" || ext == ".mp3")
			{
				audioAssets.push_back(entry.path().filename().string());
			}
		}
	}
}

void displayAudioTab(Engine::EnoEngine& engine_)
{
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	Event::EnoEventBus& bus = engine_.getEventBus();


	ImGui::Text("=== AUDIO ASSETS ===");

	// Button to load audio assets exactly once
	if (ImGui::Button("Load Audio Files"))
	{
		LoadAudioAssets(engine_);
	}

	ImGui::Separator();


	static int selectedAsset = -1;
	static std::string selectedName{};
	static FMOD::Sound* selectedSound = nullptr;

	// ============== FILE LIST ==============
	ImGui::BeginChild("AudioAssetList", ImVec2(0, 150), true);

	if (audioAssets.empty())
	{
		ImGui::TextDisabled("(No audio loaded. Press button above)");
	}
	else
	{
		for (int i = 0; i < audioAssets.size(); ++i)
		{
			if (ImGui::Selectable(audioAssets[i].c_str(), selectedAsset == i))
			{
				selectedAsset = i;

				selectedName = audioAssets[i];

				selectedName = selectedName.substr(0, selectedName.find_last_of('.'));

				std::cout << "[Audio] User selected: " << selectedName << "\n";

				// Request engine to load this sound
				bus.publish(Audio::RequestSound{ selectedName });

				auto& sounds = loader.getSounds();
				auto it = sounds.find(selectedName);

				if (it != sounds.end())
				{
					selectedSound = it->second;
					std::cout << "[Audio] Loaded Sound: " << selectedName << "\n";
				}
				else
				{
					selectedSound = nullptr;
					std::cout << "[Audio] ERROR: Sound not found after request.\n";
				}
			}
		}
	}

	ImGui::EndChild();
	ImGui::Separator();

	// ====== Waveform Viewer (Mock) ======
	ImGui::Text("Waveform Preview");
	ImGui::BeginChild("WaveformViewer", ImVec2(0, 200), true);

	ImDrawList* draw = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();

	float midY = pos.y + size.y * 0.5f;
	float step = size.x / 60.0f;

	// mock waveform
	for (int i = 0; i < 60; i++)
	{
		float h = (sinf(i * 0.3f) * 0.5f + 0.5f) * (size.y * 0.9f);
		draw->AddLine(
			ImVec2(pos.x + i * step, midY - h * 0.5f),
			ImVec2(pos.x + i * step, midY + h * 0.5f),
			IM_COL32(100, 200, 255, 255), 2.0f
		);
	}

	ImGui::EndChild();



	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("=== Test Playback ===");

	if (selectedName.empty())
	{
		ImGui::TextDisabled("Select an audio file to test.");
	}
	else
	{
		ImGui::Text("Selected: %s", selectedName.c_str());

		if (ImGui::Button("Play as SFX"))
		{
			bus.publish(Audio::PlayInstant{ "SFX", selectedName });
			std::cout << "PlayInstant SFX: " << selectedName << "\n";
		}

		if (ImGui::Button("Set as BGM"))
		{
			bus.publish(Audio::SetNewAudio{ selectedName });
			std::cout << "SetNewAudio (BGM): " << selectedName << "\n";
		}

		if (ImGui::Button("Play BGM preview"))
		{
			bus.publish(Audio::SetAudioState{ "BGM" });
		}

		if (ImGui::Button("Stop BGM preview"))
		{
			bus.publish(Audio::StopBGM{});
		}

		auto& map = engine_.getEntityManager().getMap();
		auto it = map.find(selectedEntityID);

		if (it == map.end())
		{
			ImGui::TextDisabled("Invalid entity selected");
		}
		else
		{
			// Channel Group Selection
			static int selectedChannelGroup = 1; // 0=Master, 1=SFX, 2=BGM
			const char* channelGroups[] = { "Master", "SFX", "BGM" };

			ImGui::Text("Channel Group:");
			ImGui::Combo("##ChannelGroup", &selectedChannelGroup, channelGroups, IM_ARRAYSIZE(channelGroups));

			if (ImGui::Button("Attach Audio to Selected Entity"))
			{
				ECS::Entity selectedEntity{ selectedEntityID };

				// Add component if missing
				if (!selectedEntity.has<ECSComponents::AudioComponent>(engine_.getEntityManager())) {
					selectedEntity.add<ECSComponents::AudioComponent>(engine_.getEntityManager());
					std::cout << "[Audio] Added AudioComponent.\n";
				}

				// Fill it
				ECSComponents::AudioComponent& ac = selectedEntity.get<ECSComponents::AudioComponent>(engine_.getEntityManager());

				strncpy(ac.audioMod_.name_, selectedName.c_str(), sizeof(ac.audioMod_.name_) - 1);
				strncpy(ac.audioMod_.chGroup_, channelGroups[selectedChannelGroup], sizeof(ac.audioMod_.chGroup_) - 1);

				ac.audioMod_.sound_ = selectedSound;

				std::cout << "[Audio] Attached '" << selectedName
					<< "' to entity " << selectedEntityID
					<< " with channel group '" << channelGroups[selectedChannelGroup] << "'\n";
			}
		}


	}

}




void displayEntityInspector(Engine::EnoEngine& engine_, ECS::Entity& entity, Editor::UndoManager& undoManager, Editor::EnoSceneTree& sceneTree)
{
	auto& entMng = engine_.getEntityManager();

	// Note: Entity ID 0 is valid, so don't early return based on ID
	ImGui::Begin("Entity Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Entity ID: %u", entity.id());

	// Debug output to console when inspector opens
	static uint32_t lastInspectedID = UINT32_MAX;
	if (lastInspectedID != entity.id()) {
		lastInspectedID = entity.id();
	}

	// --- NameComponent ---
	if (entity.has<ECSComponents::NameComponent>(entMng)) {
		auto& nameComp = entity.get<ECSComponents::NameComponent>(entMng);

		char buffer[32];
		strncpy_s(buffer, nameComp.name, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';

		ImGui::Text("Name: %s", nameComp.name);

	}

	// Check if this is a player entity
	bool isPlayer = entity.has<ECSComponents::PlayerComponent>(entMng);

	// --- TransformComponent ---
	if (entity.has<ECSComponents::TransformComponent>(entMng)) {
		auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
		ImGui::SeparatorText("Transform");

		// Track position editing for undo
		static Math::eVec2<float> oldPosInspector;
		static bool editingPosInspector = false;

		if (ImGui::IsItemActive() && !editingPosInspector) {
			oldPosInspector = transform.pos;
			editingPosInspector = true;
		}

		ImGui::DragFloat2("Position", (float*)&transform.pos, 0.5f);

		if (editingPosInspector && !ImGui::IsItemActive()) {
			// Record undo when user finishes editing
			if (oldPosInspector.x != transform.pos.x || oldPosInspector.y != transform.pos.y) {
				undoManager.recordMove(entity, oldPosInspector, transform.pos);
			}
			editingPosInspector = false;
		}

		// Scale removed - doesn't do anything currently
	}

	// --- PhysicsComponent --- (skip for player)
	if (!isPlayer && entity.has<ECSComponents::PhysicsComponent>(entMng)) {
		auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);
		ImGui::SeparatorText("Physics");
		ImGui::DragFloat2("Velocity", (float*)&physics.velocity, 0.5f);
		ImGui::DragFloat("Mass", &physics.mass, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Drag", &physics.drag, 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Use Gravity", &physics.useGravity);
	}

	// --- DrawInfoComponent --- (skip for player)
	if (!isPlayer && entity.has<ECSComponents::DrawInfoComponent>(entMng)) {
		auto& draw = entity.get<ECSComponents::DrawInfoComponent>(entMng);

		ImGui::SeparatorText("Draw Info");

		// Track if width/height changed to auto-update collision
		float oldWidth = draw.width;
		float oldHeight = draw.height;

		ImGui::DragFloat("Width", &draw.width, 0.5f, 1.0f, 500.0f);
		ImGui::DragFloat("Height", &draw.height, 0.5f, 1.0f, 500.0f);

		// Auto-update collision when width/height changes
		if ((draw.width != oldWidth || draw.height != oldHeight) && entity.has<ECSComponents::CollisionComponent>(entMng)) {

			auto& collision = entity.get<ECSComponents::CollisionComponent>(entMng);

			if (strcmp(draw.shape, "Circle") == 0) {
				// Update circle radius
				collision.data.circle.radius = draw.width * 0.5f;
			}
			else {
				// Update rectangle dimensions
				collision.data.rectangle.width = draw.width;
				collision.data.rectangle.height = draw.height;
			}
		}

		// Rotation field (edits TransformComponent rotation)
		if (entity.has<ECSComponents::TransformComponent>(entMng)) {
			auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);

			// Track rotation editing for undo
			static Math::eVec2<float> oldRotInspector;
			static bool editingRotInspector = false;

			if (ImGui::IsItemActive() && !editingRotInspector) {
				oldRotInspector = transform.rot;
				editingRotInspector = true;
			}

			// Convert radians to degrees for user-friendly editing
			float rotationDegrees = transform.rot.x * (180.0f / 3.14159265f);
			if (ImGui::DragFloat("Rotation", &rotationDegrees, 1.0f, -360.0f, 360.0f, "%.1f deg")) {
				// Convert degrees back to radians
				transform.rot.x = rotationDegrees * (3.14159265f / 180.0f);
				if (g_game) g_game->markEntitiesDirty();
			}

			if (editingRotInspector && !ImGui::IsItemActive()) {
				// Record undo when user finishes editing
				if (oldRotInspector.x != transform.rot.x || oldRotInspector.y != transform.rot.y) {
					undoManager.recordRotation(entity, oldRotInspector, transform.rot);
				}
				editingRotInspector = false;
			}
		}

		// Shape type selector (Circle or Rectangle)
		const char* shapeTypes[] = { "Circle", "Rectangle" };
		int currentShapeIndex = (strcmp(draw.shape, "Circle") == 0) ? 0 : 1;
		if (ImGui::Combo("Shape", &currentShapeIndex, shapeTypes, IM_ARRAYSIZE(shapeTypes))) {
			strncpy_s(draw.shape, shapeTypes[currentShapeIndex], sizeof(draw.shape) - 1);
			draw.shape[sizeof(draw.shape) - 1] = '\0';

			// Auto-update collision component to match new shape
			if (entity.has<ECSComponents::CollisionComponent>(entMng)) {
				auto& collision = entity.get<ECSComponents::CollisionComponent>(entMng);

				if (strcmp(draw.shape, "Circle") == 0) {
					// Changed to circle - auto-calculate circle collider
					collision.type = ECSComponents::ColliderType::CIRCLE;
					collision.data.circle.radius = draw.width * 0.5f;
				}
				else {
					// Changed to Rectangle - auto-calculate rectangle collider
					collision.type = ECSComponents::ColliderType::RECTANGLE;
					collision.data.rectangle.width = draw.width;
					collision.data.rectangle.height = draw.height; 
				}
			}
			if (g_game) g_game->markEntitiesDirty();
		}

		// Color selector (Red or Green)
		const char* colorTypes[] = { "Green", "Red" };
		int currentColorIndex = 0;
		if (draw.color.r == 1.0f && draw.color.g == 0.0f && draw.color.b == 0.0f) {
			currentColorIndex = 1; // Red
		}
		else if (draw.color.r == 0.0f && draw.color.g == 1.0f && draw.color.b == 0.0f) {
			currentColorIndex = 0; // Green
		}

		if (ImGui::Combo("Color", &currentColorIndex, colorTypes, IM_ARRAYSIZE(colorTypes))) {
			if (currentColorIndex == 0) {
				draw.color = Graphics::Color::Green;
			}
			else {
				draw.color = Graphics::Color::Red;
			}
			if (g_game) g_game->markEntitiesDirty();
		}

		// Display color preview
		ImVec4 colorVec(draw.color.r, draw.color.g, draw.color.b, draw.color.a);
		ImGui::ColorButton("##color", colorVec, 0, ImVec2(20, 20));

		// Display texture info if present
		if (draw.texture != nullptr && draw.texture->isValid()) {
			ImGui::Text("Texture ID: %d", draw.texID);
			ImGui::Image((void*)(intptr_t)draw.texture->getID(), ImVec2(64, 64));
		}
		else {
			ImGui::TextDisabled("No texture assigned");
		}
	}

	// --- CollisionComponent ---
	if (entity.has<ECSComponents::CollisionComponent>(entMng)) {
		auto& collider = entity.get<ECSComponents::CollisionComponent>(entMng);
		ImGui::SeparatorText("Collider");

		// Show Collider checkbox
		if (ImGui::Checkbox("Show Collider", &showColliders_)) {
			// Toggle visibility immediately
		}

		// Collider type selector (only Circle and Rectangle)
		const char* colliderTypes[] = { "Circle", "Rectangle" };

		// Map the current type to the combo index (0 = Circle, 1 = Rectangle)
		int comboIndex = (collider.type == ECSComponents::ColliderType::CIRCLE) ? 0 : 1;

		if (ImGui::Combo("Type", &comboIndex, colliderTypes, IM_ARRAYSIZE(colliderTypes))) {
			ECSComponents::ColliderType newType = (comboIndex == 0) ?
				ECSComponents::ColliderType::CIRCLE : ECSComponents::ColliderType::RECTANGLE;

			// Only change if it's a different type
			if (newType != collider.type) {
				collider.type = newType;

				// Initialize default values for the new collider type
				if (newType == ECSComponents::ColliderType::CIRCLE) {
					collider.data.circle.radius = 10.0f;
				}
				else if (newType == ECSComponents::ColliderType::RECTANGLE) {
					collider.data.rectangle.width = 20.0f;
					collider.data.rectangle.height = 20.0f;
				}

				if (g_game) g_game->markEntitiesDirty();
			}
		}

		// Display and edit collider dimensions based on type
		if (collider.type == ECSComponents::ColliderType::CIRCLE) {
			if (ImGui::DragFloat("Radius", &collider.data.circle.radius, 0.5f, 1.0f, 200.0f)) {
				if (g_game) g_game->markEntitiesDirty();
			}
		}
		else if (collider.type == ECSComponents::ColliderType::RECTANGLE) {
			if (ImGui::DragFloat("Width##collider", &collider.data.rectangle.width, 0.5f, 1.0f, 500.0f)) {
				if (g_game) g_game->markEntitiesDirty();
			}
			if (ImGui::DragFloat("Height##collider", &collider.data.rectangle.height, 0.5f, 1.0f, 500.0f)) {
				if (g_game) g_game->markEntitiesDirty();
			}

			// Show auto-calculated dimensions if entity has RenderComponent
			if (entity.has<ECSComponents::RenderComponent>(entMng)) {
				auto& render = entity.get<ECSComponents::RenderComponent>(entMng);
				ImGui::TextDisabled("Auto : %.1f x %.1f", render.width, render.height);

				if (ImGui::Button("Reset to Render Size")) {
					collider.data.rectangle.width = render.width;
					collider.data.rectangle.height = render.height;
					if (g_game) g_game->markEntitiesDirty();
				}
			}
		}

		// Offset controls
		ImGui::Spacing();
		ImGui::Text("Offset:");
		if (ImGui::DragFloat("X##offset", &collider.offset.x, 0.5f, -200.0f, 200.0f)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		if (ImGui::DragFloat("Y##offset", &collider.offset.y, 0.5f, -200.0f, 200.0f)) {
			if (g_game) g_game->markEntitiesDirty();
		}

		// Trigger and Active flags
		ImGui::Spacing();
		if (ImGui::Checkbox("Is Trigger", &collider.isTrigger)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		if (ImGui::Checkbox("Is Active", &collider.isActive)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		
	}
	else {
		// Allow adding a collider if the entity doesn't have one
		ImGui::SeparatorText("Collider");
		ImGui::TextDisabled("No collider attached");

		if (ImGui::Button("Add Collider")) {
			if (entity.has<ECSComponents::RenderComponent>(entMng)) {
				auto& render = entity.get<ECSComponents::RenderComponent>(entMng);
				auto collision = ECSComponents::CreateAABBFromRender(render);
				if (!entity.has<ECSComponents::CollisionComponent>(entMng)) {
					std::cout << "attempting to add collision component\n";
					entity.add<ECSComponents::CollisionComponent>(entMng, collision);
				
				}
			}
			else {
				entity.add<ECSComponents::CollisionComponent>(entMng, ECSComponents::CollisionComponent(10.0f));
			}
			if (g_game) g_game->markEntitiesDirty();
		}

		if (entity.has<ECSComponents::RenderComponent>(entMng)) {
			auto& render = entity.get<ECSComponents::RenderComponent>(entMng);
			ImGui::TextDisabled("Will auto-size: %.1f x %.1f", render.width, render.height);
		}
	}
	if (entity.has<ECSComponents::BoxComponent>(entMng)) {
		auto& pushComp = entity.get<ECSComponents::BoxComponent>(entMng);
		ImGui::SeparatorText("Box (Pushable)");
		if (ImGui::Checkbox("Is Pushable", &pushComp.isPushable)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		ImGui::DragFloat("Push Resistance", &pushComp.pushResistance, 0.01f, 0.0f, 1.0f);
	}
#if 0
	else {
		// Allow adding Box component if the entity doesn't have one
		ImGui::SeparatorText("Box (Pushable)");
		ImGui::TextDisabled("No Box component attached");

		if (ImGui::Button("Add Box Component")) {
			entity.add<ECSComponents::BoxComponent>(entMng);
			if (g_game) g_game->markEntitiesDirty();
		}
	}
#endif
	
	// --- AIComponent ---
	if (entity.has<ECSComponents::AIComponent>(entMng)) {
		auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
		ImGui::SeparatorText("AI");

		// State selector
		const char* states[] = { "Idle", "Chasing" };

		// Map the current state to the combo index (0 = Idle, 1 = Chasing)
		int stateIndex = (strcmp(ai.currentState, "Idle") == 0) ? 0 : 1;

		if (ImGui::Combo("State", &stateIndex, states, IM_ARRAYSIZE(states))) {
			std::strncpy(ai.nextState, states[stateIndex], sizeof(ai.nextState));
		}

		ImGui::DragFloat("Radar radius", &ai.awarenessRadius, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Speed", &ai.speed, 1.f, 10.f, 300.0f);
	}
	else {
		// Allow adding AI component if the entity doesn't have one
		ImGui::SeparatorText("AI");
		ImGui::TextDisabled("No AI attached");

		if (ImGui::Button("Add AI")) {
			// Add a default AI component
			auto& aiMng = engine_.getAIManager();
			auto& pathfinder = g_game->getPathfinder();
			entity.add<ECSComponents::AIComponent>(entMng);
			aiMng.registerEntity(entity, entMng);

			// Setup FSM
			AI::StateMachine& sm = aiMng.getStateMachine(entity);
			sm.addState("Idle", std::make_unique<AI::IdleState>(aiMng, pathfinder));
			sm.addState("Chase", std::make_unique<AI::ChaseState>(pathfinder, aiMng));
			sm.changeState("Idle", entity, entMng);
		}
	}

	//for dropdown selection of Looping menu.
	const char* loopOptions[] = { "Disabled","Forward","PingPong" };

	if (entity.has<ECSComponents::AudioComponent>(entMng))
	{
		auto& ac = entity.get<ECSComponents::AudioComponent>(entMng);

		// Main Audio Section
		ImGui::SeparatorText("Audio Component");

		// Audio Info with playback indicator
		ImGui::Text("Sound: %s", strlen(ac.audioMod_.name_) > 0 ? ac.audioMod_.name_ : "(No audio attached)");

		// Playback state indicator
		if (ac.isPlaying_) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[PLAYING]");
		}
		else if (ac.isAutoplay_) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[AUTOPLAY]");
		}

		// Channel Group Selection
		ImGui::Text("Channel Group: %s", ac.audioMod_.chGroup_);
		const char* channelGroupOptions[] = { "Master", "SFX", "BGM" };
		static int currentChannelGroup = 1;

		// Find current channel group index
		for (int i = 0; i < IM_ARRAYSIZE(channelGroupOptions); i++) {
			if (strcmp(ac.audioMod_.chGroup_, channelGroupOptions[i]) == 0) {
				currentChannelGroup = i;
				break;
			}
		}

		if (ImGui::Combo("##ChangeChannelGroup", &currentChannelGroup, channelGroupOptions, IM_ARRAYSIZE(channelGroupOptions))) {
			strncpy(ac.audioMod_.chGroup_, channelGroupOptions[currentChannelGroup], sizeof(ac.audioMod_.chGroup_) - 1);
			ac.audioMod_.chGroup_[sizeof(ac.audioMod_.chGroup_) - 1] = '\0';
		}

		ImGui::Spacing();
		ImGui::Separator();

		// Playback Controls
		if (ImGui::TreeNodeEx("Playback Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Play/Pause/Stop buttons in a row
			if (ImGui::Button("Play", ImVec2(80, 0))) {
				// If audio is already playing, stop it first to restart from beginning
				if (ac.isPlaying_) {
					engine_.getAudioManager().stopChannelGroup(ac.audioMod_.chGroup_);
					ac.isPlaying_ = false;
				}

				// Use the proper play function that applies volume/pitch/pan from the component
				engine_.getAudioManager().play(ac);
				std::cout << "[Audio] Playing: " << ac.audioMod_.name_
					<< " (Vol:" << ac.volume_ << ", Pitch:" << ac.pitchScale_ << ", Pan:" << ac.panScale_ << ")\n";
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop", ImVec2(80, 0))) {
				// Stop the channel group associated with this entity's audio
				engine_.getAudioManager().stopChannelGroup(ac.audioMod_.chGroup_);
				ac.isPlaying_ = false;
				std::cout << "[Audio] Stopped: " << ac.audioMod_.name_ << "\n";
			}

			ImGui::Checkbox("Autoplay on Start", &ac.isAutoplay_);
			ImGui::TreePop();
		}

		ImGui::Separator();

		// Stream Properties
		if (ImGui::TreeNodeEx("Stream Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Format:");
			ImGui::SameLine(160);
			ImGui::TextDisabled("16-Bit");

			int loopMode = static_cast<int>(ac.audioMod_.loop_);
			ImGui::Text("Loop Mode:");
			ImGui::SameLine(160);
			if (ImGui::Combo("##LoopMode", &loopMode, loopOptions, IM_ARRAYSIZE(loopOptions)))
			{
				ac.audioMod_.loop_ = static_cast<Audio::EnoAudioModule::LoopMode>(loopMode);
			}

			int loopBegin = static_cast<int>(ac.audioMod_.loopStart_);
			ImGui::Text("Loop Begin:");
			ImGui::SameLine(160);
			if (ImGui::InputInt("##LoopBegin", &loopBegin))
			{
				ac.audioMod_.loopStart_ = static_cast<float>(loopBegin);
			}

			int loopEnd = static_cast<int>(ac.audioMod_.loopEnd_);
			ImGui::Text("Loop End:");
			ImGui::SameLine(160);
			if (ImGui::InputInt("##LoopEnd", &loopEnd))
			{
				ac.audioMod_.loopEnd_ = static_cast<float>(loopEnd);
			}

			ImGui::Text("Mix Rate:");
			ImGui::SameLine(160);
			ImGui::TextDisabled("44100");

			ImGui::Text("Stereo:");
			ImGui::SameLine(160);
			static bool stereoToggle = true;
			ImGui::Checkbox("##StereoChk", &stereoToggle);

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Basic Audio Parameters
		if (ImGui::TreeNodeEx("Audio Parameters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Volume:");
			ImGui::SameLine(160);
			float tmpVol = static_cast<float>(ac.volume_);
			if (ImGui::SliderFloat("##Volume", &tmpVol, 0.f, 100.f, "%.1f")) {
				ac.volume_ = static_cast<double>(tmpVol);
			}

			ImGui::Text("Pitch Scale:");
			ImGui::SameLine(160);
			float tmppitch = static_cast<float>(ac.pitchScale_);
			if (ImGui::SliderFloat("##PitchScale", &tmppitch, 0.1f, 5.f, "%.2f")) {
				ac.pitchScale_ = static_cast<double>(tmppitch);
			}

			ImGui::Text("Pan:");
			ImGui::SameLine(160);
			float pan = static_cast<float>(ac.panScale_);
			if (ImGui::SliderFloat("##Pan", &pan, -1.f, 1.f, "%.2f"))
			{
				ac.panScale_ = static_cast<double>(pan);
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Advanced Effects - Reverb (now per-entity!)
		if (ImGui::TreeNodeEx("Reverb Effects", ImGuiTreeNodeFlags_None))
		{
			ImGui::Checkbox("Enable Reverb", &ac.enableReverb_);

			if (ac.enableReverb_) {
				ImGui::Text("Wet Level:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##ReverbWet", &ac.reverbWet_, 0.0f, 1.0f, "%.2f");

				ImGui::Text("Dry Level:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##ReverbDry", &ac.reverbDry_, 0.0f, 1.0f, "%.2f");

				ImGui::Text("Decay Time:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##ReverbDecay", &ac.reverbDecay_, 100.0f, 5000.0f, "%.0f ms");

				ImGui::TextDisabled("Note: Reverb effects are applied via Audio Manager");
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Advanced Effects - EQ (now per-entity!)
		if (ImGui::TreeNodeEx("EQ Effects", ImGuiTreeNodeFlags_None))
		{
			ImGui::Checkbox("Enable 3-Band EQ", &ac.enableEQ_);

			if (ac.enableEQ_) {
				ImGui::Text("Low Gain:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##EQLow", &ac.eqLowGain_, -30.0f, 30.0f, "%.1f dB");

				ImGui::Text("Mid Gain:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##EQMid", &ac.eqMidGain_, -30.0f, 30.0f, "%.1f dB");

				ImGui::Text("High Gain:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##EQHigh", &ac.eqHighGain_, -30.0f, 30.0f, "%.1f dB");

				ImGui::TextDisabled("Note: EQ effects are applied via Audio Manager");
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Debug Section (collapsed by default)
		if (ImGui::TreeNodeEx("Debug Info", ImGuiTreeNodeFlags_None))
		{
			if (ImGui::Button("Print Audio Component to Console"))
			{
				std::cout << "===== AUDIO COMPONENT DEBUG =====\n";
				std::cout << "Entity:       " << entity.id() << "\n";
				std::cout << "Volume:       " << ac.volume_ << "\n";
				std::cout << "Pitch Scale:  " << ac.pitchScale_ << "\n";
				std::cout << "Pan Scale:    " << ac.panScale_ << "\n";
				std::cout << "Autoplay:     " << (ac.isAutoplay_ ? "true" : "false") << "\n";
				std::cout << "Is Playing:   " << (ac.isPlaying_ ? "true" : "false") << "\n";
				std::cout << "-- MODULE --\n";
				std::cout << "Name:         " << ac.audioMod_.name_ << "\n";
				std::cout << "Path:         " << ac.audioMod_.path_ << "\n";
				std::cout << "ChannelGroup: " << ac.audioMod_.chGroup_ << "\n";
				std::cout << "Loop Mode:    " << static_cast<int>(ac.audioMod_.loop_) << "\n";
				std::cout << "Loop Start:   " << ac.audioMod_.loopStart_ << "\n";
				std::cout << "Loop End:     " << ac.audioMod_.loopEnd_ << "\n";
				std::cout << "Sound Ptr:    " << ac.audioMod_.sound_ << "\n";
				std::cout << "-- REVERB --\n";
				std::cout << "Enabled:      " << (ac.enableReverb_ ? "true" : "false") << "\n";
				std::cout << "Wet:          " << ac.reverbWet_ << "\n";
				std::cout << "Dry:          " << ac.reverbDry_ << "\n";
				std::cout << "Decay:        " << ac.reverbDecay_ << " ms\n";
				std::cout << "-- EQ --\n";
				std::cout << "Enabled:      " << (ac.enableEQ_ ? "true" : "false") << "\n";
				std::cout << "Low Gain:     " << ac.eqLowGain_ << " dB\n";
				std::cout << "Mid Gain:     " << ac.eqMidGain_ << " dB\n";
				std::cout << "High Gain:    " << ac.eqHighGain_ << " dB\n";
				std::cout << "=================================\n";
			}

			ImGui::TreePop();
		}

	}
	else
	{
		ImGui::SeparatorText("Audio");
		ImGui::TextDisabled("No AudioComponent attached.");

		//if (ImGui::Button("Add Audio Component"))
		//{
		//	entity.add<ECSComponents::AudioComponent>(entMng);
		//	std::cout << "[Audio] Added AudioComponent to entity "
		//		<< entity.id() << "\n";
		//}
	}
	// --- Prefab Section ---
	ImGui::SeparatorText("Prefab");

	// Check if this entity is a prefab instance
	bool isPrefabInstance = entity.has<ECSComponents::PrefabInstanceComponent>(entMng);
	std::string currentPrefabName;

	if (isPrefabInstance) {
		auto& prefabInstance = entity.get<ECSComponents::PrefabInstanceComponent>(entMng);
		currentPrefabName = prefabInstance.prefabName;
		ImGui::Text("Instance of: %s", currentPrefabName.c_str());
	}
	else {
		ImGui::TextDisabled("Not a prefab instance");
	}

	// Input field for prefab name
	static char prefabNameBuffer[64] = "";

	// If this is a prefab instance, pre-fill with current prefab name
	if (isPrefabInstance && prefabNameBuffer[0] == '\0') {
		strncpy_s(prefabNameBuffer, currentPrefabName.c_str(), sizeof(prefabNameBuffer) - 1);
	}

	ImGui::InputText("Prefab Name", prefabNameBuffer, sizeof(prefabNameBuffer));

	// Save as Prefab button
	if (ImGui::Button("Save as Prefab")) {
		if (strlen(prefabNameBuffer) > 0) {
			try {
				createPrefabFromEntity(entity, prefabNameBuffer, entMng, sceneTree);

				// Add or update PrefabInstanceComponent
				if (!isPrefabInstance) {
					entity.add<ECSComponents::PrefabInstanceComponent>(entMng, prefabNameBuffer);
				}
				else {
					auto& prefabInstance = entity.get<ECSComponents::PrefabInstanceComponent>(entMng);
					strncpy_s(prefabInstance.prefabName, prefabNameBuffer, sizeof(prefabInstance.prefabName) - 1);
					prefabInstance.prefabName[sizeof(prefabInstance.prefabName) - 1] = '\0';
				}

				// Register this entity as an instance
				registerPrefabInstance(entity.id(), prefabNameBuffer);

				std::cout << "[UI] Saved prefab: " << prefabNameBuffer << "\n";
			}
			catch (const std::exception& e) {
				std::cerr << "[UI] Failed to save prefab: " << e.what() << "\n";
			}
		}
		else {
			std::cerr << "[UI] Please enter a prefab name\n";
		}
	}

	ImGui::SameLine();

	// Update All Instances button (only show if this is a prefab instance)
	if (isPrefabInstance) {
		if (ImGui::Button("Update All Instances")) {
			try {
				int count = updateAllPrefabInstances(currentPrefabName, entity, entMng, sceneTree);
				std::cout << "[UI] Updated " << count << " instances of " << currentPrefabName << "\n";

				// Mark entities as dirty to refresh visuals
				if (g_game) g_game->markEntitiesDirty();
			}
			catch (const std::exception& e) {
				std::cerr << "[UI] Failed to update instances: " << e.what() << "\n";
			}
		}

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Updates all instances of '%s' with this entity's components", currentPrefabName.c_str());
		}
	}

	ImGui::End();
}

void assetPresetsTab(Engine::EnoEngine& engine_, ECS::Entity& entity) {

	Resources::EnoLoader& loader = engine_.getResourceLoader();
	const auto& textures = loader.getAllTextures();
	static std::string idleSelected = "";
	static std::string walkSelected = "";
	static std::string attackSelected = "";
	static std::string activeSlot = ""; // identifies which box you're assigning assets to
	auto& entMng = engine_.getEntityManager();

	ImGui::Begin("Assets Presets", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Check if the selected entity has an AnimationControllerComponent
	if (!entity.has<ECSComponents::AnimationControllerComponent>(entMng))
	{
		ImGui::TextWrapped("The currently selected entity does not have an Animation Controller component.");
		ImGui::End();
		return;
	}

	//================================= IDLE ANIMATION ===================================
	ImGui::Text("IDLE");


	static Graphics::TextureData idleTextureData;
	static int idleMode = 1;              // 0 = rows/cols, 1 = pixel size
	static int idleRows = 1;
	static int idleCols = 1;
	static int idleFrameW = 96;
	static int idleFrameH = 96;


	// LEFT SIDE PREVIEW
	if (!idleSelected.empty())
	{
		auto it = textures.find(idleSelected);
		if (it != textures.end())
		{
			auto tex = it->second;   // shared_ptr<EnoTexture>
			ImTextureID texID = (ImTextureID)(intptr_t)tex->getID();

			ImGui::Image(texID, ImVec2(180, 150));
		}
		else
		{
			ImGui::Button("Missing Texture", ImVec2(180, 150));
		}
	}
	else
	{
		ImGui::Button("##IdlePreview", ImVec2(180, 150));
	}

	//Drag and Drop target
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME"))
		{
			idleSelected = (const char*)payload->Data;

			auto it = textures.find(idleSelected);
			if (it != textures.end())
			{
				idleTexData.texture = it->second; // shared_ptr<EnoTexture>
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Text("Assigned: %s", idleSelected.empty() ? "(none)" : idleSelected.c_str());


	// RIGHT SIDE SETTINGS
	if (!idleSelected.empty())
	{
		ImGui::Separator();
		ImGui::Text("Animation Settings");

		const char* modes[] = { "Use Rows / Columns", "Use Pixel Width / Height" };
		ImGui::Combo("Input Mode##Idle", &idleMode, modes, IM_ARRAYSIZE(modes));

		auto it = textures.find(idleSelected);
		if (it != textures.end())
		{
			auto tex = it->second;
			int texW = tex->getWidth();
			int texH = tex->getHeight();

			if (idleMode == 0)
			{
				// ROWS / COLUMNS MODE
				ImGui::InputInt("Rows##Idle", &idleRows);
				ImGui::InputInt("Columns##Idle", &idleCols);

				idleRows = std::max(1, idleRows);
				idleCols = std::max(1, idleCols);

				ImGui::Text("Frame Size: %d × %d", idleFrameW, idleFrameH);
			}
			else if (idleMode == 1)
			{
				// PIXEL MODE
				ImGui::InputInt("Frame Width##Idle", &idleFrameW);
				ImGui::InputInt("Frame Height##Idle", &idleFrameH);

				idleFrameW = std::max(1, idleFrameW);
				idleFrameH = std::max(1, idleFrameH);

				ImGui::Text("Grid: %d rows × %d cols", idleRows, idleCols);
			}

			idleTexData.frameWidth = idleFrameW;
			idleTexData.frameHeight = idleFrameH;
			idleTexData.rows = idleRows;
			idleTexData.cols = idleCols;
		}
	}

	ImGui::Spacing();

	// ================== APPLY BUTTON FOR IDLE ANIMATION ==================
	// Disable button if no texture is selected
	bool idleHasTexture = idleTexData.texture != nullptr && !idleSelected.empty();
	if (!idleHasTexture)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("APPLY IDLE"))
	{
		ECS::Entity e{ selectedEntityID };
		if (!e.has<ECSComponents::AnimationControllerComponent>(entMng)) return;

		auto& animComp = e.get<ECSComponents::AnimationControllerComponent>(entMng);
		auto& drawComp = e.get<ECSComponents::DrawInfoComponent>(entMng);
		Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);
		if (!ctrl) return;

		// Safety check: Ensure texture is valid before proceeding
		if (!idleTexData.texture)
		{
			ImGui::OpenPopup("Error##IdleNoTexture");
			if (!idleHasTexture) ImGui::EndDisabled();
			return;
		}

		//drawComp.texture = idleTexData.texture;
		//drawComp.color = Graphics::Color::White;
		drawComp.setTexture(idleTexData.texture, idleSelected);
		drawComp.color = Graphics::Color::White;


		//std::strncpy(drawComp.texturePath, ctrl->currentAnimation_->.c_str(), sizeof(drawComp.texturePath) - 1);
		//drawComp.texturePath[sizeof(drawComp.texturePath) - 1] = '\0';

		drawComp.texID = 0;

		//checkpoint
		if (idleMode == 0)   // rows/cols mode
		{
			idleTexData.rows = std::max(1, idleRows);
			idleTexData.cols = std::max(1, idleCols);
			idleTexData.computeFrameSize();
		}
		else                 // pixel mode
		{
			idleTexData.rows = idleTexData.texture->getHeight() / std::max(1, idleFrameH);
			idleTexData.cols = idleTexData.texture->getWidth()  / std::max(1, idleFrameW);
			idleTexData.frameWidth = idleFrameW;
			idleTexData.frameHeight = idleFrameH;
		}


		// Create animation with explicit frame calculation
		Animation::EnoAnimation idleAnim = Animation::createSpriteSheetAnimation(
			idleTexData.frameWidth,
			idleTexData.frameHeight,
			idleTexData.texture->getWidth(),
			idleTexData.texture->getHeight(),
			0,                                  // start frame
			idleTexData.rows * idleTexData.cols,// total frames
			0.1f                                // duration per frame
		);

		// Optional: Print out frame UV coordinates for debugging
			//const auto& frame = idleAnim.getCurrentFrame();
			//std::cout << "Frame " << 0 << " UV Start: (" << frame.uvStart.x << ", " << frame.uvStart.y
			//	<< ") UV End: (" << frame.uvEnd.x << ", " << frame.uvEnd.y << ")" << std::endl;



		idleAnim.setLooping(true);
		ctrl->addAnimation("idle", idleAnim);
		ctrl->setAnimation("idle");

		// Save animation configuration to component for serialization
		animComp.frameWidth = idleTexData.frameWidth;
		animComp.frameHeight = idleTexData.frameHeight;
		animComp.rows = idleTexData.rows;
		animComp.cols = idleTexData.cols;
		animComp.frameDuration = 0.1f;
		animComp.looping = true;
		std::strncpy(animComp.currentAnimationName, "idle", sizeof(animComp.currentAnimationName) - 1);
	}

	if (!idleHasTexture)
	{
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Please select a texture first");
		}
	}

	// Error popup for missing texture
	if (ImGui::BeginPopupModal("Error##IdleNoTexture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Error: No texture selected!");
		ImGui::Text("Please drag and drop a texture onto the preview box first.");
		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}


	//================================= WALKING ANIMATION ===================================
	ImGui::Text("WALKING");

	static Graphics::TextureData walkTextureData;
	static int walkMode = 1;              // 0 = rows/cols, 1 = pixel size
	static int walkRows = 1;
	static int walkCols = 1;
	static int walkFrameW = 96;
	static int walkFrameH = 96;


	if (!walkSelected.empty())
	{
		auto it = textures.find(walkSelected);
		if (it != textures.end())
		{
			auto tex = it->second;
			ImTextureID texID = (ImTextureID)(intptr_t)tex->getID();
			ImGui::Image(texID, ImVec2(180, 150));
		}
		else {
			ImGui::Button("Missing Texture", ImVec2(180, 150));
		}
	}
	else
	{
		ImGui::Button("##WalkPreview", ImVec2(180, 150));
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME"))
		{
			walkSelected = (const char*)payload->Data;

			auto it = textures.find(walkSelected);
			if (it != textures.end())
			{
				walkTexData.texture = it->second; // shared_ptr<EnoTexture>
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Text("Assigned: %s", walkSelected.empty() ? "(none)" : walkSelected.c_str());

	// RIGHT SIDE SETTINGS
	if (!walkSelected.empty())
	{
		ImGui::Separator();
		ImGui::Text("Animation Settings");

		const char* modes[] = { "Use Rows / Columns", "Use Pixel Width / Height" };
		ImGui::Combo("Input Mode##walk", &walkMode, modes, IM_ARRAYSIZE(modes));

		auto it = textures.find(walkSelected);
		if (it != textures.end())
		{
			auto tex = it->second;
			int texW = tex->getWidth();
			int texH = tex->getHeight();

			if (walkMode == 0)
			{
				// ROWS / COLUMNS MODE
				ImGui::InputInt("Rows##Walk", &walkRows);
				ImGui::InputInt("Columns##Walk", &walkCols);

				walkRows = std::max(1, walkRows);
				walkCols = std::max(1, walkCols);

				ImGui::Text("Frame Size: %d × %d", walkFrameW, walkFrameH);
			}
			else if (walkMode == 1)
			{
				// PIXEL MODE
				ImGui::InputInt("Frame Width##Walk", &walkFrameW);
				ImGui::InputInt("Frame Height##Walk", &walkFrameH);

				walkFrameW = std::max(1, walkFrameW);
				walkFrameH = std::max(1, walkFrameH);

				ImGui::Text("Grid: %d rows × %d cols", walkRows, walkCols);
			}

			walkTexData.frameWidth = walkFrameW;
			walkTexData.frameHeight = walkFrameH;
			walkTexData.rows = walkRows;
			walkTexData.cols = walkCols;
		}
	}

	//  ================== APPLY BUTTON FOR WALK ANIMATION ==================
	// Disable button if no texture is selected
	bool walkHasTexture = walkTexData.texture != nullptr && !walkSelected.empty();
	if (!walkHasTexture)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("APPLY WALK"))
	{
		ECS::Entity e{ selectedEntityID };
		if (!e.has<ECSComponents::AnimationControllerComponent>(entMng)) return;

		auto& animComp = e.get<ECSComponents::AnimationControllerComponent>(entMng);
		auto& drawComp = e.get<ECSComponents::DrawInfoComponent>(entMng);
		Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);
		if (!ctrl) return;

		
		// Safety check: Ensure texture is valid before proceeding
		if (!walkTexData.texture)
		{
			ImGui::OpenPopup("Error##WalkNoTexture");
			if (!walkHasTexture) ImGui::EndDisabled();
			return;
		}

		//drawComp.texture = walkTexData.texture;
		drawComp.setTexture(walkTexData.texture, walkSelected);
		drawComp.color = Graphics::Color::White;


		drawComp.texID = 0;

		//checkpoint
		if (walkMode == 0)   // rows/cols mode
		{
			walkTexData.rows = std::max(1, walkRows);
			walkTexData.cols = std::max(1, walkCols);
			walkTexData.computeFrameSize();
		}
		else                 // pixel mode
		{
			walkTexData.rows = walkTexData.texture->getHeight() / std::max(1, walkFrameH);
			walkTexData.cols = walkTexData.texture->getWidth() / std::max(1,  walkFrameW);
			walkTexData.frameWidth = walkFrameW;
			walkTexData.frameHeight = walkFrameH;
		}


		// Create animation with explicit frame calculation
		Animation::EnoAnimation walkAnim = Animation::createSpriteSheetAnimation(
			walkTexData.frameWidth,
			walkTexData.frameHeight,
			walkTexData.texture->getWidth(),
			walkTexData.texture->getHeight(),
			0,                                  // start frame
			walkTexData.rows * walkTexData.cols,// total frames
			0.1f                                // duration per frame
		);

		// Optional: Print out frame UV coordinates for debugging
		//const auto& frame = walkAnim.getCurrentFrame();
		//std::cout << "Frame " << 0 << " UV Start: (" << frame.uvStart.x << ", " << frame.uvStart.y
		//	<< ") UV End: (" << frame.uvEnd.x << ", " << frame.uvEnd.y << ")" << std::endl;

		walkAnim.setLooping(true);
		ctrl->addAnimation("walk", walkAnim);
		ctrl->setAnimation("walk");

		// Save animation configuration to component for serialization
		animComp.frameWidth = walkTexData.frameWidth;
		animComp.frameHeight = walkTexData.frameHeight;
		animComp.rows = walkTexData.rows;
		animComp.cols = walkTexData.cols;
		animComp.frameDuration = 0.1f;
		animComp.looping = true;
		std::strncpy(animComp.currentAnimationName, "walk", sizeof(animComp.currentAnimationName) - 1);
	}

	if (!walkHasTexture)
	{
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Please select a texture first");
		}
	}

	// Error popup for missing texture
	if (ImGui::BeginPopupModal("Error##WalkNoTexture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Error: No texture selected!");
		ImGui::Text("Please drag and drop a texture onto the preview box first.");
		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}


	//================================= ATTACK ANIMATION ===================================
	ImGui::Text("ATTACK");

	static Graphics::TextureData attackTextureData;
	static int attackMode = 1;              // 0 = rows/cols, 1 = pixel size
	static int attackRows = 1;
	static int attackCols = 1;
	static int attackFrameW = 96;
	static int attackFrameH = 96;


	if (!attackSelected.empty())
	{
		auto it = textures.find(attackSelected);
		if (it != textures.end())
		{
			auto tex = it->second;
			ImTextureID texID = (ImTextureID)(intptr_t)tex->getID();
			ImGui::Image(texID, ImVec2(180, 150));
		}
		else {
			ImGui::Button("Missing Texture", ImVec2(180, 150));
		}
	}
	else
	{
		ImGui::Button("##AttackPreview", ImVec2(180, 150));
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME"))
		{
			
			attackSelected = (const char*)payload->Data;

			auto it = textures.find(attackSelected);
			if (it != textures.end())
			{
				attackTexData.texture = it->second; // shared_ptr<EnoTexture>
			}
			
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Text("Assigned: %s", attackSelected.empty() ? "(none)" : attackSelected.c_str());

	// RIGHT SIDE SETTINGS
	if (!attackSelected.empty())
	{
		ImGui::Separator();
		ImGui::Text("Animation Settings");

		const char* modes[] = { "Use Rows / Columns", "Use Pixel Width / Height" };
		ImGui::Combo("Input Mode##attack", &attackMode, modes, IM_ARRAYSIZE(modes));

		auto it = textures.find(attackSelected);
		if (it != textures.end())
		{
			auto tex = it->second;
			int texW = tex->getWidth();
			int texH = tex->getHeight();

			if (attackMode == 0)
			{
				// ROWS / COLUMNS MODE
				ImGui::InputInt("Rows##Attack", &attackRows);
				ImGui::InputInt("Columns##Attack", &attackCols);

				attackRows = std::max(1, attackRows);
				attackCols = std::max(1, attackCols);

				ImGui::Text("Frame Size: %d × %d", attackFrameW, attackFrameH);
			}
			else if (attackMode == 1)
			{
				// PIXEL MODE
				ImGui::InputInt("Frame Width##Attack", &attackFrameW);
				ImGui::InputInt("Frame Height##Attack", &attackFrameH);

				attackFrameW = std::max(1, attackFrameW);
				attackFrameH = std::max(1, attackFrameH);

				ImGui::Text("Grid: %d rows × %d cols", attackRows, attackCols);
			}

			attackTexData.frameWidth = attackFrameW;
			attackTexData.frameHeight = attackFrameH;
			attackTexData.rows = attackRows;
			attackTexData.cols = attackCols;
		}
	}


	// Disable button if no texture is selected
	bool attackHasTexture = attackTexData.texture != nullptr && !attackSelected.empty();
	if (!attackHasTexture)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("APPLY ATTACK"))
	{
		ECS::Entity e{ selectedEntityID };
		if (!e.has<ECSComponents::AnimationControllerComponent>(entMng)) return;

		auto& animComp = e.get<ECSComponents::AnimationControllerComponent>(entMng);
		auto& drawComp = e.get<ECSComponents::DrawInfoComponent>(entMng);
		Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);
		if (!ctrl) return;

		// Safety check: Ensure texture is valid before proceeding
		if (!attackTexData.texture)
		{
			ImGui::OpenPopup("Error##AttackNoTexture");
			if (!attackHasTexture) ImGui::EndDisabled();
			return;
		}

		//drawComp.texture = attackTexData.texture;
		drawComp.setTexture(attackTexData.texture, attackSelected);
		drawComp.color = Graphics::Color::White;


		drawComp.texID = 0;

		//checkpoint
		if (attackMode == 0)   // rows/cols mode
		{
			attackTexData.rows = std::max(1, attackRows);
			attackTexData.cols = std::max(1, attackCols);
			attackTexData.computeFrameSize();
		}
		else                 // pixel mode
		{
			attackTexData.rows = attackTexData.texture->getHeight() / std::max(1, attackFrameH);
			attackTexData.cols = attackTexData.texture->getWidth() / std::max(1, attackFrameW);
			attackTexData.frameWidth = attackFrameW;
			attackTexData.frameHeight = attackFrameH;
		}


		// Create animation with explicit frame calculation
		Animation::EnoAnimation attackAnim = Animation::createSpriteSheetAnimation(
			attackTexData.frameWidth,
			attackTexData.frameHeight,
			attackTexData.texture->getWidth(),
			attackTexData.texture->getHeight(),
			0,                                  // start frame
			attackTexData.rows * attackTexData.cols,// total frames
			0.1f                                // duration per frame
		);

		// Optional: Print out frame UV coordinates for debugging
		//const auto& frame = attackAnim.getCurrentFrame();
		//std::cout << "Frame " << 0 << " UV Start: (" << frame.uvStart.x << ", " << frame.uvStart.y
		//	<< ") UV End: (" << frame.uvEnd.x << ", " << frame.uvEnd.y << ")" << std::endl;
		attackAnim.setLooping(true);
		ctrl->addAnimation("attack", attackAnim);
		ctrl->setAnimation("attack");

		// Save animation configuration to component for serialization
		animComp.frameWidth = attackTexData.frameWidth;
		animComp.frameHeight = attackTexData.frameHeight;
		animComp.rows = attackTexData.rows;
		animComp.cols = attackTexData.cols;
		animComp.frameDuration = 0.1f;
		animComp.looping = true;
		std::strncpy(animComp.currentAnimationName, "attack", sizeof(animComp.currentAnimationName) - 1);
	}

	if (!attackHasTexture)
	{
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Please select a texture first");
		}
	}

	// Error popup for missing texture
	if (ImGui::BeginPopupModal("Error##AttackNoTexture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Error: No texture selected!");
		ImGui::Text("Please drag and drop a texture onto the preview box first.");
		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ImGui::Separator();
	ImGui::End();
}


void drawNodeRecursive(Editor::EnoSceneTree& sceneTree, uint32_t entityID, ECS::EntityManager& entityManager)
{
	using namespace Editor;
	Editor::Node* node = sceneTree.getNodeByEntityID(entityID);
	if (!node) return;

	// Get entity name from NameComponent if it exists
	std::string label = "Entity " + std::to_string(node->entity_.id());
#if 1
	// Try to get the actual name from the entity
	try {
		for (ECS::Entity& e : entityManager.findEntity<ECSComponents::NameComponent>()) {
			if (e.id() == node->entity_.id()) {
				auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
				label = nameComp.name;
				break;
			}
		}
	}
	catch (...) {
		// If we can't get the name, use the default
	}
#else
	try {
		for (Node* node : sceneTree.getAllNodes()) {
			label = node->name_;
			break;
		}
	}
	catch (...) {
		// If we can't get the name, use the default
	}


#endif
	// If node has children -> use TreeNodeEx
	if (node->children_.empty()) {
		bool selected = (selectedEntityID == node->entity_.id());
		// Use ## to separate visible label from unique ID
		std::string uniqueLabel = label + "##" + std::to_string(node->entity_.id());
		if (ImGui::Selectable(uniqueLabel.c_str(), selected))
			selectedEntityID = node->entity_.id();
	}
	else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (selectedEntityID == node->entity_.id())
			flags |= ImGuiTreeNodeFlags_Selected;

		// Use ## to separate visible label from unique ID
		std::string uniqueLabel = label + "##" + std::to_string(node->entity_.id());
		bool open = ImGui::TreeNodeEx(uniqueLabel.c_str(), flags);
		if (ImGui::IsItemClicked())
			selectedEntityID = node->entity_.id();

		if (open) {
			for (Editor::Node* child : node->children_)
				drawNodeRecursive(sceneTree, child->entity_.id(), entityManager);
			ImGui::TreePop();
		}
	}
}

void displayAssetTab(Engine::EnoEngine& engine_) {
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	const auto& textures = loader.getAllTextures();
	auto& g_droppedFiles = Window::EnoWindow::GetDroppedFiles();

	ImGui::Text("=== Loaded Textures ===");
	ImGui::Separator();

	static std::string selectedTexture;
	static Graphics::TextureData textureData;
	static int inputRows = 1, inputCols = 1;
	static int inputFrameW = 96, inputFrameH = 96;
	static int mode = 0; // 0 = Rows/Cols, 1 = Pixels

	// --- Handle dropped files ---
	if (!g_droppedFiles.empty()) {
		for (auto& path : g_droppedFiles) {
			std::filesystem::path filePath(path);
			auto ext = filePath.extension().string();

			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
				auto tex = std::make_shared<Graphics::EnoTexture>();
				if (tex->loadFromFile(path)) {
					std::string name = filePath.stem().string();
					loader.addTexture(name, tex);
					std::cout << "[DEBUG] Imported: " << name << std::endl;
				}
			}
		}
	}

	// Texture list
	for (auto& [name, texture] : textures) {
		if (!texture || !texture->isValid())
			continue;

		ImGui::Image((void*)(intptr_t)texture->getID(), ImVec2(48, 48));
		ImGui::SameLine();
		bool isSelected = (selectedTexture == name);
		if (ImGui::Selectable(name.c_str(), isSelected, 0, ImVec2(200, 48))) {
			selectedTexture = name;
			textureData.texture = texture;
		}
		if (ImGui::BeginDragDropSource())
		{
			// Store C-string name inside drag payload
			ImGui::SetDragDropPayload("TEXTURE_NAME", name.c_str(), name.size() + 1);

			ImGui::Text("%s", name.c_str());

			ImGui::EndDragDropSource();
		}

		ImGui::Spacing();
		ImGui::Separator();
	}

	ImGui::Separator();

	//YunQi Bookmark  (i believe this below section will have to be moved to Asset Prefabs)
	// If texture selected
	if (!selectedTexture.empty()) {
		ImGui::Text("Selected: %s", selectedTexture.c_str());

		// Mode selector
		const char* modes[] = { "Use Rows / Columns", "Use Pixel Width / Height" };
		ImGui::Combo("Input Mode", &mode, modes, IM_ARRAYSIZE(modes));

		if (mode == 0) {
			// ROWS/COLS MODE
			ImGui::InputInt("Rows", &inputRows);
			ImGui::InputInt("Columns", &inputCols);

			if (ImGui::Button("Apply Texture (Rows/Cols)")) {
				textureData.rows = std::max(1, inputRows);
				textureData.cols = std::max(1, inputCols);
				textureData.computeFrameSize();

				if (g_game)
					g_game->applyTextureToPlayer(textureData);
			}

			ImGui::Text("Frame Size: %d x %d",
				textureData.frameWidth, textureData.frameHeight);
		}
		else {
			// PIXEL INPUT MODE
			ImGui::InputInt("Frame Width", &inputFrameW);
			ImGui::InputInt("Frame Height", &inputFrameH);

			if (ImGui::Button("Apply Texture (Pixels)")) {
				textureData.rows = textureData.texture->getHeight() / std::max(1, inputFrameH);
				textureData.cols = textureData.texture->getWidth() / std::max(1, inputFrameW);
				textureData.frameWidth = inputFrameW;
				textureData.frameHeight = inputFrameH;

				if (g_game)
					g_game->applyTextureToPlayer(textureData);
			}

			ImGui::Text("Grid: %d rows × %d cols",
				textureData.rows, textureData.cols);
		}
	}


	// Loading audio
	const auto& sounds = loader.getSounds();
	if (!g_droppedFiles.empty())
	{
		for (auto& path : g_droppedFiles)
		{
			std::filesystem::path filePath(path);
			auto name = filePath.stem().string();
			auto ext = filePath.extension().string();

			std::cout << "path: " << path << std::endl;

			if (ext == ".wav" || ext == ".mp3")
			{
				auto it = sounds.find(filePath.stem().string());
				if (it == sounds.end())
				{
					throw std::runtime_error("Failed to find dropped in sound!");
				}

				std::cout << "Sound obtained: " << name << std::endl;

				// Construct Audio module
				auto audioMod = std::make_unique<Audio::EnoAudioModule>(name, ext, it->second);
				loader.storeAudioMod(std::move(audioMod));
				std::cout << "Construction done" << std::endl;
			}
		}
	}

	// Display audio on asset list



	static std::string selectedAudio;

	// Selected audio
	if (!selectedAudio.empty())
	{
		ImGui::Text("Selected: %s", selectedAudio.c_str());
	}

	g_droppedFiles.clear();
}

// Docking shi

void imGuiBeginDockspace() {
	// Get the main viewport
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Set the dockspace to cover the entire viewport
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	// Configure the dockspace window flags
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	window_flags |= ImGuiWindowFlags_NoBackground;

	// Push style vars to remove padding and borders
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	// Begin the dockspace window
	ImGui::Begin("EnoEngine Dockspace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// Create the dockspace
	ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void displayGameView() {
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// Display info text - this is now a read-only overview window
	ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Game View - Full Scene Overview (Read-Only)");
	ImGui::Separator();

	if (g_game && g_game->getGameFramebuffer()) {
		auto* fb = g_game->getGameFramebuffer();

		if (contentSize.x > 0 && contentSize.y > 0) {
			// Get fixed virtual resolution from game
			float virtualWidth = static_cast<float>(g_game->getVirtualWidth());
			float virtualHeight = static_cast<float>(g_game->getVirtualHeight());
			float virtualAspect = virtualWidth / virtualHeight;

			// Calculate aspect-ratio-preserving size to fit in the Game View
			float contentAspect = contentSize.x / contentSize.y;
			ImVec2 displaySize;
			ImVec2 offset(0, 0);

			if (contentAspect > virtualAspect) {
				// Content is wider - fit to height, add horizontal letterboxing
				displaySize.y = contentSize.y;
				displaySize.x = contentSize.y * virtualAspect;
				offset.x = (contentSize.x - displaySize.x) * 0.5f; // Center horizontally
			}
			else {
				// Content is taller - fit to width, add vertical letterboxing
				displaySize.x = contentSize.x;
				displaySize.y = contentSize.x / virtualAspect;
				offset.y = (contentSize.y - displaySize.y) * 0.5f; // Center vertically
			}

			// Draw black letterbox bars
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 windowPos = ImGui::GetCursorScreenPos();

			// Fill entire content area with black
			drawList->AddRectFilled(
				windowPos,
				ImVec2(windowPos.x + contentSize.x, windowPos.y + contentSize.y),
				IM_COL32(0, 0, 0, 255)
			);

			// Position the image with offset (centered with letterboxing)
			ImGui::SetCursorScreenPos(ImVec2(windowPos.x + offset.x, windowPos.y + offset.y));

			// Display the framebuffer texture scaled to fit (READ-ONLY - no interactions)
			ImGui::Image(
				(void*)(intptr_t)fb->getTextureID(),
				displaySize,
				ImVec2(0, 1),  // UV coordinates (flipped Y for OpenGL)
				ImVec2(1, 0)
			);

			// Note: All interactions (drag-drop, delete, prefab placement) have been moved to Editor View
			// This window is now read-only for observing the full game scene
		}
	}
}

// Draw gizmos for selected entity
void drawGizmos(ImDrawList* drawList, ImVec2 screenPos, ImVec2 entityCenter, float gizmoSize, GizmoMode mode) {
	const float handleSize = 8.0f;
	const float rotateRadius = gizmoSize * 0.6f;

	// Translate gizmo handles
	if (mode == GIZMO_TRANSLATE) {
		// Draw X axis handle (blue square to the right)
		ImVec2 xHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		drawList->AddRectFilled(
			ImVec2(xHandle.x - handleSize / 2, xHandle.y - handleSize / 2),
			ImVec2(xHandle.x + handleSize / 2, xHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, xHandle, IM_COL32(0, 150, 255, 255), 2.0f);

		// Draw Y axis handle (blue square above)
		ImVec2 yHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		drawList->AddRectFilled(
			ImVec2(yHandle.x - handleSize / 2, yHandle.y - handleSize / 2),
			ImVec2(yHandle.x + handleSize / 2, yHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, yHandle, IM_COL32(0, 150, 255, 255), 2.0f);
	}
	// Rotate gizmo
	else if (mode == GIZMO_ROTATE) {
		// Draw rotation circle
		drawList->AddCircle(entityCenter, rotateRadius, IM_COL32(0, 150, 255, 255), 32, 2.0f);

		// Draw rotation handle at top of circle
		ImVec2 rotHandle = ImVec2(entityCenter.x, entityCenter.y - rotateRadius);
		drawList->AddCircleFilled(rotHandle, handleSize / 2, IM_COL32(0, 150, 255, 255));
	}
	// Scale gizmo
	else if (mode == GIZMO_SCALE) {
		// Draw uniform scale handle (diagonal bottom-right corner - blue)
		ImVec2 uniformScaleHandle = ImVec2(entityCenter.x + gizmoSize * 0.7f, entityCenter.y + gizmoSize * 0.7f);
		drawList->AddRectFilled(
			ImVec2(uniformScaleHandle.x - handleSize / 2, uniformScaleHandle.y - handleSize / 2),
			ImVec2(uniformScaleHandle.x + handleSize / 2, uniformScaleHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, uniformScaleHandle, IM_COL32(0, 150, 255, 255), 2.0f);

		// Draw horizontal scale handle (right side - blue)
		ImVec2 xScaleHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		drawList->AddRectFilled(
			ImVec2(xScaleHandle.x - handleSize / 2, xScaleHandle.y - handleSize / 2),
			ImVec2(xScaleHandle.x + handleSize / 2, xScaleHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, xScaleHandle, IM_COL32(0, 150, 255, 255), 2.0f);

		// Draw vertical scale handle (top - blue)
		ImVec2 yScaleHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		drawList->AddRectFilled(
			ImVec2(yScaleHandle.x - handleSize / 2, yScaleHandle.y - handleSize / 2),
			ImVec2(yScaleHandle.x + handleSize / 2, yScaleHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, yScaleHandle, IM_COL32(0, 150, 255, 255), 2.0f);
	}
}

// Check if mouse is over a gizmo handle
bool isMouseOverGizmoHandle(ImVec2 mousePos, ImVec2 entityCenter, float gizmoSize, GizmoMode mode, int& handleType) {
	const float handleSize = 12.0f;
	const float rotateRadius = gizmoSize * 0.6f;

	if (mode == GIZMO_TRANSLATE) {
		// Check X handle (right)
		ImVec2 xHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		if (fabs(mousePos.x - xHandle.x) < handleSize && fabs(mousePos.y - xHandle.y) < handleSize) {
			handleType = 1;
			return true;
		}
		// Check Y handle (up)
		ImVec2 yHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		if (fabs(mousePos.x - yHandle.x) < handleSize && fabs(mousePos.y - yHandle.y) < handleSize) {
			handleType = 2;
			return true;
		}
	}
	else if (mode == GIZMO_ROTATE) {
		// Check rotation handle
		ImVec2 rotHandle = ImVec2(entityCenter.x, entityCenter.y - rotateRadius);
		float dist = sqrtf((mousePos.x - rotHandle.x) * (mousePos.x - rotHandle.x) +
			(mousePos.y - rotHandle.y) * (mousePos.y - rotHandle.y));
		if (dist < handleSize) {
			handleType = 3;
			return true;
		}
	}
	else if (mode == GIZMO_SCALE) {
		// Check horizontal scale handle (right side - handleType 4)
		ImVec2 xScaleHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		if (fabs(mousePos.x - xScaleHandle.x) < handleSize && fabs(mousePos.y - xScaleHandle.y) < handleSize) {
			handleType = 4;
			return true;
		}
		// Check vertical scale handle (top - handleType 5)
		ImVec2 yScaleHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		if (fabs(mousePos.x - yScaleHandle.x) < handleSize && fabs(mousePos.y - yScaleHandle.y) < handleSize) {
			handleType = 5;
			return true;
		}
		// Check uniform scale handle (diagonal - handleType 3)
		ImVec2 uniformScaleHandle = ImVec2(entityCenter.x + gizmoSize * 0.7f, entityCenter.y + gizmoSize * 0.7f);
		if (fabs(mousePos.x - uniformScaleHandle.x) < handleSize && fabs(mousePos.y - uniformScaleHandle.y) < handleSize) {
			handleType = 3;
			return true;
		}
	}

	handleType = 0;
	return false;
}

void displayEditorView(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager) {
	auto& entityManager = engine_.getEntityManager();
	auto& sceneEditor = engine_.getEnoEditor();
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// Display toolbar for PLAY/PAUSE/STOP
	static bool isPlaying = false;
	isPlaying = (sceneEditor.getCurrentScene().getState() != Editor::State::Stopped) ? true : false;

	if (ImGui::Button(isPlaying ? "Stop" : "Play")) {
		isPlaying = !isPlaying;
		if (isPlaying) {
			sceneEditor.playScene();
		} else {
			sceneEditor.stopScene();
			// Immediately invalidate entity cache after stopping scene
			if (g_game) g_game->markEntitiesDirty();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(sceneEditor.getCurrentScene().getState() != Editor::State::Paused ? "Pause" : "Unpause")) {
		if (sceneEditor.getCurrentScene().getState() != Editor::State::Stopped) {
			(sceneEditor.getCurrentScene().getState() == Editor::State::Playing ? sceneEditor.pauseScene() : sceneEditor.playScene());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Next")) {
		if (sceneEditor.getCurrentScene().getState() == Editor::State::Paused) {
			sceneEditor.getCurrentScene().next();
		}
	}

	ImGui::SameLine();
	ImGui::Text("|");
	ImGui::SameLine();

	// Gizmo mode selector buttons
	if (ImGui::Button("Move (Q)")) {
		currentGizmoMode = GIZMO_TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Rotate (E)")) {
		currentGizmoMode = GIZMO_ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Scale (R)")) {
		currentGizmoMode = GIZMO_SCALE;
	}

	// Keyboard shortcuts for gizmo modes
	if (ImGui::IsKeyPressed(ImGuiKey_Q)) currentGizmoMode = GIZMO_TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E)) currentGizmoMode = GIZMO_ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoMode = GIZMO_SCALE;

	ImGui::Separator();

	// Informational text
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Editor View - Use WASD to move camera");
	ImGui::Separator();

	if (g_game && g_game->getEditorFramebuffer()) {
		auto* fb = g_game->getEditorFramebuffer();

		if (contentSize.x > 0 && contentSize.y > 0) {
			// Get editor resolution from game
			float editorWidth = static_cast<float>(g_game->getEditorWidth());
			float editorHeight = static_cast<float>(g_game->getEditorHeight());
			float editorAspect = editorWidth / editorHeight;

			// Calculate aspect-ratio-preserving size to fit in the Editor View
			float contentAspect = contentSize.x / contentSize.y;
			ImVec2 displaySize;
			ImVec2 offset(0, 0);

			if (contentAspect > editorAspect) {
				displaySize.y = contentSize.y;
				displaySize.x = contentSize.y * editorAspect;
				offset.x = (contentSize.x - displaySize.x) * 0.5f;
			}
			else {
				displaySize.x = contentSize.x;
				displaySize.y = contentSize.x / editorAspect;
				offset.y = (contentSize.y - displaySize.y) * 0.5f;
			}

			// Draw black letterbox bars
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 windowPos = ImGui::GetCursorScreenPos();

			drawList->AddRectFilled(
				windowPos,
				ImVec2(windowPos.x + contentSize.x, windowPos.y + contentSize.y),
				IM_COL32(0, 0, 0, 255)
			);

			// Position the image with offset (centered with letterboxing)
			ImGui::SetCursorScreenPos(ImVec2(windowPos.x + offset.x, windowPos.y + offset.y));

			// Display the editor framebuffer texture scaled to fit
			ImGui::Image(
				(void*)(intptr_t)fb->getTextureID(),
				displaySize,
				ImVec2(0, 1),  // UV coordinates (flipped Y for OpenGL)
				ImVec2(1, 0)
			);

			// Capture the editor view min/max coordinates and store them in GameApplication
			ImVec2 imGuiImageMin = ImGui::GetItemRectMin();
			ImVec2 imGuiImageMax = ImGui::GetItemRectMax();
			g_game->setEditorViewRect({imGuiImageMin.x, imGuiImageMin.y}, {imGuiImageMax.x, imGuiImageMax.y});

			// ========== DRAG-AND-DROP TEXTURE FROM ASSET BROWSER ==========
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME"))
				{
					std::string textureName = (const char*)payload->Data;
					Resources::EnoLoader& loader = engine_.getResourceLoader();

					// Get the texture from the loader
					auto texture = loader.getTexture(textureName);
					if (texture && texture->isValid())
					{
						// Get mouse position and convert to world coordinates
						ImVec2 mousePos = ImGui::GetMousePos();
						Math::eVec2<float> worldPos = Graphics::screenToWorld(
							{ mousePos.x, mousePos.y },
							g_game->getEditorViewMin(),
							g_game->getEditorViewMax(),
							editorWidth,
							editorHeight,
							g_game->getEditorCameraX(),
							g_game->getEditorCameraY()
						);

						// Create entity at drop location
						ECSComponents::TransformComponent transform;
						transform.pos = worldPos;
						transform.rot = Math::eVec2<float>(0.0f, 0.0f);
						transform.scale = Math::eVec2<float>(1.0f, 1.0f);

						ECS::Entity newEntity = entityManager.create<ECSComponents::TransformComponent>(transform);

						// Add render component with texture
						ECSComponents::RenderComponent render;
						render.texture = texture;
						render.width = static_cast<float>(texture->getWidth());
						render.height = static_cast<float>(texture->getHeight());
						strncpy_s(render.texturePath, textureName.c_str(), sizeof(render.texturePath) - 1);
						render.texturePath[sizeof(render.texturePath) - 1] = '\0';
						render.color = Graphics::Color::White;
						newEntity.add<ECSComponents::RenderComponent>(entityManager, render);

						// Add name component
						ECSComponents::NameComponent nameComp;
						strncpy_s(nameComp.name, textureName.c_str(), sizeof(nameComp.name) - 1);
						nameComp.name[sizeof(nameComp.name) - 1] = '\0';
						newEntity.add<ECSComponents::NameComponent>(entityManager, nameComp);

						// Create animation controller
						uint32_t controllerID = engine_.getAnimationRegistry().createController();
						newEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, controllerID);

						// Get the controller and add a simple 1x1 animation
						Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(controllerID);
						if (ctrl)
						{
							// Create a single-frame animation (1x1 grid, UV 0,0 to 1,1)
							Animation::EnoAnimation singleFrameAnim = Animation::createSpriteSheetAnimation(
								texture->getWidth(),   // frame width = full texture width
								texture->getHeight(),  // frame height = full texture height
								texture->getWidth(),   // sheet width
								texture->getHeight(),  // sheet height
								0,                     // start frame
								1,                     // frame count (single frame)
								0.1f                   // frame duration (doesn't matter for single frame)
							);

							ctrl->addAnimation("idle", singleFrameAnim);
							ctrl->setAnimation("idle");
						}

						// Add to scene tree
						Node* node = sceneTree.createFromEntity(newEntity, nullptr);
						node->name_ = textureName;

						// Record creation for undo/redo
						auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
							sceneTree,
							entityManager,
							newEntity
						);
						undoManager.executeCommand(std::move(createCmd));

						std::cout << "[Drag-Drop] Created entity '" << textureName
								  << "' at world position (" << worldPos.x << ", " << worldPos.y << ")\n";

						if (g_game) g_game->markEntitiesDirty();
					}
				}
				ImGui::EndDragDropTarget();
			}
			// ========== ALL EDITOR INTERACTIONS (moved from Game View) ==========

			// Helper function to convert screen coordinates to world coordinates (accounting for camera)                                                                                               

			// auto screenToWorldCoords = [&](ImVec2 screenPos) -> ImVec2 {                      
				//ImVec2 imageMin = ImGui::GetItemRectMin
				//ImVec2 imageMax = ImGui::GetItemRectMax();                                

				// Get mouse position relative to the image (0.0 to 1.0)
				//float relX = (screenPos.x - imageMin.x) / (imageMax.x - imageMin.x);      

				//float relY = (screenPos.y - imageMin.y) / (imageMax.y - imageMin.y);                                                                                   
				//relX = std::clamp(relX, 0.0f, 1.0f);                                      
				//relY = std::clamp(relY, 0.0f, 1.0f); 
                                    
				// Convert to screen space coordinates (within the editor viewport)       
				//float screenX = relX * editorWidth;                                       
				//float screenY = relY * editorHeight;                                      

				// Add camera offset to convert screen space to world space               
				//float worldX = screenX + g_game->getEditorCameraX();                      
				//float worldY = screenY + g_game->getEditorCameraY();                      

				//return ImVec2(worldX, worldY);                                            

			//};

			// Helper function to convert world coordinates to screen coordinates (for drawing gizmos)                                                                                                  
			//auto worldToScreenCoords = [&](ImVec2 worldPos) -> ImVec2 {                       

				//ImVec2 imageMin = ImGui::GetItemRectMin();                                
				//ImVec2 imageMax = ImGui::GetItemRectMax();                                

				// Subtract camera offset to convert world space to screen space          
				//float screenX = worldPos.x - g_game->getEditorCameraX();                  
				//float screenY = worldPos.y - g_game->getEditorCameraY();                  

				// Convert to normalized coordinates (0.0 to 1.0)                         
				//float relX = screenX / editorWidth;                                       
				//float relY = screenY / editorHeight;                                                                                                             

				// Convert to actual screen position                                      
				//float finalX = imageMin.x + relX * (imageMax.x - imageMin.x);             
				//float finalY = imageMin.y + relY * (imageMax.y - imageMin.y);             

				//return ImVec2(finalX, finalY);                                            

			//};

			// --- Handle entity deletion by click ---                                        

			if (g_waitingForEntityDeletion && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {                                                                                                     

				Math::eVec2<float> worldPos = Graphics::screenToWorld(
											{ ImGui::GetMousePos().x, ImGui::GetMousePos().y },
											g_game->getEditorViewMin(),
											g_game->getEditorViewMax(),
											editorWidth,
											editorHeight,
											g_game->getEditorCameraX(),
											g_game->getEditorCameraY());

				auto clicked = getEntityUnderMouse(engine_, worldPos);
				if (clicked.has_value()) {
					std::cout << "Deleting entity: " << clicked->id()
						<< " and all children at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";

					// Get the node first before destroying
					Editor::Node* node = sceneTree.getNodeByEntityID(clicked->id());
					if (node) {
						// Create delete command (captures snapshot before deletion)
						auto deleteCmd = std::make_unique<Editor::DeleteEntityCommand>(
							sceneTree,
							entityManager,
							clicked.value()
						);

						// Execute the deletion (also adds to undo history)
						undoManager.executeCommand(std::move(deleteCmd));

						std::cout << "[DEBUG] Entity deletion completed with undo support\n";
					}

					if (g_game) g_game->markEntitiesDirty();
				}
				else {
					std::cout << "No entity found at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
				}

				g_waitingForEntityDeletion = false;
			}

			// --- Handle prefab placement ---                                                

			if (!g_selectedPrefabForPlacement.empty() && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)) {                                                                                         

				Math::eVec2<float> worldPos = Graphics::screenToWorld(
											{ ImGui::GetMousePos().x, ImGui::GetMousePos().y },
											g_game->getEditorViewMin(),
											g_game->getEditorViewMax(),
											editorWidth,
											editorHeight,
											g_game->getEditorCameraX(),
											g_game->getEditorCameraY());

				// Spawn prefab at clicked position
				spawnPrefabAt(worldPos.x, worldPos.y, g_selectedPrefabForPlacement, engine_, sceneTree, undoManager);
				g_selectedPrefabForPlacement.clear();

				std::cout << "Placed prefab at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
			}

			// --- Gizmo-based entity transformation logic ---
			// Draw gizmos for selected entity
			if (selectedEntityID != UINT32_MAX && sceneEditor.getCurrentScene().getState() == Editor::State::Stopped) {
				auto allEntities = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
				for (auto& e : allEntities) {
					if (e.id() == selectedEntityID) {
						auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);
						auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);

						// Convert entity position to screen coordinates          
						Math::eVec2<float> entityWorldPos = {tc.pos.x, tc.pos.y}; 
						Math::eVec2<float> screenPos = Graphics::worldToScreen(
														entityWorldPos,
														g_game->getEditorViewMin(),
														g_game->getEditorViewMax(),
														editorWidth,
														editorHeight,
														g_game->getEditorCameraX(),
														g_game->getEditorCameraY());

						// Convert to ImVec2 for easier logic comparison
						ImVec2 entityScreenPos = { screenPos.x, screenPos.y };

						// Draw gizmo based on current mode
						float gizmoSize = 50.0f;
						drawGizmos(drawList, windowPos, entityScreenPos, gizmoSize, currentGizmoMode);
						break;
					}
				}
			}

			// Begin gizmo interaction                                                    
			if (!g_waitingForEntityDeletion && g_selectedPrefabForPlacement.empty() &&        
				ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !isDragging && !isRotating && !isScaling 
				&& sceneEditor.getCurrentScene().getState() == Editor::State::Stopped) 
			{             
				                                                                                                       
				ImVec2 mouseScreenPos = ImGui::GetMousePos();                             
				Math::eVec2<float> worldPos = 
					Graphics::screenToWorld(
					{ mouseScreenPos.x, mouseScreenPos.y },
					g_game->getEditorViewMin(),
					g_game->getEditorViewMax(),
					editorWidth,
					editorHeight,
					g_game->getEditorCameraX(),
					g_game->getEditorCameraY());

				// Check if we have a selected entity
				if (selectedEntityID != UINT32_MAX) {
					auto allEntities = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
					for (auto& e : allEntities) {
						if (e.id() == selectedEntityID) {
							auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);                                                                                         
							Math::eVec2<float> currentEntityWorldPos = {tc.pos.x, tc.pos.y};
							Math::eVec2<float> screenPos = Graphics::worldToScreen(
								currentEntityWorldPos,
								g_game->getEditorViewMin(),
								g_game->getEditorViewMax(),
								editorWidth,
								editorHeight,
								g_game->getEditorCameraX(),
								g_game->getEditorCameraY());
							ImVec2 entityScreenPos = {screenPos.x,screenPos.y};

							int handleType = 0;
							float gizmoSize = 50.0f;

							// Check if clicking on gizmo handle
							if (isMouseOverGizmoHandle(mouseScreenPos, entityScreenPos, gizmoSize, currentGizmoMode, handleType)) {
								// Collect all affected entity IDs (parent + children) for undo
								affectedEntityIDs.clear();
								Editor::Node* node = sceneTree.getNodeByEntityID(selectedEntityID);
								if (node) {
									collectEntityIDsRecursive(node, affectedEntityIDs);
								}

								// Create command and capture old state before transformation
								if (!affectedEntityIDs.empty()) {
									pendingTransformCommand = std::make_unique<Editor::MultiTransformCommand>(
										undoManager.getEntityManager(),
										currentGizmoMode == GIZMO_TRANSLATE ? "Move" :
										(currentGizmoMode == GIZMO_ROTATE ? "Rotate" : "Scale")
									);
									pendingTransformCommand->captureOldState(affectedEntityIDs);
								}

								if (currentGizmoMode == GIZMO_ROTATE) {   

									// Start rotation (handleType 3 for rotation)                                                                                               
									isRotating = true;                
									draggedEntityID = selectedEntityID;                                                                                                         
									activeTransformAxis = 3;          
									initialAngle = atan2f(mouseScreenPos.y - entityScreenPos.y, mouseScreenPos.x - entityScreenPos.x);                                          
									initialRotation = tc.rot.x;       
									rotationBeforeDrag = tc.rot;  // Store for undo

									std::cout << "Started rotating entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";                

								}
								else if (currentGizmoMode == GIZMO_SCALE) {
									// Start scaling
									isScaling = true;
									draggedEntityID = selectedEntityID;
									activeTransformAxis = handleType; // 3=uniform, 4=X-scale, 5=Y-scale
									scaleStartPos = mouseScreenPos;
									dragStartWorldPos = { worldPos.x, worldPos.y };
									initialScale = ImVec2(tc.scale.x, tc.scale.y);
									lastScale = ImVec2(tc.scale.x, tc.scale.y);  // Initialize last scale
									scaleBeforeDrag = tc.scale;  // Store for undo
									if (handleType == 3) {
										std::cout << "Started uniform scaling entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
									else if (handleType == 4) {
										std::cout << "Started horizontal scaling entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
									else if (handleType == 5) {
										std::cout << "Started vertical scaling entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
								}
								else if (currentGizmoMode == GIZMO_TRANSLATE) {
									// Start dragging with axis constraint
									draggedEntityID = selectedEntityID;
									isDragging = true;
									activeTransformAxis = handleType; // 1=X-axis, 2=Y-axis
									dragOffset.x = worldPos.x - tc.pos.x;
									dragOffset.y = worldPos.y - tc.pos.y;
									dragStartWorldPos = { worldPos.x, worldPos.y };
									lastDragPos = ImVec2(tc.pos.x, tc.pos.y);
									transformBeforeDrag = tc.pos;  // Store for undo
									if (handleType == 1) {
										std::cout << "Started dragging entity " << selectedEntityID << " on X-axis (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
									else if (handleType == 2) {
										std::cout << "Started dragging entity " << selectedEntityID << " on Y-axis (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
								}
								break;
							}
						}
					}
				}

				// If not clicking gizmo, check if clicking on entity
				if (!isDragging && !isRotating && !isScaling) {
					auto clicked = getEntityUnderMouse(engine_, worldPos);
					if (clicked) {
						// Select entity in hierarchy when clicked in editor window
						selectedEntityID = clicked->id();
						std::cout << "Selected entity " << selectedEntityID << "\n";
					}
				}
			}

			// While transforming (drag, rotate, or scale)
			if ((isDragging || isRotating || isScaling) && ImGui::IsMouseDown(0)) {
				ImVec2 mouseScreenPos = ImGui::GetMousePos();
				Math::eVec2<float> eWorldPos = Graphics::screenToWorld(
					{ mouseScreenPos.x, mouseScreenPos.y },
					g_game->getEditorViewMin(),
					g_game->getEditorViewMax(),
					editorWidth,
					editorHeight,
					g_game->getEditorCameraX(),
					g_game->getEditorCameraY());
				ImVec2 worldPos = { eWorldPos.x, eWorldPos.y };

				auto all = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
				for (auto& e : all) {
					if (e.id() == draggedEntityID) {
						auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);

						if (isDragging) {
							// Calculate delta movement with axis constraints
							float newX = worldPos.x - dragOffset.x;
							float newY = worldPos.y - dragOffset.y;

							// Apply axis constraint based on which handle was grabbed
							if (activeTransformAxis == 1) {
								// X-axis only movement
								newY = lastDragPos.y;
							}
							else if (activeTransformAxis == 2) {
								// Y-axis only movement
								newX = lastDragPos.x;
							}

							float deltaX = newX - lastDragPos.x;
							float deltaY = newY - lastDragPos.y;

							// Move parent entity
							tc.pos.x = newX;
							tc.pos.y = newY;

							// Move all children recursively
							Editor::Node* draggedNode = sceneTree.getNodeByEntityID(draggedEntityID);
							if (draggedNode) {
								moveChildrenRecursive(draggedNode, deltaX, deltaY, entityManager);
							}

							// Update last position
							lastDragPos = ImVec2(newX, newY);
						}
						else if (isRotating) {
							// Calculate rotation based on mouse angle
							Math::eVec2<float> eEntityScreenPos =
								Graphics::worldToScreen(
									tc.pos,
									g_game->getEditorViewMin(),
									g_game->getEditorViewMax(),
									editorWidth,
									editorHeight,
									g_game->getEditorCameraX(),
									g_game->getEditorCameraY()
								);

							ImVec2 entityScreenPos = {eEntityScreenPos.x, eEntityScreenPos.y};
							float currentAngle = atan2f(mouseScreenPos.y - entityScreenPos.y, mouseScreenPos.x - entityScreenPos.x);
							float newRotation = initialRotation + (currentAngle - initialAngle);
							float deltaAngle = newRotation - tc.rot.x;
							tc.rot.x = newRotation;

							// Rotate all children recursively around parent position
							Editor::Node* rotatedNode = sceneTree.getNodeByEntityID(draggedEntityID);
							if (rotatedNode) {
								rotateChildrenRecursive(rotatedNode, deltaAngle, { tc.pos.x, tc.pos.y }, entityManager);
							}
						}
						else if (isScaling) {
							// Calculate scale based on which handle was grabbed
							float newScaleX = tc.scale.x;
							float newScaleY = tc.scale.y;

							if (activeTransformAxis == 3) {
								// Uniform scaling using diagonal handle
								Math::eVec2<float> entityWorldPosForScaling = {tc.pos.x, tc.pos.y};
								ImVec2 entityScreenPosForScaling = {
									Graphics::worldToScreen(
									entityWorldPosForScaling,
									g_game->getEditorViewMin(),
									g_game->getEditorViewMax(),
									editorWidth,
									editorHeight,
									g_game->getEditorCameraX(),
									g_game->getEditorCameraY()).x,
									Graphics::worldToScreen(
									entityWorldPosForScaling,
									g_game->getEditorViewMin(),
									g_game->getEditorViewMax(),
									editorWidth,
									editorHeight,
									g_game->getEditorCameraX(),
									g_game->getEditorCameraY()).y
								};

								float initialDist = sqrtf((scaleStartPos.x - entityScreenPosForScaling.x) * (scaleStartPos.x - entityScreenPosForScaling.x)
														+ (scaleStartPos.y - entityScreenPosForScaling.y) * (scaleStartPos.y - entityScreenPosForScaling.y));

								float currentDist = sqrtf((mouseScreenPos.x - entityScreenPosForScaling.x) * (mouseScreenPos.x - entityScreenPosForScaling.x)
														+ (mouseScreenPos.y - entityScreenPosForScaling.y) * (mouseScreenPos.y - entityScreenPosForScaling.y));

								float scaleFactor = currentDist / std::max(initialDist, 1.0f);

								newScaleX = initialScale.x * scaleFactor;
								newScaleY = initialScale.y * scaleFactor;
							}
							else if (activeTransformAxis == 4) {
								// Horizontal scaling only (X-axis)
								float deltaX = worldPos.x - dragStartWorldPos.x;
								auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);
								float scaleChange = deltaX / std::max(dc.width, 1.0f);
								newScaleX = std::max(0.1f, initialScale.x + scaleChange);
								newScaleY = tc.scale.y;  // Keep Y unchanged
							}
							else if (activeTransformAxis == 5) {
								// Vertical scaling only (Y-axis)
								float deltaY = worldPos.y - dragStartWorldPos.y;
								auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);
								float scaleChange = -deltaY / std::max(dc.height, 1.0f);
								newScaleX = tc.scale.x;  // Keep X unchanged
								newScaleY = std::max(0.1f, initialScale.y + scaleChange);
							}

							// Calculate scale factors (delta)
							float scaleFactorX = newScaleX / std::max(lastScale.x, 0.01f);
							float scaleFactorY = newScaleY / std::max(lastScale.y, 0.01f);

							// Update parent scale
							tc.scale.x = newScaleX;
							tc.scale.y = newScaleY;
							lastScale = ImVec2(newScaleX, newScaleY);

							// Update DrawInfo width and height to reflect the scale
							if (e.has<ECSComponents::DrawInfoComponent>(entityManager)) {
								auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);
								dc.width = dc.width * scaleFactorX;
								dc.height = dc.height * scaleFactorY;

								// Auto-update collision component if it exists
								if (e.has<ECSComponents::CollisionComponent>(entityManager)) {
									auto& collision = e.get<ECSComponents::CollisionComponent>(entityManager);
									if (collision.type == ECSComponents::ColliderType::CIRCLE) {
										collision.data.circle.radius = dc.width * 0.5f;
									} else if (collision.type == ECSComponents::ColliderType::RECTANGLE) {
										collision.data.rectangle.width = dc.width;
										collision.data.rectangle.height = dc.height;
									}
								}
							}

							// Scale all children recursively from parent position
							Editor::Node* scaledNode = sceneTree.getNodeByEntityID(draggedEntityID);
							if (scaledNode) {
								scaleChildrenRecursive(scaledNode, scaleFactorX, scaleFactorY, { tc.pos.x, tc.pos.y }, entityManager);
							}
						}

						if (g_game)
							g_game->markEntitiesDirty();
						break;
					}
				}
			}

			// End transformation
			if ((isDragging || isRotating || isScaling) && ImGui::IsMouseReleased(0)) {
				std::cout << "Finished transformation on entity " << draggedEntityID << "\n";

				// Use MultiTransformCommand if we have multiple entities affected
				if (pendingTransformCommand && !affectedEntityIDs.empty()) {
					// Capture the new state after transformation
					pendingTransformCommand->captureNewState(affectedEntityIDs);

					// Execute the command through undo manager
					undoManager.executeCommand(std::move(pendingTransformCommand));
					std::cout << "Recorded multi-entity transform for " << affectedEntityIDs.size() << " entities\n";

					affectedEntityIDs.clear();
				}
				else {
					// Fallback to single entity undo (shouldn't normally happen)
					auto allEntities = entityManager.findEntity<ECSComponents::TransformComponent>();
					for (auto& e : allEntities) {
						if (e.id() == draggedEntityID) {
							auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);

							if (isDragging) {
								undoManager.recordMove(e, transformBeforeDrag, tc.pos);
							}
							else if (isRotating) {
								undoManager.recordRotation(e, rotationBeforeDrag, tc.rot);
							}
							else if (isScaling) {
								undoManager.recordScale(e, scaleBeforeDrag, tc.scale);
							}
							break;
						}
					}
				}

				// Clear transformation state
				isDragging = false;
				isRotating = false;
				isScaling = false;
				activeTransformAxis = 0;
				draggedEntityID = 0;
			}

			// Tooltips for different interaction modes
			if (ImGui::IsItemHovered()) {
				if (g_waitingForEntityDeletion) {
					ImGui::BeginTooltip();
					ImGui::Text("Click on an entity to delete it");
					ImGui::EndTooltip();
				}
				else if (!g_selectedPrefabForPlacement.empty()) {
					ImGui::BeginTooltip();
					ImGui::Text("Click to place: %s", g_selectedPrefabForPlacement.c_str());
					ImGui::EndTooltip();
				}
			}
		}
	}
}

void imGuiSetupDefaultLayout() { // Change this if you wanna edit the amount of space the docking takes
	static bool firstRun = true;
	if (!firstRun) return;
	firstRun = false;

	ImGuiID dockspace_id = ImGui::GetID("MainDockspace");

	// Clear any existing layout
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

	// Split the dockspace into sections
	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
	ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
	ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.32f, nullptr, &dock_main_id);

	// Split right side vertically to have Assets on top and Game View on bottom
	ImGuiID dock_id_right_top = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.5f, nullptr, &dock_id_right);

	// Dock windows to their respective nodes
	// Left side - all three windows docked to same node will appear as tabs
	ImGui::DockBuilderDockWindow("Heirarchy", dock_id_left);
	ImGui::DockBuilderDockWindow("Prefabs", dock_id_left);
	ImGui::DockBuilderDockWindow("Edit", dock_id_left);
	ImGui::DockBuilderDockWindow("Scene", dock_id_left);

	// Center - Editor View (zoomed in, WASD camera control)
	ImGui::DockBuilderDockWindow("Editor View", dock_main_id);

	// Right side top - Assets Browser
	ImGui::DockBuilderDockWindow("Entity Inspector", dock_id_right_top);
	ImGui::DockBuilderDockWindow("Assets Presets", dock_id_right_top);

	// Right side bottom - Game View (full scene)
	ImGui::DockBuilderDockWindow("Game View", dock_id_right);

	// Bottom - FPS
	ImGui::DockBuilderDockWindow("FPS", dock_id_bottom);
	ImGui::DockBuilderDockWindow("Assets Browser", dock_id_bottom);
	ImGui::DockBuilderDockWindow("Audio Browser", dock_id_bottom);

	ImGui::DockBuilderFinish(dockspace_id);

	std::cout << "Default dockspace layout applied" << std::endl;
}

void imGuiEndDockspace() {
	ImGui::End();
}

void imGuiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


