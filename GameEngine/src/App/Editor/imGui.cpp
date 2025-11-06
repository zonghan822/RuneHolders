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
#include "imGui.hpp"
#include "game_logic.hpp"
#include "player/player_config.hpp"
#include "player/player_archetype_config.hpp"

#undef min
#undef max

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

// Hierarchy tree
static GameInstance::EnoSceneTree sceneTree_;

// Red Indicator
float indicatorX_ = 600.0f;
float indicatorY_ = 425.0f;
float moveSpeed_ = 200.0f; // pixels per second


// Option Menu
const char* shapes[] = { "Rectangle" };
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

// Prefab placement
static std::string g_selectedPrefabForPlacement = "";

// Entity deletion by click
static bool g_waitingForEntityDeletion = false;
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


void imGuiDisplay(Engine::EnoEngine& engine_) {
	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// get engine systems
	auto& inputManager = engine_.getInputManager();
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

	// (kim) get delta time for smooth movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;
	float moveDelta = moveSpeed_ * deltaTime;

	// (kim) changed to ARROW KEYS for indicator control 
	if (inputManager.isKeyPressed(GLFW_KEY_UP)) indicatorY_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_DOWN)) indicatorY_ += moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_LEFT)) indicatorX_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_RIGHT)) indicatorX_ += moveDelta;

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

	// Edit (Hierarchy)
	ImGui::Begin("Edit", nullptr, ImGuiWindowFlags_NoCollapse);
	displayEditTab(engine_);
	displayAllEntitiesTab(engine_);
	ImGui::End();

	//
	ImGui::Begin("Heirarchy", nullptr, ImGuiWindowFlags_NoCollapse);
	displayHierarchyTab(engine_, sceneTree_);
	ImGui::End();

	// Prefabs
	ImGui::Begin("Prefabs", nullptr, ImGuiWindowFlags_NoCollapse);
	displayPrefabsTab(engine_);
	ImGui::End();

	// Main Controls
	ImGui::Begin("Main Controls", nullptr, ImGuiWindowFlags_NoCollapse);
	displayMainTab(engine_);
	ImGui::End();

	// Game View (full scene, bottom-right)
	ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayGameView(engine_);
	ImGui::End();

	// Editor View (zoomed in, center - replaces old Game View behavior)
	ImGui::Begin("Editor View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayEditorView(engine_);
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

void displayMainTab(Engine::EnoEngine& engine_) {

	auto& entityManager = engine_.getEntityManager();

	//=======================PLAYER CONTROLS =======================
	ImGui::Text("=== PLAYER CONTROLS ===");
	ImGui::Text("WASD to move player");

	if (!playerSpawned_) {
		if (ImGui::Button("Spawn Player")) {

			// Load player configuration from file
			Player::PlayerConfig playerConfig;
			playerConfig.LoadFromFile(CONFIG_DIR_PATH "player/player_config.txt");

			// Create complete player entity using the config
			ECS::Entity player = Player::CreatePlayerEntityFromConfig(
				engine_.getEntityManager(),
				Math::eVec2<float>{indicatorX_, indicatorY_},
				playerConfig
			);

			// Store player ID for reference
			playerEntityID_ = player.id();
			playerSpawned_ = true;

			std::cout << "Player spawned at (" << indicatorX_ << ", " << indicatorY_ << ")\n";
		}
	}
	else {
		ImGui::Text("Player Active (ID: %u)", playerEntityID_);
		if (ImGui::Button("Destroy Player")) {
			if (playerSpawned_ == true) {
				engine_.getEntityManager().destroyEntity(playerEntityID_);
			}
			playerSpawned_ = false;
			if (g_game) g_game->markEntitiesDirty();
			std::cout << "Player destroyed\n";
		}
	}
	ImGui::Separator();

	// ==================== SPAWN POSITION INDICATOR ====================
	ImGui::Text("=== SPAWN POSITION INDICATOR ===");
	ImGui::Text("Position (%.1f, %.1f)", indicatorX_, indicatorY_);
	ImGui::DragFloat("Move Speed", &moveSpeed_, 10.0f, 50.0f, 500.0f);
	//collision visualization toggle
	ImGui::Checkbox("Show Colliders", &showColliders_);

	// entity spawn settings
	ImGui::Text("Entity Spawn Settings");
	ImGui::DragFloat("Entity Width", &eWidth, 0.5f, 1.0f, 100.0f);
	ImGui::DragFloat("Entity Height", &eHeight, 0.5f, 1.0f, 100.0f);
	//dropdown menu
	ImGui::Combo("Shape", &eShapeIndex, shapes, IM_ARRAYSIZE(shapes));
	ImGui::Combo("Color", &eColor, colors, IM_ARRAYSIZE(colors));


	//collider settings
	ImGui::Text("Collider Settings");
	ImGui::Combo("ColliderType", &selectedCollider, colliderShapes, IM_ARRAYSIZE(colliderShapes));

	if (selectedCollider == 1) { // circle
		ImGui::DragFloat("Collider Width", &colliderRadius, 0.5f, 1.0f, 100.0f);
	}
	else if (selectedCollider == 2) { // rectangle
		ImGui::DragFloat("Collider Width", &colliderWidth, 0.5f, 1.0f, 100.0f);
		ImGui::DragFloat("Collider Height", &colliderHeight, 0.5f, 1.0f, 100.0f);
	}

	ImGui::Separator();

	//Spawn Button (with optional collider)
	if (ImGui::Button("Spawn Entity")) {
		// Calculate center position from top-left for transform
		float centerX = indicatorX_ + (eWidth * 0.5f);
		float centerY = indicatorY_ + (eHeight * 0.5f);

		// ALWAYS add TransformComponent (required for drag-drop functionality)
		ECSComponents::TransformComponent transform{
			{centerX, centerY},
			{0.0f, 0.0f},
			{1.0f, 1.0f}
		};

		// RenderComponent stores only rendering properties (no position!)
		ECSComponents::RenderComponent renderComp{ eWidth, eHeight, shapes[eShapeIndex], eColor };

		// Create entity with both Transform and DrawInfo components
		ECS::Entity e = entityManager.create<ECSComponents::TransformComponent>(transform);
		e.add<ECSComponents::DrawInfoComponent>(entityManager, renderComp);
		e.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("John"));

		// Add collider if selected
		if (selectedCollider != 0) {
			// Add appropriate collider
			if (selectedCollider == 1) {//circle
				e.add<ECSComponents::CollisionComponent>(entityManager, ECSComponents::CollisionComponent(colliderRadius));
				if (e.has<ECSComponents::DrawInfoComponent>(entityManager)) {
					// DrawInfo already set above (now uses only width, height, shape, color - no position)
				}
			}
			else if (selectedCollider == 2) { // rectangle
				e.add<ECSComponents::CollisionComponent>(entityManager, ECSComponents::CollisionComponent(colliderWidth, colliderHeight));
				if (e.has<ECSComponents::DrawInfoComponent>(entityManager)) {
					// DrawInfo already set above (now uses only width, height, shape, color - no position)
				}

				// Add PhysicsComponent to make the entity pushable
				ECSComponents::PhysicsComponent physicsComp(1.5f);  // Default mass: slightly heavier than player
				physicsComp.drag = 0.5f;                  // Moderate friction when pushed
				physicsComp.restitution = 0.0f;           // No bounce
				physicsComp.isKinematic = false;          // Can be pushed (not static)
				physicsComp.useGravity = false;           // No gravity for top-down game
				e.add<ECSComponents::PhysicsComponent>(entityManager, physicsComp);

				std::cout << "PhysicsComponent added - entity is now pushable!\n";

				std::cout << "Spawned entity WITH collider\n";
			}
			else {
				std::cout << "Spawned entity without collider\n";
			}
		}

		

		if (g_game) g_game->markEntitiesDirty();
		(void)e; // to silence compiler warning for unused param
	}
	// NOTE: Drag-drop logic moved to Game View window to work with framebuffer coordinates


	//Clone Button
	if (ImGui::Button("Spawn 2500 Entities")) {
		for (size_t i = 0; i < 2500; i++) {
			float randX = randomFloat(0.0f, 1200.0f); //screen width
			float randY = randomFloat(0.0f, 900.0f); // screen height
			float randScalar = randomFloat(0, 10.f);
			float randW = eWidth + randScalar;
			float randH = eHeight + randScalar;
			Color randC = (randomFloat(0, 1.0f) < 0.5f ? Color::Green : Color::Red);

			// Calculate center position for transform
			float centerX = randX + (randW * 0.5f);
			float centerY = randY + (randH * 0.5f);

			// Create entity with TransformComponent so it can be dragged
			ECSComponents::TransformComponent transform{
				{centerX, centerY},
				{0.0f, 0.0f},
				{1.0f, 1.0f}
			};
			ECS::Entity e = engine_.getEntityManager().create<ECSComponents::TransformComponent>(transform);

			// Add DrawInfoComponent
			ECSComponents::RenderComponent renderComp{ randW, randH, shapes[eShapeIndex], randC };
			e.add<ECSComponents::DrawInfoComponent>(entityManager, renderComp);
			e.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("John"));
		}
		entity_count += 2500;
		if (g_game) g_game->markEntitiesDirty();
		std::cout << "Spawned " << entity_count << " Entities\n";
	}


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

