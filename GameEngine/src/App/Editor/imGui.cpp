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

#undef min
#undef max

// Forward declaration for Editor namespace
namespace Editor {
	struct Node;
}

using Editor::Node;

#pragma region Game Variables

extern GameApplication game;

// Collision system
std::unique_ptr<Collision::CollisionSystem> collisionSystem_;
std::unique_ptr<Player::PlayerSystem> playerSystem_;
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

// Prefab placement
static std::string g_selectedPrefabForPlacement = "";

// Entity deletion by click
static bool g_waitingForEntityDeletion = false;

// Audio filesystems
static std::vector<std::string> audioAssets;

#pragma endregion

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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingNoSplit = false;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void imGuiDisplay(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree) {
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
		configLoaded = playerConfig.LoadFromFile(CONFIG_DIR_PATH "player/player_config.txt");
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
	displayHierarchyTab(engine_, sceneTree);
	ImGui::End();

	// Scene select
	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse);
	displaySceneTab(sceneTree);
	ImGui::End();

	// Audio select
	ImGui::Begin("Audio Browser", nullptr, ImGuiWindowFlags_NoCollapse);
	displayAudioTab(engine_);
	ImGui::End();

	// Edit tab 
	ImGui::Begin("Edit", nullptr, ImGuiWindowFlags_NoCollapse);
	displayEditTab(engine_, sceneTree);
	displayAllEntitiesTab(engine_);
	ImGui::End();

	// Prefabs
	ImGui::Begin("Prefabs", nullptr, ImGuiWindowFlags_NoCollapse);
	displayPrefabsTab(engine_, sceneTree);
	ImGui::End();

	// Game View (full scene, bottom-right)
	ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayGameView();
	ImGui::End();

	// Editor View (zoomed in, center - replaces old Game View behavior)
	ImGui::Begin("Editor View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayEditorView(engine_, sceneTree);
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

std::optional<ECS::Entity> getEntityUnderMouse(Engine::EnoEngine& engine_, ImVec2 mouse)
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

ECS::Entity spawnPrefabAt(float x, float y, std::string name, Engine::EnoEngine& engine, Editor::EnoSceneTree& sceneTree)
{
	auto& entityManager = engine.getEntityManager();

	const PrefabData& prefab = prefabRegistry[name];

	// Treat x,y as the CENTER of where the user clicked
	float centerX = x;
	float centerY = y;

	// create the entity
	ECS::Entity e = entityManager.create<>();

	// Create unique name by appending entity ID
	std::string uniqueName = prefab.name + " #" + std::to_string(e.id());

	// attach components
	e.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent(uniqueName.c_str()));

	// transform uses center position
	ECSComponents::TransformComponent transform{
		{centerX, centerY},
		{0.0f, 0.0f},
		{1.0f, 1.0f}
	};
	e.add<ECSComponents::TransformComponent>(entityManager, transform);

	// draw info uses top-left position
	ECSComponents::RenderComponent draw{
		prefab.width, prefab.height,
		prefab.shape == 0 ? "Circle" : "Rectangle",  // 0=Circle, 1=Rectangle (matches dropdown order)
		prefab.color
	};
	e.add<ECSComponents::DrawInfoComponent>(entityManager, draw);

	// collider (optional)
	if (prefab.hasCollider) {
		e.add<ECSComponents::CollisionComponent>(
			entityManager,
			ECSComponents::CollisionComponent(prefab.width, prefab.height));
	}

	// Button component
	if (prefab.name == "ExitButton")
	{
		ECSComponents::VisibilityComponent visibility;
		visibility.visible = true;
		e.add<ECSComponents::VisibilityComponent>(entityManager, visibility);

		e.add<ECSComponents::ButtonComponent>(entityManager);
	}

	// Add entity to scene hierarchy as root node
	sceneTree.createFromEntity(e, nullptr);

	// update render cache
	if (g_game) g_game->markEntitiesDirty();

	std::cout << "Spawned prefab '" << prefab.name
		<< "' at (" << x << ", " << y << ")\n";

	return e;
}

void displayPrefabsTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree) {

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
	}

	//ImGui::Separator();

	// Load performs a "read all file in directory" now
	if (ImGui::Button("Load Existing Prefab")) {
		for (auto& entry : std::filesystem::directory_iterator("../../../assets/saves/")) {
			if (entry.path().extension() == ".txt") {
				PrefabData loaded = loadPrefab(entry.path().string());
				prefabRegistry[loaded.name] = loaded;
			}
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
			ECS::Entity e = instantiatePrefab(prefabRegistry[selectedPrefabName], entityManager);

			// Add entity to scene hierarchy as root node
			sceneTree.createFromEntity(e, nullptr);

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

void displayAllEntitiesTab(Engine::EnoEngine& engine_)
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
				displayEntityInspector(engine_, e);
				
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

void displayHierarchyTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree)
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
			playerConfig.LoadFromFile(CONFIG_DIR_PATH "player/player_config.txt");

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

			for (int i = 0; i < 1000; ++i) {
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
				renderComp.texture = walkTexture;
				strncpy_s(renderComp.texturePath, "Protagonist-Walk-Sheet", sizeof(renderComp.texturePath) - 1);
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

		// Name root nodes as "Entity 1", "Entity 2", etc.
		auto& nameComp = newEntity.get<ECSComponents::NameComponent>(entityManager);
		std::string entityName = "Entity " + std::to_string(entityCounter++);
		strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
		nameComp.name[sizeof(nameComp.name) - 1] = '\0';

		Node* createdNode = sceneTree.createFromEntity(newEntity, nullptr);

		std::cout << "[DEBUG] Add Root Node: Entity " << entityName << " created\n";
		if (attachScript && selectedScriptIndex >= 0) {
			std::wstring selectedScript = availableScripts[selectedScriptIndex];
			std::wcout << L"[DEBUG] Selected Script: " << selectedScript << std::endl;
			auto script = scriptMng.loadScript(selectedScript); // Replace selectScript() with this new call
			if (script)
				sceneTree.attachScript(createdNode, std::move(script));
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

				// Name child nodes as "Entity #" using the same counter
				auto& nameComp = childEntity.get<ECSComponents::NameComponent>(entityManager);
				std::string entityName = "Entity " + std::to_string(entityCounter++);
				strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
				nameComp.name[sizeof(nameComp.name) - 1] = '\0';

				// Get parent node and add child entity to scene tree
				Editor::Node* parentNode = sceneTree.getNodeByEntityID(selectedEntityID);
				Editor::Node* childNode = sceneTree.createFromEntity(childEntity, parentNode);

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
					<< "') to parent " << selectedEntityID << "\n";

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

void displaySceneTab(Editor::EnoSceneTree& sceneTree)
{
	// Heading
	ImGui::Text("=== Scene Select===");
	ImGui::Text("Current Scene:");

	// Get current scene
	std::string currScene = sceneTree.getSceneName();

	ImGui::Text(currScene.c_str());
	ImGui::Separator();

	// Create a container for all scene names inside my scene folder
	static std::vector<std::filesystem::path> sceneNames;
	// A toggle flag to refresh the scene list
	static bool refreshSceneName = true;
	// Buffer to change scene name
	static char scene_name[64] = "";

	ImGui::Separator();

	if (refreshSceneName)
	{
		// Clear the current container
		sceneNames.clear();
		// A string path to the assets folder
		std::string sceneSavePath = (ASSETS_DIR_PATH"saves/scenes/");

		// Check that the file is a .txt file
		try {
			for (const auto& fileEntry : std::filesystem::directory_iterator(sceneSavePath))
			{
				if (fileEntry.is_regular_file() && fileEntry.path().extension() == ".txt" && fileEntry.path().string() != (sceneSavePath + "CurrentSavedScene.txt"))
				{
					// Add file to container
					sceneNames.push_back(fileEntry.path());
					//std::cout << "Filename: " << fileEntry.path() << std::endl;
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
		if(ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			// Load scene
			std::cout << "sceneName: " << sceneTree.getSceneName() << ", filename: " << filename << std::endl;
			// Check that scene is not currently loaded
			if (sceneTree.getSceneName() != filename)
			{
				std::cout << "Loading file: " << filename << std::endl;
				sceneTree.load(ASSETS_DIR_PATH"saves/scenes/" + filename, filename);
				g_game->markEntitiesDirty();
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

						std::filesystem::path newPath = ASSETS_DIR_PATH"saves/scenes/" + newSceneName;
						std::filesystem::rename(currentPath, newPath);

						// Check if we're renaming the currently active scene
						bool isCurrentScene = (filename == sceneTree.getSceneName());

						// If we renamed the active scene, update the current scene name
						if (isCurrentScene)
						{
							sceneTree.getSceneName() = newSceneName;

							// Update CurrentSavedScene.txt
							std::ofstream currentSceneSave{ ASSETS_DIR_PATH "/saves/scenes/CurrentSavedScene.txt" };
							if (currentSceneSave.is_open())
							{
								currentSceneSave << newSceneName;
								currentSceneSave.close();
							}
						}

						// Close popup after renaming
						ImGui::CloseCurrentPopup();
						// Clear buffer
						memset(scene_name, 0, sizeof(scene_name));

						// If file rename is successful
						refreshSceneName = true;
					}
					catch(const std::filesystem::filesystem_error& e)
					{
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
						return;
					}
				}
				
			}
			// Cancel edits
			if (ImGui::Button("Cancel"))
			{
				// Close popup
				ImGui::CloseCurrentPopup();
				// In case theres some random stuff inside the input box, clear the buffer
				memset(scene_name, 0, sizeof(scene_name));
			}

			// Save scene
			if (ImGui::Button("Save scene"))
			{
				// Save scene
				sceneTree.save(ASSETS_DIR_PATH"saves/scenes/" + filename, filename);
			}

			// Delete scene
			if (ImGui::Button("Delete scene"))
			{
				try
				{
					// Remove file
					std::filesystem::remove(filePath);
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

	if (ImGui::Button("Create new scene"))
	{
		sceneTree.createEmptyScene(ASSETS_DIR_PATH"saves/scenes/");
		refreshSceneName = true;
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

	ImGui::Spacing();
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




		auto& entMng = engine_.getEntityManager();
		auto& map = entMng.getMap();

		auto it = map.find(selectedEntityID);

		if (it == map.end())
		{
			ImGui::TextDisabled("Invalid entity selected");
		}
		else if (ImGui::Button("Attach SFX to Selected Entity"))
		{
			ECS::Entity selectedEntity{ selectedEntityID }; 

			// Add component if missing
			if (!selectedEntity.has<ECSComponents::AudioComponent>(entMng)) {
				selectedEntity.add<ECSComponents::AudioComponent>(entMng);
				std::cout << "[Audio] Added AudioComponent.\n";
			}

			// Fill it
			auto& ac = selectedEntity.get<ECSComponents::AudioComponent>(entMng);

			strncpy(ac.audioMod_.name_, selectedName.c_str(), sizeof(ac.audioMod_.name_) - 1);
			strncpy(ac.audioMod_.chGroup_, "SFX", sizeof(ac.audioMod_.chGroup_) - 1);

			ac.audioMod_.sound_ = selectedSound;

			std::cout << "[Audio] Attached SFX '" << selectedName
				<< "' to entity " << selectedEntityID << "\n";
		}


	}

}

void displayEntityInspector(Engine::EnoEngine& engine_, ECS::Entity& entity)
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
		ImGui::DragFloat2("Position", (float*)&transform.pos, 0.5f);
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
		ImGui::DragFloat("Width", &draw.width, 0.5f, 1.0f, 500.0f);
		ImGui::DragFloat("Height", &draw.height, 0.5f, 1.0f, 500.0f);

		// Shape type selector (Circle or Rectangle)
		const char* shapeTypes[] = { "Circle", "Rectangle" };
		int currentShapeIndex = (strcmp(draw.shape, "Circle") == 0) ? 0 : 1;
		if (ImGui::Combo("Shape", &currentShapeIndex, shapeTypes, IM_ARRAYSIZE(shapeTypes))) {
			strncpy_s(draw.shape, shapeTypes[currentShapeIndex], sizeof(draw.shape) - 1);
			draw.shape[sizeof(draw.shape) - 1] = '\0';
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
			ImGui::DragFloat("Collider Radius", &collider.data.circle.radius, 0.5f, 1.0f, 200.0f);
		}
		else if (collider.type == ECSComponents::ColliderType::RECTANGLE) {
			ImGui::DragFloat("Collider Width", &collider.data.rectangle.width, 0.5f, 1.0f, 200.0f);
			ImGui::DragFloat("Collider Height", &collider.data.rectangle.height, 0.5f, 1.0f, 200.0f);
		}
	}
	else {
		// Allow adding a collider if the entity doesn't have one
		ImGui::SeparatorText("Collider");
		ImGui::TextDisabled("No collider attached");

		if (ImGui::Button("Add Collider")) {
			// Add a default circle collider
			entity.add<ECSComponents::CollisionComponent>(entMng, ECSComponents::CollisionComponent(10.0f));
			if (g_game) g_game->markEntitiesDirty();
		}
	}

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
		ImGui::Spacing();
		ImGui::Separator();
	}

	ImGui::Separator();

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

void displayEditorView(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree) {
	auto& entityManager = engine_.getEntityManager();
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// Display toolbar for PLAY/PAUSE/STOP (moved from Game View)
	static bool isPlaying = false;
	isPlaying = (g_game->getEditorState() == Playing) ? true : false;

	if (ImGui::Button(isPlaying ? "Stop" : "Play")) {
		isPlaying = !isPlaying;
		g_game->setEditorState(isPlaying ? Playing : Stopped);
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause")) {
		if (g_game->getEditorState() != Stopped) {
			g_game->setEditorState(g_game->getEditorState() == Playing ? Paused : Playing);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Next")) {
		if (g_game->getEditorState() == Paused) {
			g_game->setEditorState(NextKeyFrame);
		}
	}
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

			// ========== ALL EDITOR INTERACTIONS (moved from Game View) ==========

			// Helper function to convert screen coordinates to world coordinates (accounting for camera)
			auto screenToWorldCoords = [&](ImVec2 screenPos) -> ImVec2 {
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

				return ImVec2(worldX, worldY);
				};

			// --- Handle entity deletion by click ---
			if (g_waitingForEntityDeletion && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());

				auto clicked = getEntityUnderMouse(engine_, worldPos);
				if (clicked.has_value()) {
					std::cout << "Deleted entity: " << clicked->id()
						<< " and all children at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";

					// Get the node and destroy all child entities
					Editor::Node* node = sceneTree.getNodeByEntityID(clicked->id());
					if (node) {
						destroyChildrenRecursive(node, entityManager);
					}

					// Remove from scene hierarchy and destroy parent entity
					sceneTree.select(clicked->id());
					sceneTree.kill();

					if (g_game) g_game->markEntitiesDirty();
				}
				else {
					std::cout << "No entity found at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
				}

				g_waitingForEntityDeletion = false;
			}

			// --- Handle prefab placement ---
			if (!g_selectedPrefabForPlacement.empty() && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());

				// Spawn prefab at clicked position
				spawnPrefabAt(worldPos.x, worldPos.y, g_selectedPrefabForPlacement, engine_, sceneTree);
				g_selectedPrefabForPlacement.clear();

				std::cout << "Placed prefab at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
			}

			// --- Entity drag-drop logic ---
			// Begin drag
			if (!g_waitingForEntityDeletion && g_selectedPrefabForPlacement.empty() &&
				ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !isDragging && g_game->getEditorState() == Stopped) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());
				auto clicked = getEntityUnderMouse(engine_, worldPos);
				if (clicked) {
					draggedEntityID = clicked->id();
					isDragging = true;

					auto& tc = clicked->get<ECSComponents::TransformComponent>(entityManager);
					dragOffset.x = worldPos.x - tc.pos.x;
					dragOffset.y = worldPos.y - tc.pos.y;
					lastDragPos = ImVec2(tc.pos.x, tc.pos.y);

					std::cout << "Started dragging entity " << draggedEntityID << "\n";
				}
			}

			// While dragging
			if (isDragging && ImGui::IsMouseDown(0)) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());

				auto all = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
				for (auto& e : all) {
					if (e.id() == draggedEntityID) {
						auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);

						// Calculate delta movement
						float newX = worldPos.x - dragOffset.x;
						float newY = worldPos.y - dragOffset.y;
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

						if (g_game)
							g_game->markEntitiesDirty();
						break;
					}
				}
			}

			// Drop
			if (isDragging && ImGui::IsMouseReleased(0)) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());
				std::cout << "Entity " << draggedEntityID
					<< " dropped at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
				isDragging = false;
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
	ImGui::DockBuilderDockWindow("Assets Browser", dock_id_right_top);
	ImGui::DockBuilderDockWindow("Entity Inspector", dock_id_right_top);
	ImGui::DockBuilderDockWindow("Audio Browser", dock_id_right_top);

	// Right side bottom - Game View (full scene)
	ImGui::DockBuilderDockWindow("Game View", dock_id_right);

	// Bottom - FPS
	ImGui::DockBuilderDockWindow("FPS", dock_id_bottom);

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


