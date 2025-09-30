#include "game_logic.hpp"
#include <GLFW/glfw3.h>
#include "imGui.hpp"
#include "math/math.hpp"

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
    testTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Idle-Sheet");

    if (!testTexture_)
    {
        std::cout << "Warning: Could not load animation sprite sheet!" << std::endl;
        return;
    }

    // Create attack animation
    // Adjust these values based on YOUR sprite sheet:
    // - Frame size: 89x89 pixels (your frames appear to be square based on height)
    // - Sheet size: 502x89 pixels
    // - Start frame: 0
    // - Frame count: 5 (502 / 89 = 5.6, so probably 5 frames)
    // - Duration: 0.1 seconds per frame

    auto attackAnim = Animation::createSpriteSheetAnimation(
        96,          // Frame width
        96,          // Frame height
        2304,         // Sheet width
        96,          // Sheet height
        0,           // Start frame
        24,           // Number of frames
        0.1f         // Duration per frame (seconds)
    );

    attackAnim.setLooping(true);
    animController_.addAnimation("attack", attackAnim);

    // Set the initial animation
    animController_.setAnimation("attack");

    std::cout << "Animation setup complete!" << std::endl;
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

    // Update animation with delta time
    if (testTexture_)
    {
        animController_.update(deltaTime);
    }

    updateEntities();
    updateAudio();

    // Add any other per-frame game logic here
    drawShapes();
    drawAnimatedSprite();
	updateEntities();
	updateAudio();
	updateImGui();
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


	auto end = Clock::now();
	systemTimes_["Graphics"] = std::chrono::duration<double, std::micro>(end - start).count();
}



void GameApplication::drawAnimatedSprite() {
    if (!testTexture_ || !testTexture_->isValid())
        return;

    auto& graphics = engine_.getGraphicsManager();

    // Get the current frame from the animation
    auto frame = animController_.getCurrentFrame();

    // CRITICAL: Round to whole pixels to prevent sub-pixel jittering
    float xPos = std::floor(400.0f);
    float yPos = std::floor(300.0f);

    // Draw the animated sprite
    graphics.drawTexturedQuadUV(
        Math::eVec2<float>{xPos, yPos},
        Math::eVec2<float>{192.0f, 192.0f},  // Size (2x scale for visibility)
        testTexture_,
        frame.uvStart,  // UV start from current frame
        frame.uvEnd,    // UV end from current frame
        Graphics::Color::White
    );
}