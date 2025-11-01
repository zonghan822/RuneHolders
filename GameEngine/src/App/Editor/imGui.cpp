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
static int             eShapeIndex = Shapes::Rectangle;
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
static uint32_t selectedEntityID;

// Drag-Drop Gizmos
static bool isDragging = false;
static uint32_t draggedEntityID = 0;
static ImVec2 dragOffset;

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

	// Apply default layout once
	static bool layoutApplied = false;
	if (!layoutApplied) {
		imGuiSetupDefaultLayout();
		layoutApplied = true;
	}

	// BEGIN DOCKSPACE
	imGuiBeginDockspace();

	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;
	float moveDelta = moveSpeed_ * deltaTime;

	auto& inputManager = engine_.getInputManager();
	if (inputManager.isKeyPressed(GLFW_KEY_UP)) indicatorY_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_DOWN)) indicatorY_ += moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_LEFT)) indicatorX_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_RIGHT)) indicatorX_ += moveDelta;

	// Update player entity
	for (ECS::Entity player : engine_.getEntityManager().findEntity<Player::PlayerComponent>()) {
		playerEntityID_ = player.getEntityID();
	}

	// FPS display throttling
	double now = ImGui::GetTime();
	if (now >= next_update) {
		shown_fps = io.Framerate;
		throttledTimes_ = systemTimes_;
		next_update = now + 0.5;
	}

	// ========== RENDER ALL WINDOWS IN ORDER ==========

	// LEFT TOP: Edit (Hierarchy)
	ImGui::Begin("Edit", nullptr, ImGuiWindowFlags_NoCollapse);
	displayEditTab(engine_);
	displayHierarchyTab(engine_);
	ImGui::End();

	// LEFT MIDDLE: Prefabs
	ImGui::Begin("Prefabs", nullptr, ImGuiWindowFlags_NoCollapse);
	displayPrefabsTab(engine_);
	ImGui::End();

	// LEFT BOTTOM: Main Controls
	ImGui::Begin("Main Controls", nullptr, ImGuiWindowFlags_NoCollapse);
	displayMainTab(engine_);
	ImGui::End();

	// CENTER: Game View
	ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayGameView(engine_);
	ImGui::End();

	// RIGHT: Assets Browser
	ImGui::Begin("Assets Browser", nullptr, ImGuiWindowFlags_NoCollapse);
	displayAssetTab(engine_);
	ImGui::End();

	// BOTTOM: FPS
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoCollapse);
	displayFPSTab(engine_);
	ImGui::End();

	// END DOCKSPACE
	imGuiEndDockspace();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void displayMainTab(Engine::EnoEngine& engine_) {
	//=======================PLAYER CONTROLS =======================
	ImGui::Text("=== PLAYER CONTROLS ===");
	ImGui::Text("WASD to move player");

	if (!playerSpawned_) {
		if (ImGui::Button("Spawn Player")) {


			// create player entity at indicator position
			ECSComponents::TransformComponent transform{
				{indicatorX_, indicatorY_},
				{0.0f, 0.0f},
				{1.0f, 1.0f}
			};

			// create player with rectangle collider (can change to circle if u want)
			Collision::CollisionComponent playerCollider(
				Player::PLAYER_COLLIDER_WIDTH,
				Player::PLAYER_COLLIDER_HEIGHT
			); // 10x10 rectangle

			// for circle: (uncomment if u want circle!)
			// Collision::CollisionComponent playerCollider(5.0f); // radius 5.0
			Player::PlayerComponent playerComp(moveSpeed_);

			// Create entity with TransformComponent first
			ECS::Entity entity = engine_.getEntityManager().create<ECSComponents::TransformComponent>(transform);

			// Store ID by finding the last created entity in the map
			// Get the entity map and find the most recent entity

			// Then add CollisionComponent
			entity.add<Collision::CollisionComponent>(playerCollider);

			// Then add PlayerComponent
			entity.add<Player::PlayerComponent>(playerComp);

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
		ECSComponents::DrawInfoComponent DIC{ indicatorX_, indicatorY_, eWidth, eHeight, shapes[eShapeIndex], eColor };
		ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);
		e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent("John"));


		// add collider if selected
		if (selectedCollider != 0) {

			// calculate center position from top-left
			float centerX = indicatorX_ + (eWidth * 0.5f);
			float centerY = indicatorY_ + (eHeight * 0.5f);

			// add transform component (required for collision)
			ECSComponents::TransformComponent transform{
				{centerX, centerY},
				{0.0f, 0.0f},
				{1.0f, 1.0f}
			};

			e.add<ECSComponents::TransformComponent>(transform);

			// then add appropriate collider
			if (selectedCollider == 1) {//circle
				e.add<Collision::CollisionComponent>(Collision::CollisionComponent(colliderRadius));
				if (e.has<ECSComponents::DrawInfoComponent>()) {
					e.set<ECSComponents::DrawInfoComponent>(ECSComponents::DrawInfoComponent{ indicatorX_, indicatorY_, eWidth, eHeight, shapes[eShapeIndex], eColor });
				}
			}
			else if (selectedCollider == 2) { // rectangle
				e.add<Collision::CollisionComponent>(Collision::CollisionComponent(colliderWidth, colliderHeight));
				if (e.has<ECSComponents::DrawInfoComponent>()) {
					e.set<ECSComponents::DrawInfoComponent>(ECSComponents::DrawInfoComponent{ indicatorX_, indicatorY_, eWidth, eHeight, shapes[eShapeIndex], eColor });
				}

			}

			std::cout << "Spawned entity WITH collider\n";
		}


		else {
			std::cout << "Spawned entity without collider\n";
			//if (e.has<ECSComponents::DrawInfoComponent>())std::cout << "drawcomponent XY: " << e.get<ECSComponents::DrawInfoComponent>() << "\n";
		}
		if (g_game) g_game->markEntitiesDirty();
		(void)e; // to silence compiler warning for unused param
	}
	// Intentional: To only be able to drag and Drop in Main Tab
	ImGuiIO& io = ImGui::GetIO();

	// --- begin drag ---
	if (!io.WantCaptureMouse && ImGui::IsMouseClicked(0) && !isDragging)
	{
		auto clicked = getEntityUnderMouse(engine_, io.MousePos);
		if (clicked)
		{
			draggedEntityID = clicked->getEntityID();
			isDragging = true;

			auto& tc = clicked->get<ECSComponents::TransformComponent>();
			dragOffset.x = io.MousePos.x - tc.pos.x;
			dragOffset.y = io.MousePos.y - tc.pos.y;
		}
	}

	// --- while dragging ---
	if (isDragging && ImGui::IsMouseDown(0))
	{
		auto all = engine_.getEntityManager().findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
		for (auto& e : all)
		{
			if (e.getEntityID() == draggedEntityID)
			{
				auto& tc = e.get<ECSComponents::TransformComponent>();
				auto& dc = e.get<ECSComponents::DrawInfoComponent>();

				// Update transform
				tc.pos.x = io.MousePos.x - dragOffset.x;
				tc.pos.y = io.MousePos.y - dragOffset.y;

				// Sync DrawInfo so renderer knows the new position
				dc.xPos = tc.pos.x - (dc.width * 0.5f);
				dc.yPos = tc.pos.y - (dc.height * 0.5f);

				if (g_game)
					g_game->markEntitiesDirty();
				break;
			}
		}
	}

	// --- drop ---
	if (isDragging && ImGui::IsMouseReleased(0))
	{
		std::cout << "Entity " << draggedEntityID
			<< " dropped at (" << io.MousePos.x << ", " << io.MousePos.y << ")\n";
		isDragging = false;
		draggedEntityID = 0;
	}


	//Clone Button
	if (ImGui::Button("Spawn 2500 Entities")) {
		for (size_t i = 0; i < 2500; i++) {
			float randX = randomFloat(0.0f, 1200.0f); //screen width 
			float randY = randomFloat(0.0f, 900.0f); // screen height
			float randScalar = randomFloat(0, 10.f);
			float randW = eWidth + randScalar;
			float randH = eHeight + randScalar;
			Color randC = (randomFloat(0, 1.0f) < 0.5f ? Color::Green : Color::Red);
			//HARDCODED(COPIED) SPAWN CODE INTO HERE
			ECSComponents::DrawInfoComponent DIC{ randX, randY, randW, randH, shapes[eShapeIndex], randC };
			ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);
			e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent("John"));
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
	size_t counter = 0;

	std::vector<ECS::Entity> EntityList = engine_.getEntityManager().findEntity<>();
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
	const char* filters[] = { "Architype" , "Last Spawned", "Mouse Select", "All" };
	static int FilterType = Filters::Architype;

	std::vector<ECS::Entity> toDelete;

	ImGui::Combo("Filters", &FilterType, filters, IM_ARRAYSIZE(filters));


	if (FilterType == Filters::Architype) {
		if (ImGui::Button("DEBUG: List Named Entities")) {
			for (ECS::Entity& e : engine_.getEntityManager().findEntity<ECSComponents::NameComponent>()) {
				auto& nameComp = e.get<ECSComponents::NameComponent>();
				std::cout << "Entity " << e.getEntityID() << " name: '" << nameComp.name << "'\n";
			}
		}


		static char searchInput[32] = ""; // to enter archetype name
		ImGui::InputTextWithHint("##Search", "Enter Name to delete...", searchInput, sizeof(searchInput));


		if (ImGui::Button("Delete Entities")) {
			std::string target = searchInput;
			bool deletedSelected = false;

			// Iterate only once through all entities that have NameComponent
			for (ECS::Entity& e : engine_.getEntityManager().findEntity<ECSComponents::NameComponent>()) {
				ECSComponents::NameComponent& nameComp = e.get<ECSComponents::NameComponent>();

				if (!deletedSelected && e.getEntityID() == selectedEntityID) {
					e.destroy();
					selectedEntityID = 0;
					deletedSelected = true;
					std::cout << "Deleted selected entity\n";
					continue;
				}

				if (nameComp.name == target) {
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
				for (auto& e : engine_.getEntityManager().findEntity<>()) {
					if (e.getEntityID() == lastID) {
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
		static bool waitingForClick = false;
		ImGuiIO& io = ImGui::GetIO();

		if (!waitingForClick) {
			if (ImGui::Button("Delete an Entity by Click")) {
				waitingForClick = true;
			}
		}
		else {
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Click anywhere to delete an entity");
			// Cancel option
			if (ImGui::Button("Cancel")) {
				waitingForClick = false;
			}

			if (!io.WantCaptureMouse && ImGui::IsMouseClicked(0)) {
				auto clicked = getEntityUnderMouse(engine_, io.MousePos);
				if (clicked.has_value()) {
					std::cout << "Deleted entity: " << clicked->getEntityID() << "\n";
					toDelete.push_back(*clicked);
					if (g_game) g_game->markEntitiesDirty();
					waitingForClick = false;
				}
			}

		}
	}
	else if (FilterType == Filters::All) {
		if (ImGui::Button("Delete ALL Entities")) {
			for (auto& e : engine_.getEntityManager().findEntity<>()) {
				toDelete.push_back(e);
			}
			playerSpawned_ = false;
		}

	}

	if (!toDelete.empty()) {
		for (auto& a : toDelete) {
			a.destroy();
		}

		// FIX: Mark cache as dirty after deletion
		if (g_game) {
			g_game->markEntitiesDirty();
		}
	}
}



std::optional<ECS::Entity> getEntityUnderMouse(Engine::EnoEngine& engine_, ImVec2 mouse)
{
	for (ECS::Entity& e : engine_.getEntityManager().findEntity<
		ECSComponents::TransformComponent,
		ECSComponents::DrawInfoComponent>())
	{
		auto& tc = e.get<ECSComponents::TransformComponent>();
		auto& dc = e.get<ECSComponents::DrawInfoComponent>();

		float halfW = dc.width * 0.5f;
		float halfH = dc.height * 0.5f;
		float left = tc.pos.x - halfW;
		float right = tc.pos.x + halfW;
		float top = tc.pos.y - halfH;
		float bottom = tc.pos.y + halfH;

		if (mouse.x >= left && mouse.x <= right &&
			mouse.y >= top && mouse.y <= bottom)
		{
			return e;
		}
	}

	return std::nullopt;
}

ECS::Entity spawnPrefabAt(float x, float y, std::string name, Engine::EnoEngine& engine)
{

	const PrefabData& prefab = prefabRegistry[name];

	// center position if your prefab uses width/height as top-left reference
	float centerX = x + prefab.width * 0.5f;
	float centerY = y + prefab.height * 0.5f;

	// create the entity
	ECS::Entity e = engine.getEntityManager().create<>();

	// attach components
	e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent(prefab.name.c_str()));

	// transform
	ECSComponents::TransformComponent transform{
		{centerX, centerY},
		{0.0f, 0.0f},
		{1.0f, 1.0f}
	};
	e.add<ECSComponents::TransformComponent>(transform);

	// draw info
	ECSComponents::DrawInfoComponent draw{
		x, y, prefab.width, prefab.height,
		prefab.shape == 0 ? "Circle" : "Rectangle",
		prefab.color
	};
	e.add<ECSComponents::DrawInfoComponent>(draw);

	// collider (optional)
	if (prefab.hasCollider) {
		e.add<Collision::CollisionComponent>(
			Collision::CollisionComponent(prefab.width, prefab.height));
	}

	// update render cache
	if (g_game) g_game->markEntitiesDirty();

	std::cout << "Spawned prefab '" << prefab.name
		<< "' at (" << x << ", " << y << ")\n";

	return e;
}

void displayPrefabsTab(Engine::EnoEngine& engine_) {

	static PrefabData currentPrefab{};  // local editable prefab
	static char prefabName[64] = "";
	static bool hasCollider = false;
	static int shapeIndex = Shapes::Rectangle;
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
			}
		}
		ImGui::EndListBox();

		// Handle prefab placement overlay
		if (selectedPrefabName.size()) {
			ImVec2 mouse = ImGui::GetMousePos();
			ImGuiIO& io = ImGui::GetIO();

			// Only place if user releases mouse outside ImGui windows
			if (!io.WantCaptureMouse && ImGui::IsMouseReleased(0)) {
				spawnPrefabAt(mouse.x, mouse.y, selectedPrefabName, engine_);
			}

			// Show tooltip only when hovering outside ImGui
			if (!io.WantCaptureMouse) {
				ImGui::SetNextWindowPos(ImVec2(mouse.x + 20, mouse.y + 20));
				ImGui::Begin("Placement Tooltip", nullptr,
					ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoInputs);
				ImGui::Text("Placing %s", selectedPrefabName.c_str());
				ImGui::End();
			}
		}
	}


	//Spawn desired prefab
	ImGui::Separator();
	if (ImGui::Button("Spawn Prefab")) {
		if (prefabRegistry.count(selectedPrefabName)) {
			ECS::Entity e = instantiatePrefab(prefabRegistry[selectedPrefabName], engine_);
			if (g_game) g_game->markEntitiesDirty();
			std::cout << "Spawned prefab: " << selectedPrefabName << "\n";

		}
	}

}

