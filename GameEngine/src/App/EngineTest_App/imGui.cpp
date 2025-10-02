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
const char* shapes[] = { "Circle" , "Rectangle" };
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

//prefabs
static PrefabData prefab;
static char nameSetter[50] = "";



#pragma endregion

float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

void imGuiInit(GLFWwindow* window){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}


void imGuiDisplay(GLFWwindow* window,Engine::EnoEngine& engine_) {

	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// (kim) get delta time for smooth movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;

	// (kim) WASD movement for indicator (always active, no need to focus on ImGui)
	float moveDelta = moveSpeed_ * deltaTime;

	/* (kim) changed to ARROW KEYS for indicator control */
	auto& inputManager = engine_.getInputManager();
	if (inputManager.isKeyPressed(GLFW_KEY_UP)) indicatorY_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_DOWN)) indicatorY_ += moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_LEFT)) indicatorX_ -= moveDelta;
	if (inputManager.isKeyPressed(GLFW_KEY_RIGHT)) indicatorX_ += moveDelta;





	prefab.x = indicatorX_;
	prefab.y = indicatorY_;
	prefab.width = eWidth;
	prefab.height = eHeight;
	prefab.color = eColor;
	prefab.shape = eShapeIndex;
	prefab.hasCollider = (selectedCollider != 0);


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
	//FPS and Time Staggering for Displaying FPS
	#pragma region IMGUI Main
	if (activeWindow == WindowID::Main) {
		ImGui::Begin("Main");

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
				auto& entityMap = engine_.getEntityManager().getMap();
				uint32_t lastID = 0;
				for (auto& [id, loc] : entityMap) {
					if (id > lastID) lastID = id;
				}
				playerEntityID_ = lastID;

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
				engine_.getEntityManager().destroyEntity(playerEntityID_);
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
		//dropdoawn menu
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

			//adding name to the entity only for destroy() debug purposes
			e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent("John"));
			if (e.get<ECSComponents::NameComponent>().name == ECSComponents::NameComponent("John")) std::cout << "he john\n";


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

				}
				else if (selectedCollider == 2) { // rectangle
					e.add<Collision::CollisionComponent>(Collision::CollisionComponent(colliderWidth, colliderHeight)
					);
				}

				std::cout << "Spawned entity WITH collider\n";
			}
			else {
				std::cout << "Spawned entity without collider\n";
			}
			if (g_game) g_game->markEntitiesDirty();
		}



		//Clone Button
		if (ImGui::Button("Spawn 2500 Entities")) {
			for (size_t i = 0; i < 2500; i++) {
				float randX = randomFloat(0.0f, 1200.0f); //screen width
				float randY = randomFloat(0.0f, 900.0f); // screen height
				//HARDCODED(COPIED) SPAWN CODE INTO HERE
				ECSComponents::DrawInfoComponent DIC{ randX, randY, eWidth, eHeight, shapes[eShapeIndex], eColor };
				ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);
				e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent("John"));
			}
			entity_count += 2500;
			if (g_game) g_game->markEntitiesDirty();
			std::cout << "Spawned " << entity_count << " Entities\n";
		}



		ImGui::End();
	}
#pragma endregion
	#pragma region IMGUI Edit

	if (activeWindow == WindowID::Edit) {
		ImGui::Begin("Edit");
		const char* filters[] = { "Name", "Type" , "ID" };
		static int FilterType = Filters::Name;
		std::vector<ECS::Entity> toDelete;

		ImGui::Combo("Filters", &FilterType, filters, IM_ARRAYSIZE(filters));
		if (FilterType == Filters::Name) {

			if (ImGui::Button("Delete John")) {
				for (ECS::Entity& e : engine_.getEntityManager().findEntity<ECSComponents::NameComponent>()) {
					if (e.get<ECSComponents::NameComponent>().name == ECSComponents::NameComponent("John")) {
						toDelete.push_back(e);
					}
				}
			}
		}

		if (FilterType == Filters::Type) {
			//do another list to look for the type here
			for (ECS::Entity& e : engine_.getEntityManager().findEntity<>()) {

			}

		}
		if (FilterType == Filters::ID) {

			for (ECS::Entity& e : engine_.getEntityManager().findEntity<>()) {

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

		ImGui::End();
	}
#pragma endregion
	#pragma region IMGUI FPS
	if (activeWindow == WindowID::FPS) {
		ImGui::Begin("FPS");
		ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
		ImGui::Text("Frame (%.1f FPS)", shown_fps);
		ImGui::Text("Per-System Timings:");
		for (auto& time : throttledTimes_) {
			ImGui::Text("%s: %.3f ms", time.first.c_str(), time.second / 1000.0);
			// divide by 1000 to show microseconds as milliseconds
		
		}
		if (ImGui::Button("Show Entity Count")) {
			displayEntityCount(engine_);
		}
		ImGui::End();
	}
#pragma endregion
	#pragma region IMGUI Prefabs and Serialization
	//to use, do your settings on main tab. then enter prefabs tab to save your file
	//1. Enter file/prefab name
	//2. Load from the name tab
	//3. Spawn from your prefabs

	if (activeWindow == WindowID::Prefabs) {
		ImGui::Begin("Prefabs");


		if ( ImGui::InputText("Name for Prefab", nameSetter,sizeof(nameSetter)) ) {
			prefab.name = static_cast<std::string>(nameSetter);
		}

		if (ImGui::Button("Save Prefab")) {
			//this will save to assets/saves/..
			savePrefab(prefab);
		}		

		if (ImGui::Button("Load Prefab")) {
			std::string fileStr = "../../../assets/saves/" + static_cast<std::string>(nameSetter) + ".txt";

			//PrefabData loaded = loadPrefab("../../../assets/saves/prefabs.txt");
			PrefabData loaded = loadPrefab(fileStr);
			prefabRegistry[loaded.name] = loaded;
		}		

		if (ImGui::Button("Spawn Prefab")) {
			if (prefabRegistry.count(prefab.name)) {
				instantiatePrefab(prefabRegistry[prefab.name], engine_);
			}
		}
	
		ImGui::End();
	}
#pragma endregion

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void displayEntityCount(Engine::EnoEngine& engine_) {
	size_t counter = 0;
	
	std::vector<ECS::Entity> EntityList = engine_.getEntityManager().findEntity<>();
	for (ECS::Entity const& e : EntityList) {
		counter++;
	}
	std::cout << "Number of entities: " << counter << "\n";
}



void imGuiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}