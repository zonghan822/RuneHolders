#include "game_logic.hpp"
#include <GLFW/glfw3.h>
#include "imGui.hpp"
#include "math/math.hpp"
#include "math/movementTest.hpp"
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
    // Setup all game content
    setupGameEntities();
    setupGameAudio();
    setupGameEvents();
    setupGameMath();
    setupAnimations();

    //this is for caven's movement test object
    createTestObject(engine_.getEntityManager());
    

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

void GameApplication::setupAnimations() {
    // Load the sprite sheet texture
	backgroundTexture_ = engine_.getResourceLoader().getTexture("Background");
	playerTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Idle-Sheet");

	if (!playerTexture_)
	{
		std::cout << "Warning: Could not load player sprite sheet!" << std::endl;
		return;
	}

	// Create player idle animation (same sprite sheet as test)
	auto playerIdleAnim = Animation::createSpriteSheetAnimation(
		96,    // Frame width
		96,    // Frame height
		2304,  // Sheet width
		96,    // Sheet height
		0,     // Start frame
		24,    // Frame count
		0.1f   // Duration per frame
	);

	playerIdleAnim.setLooping(true);
	playerAnimController_.addAnimation("idle", playerIdleAnim);
	playerAnimController_.setAnimation("idle");

	std::cout << "Player animation setup complete!" << std::endl;
}

void GameApplication::setupImGui() {
	GLFWwindow* window = engine_.getWindow().getWindow();
	imGuiInit(window);
}

void GameApplication::demonstrateSignals() {
	// signalling code
}

void GameApplication::updateGameLogic() {
    // This is called every frame (replaces your while loop in eno_engine.cpp run())

    // Calculate delta time
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastFrameTime_;
    lastFrameTime_ = currentTime;

	// Draw background before everything else
	if (backgroundTexture_ && backgroundTexture_->isValid()) {
		auto& graphics = engine_.getGraphicsManager();

		// Get window dimensions from config
		auto windowDim = engine_.getConfigManager().getWindowDim();
		Math::eVec2<float> windowSize{
			static_cast<float>(windowDim.first),
			static_cast<float>(windowDim.second)
		};

		graphics.drawTexturedQuadUV(
			Math::eVec2<float>{0.0f, 0.0f},           // Position at origin
			windowSize,                                // Full screen size from config
			backgroundTexture_,
			Math::eVec2<float>{0.0f, 0.0f},           // UV start
			Math::eVec2<float>{1.0f, 1.0f},           // UV end
			Graphics::Color::White
		);
	}

	// ===== UPDATE PLAYER ANIMATION =====
	if (playerTexture_ && playerSpawned_)
	{
		playerAnimController_.update(deltaTime);
	}

    updateEntities();
    updateAudio();
    drawShapes();
	drawPlayerAndIndicator();

	auto& graphics = engine_.getGraphicsManager();
	engine_.getGraphicsManager().flush();

	updateImGui();
}

void GameApplication::updateEntities() {
	// start timing
	auto start = Clock::now();

	// Update game entities each frame
	// Add entity movement, AI, collision detection etc here

	//caven's test object
	updateTestObject(engine_.getEntityManager(), engine_);

	// delta time for movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;
	GLFWwindow* window = engine_.getWindow().getWindow();

	// UPDATED: Pass the input manager instead of window
	// Get input manager from engine
	auto& inputManager = engine_.getInputManager();

	// update player movement with collision using the input manager
	if (playerSystem_ && playerSpawned_) {
		playerSystem_->Update(engine_.getEntityManager(), inputManager, deltaTime);
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

void GameApplication::drawPlayerAndIndicator() {
	auto& graphics = engine_.getGraphicsManager();

	// Draw player sprite
	if (playerSpawned_) {
		try {
			auto& entityMap = engine_.getEntityManager().getMap();
			auto it = entityMap.find(playerEntityID_);
			if (it != entityMap.end()) {
				auto& transform = playerSystem_->GetTransform(it->second);

				if (playerTexture_ && playerTexture_->isValid()) {
					auto frame = playerAnimController_.getCurrentFrame();
					graphics.drawTexturedQuadUV(
						Math::eVec2<float>{transform.pos.x, transform.pos.y},
						Math::eVec2<float>{48.0f, 48.0f},
						playerTexture_,
						frame.uvStart,
						frame.uvEnd,
						Graphics::Color::White
					);
				}
				else {
					graphics.drawRect(
						Math::eVec2<float>{transform.pos.x, transform.pos.y},
						Math::eVec2<float>{10, 10},
						Graphics::Color::Blue,
						true
					);
				}
			}
		}
		catch (...) {}
	}

	// Draw red indicator
	graphics.drawRect(
		Math::eVec2<float>{indicatorX_, indicatorY_},
		Math::eVec2<float>{10, 10},
		Graphics::Color::Red,
		true
	);
}

void GameApplication::updateImGui() {
	auto start = Clock::now();

	imGuiDisplay(engine_.getWindow().getWindow(), engine_);

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


	auto end = Clock::now();
	systemTimes_["Graphics"] = std::chrono::duration<double, std::micro>(end - start).count();
}