void displayHierarchyTab(Engine::EnoEngine& engine_)
{

	std::vector<ECS::Entity> NamedEntityList = engine_.getEntityManager().findEntity<ECSComponents::NameComponent>();

	ImGui::Text("Total Entities: %d", static_cast<int>(NamedEntityList.size()));
	ImGui::Separator();

	// Iterate through each entity
	for (ECS::Entity& e : NamedEntityList)
	{
		const auto& nameComp = e.get<ECSComponents::NameComponent>();
		std::string name = nameComp.name;
		bool selected = (selectedEntityID == e.getEntityID());
		std::string label = std::to_string(e.getEntityID()) + ": " + name;
		if (ImGui::Selectable(label.c_str(), selected))
		{
			selectedEntityID = e.getEntityID();
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

	// --- Handle newly dropped files ---
	if (!g_droppedFiles.empty()) {
		std::cout << "[DEBUG] Processing dropped files...\n";

		for (auto& path : g_droppedFiles) {
			std::filesystem::path filePath(path);
			auto ext = filePath.extension().string();

			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
				std::cout << "[DEBUG] Loading: " << path << std::endl;

				auto tex = std::make_shared<Graphics::EnoTexture>();
				if (tex->loadFromFile(path)) {
					std::string name = filePath.stem().string();
					loader.addTexture(name, tex);
					std::cout << "[DEBUG] Imported: " << name << std::endl;
				}
				else {
					std::cout << "[ERROR] Failed to load: " << path << std::endl;
				}
			}
		}

		g_droppedFiles.clear();
	}

	// --- Inline preview list (thumbnail + name in one line) ---
	ImGui::Text("Loaded Textures:");
	for (auto& [name, texture] : textures) {
		if (!texture || !texture->isValid())
			continue;

		// Draw thumbnail first
		ImGui::Image((void*)(intptr_t)texture->getID(), ImVec2(48, 48));

		// Keep texture name on same horizontal line
		ImGui::SameLine();

		// Make name selectable for preview/application
		bool isSelected = (selectedTexture == name);
		if (ImGui::Selectable(name.c_str(), isSelected, 0, ImVec2(200, 48))) {
			selectedTexture = name;
		}

		// Add spacing between rows
		ImGui::Spacing();
		ImGui::Separator();
	}

	ImGui::Separator();


	// --- Preview & Apply Button ---
	if (!selectedTexture.empty()) {
		ImGui::Text("Selected: %s", selectedTexture.c_str());

		ImGui::Button("Apply to Player", ImVec2(200, 40));

		// Handle drag-drop texture application
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE")) {
				const char* droppedName = static_cast<const char*>(payload->Data);
				auto newTex = loader.getTexture(droppedName);
				if (newTex && newTex->isValid()) {
					g_game->setPlayerTexture(newTex);
					std::cout << "Dropped texture applied: " << droppedName << std::endl;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Handle direct button click (apply selected texture)
		if (ImGui::IsItemClicked()) {
			if (playerSpawned_ && playerSystem_) {
				auto newTex = loader.getTexture(selectedTexture);
				if (newTex && newTex->isValid()) {
					g_game->setPlayerTexture(newTex);
					std::cout << "Applied new texture: " << selectedTexture << std::endl;
				}
			}
		}
	}
}

// docking shi

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
	bool open = true;
	ImGui::Begin("EnoEngine Dockspace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// Create the dockspace
	ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void displayGameView(Engine::EnoEngine& engine_) {
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// Game controls
	//if (ImGui::Button("Play")) { /* Play logic */ }
	//ImGui::SameLine();
	//if (ImGui::Button("Pause")) { /* Pause logic */ }
	//ImGui::SameLine();
	//if (ImGui::Button("Stop")) { /* Stop logic */ }

	//ImGui::Separator();

	// Display the game framebuffer texture
	if (g_game && g_game->getGameFramebuffer()) {
		auto* fb = g_game->getGameFramebuffer();

		// Resize framebuffer if window size changed
		if (contentSize.x > 0 && contentSize.y > 0) {
			fb->resize((int)contentSize.x, (int)contentSize.y);

			// Display the texture
			ImGui::Image(
				(void*)(intptr_t)fb->getTextureID(),
				contentSize,
				ImVec2(0, 1), // UV start (flipped Y)
				ImVec2(1, 0)  // UV end (flipped Y)
			);
		}
	}
}

void imGuiSetupDefaultLayout() {
	static bool firstRun = true;
	if (!firstRun) return;
	firstRun = false;

	// Get main viewport
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	float screenWidth = viewport->Size.x;
	float screenHeight = viewport->Size.y;

	// Define layout proportions
	const float leftWidth = screenWidth * 0.20f;     // 20% left
	const float rightWidth = screenWidth * 0.25f;    // 25% right
	const float centerWidth = screenWidth - leftWidth - rightWidth;
	const float bottomHeight = screenHeight * 0.20f; // 20% bottom
	const float mainHeight = screenHeight - bottomHeight;

	// Set initial window positions and sizes

	// LEFT PANEL - Edit (top 1/3 of left area)
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(leftWidth, mainHeight * 0.33f));

	// LEFT PANEL - Prefabs (middle 1/3 of left area)
	ImGui::SetNextWindowPos(ImVec2(0, mainHeight * 0.33f));
	ImGui::SetNextWindowSize(ImVec2(leftWidth, mainHeight * 0.33f));

	// LEFT PANEL - Main Controls (bottom 1/3 of left area)
	ImGui::SetNextWindowPos(ImVec2(0, mainHeight * 0.66f));
	ImGui::SetNextWindowSize(ImVec2(leftWidth, mainHeight * 0.34f));

	// CENTER - Game View (main area)
	ImGui::SetNextWindowPos(ImVec2(leftWidth, 0));
	ImGui::SetNextWindowSize(ImVec2(centerWidth, mainHeight));

	// RIGHT PANEL - Assets
	ImGui::SetNextWindowPos(ImVec2(leftWidth + centerWidth, 0));
	ImGui::SetNextWindowSize(ImVec2(rightWidth, mainHeight));

	// BOTTOM PANEL - FPS/Console
	ImGui::SetNextWindowPos(ImVec2(0, mainHeight));
	ImGui::SetNextWindowSize(ImVec2(screenWidth, bottomHeight));

	std::cout << "Default layout applied to windows" << std::endl;
}

void imGuiEndDockspace() {
	ImGui::End();
}

void imGuiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