void displayEditTab(Engine::EnoEngine& engine_) {

	auto& entityManager = engine_.getEntityManager();
	const char* filters[] = { "Architype" , "Last Spawned", "Mouse Select", "All" };
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
					e.destroy(entityManager);
					selectedEntityID = UINT32_MAX;
					deletedSelected = true;
					std::cout << "Deleted selected entity\n";
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
			a.destroy(entityManager);
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

ECS::Entity spawnPrefabAt(float x, float y, std::string name, Engine::EnoEngine& engine)
{
	auto& entityManager = engine.getEntityManager();

	const PrefabData& prefab = prefabRegistry[name];

	// Treat x,y as the CENTER of where the user clicked
	float centerX = x;
	float centerY = y;

	// create the entity
	ECS::Entity e = entityManager.create<>();

	// attach components
	e.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent(prefab.name.c_str()));

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
		prefab.shape == 0 ? "Circle" : "Rectangle",
		prefab.color
	};
	e.add<ECSComponents::DrawInfoComponent>(entityManager, draw);

	// collider (optional)
	if (prefab.hasCollider) {
		e.add<ECSComponents::CollisionComponent>(
			entityManager,
			ECSComponents::CollisionComponent(prefab.width, prefab.height));
	}

	if (prefab.name == "ExitButton")
	{
		ECSComponents::VisibilityComponent visibility;
		visibility.visible = true;
		e.add<ECSComponents::VisibilityComponent>(entityManager, visibility);

		e.add<ECSComponents::ButtonComponent>(entityManager);
	}

	// update render cache
	if (g_game) g_game->markEntitiesDirty();

	std::cout << "Spawned prefab '" << prefab.name
		<< "' at (" << x << ", " << y << ")\n";

	return e;
}

