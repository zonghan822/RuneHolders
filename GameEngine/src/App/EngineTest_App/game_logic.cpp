#include "game_logic.hpp"
#include "math/math.hpp"
#include "../Engine/collision/collision.hpp"



//for testing purposes
#include <random>
static std::mt19937 rng(std::random_device{}());
// returns a random float between min and max
float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

using Clock = std::chrono::high_resolution_clock;
std::unordered_map<std::string, double> systemTimes_;
//throttle timing for systemTimes_
std::unordered_map<std::string, double> throttledTimes_;


//throw std::runtime_error("error logging");

void GameApplication::initialize() {
	// Setup engine systems first
	engine_.setup();

	// Setup all game content
	setupGameEntities();
	setupGameAudio();
	setupGameEvents();
	setupGameMath();
	setupImGui();

	std::cout << "Game initialized successfully!" << std::endl;
}

void GameApplication::run() {
	std::cout << "Starting game..." << std::endl;

	// Start the engine with game update callback
	engine_.run([this]() {
		updateGameLogic();  // This gets called every frame
		});

	//------------------------------------HARDCODED TERMINATE OF IMGUI FUNCTIONS------------------------------------
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	//------------------------------------END OF HARDCODED TERMINATE OF IMGUI FUNCTIONS-----------------------------

}

void GameApplication::setupGameEntities() {
	// To setup the entities in our game
}

void GameApplication::setupGameAudio() {
	// Setup audio
}

void GameApplication::setupGameEvents() {
	// setup event bus
}

void GameApplication::setupGameMath() {
	// math lib and maybe collision
	std::cout << "Calling math function\n";
	Math::eVec2 vec{ 1.1f, 1.2f };
	Math::eVec2 vec2{ 1.3f, 2.f };
	vec += vec2;
	std::cout << "vector : " << vec.v1 << ", " << vec.v2 << std::endl;

	Collision::RunBasicCollisionTest();

	//initialize collision system
	collisionSystem_ = std::make_unique<Collision::CollisionSystem>(engine_.getEventBus());

	// initialize player system
	playerSystem_ = std::make_unique<Player::PlayerSystem>();
}

void GameApplication::setupImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();
	GLFWwindow* window = engine_.getWindow().getWindow();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

}

void GameApplication::demonstrateSignals() {
	// signalling code
}

void GameApplication::updateGameLogic() {
	updateEntities();
	updateAudio();
	updateImGui();
	drawShapes();
}

void GameApplication::updateEntities() {
	// Update game entities each frame
	auto start = Clock::now();
	// Add entity movement, AI, collision detection etc here

	// delta time for movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;
	GLFWwindow* window = engine_.getWindow().getWindow();

	// update player movement with collision
	if (playerSystem_ && playerSpawned_) {
		playerSystem_->Update(engine_.getEntityManager(), window, deltaTime);
	}

	// update collision system every frame
	if (collisionSystem_) {
		collisionSystem_->Update(engine_.getEntityManager());
	}
	auto end = Clock::now();
	systemTimes_["Entities"] = std::chrono::duration<double, std::micro>(end - start).count();
}

void GameApplication::updateAudio() {
	// audio update
	auto start = Clock::now();

	auto end = Clock::now();
	systemTimes_["Audio"] = std::chrono::duration<double, std::micro>(end - start).count();

}

