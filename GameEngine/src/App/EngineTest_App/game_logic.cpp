#include "game_logic.hpp"
#include "imGui.hpp"
#include "math/math.hpp"
#include "../Engine/collision/collision.hpp"

using Clock = std::chrono::high_resolution_clock;
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

	imGuiTerminate();

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
	GLFWwindow* window = engine_.getWindow().getWindow();
	imGuiInit(window);
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


	imGuiDisplay(engine_.getWindow().getWindow(),engine_);


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
		Math::eVec2<float>{10, 10},                    // Size 10 wide 10 tall
		Graphics::Color::Red,						   //  Colour
		true										   //  True
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