void displayPrefabsTab(Engine::EnoEngine& engine_) {

	auto& entityManager = engine_.getEntityManager();

	static PrefabData currentPrefab{};  // local editable prefab
	static char prefabName[64] = "";
	static bool hasCollider = false;
	static int shapeIndex = Shapes::Rect;
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
				// Found the entity - display inspector immediately
				displayEntityInspector(entityManager, e);
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

void displayHierarchyTab(Engine::EnoEngine& engine_, GameInstance::EnoSceneTree& sceneTree)
{
	ImGui::Text("=== Scene Hierarchy ===");
	ImGui::Separator();

	if (ImGui::Button("Add Root Node")) {
		auto& entityManager = engine_.getEntityManager();
		ECS::Entity newEntity = entityManager.create<
			ECSComponents::TransformComponent,
			ECSComponents::DrawInfoComponent,
			ECSComponents::NameComponent
		>();

		sceneTree.addEntityNode(newEntity);
		if (g_game) g_game->markEntitiesDirty();
	}

	ImGui::SameLine();

	static uint32_t parentToAdd = 1;
	ImGui::InputInt("Parent ID", (int*)&parentToAdd);

	if (ImGui::Button("Add Child")) {
		auto& entityManager = engine_.getEntityManager();
		ECS::Entity childEntity = entityManager.create<
			ECSComponents::TransformComponent,
			ECSComponents::DrawInfoComponent,
			ECSComponents::NameComponent
		>();

		sceneTree.addEntityNode(childEntity);
		if (g_game) g_game->markEntitiesDirty();

		sceneTree.setParent(childEntity.id(), parentToAdd);
	}


	ImGui::Separator();

	// --- Delete Node Button ---
	if (ImGui::Button("Delete Selected Node")) {
			auto& em = engine_.getEntityManager();
			if (sceneTree.getNode(selectedEntityID)) {
				std::cout << "[DEBUG] Deleting node " << selectedEntityID << " from hierarchy\n";
				sceneTree.removeNodeAndDestroyEntity(selectedEntityID, em);
				selectedEntityID = 0;

				if (g_game)
					g_game->markEntitiesDirty();
			}
			else {
				std::cout << "[WARN] Selected node " << selectedEntityID << " not found in scene tree\n";
			}
	}


	ImGui::Separator();

	const auto& allNodes = sceneTree.getAllNodes();
	bool anyNode = false;
	for (const auto& [id, node] : allNodes) {
		if (node.parentID == 0) {
			drawNodeRecursive(sceneTree, id);
			anyNode = true;
		}
	}

	if (!anyNode)
		ImGui::TextDisabled("(no nodes to display)");

	ImGui::Separator();

	if (ImGui::Button("Print Tree to Terminal")) {
		const auto& all = sceneTree.getAllNodes();
		for (auto& [id, node] : all) {
			std::cout << "Entity " << node.entityID
				<< " | Parent: " << node.parentID
				<< " | Children: ";
			for (uint32_t c : node.children) std::cout << c << " ";
			std::cout << "\n";
		}
	}
}

void displayEntityInspector(ECS::EntityManager& entMng, ECS::Entity& entity)
{
	// Note: Entity ID 0 is valid, so don't early return based on ID
	ImGui::Begin("Entity Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Entity ID: %u", entity.id());

	// Debug output to console when inspector opens
	static uint32_t lastInspectedID = UINT32_MAX;
	if (lastInspectedID != entity.id()) {
		lastInspectedID = entity.id();
		std::cout << "\n[INSPECTOR DEBUG] Inspecting Entity ID: " << entity.id() << "\n";
	}

	// --- NameComponent ---
	if (entity.has<ECSComponents::NameComponent>(entMng)) {
		auto& nameComp = entity.get<ECSComponents::NameComponent>(entMng);

		char buffer[32];
		strncpy_s(buffer, nameComp.name, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';

		ImGui::Text("Name: %s", nameComp.name);

	}

	// --- TransformComponent ---
	if (entity.has<ECSComponents::TransformComponent>(entMng)) {
		auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
		ImGui::SeparatorText("Transform");
		ImGui::DragFloat2("Position", (float*)&transform.pos, 0.5f);
		ImGui::DragFloat2("Scale", (float*)&transform.scale, 0.05f, 0.1f, 10.0f);
	}

	// --- PhysicsComponent ---
	if (entity.has<ECSComponents::PhysicsComponent>(entMng)) {
		auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);
		ImGui::SeparatorText("Physics");
		ImGui::DragFloat2("Velocity", (float*)&physics.velocity, 0.5f);
		ImGui::DragFloat("Mass", &physics.mass, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Drag", &physics.drag, 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Use Gravity", &physics.useGravity);
	}

	// --- DrawInfoComponent ---
	if (entity.has<ECSComponents::DrawInfoComponent>(entMng)) {
		auto& draw = entity.get<ECSComponents::DrawInfoComponent>(entMng);

		// Debug output to console - always print to catch changes
		std::cout << "[INSPECTOR] Entity " << entity.id()
				  << " DrawInfo: width=" << draw.width
				  << ", height=" << draw.height
				  << ", shape='" << draw.shape << "'"
				  << ", color=(" << draw.color.r << "," << draw.color.g << "," << draw.color.b << "," << draw.color.a << ")\n";

		ImGui::SeparatorText("Draw Info");
		ImGui::DragFloat("Width", &draw.width, 0.5f, 1.0f, 500.0f);
		ImGui::DragFloat("Height", &draw.height, 0.5f, 1.0f, 500.0f);

		// Display shape (read-only for now)
		ImGui::Text("Shape: %s", draw.shape);

		// Display color as RGBA values and visual indicator
		ImVec4 colorVec(draw.color.r, draw.color.g, draw.color.b, draw.color.a);

		// Identify common colors by their RGB values
		if (draw.color.r == 0.0f && draw.color.g == 1.0f && draw.color.b == 0.0f) {
			ImGui::Text("Color: Green");
		} else if (draw.color.r == 1.0f && draw.color.g == 0.0f && draw.color.b == 0.0f) {
			ImGui::Text("Color: Red");
		} else if (draw.color.r == 0.0f && draw.color.g == 0.0f && draw.color.b == 1.0f) {
			ImGui::Text("Color: Blue");
		} else if (draw.color.r == 1.0f && draw.color.g == 1.0f && draw.color.b == 1.0f) {
			ImGui::Text("Color: White");
		} else if (draw.color.r == 0.0f && draw.color.g == 0.0f && draw.color.b == 0.0f) {
			ImGui::Text("Color: Black");
		} else {
			ImGui::Text("Color: Custom (%.2f, %.2f, %.2f, %.2f)",
				draw.color.r, draw.color.g, draw.color.b, draw.color.a);
		}
		ImGui::ColorButton("##color", colorVec, 0, ImVec2(20, 20));

		// Display texture info if present
		if (draw.texture != nullptr && draw.texture->isValid()) {
			ImGui::Text("Texture ID: %d", draw.texID);
			ImGui::Image((void*)(intptr_t)draw.texture->getID(), ImVec2(64, 64));
		} else {
			ImGui::TextDisabled("No texture assigned");
		}
	}

	// --- CollisionComponent ---
	if (entity.has<Collision::CollisionComponent>(entMng)) {
		auto& collider = entity.get<Collision::CollisionComponent>(entMng);
		ImGui::SeparatorText("Collider");

		// Collider type selector
		const char* colliderTypes[] = { "Circle", "Rectangle", "Line", "Point" };
		int currentType = static_cast<int>(collider.type);
		if (ImGui::Combo("Type", &currentType, colliderTypes, IM_ARRAYSIZE(colliderTypes))) {
			collider.type = static_cast<Collision::ColliderType>(currentType);
			if (g_game) g_game->markEntitiesDirty();
		}
	}
	ImGui::End();
}

void drawNodeRecursive(GameInstance::EnoSceneTree& sceneTree_, uint32_t nodeID)
{
	using namespace GameInstance;
	GameInstance::Node* node = sceneTree_.getNode(nodeID);
	if (!node) return;

	std::string label = "Entity " + std::to_string(node->entityID);

	// If node has children -> use TreeNodeEx
	if (node->children.empty()) {
		bool selected = (selectedEntityID == node->entityID);
		if (ImGui::Selectable(label.c_str(), selected))
			selectedEntityID = node->entityID;
	}
	else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (selectedEntityID == node->entityID)
			flags |= ImGuiTreeNodeFlags_Selected;

		bool open = ImGui::TreeNodeEx(label.c_str(), flags);
		if (ImGui::IsItemClicked())
			selectedEntityID = node->entityID;

		if (open) {
			for (uint32_t childID : node->children)
				drawNodeRecursive(sceneTree_, childID);
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
		g_droppedFiles.clear();
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

void displayGameView([[maybe_unused]] Engine::EnoEngine& engine_) {
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

void displayEditorView(Engine::EnoEngine& engine_) {
	auto& entityManager = engine_.getEntityManager();
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// Display toolbar for PLAY/PAUSE/STOP (moved from Game View)
	static bool isPlaying = false;
	if (g_game->getEditorState() == Playing)
	{
		isPlaying = true;
	}
	else
	{
		isPlaying = false;
	}

	if (ImGui::Button(isPlaying ? "Stop" : "Play")) {
		isPlaying = !isPlaying;
		g_game->setEditorState(isPlaying ? Playing : Stopped);
	}
	ImGui::SameLine();
	if (ImGui::Button(g_game->getEditorState() == Playing || g_game->getEditorState() == Stopped ? "Pause" : "Resume")) {
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
			} else {
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
						<< " at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
					clicked->destroy(entityManager);
					if (g_game) g_game->markEntitiesDirty();
				} else {
					std::cout << "No entity found at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
				}

				g_waitingForEntityDeletion = false;
			}

			// --- Handle prefab placement ---
			if (!g_selectedPrefabForPlacement.empty() && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());

				spawnPrefabAt(worldPos.x, worldPos.y, g_selectedPrefabForPlacement, engine_);
				g_selectedPrefabForPlacement.clear();

				std::cout << "Placed prefab at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
			}

			// --- Entity drag-drop logic ---
			// Begin drag
			if (!g_waitingForEntityDeletion && g_selectedPrefabForPlacement.empty() &&
				ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !isDragging) {
				ImVec2 worldPos = screenToWorldCoords(ImGui::GetMousePos());
				auto clicked = getEntityUnderMouse(engine_, worldPos);
				if (clicked) {
					draggedEntityID = clicked->id();
					isDragging = true;

					auto& tc = clicked->get<ECSComponents::TransformComponent>(entityManager);
					dragOffset.x = worldPos.x - tc.pos.x;
					dragOffset.y = worldPos.y - tc.pos.y;

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
						tc.pos.x = worldPos.x - dragOffset.x;
						tc.pos.y = worldPos.y - dragOffset.y;

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
	ImGui::DockBuilderDockWindow("Main Controls", dock_id_left);
	ImGui::DockBuilderDockWindow("Prefabs", dock_id_left);
	ImGui::DockBuilderDockWindow("Edit", dock_id_left);

	// Center - Editor View (zoomed in, WASD camera control)
	ImGui::DockBuilderDockWindow("Editor View", dock_main_id);

	// Right side top - Assets Browser
	ImGui::DockBuilderDockWindow("Assets Browser", dock_id_right_top);

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


