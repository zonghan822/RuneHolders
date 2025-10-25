/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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

//collision system
std::unique_ptr<Collision::CollisionSystem> collisionSystem_;
std::unique_ptr<Player::PlayerSystem> playerSystem_;
bool showColliders_ = true;

GameApplication* g_game = nullptr;

// player entity ID (to track the player)
uint32_t playerEntityID_ = 0;
bool playerSpawned_ = false;

//containers (time display)
std::unordered_map<std::string, double> systemTimes_; //actual system time
std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)

//indicator
float indicatorX_ = 600.0f;
float indicatorY_ = 425.0f;
float moveSpeed_ = 200.0f; // pixels per second


//Option Menu
const char* shapes[] = { "Rectangle" };
const char* colors[] = { "Green"  ,     "Red" };
const char* colliderShapes[] = { "None", "Circle", "Rectangle" };

//Values to set for entities
static float           eWidth = 10;
static float           eHeight = 10;
static int             eColor = Color::Green;
static int             eShapeIndex = Shapes::Rectangle;
static int             selectedCollider = 0;         // for the newly spawned entity
static float           colliderWidth = 10.0f;
static float           colliderHeight = 10.0f;
static float           colliderRadius = 5.0f;

//Player Scale Variables
float pScaleX = 1.0f;
float pScaleY = 1.0f;

//time
static float shown_fps = 0.0f;
static double next_update = 0.0;         // next time we refresh the text

//entity counter
size_t entity_count{};

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

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}


void imGuiDisplay(Engine::EnoEngine& engine_) {

	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// (kim) get delta time for smooth movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;

	// (kim) WASD movement for indicator 
	float moveDelta = moveSpeed_ * deltaTime;

	// (kim) changed to ARROW KEYS for indicator control 
	auto& inputManager = engine_.getInputManager();
	if (inputManager.isKeyPressed(GLFW_KEY_UP)) indicatorY_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_DOWN)) indicatorY_ += moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_LEFT)) indicatorX_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_RIGHT)) indicatorX_ += moveDelta;

	// Assuming that there will only be 1 entity with player component
	for (ECS::Entity player : engine_.getEntityManager().findEntity<Player::PlayerComponent>())
	{
		playerEntityID_ = player.getEntityID();
	}


	double now = ImGui::GetTime();           // imgui monotonic time (seconds)
	if (now >= next_update) {
		shown_fps = io.Framerate;            // sample current FPS
		throttledTimes_ = systemTimes_;
		next_update = now + 0.5;             // update every 0.5s (tune this)
	}

	if (ImGui::Button("Main"))     activeWindow = WindowID::Main;
	if (ImGui::Button("Edit"))     activeWindow = WindowID::Edit;
	if (ImGui::Button("FPS"))      activeWindow = WindowID::FPS;
	if (ImGui::Button("Prefabs"))  activeWindow = WindowID::Prefabs;
	if (ImGui::Button("Assets"))   activeWindow = WindowID::Assets;
	//FPS and Time Staggering for Displaying FPS
#pragma region IMGUI Main
	if (activeWindow == WindowID::Main) {
		ImGui::Begin("Main");
		displayMainTab(engine_);
		ImGui::End();
	}
#pragma endregion
#pragma region IMGUI Edit

	if (activeWindow == WindowID::Edit) {
		ImGui::Begin("Edit");
		displayEditTab(engine_);
		ImGui::End();
	}
#pragma endregion
#pragma region IMGUI FPS
	if (activeWindow == WindowID::FPS) {
		ImGui::Begin("FPS");
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



		ImGui::End();
	}
#pragma endregion
#pragma region IMGUI Prefabs
	if (activeWindow == WindowID::Prefabs) {
		ImGui::Begin("Prefabs");
		displayPrefabsTab(engine_);
		ImGui::End();
	}
#pragma endregion
#pragma region IMGUI Assets
	if (activeWindow == WindowID::Assets) {
		ImGui::Begin("Assets Browser");

		// Display all loaded assets here
		displayAssetTab(engine_);

		ImGui::End();
	}


#pragma endregion





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
				{indicatorX_, indicatorY_, 0.0f},
				{0.0f, 0.0f, 0.0f},
				{1.0f, 1.0f, 1.0f}
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
				{centerX, centerY, 0.0f},
				{0.0f, 0.0f, 0.0f},
				{1.0f, 1.0f, 1.0f}
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
		static char searchInput[64] = ""; // to enter architype name
		ImGui::InputTextWithHint("##Search", "Enter Name to delete...", searchInput, sizeof(searchInput));
		if (ImGui::Button("Delete Entities")) {
			for (ECS::Entity& e : engine_.getEntityManager().findEntity<ECSComponents::NameComponent>()) {

				if (e.get<ECSComponents::NameComponent>().name == ECSComponents::NameComponent(searchInput)) {
					toDelete.push_back(e);
				}
			}

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
		if (ImGui::Button("Delete Entities")) {

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
		{centerX, centerY, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f}
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

void displayAssetTab(Engine::EnoEngine& engine_) {
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	std::unordered_map<std::string, std::shared_ptr<Graphics::EnoTexture>>const& textures = loader.getAllTextures();

	ImGui::Text("=== Loaded Textures ===");
	ImGui::Separator();

	static std::string selectedTexture;
	// Begin the list box
	// The first argument is a unique label (can be "##hidden_label" for no visible label)
	// The second argument is the size of the list box (ImVec2(width, height))
	ImGui::Text("Loaded Textures:");
	if (ImGui::BeginListBox("##TextureList", ImVec2(0, 200))) {
		// Structured bindings to extract name from the texture list
		for (auto& [name, texture] : textures) {
			bool isSelected = (selectedTexture == name);
			if (ImGui::Selectable(name.c_str(), isSelected)) {
				selectedTexture = name;
			}
			// Start drag operation
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				// This turns each texture name into a draggable payload
				ImGui::SetDragDropPayload("ASSET_TEXTURE", name.c_str(), name.size() + 1);
				ImGui::Text("Dragging %s", name.c_str());
				ImGui::EndDragDropSource();
			}
		}
		ImGui::EndListBox();
	}

	ImGui::Separator();
	// Preview and replace button
	if (!selectedTexture.empty()) {
		ImGui::Text("Selected: %s", selectedTexture.c_str());

		// Apply texture to player
		ImGui::Button("Apply to Player", ImVec2(200, 40));
		// Accept drag-drop payload while hovering
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

		// Separate click logic for normal apply button
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

void imGuiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