void GameApplication::updateImGui() {
	auto start = Clock::now();

	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// (kim) get delta time for smooth movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;

	// (kim) note: WASD movement is now handled by PlayerSystem in updateEntities()
	// the indicator (red square) no longer moves with WASD
	// only the player (blue square) moves with WASD after spawning

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

	static float shown_fps = 0.0f;
	static double next_update = 0.0;         // next time we refresh the text
	double now = ImGui::GetTime();           // imgui monotonic time (seconds)

	if (now >= next_update) {
		shown_fps = io.Framerate;            // sample current FPS
		throttledTimes_ = systemTimes_;
		next_update = now + 0.5;             // update every 0.5s (tune this)
	}

	if (ImGui::Button("Main"))     activeWindow = WindowID::Main;
	if (ImGui::Button("Delete"))   activeWindow = WindowID::Delete;
	if (ImGui::Button("FPS"))      activeWindow = WindowID::FPS;
	//FPS and Time Staggering for Displaying FPS

	if (activeWindow == WindowID::Main) {
		ImGui::Begin("Main");
		ImGui::Separator();

		//=======================PLAYER CONTROLS =======================
		// (kim added) add player spawn section
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
				Collision::CollisionComponent playerCollider(10.0f, 10.0f); // 10x10 rectangle

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
				std::cout << "Player destroyed\n";
			}
		}

		ImGui::Separator();

		// ==================== SPAWN POSITION INDICATOR ====================
		ImGui::Text("=== SPAWN POSITION INDICATOR ===");
		ImGui::Text("Position (%.1f, %.1f)", indicatorX_, indicatorY_);
		ImGui::DragFloat("Move Speed", &moveSpeed_, 10.0f, 50.0f, 500.0f);
		ImGui::Checkbox("Show Colliders", &showColliders_);

		ImGui::Separator();


		// ==================== ENTITY SPAWN SETTINGS ====================		
		ImGui::Text("=== Entity Spawn Settings ===");
		ImGui::DragFloat("Entity Width", &eWidth, 0.5f, 1.0f, 100.0f);
		ImGui::DragFloat("Entity Height", &eHeight, 0.5f, 1.0f, 100.0f);
		//dropdoawn menu
		ImGui::Combo("Shape", &eShapeIndex, shapes, IM_ARRAYSIZE(shapes));
		ImGui::Combo("Color", &eColor, colors, IM_ARRAYSIZE(colors));

		ImGui::Separator();

		//collider settings
		ImGui::Text("Collider Settings");
		ImGui::Combo("ColliderType", &selectedCollider, colliderShapes, IM_ARRAYSIZE(colliderShapes));

		if (selectedCollider == 1) { // circle
			ImGui::DragFloat("Collider Width", &colliderWidth, 0.5f, 1.0f, 100.0f);
		}
		else if (selectedCollider == 2) { // rectangle
			ImGui::DragFloat("Collider Width", &colliderWidth, 0.5f, 1.0f, 100.0f);
			ImGui::DragFloat("Collider Height", &colliderHeight, 0.5f, 1.0f, 100.0f);
		}


		//Spawn Button (with optional collider)
		if (ImGui::Button("Spawn Entity")) {
			ECSComponents::DrawInfoComponent DIC{ indicatorX_, indicatorY_, eWidth, eHeight, shapes[eShapeIndex], eColor };
			ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);

			// add collider if selected
			if (selectedCollider != 0) {
				// first add transform component (required for collision)
				ECSComponents::TransformComponent transform{
					{indicatorX_, indicatorY_, 0.0f},
					{0.0f, 0.0f, 0.0f},
					{1.0f, 1.0f, 1.0f}
				};

				e.add<ECSComponents::TransformComponent>(transform);

				// then add appropriate collider
				if (selectedCollider == 1) { //circle
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
		}



		//Clone Button
		if (ImGui::Button("Clone 50 Entities")) {
			static size_t counter = 0;
			for (size_t i = 0; i < 50; i++) {
				float randX = randomFloat(0.0f, 1200.0f); //screen width
				float randY = randomFloat(0.0f, 900.0f); // screen height
				//HARDCODED(COPIED) SPAWN CODE INTO HERE
				ECSComponents::DrawInfoComponent DIC{ randX, randY, eWidth, eHeight, shapes[eShapeIndex], eColor };
				ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);
			}
			counter += 50;

			std::cout << "Cloned " << counter << " Entity\n";
		}

		ImGui::End();
	}


	if (activeWindow == WindowID::Delete) {
		ImGui::Begin("Delete");



		ImGui::End();
	}


	if (activeWindow == WindowID::FPS) {
		ImGui::Begin("FPS");
		ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
		ImGui::Text("Frame (%.1f FPS)", shown_fps);
		ImGui::Text("Per-System Timings:");
		for (auto& time : throttledTimes_) {
			ImGui::Text("%s: %.3f ms", time.first.c_str(), time.second / 1000.0);
			// divide by 1000 to show microseconds as milliseconds
		}


		ImGui::End();
	}
	auto& graphics = engine_.getGraphicsManager();
	
	// ==================== DRAW PLAYER (BLUE) ====================
	if (playerSpawned_) {
		try {
			auto& entityMap = engine_.getEntityManager().getMap();
			auto it = entityMap.find(playerEntityID_);
			if (it != entityMap.end()) {
				auto& transform = playerSystem_->GetTransform(it->second);
				graphics.drawRect(
					Math::eVec2<float>{transform.pos.x, transform.pos.y},
					Math::eVec2<float>{10, 10},
					Graphics::Color::Blue,  // Player is BLUE
					true
				);
			}
		}
		catch (...) {
			// Player entity might be invalid
		}
	}

	// ==================== DRAW SPAWN POSITION INDICATOR (RED) ===================
	// Red indicator rectangle that shows where entities will spawn
	graphics.drawRect(
		Math::eVec2<float>{indicatorX_, indicatorY_},  // Position
		Math::eVec2<float>{10, 10},            // Size 10 wide 10 tall
		Graphics::Color::Red,                 //  Colour
		true                                  //  True
	);

	auto end = Clock::now();
	systemTimes_["ImGui"] = std::chrono::duration<double, std::micro>(end - start).count();
}


void GameApplication::drawShapes() { // For testing
	auto start = Clock::now();


	auto& graphics = engine_.getGraphicsManager();

	auto& entityManager = engine_.getEntityManager();

	using drawInfo = ECSComponents::DrawInfoComponent;

	//thinking here i do for each drawable entity: draw.
	// draw all entities
	std::vector<ECS::Entity> EntityList = engine_.getEntityManager().findEntity<ECSComponents::DrawInfoComponent>();

	for (ECS::Entity& entity : EntityList) {
		if (entity.has<ECSComponents::DrawInfoComponent>()) {
			graphics.drawRect(
				Math::eVec2<float>{entity.get<drawInfo>().xPos, entity.get<drawInfo>().yPos},
				Math::eVec2<float>{entity.get<drawInfo>().width, entity.get<drawInfo>().height},
				entity.get<drawInfo>().color,
				true
			);
		}
	}

	// (kim) visualize colliders if enabled
	if (showColliders_ && collisionSystem_) {
		collisionSystem_->VisualizeColliders(graphics, entityManager);
	}


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	auto end = Clock::now();
	systemTimes_["Graphics"] = std::chrono::duration<double, std::micro>(end - start).count();
}
