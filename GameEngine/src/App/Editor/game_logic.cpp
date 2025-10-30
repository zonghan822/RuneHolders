#include "game_logic.hpp"
#include <GLFW/glfw3.h>
#include "imGui.hpp"
#include "math/math.hpp"

using Clock = std::chrono::high_resolution_clock;
//throw std::runtime_error("error logging");

float scaleSize = 1.0f;
float rotationRadians = 0.f;


void GameApplication::initialize() {
	// Setup engine systems first
	engine_.setup();

	// Setup all game content
	setupGameEntities();
	setupGameAudio();
	setupTreeTesting();
	setupGameEvents();
	setupGameMath();
	setupImGui();
    // Setup all game content
    setupGameEntities();
    setupGameAudio();
    setupGameEvents();
    setupGameMath();
    setupAnimations();
	pixelifyFont_ = engine_.getResourceLoader().getFont("PixelifySans");
	timesFont_ = engine_.getResourceLoader().getFont("times");

    // This is for dynamic collision test object
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
	// setup audio
	auto& audioMng = engine_.getAudioManager();
	audioMng.setChannelGroup("BGM");
	audioMng.setChannelGroup("SFX");

	Audio::SoundParameters sound;
	Audio::ParamEQ eq;
	Audio::SFXReverb reverb;
	eq = Audio::ParamEQ{ 3.f, 1000.f, 1.f };
	sound.volume = 0.1f;
	sound.eq = Audio::EQ{};
	sound.eq->type = FMOD_DSP_TYPE_PARAMEQ;
	sound.eq->paramEQ = eq;
	sound.reverb = Audio::Reverb{};
	sound.reverb->type = FMOD_DSP_TYPE_SFXREVERB;
	sound.reverb->sfx = reverb;
	audioMng.modifyChannelGroup("BGM", sound);

	audioMng.publishSoundRequest("Close_To_The_End");
	audioMng.publishSoundRequest("Across_The_Horizon");
	audioMng.publishSoundRequest("Flask");
	audioMng.publishSoundRequest("Stone_Impact");
	audioMng.publishSoundRequest("Water_Drip");

	// setup input to play sound
	auto& inputMng = engine_.getInputManager();

	// Music Tracks
	// O to play/stop a music track, P && L for switching soundtracks when music is not playing
	// Only 1 music track may be placed at a time
	inputMng.setupKey("Play Audio", GLFW_KEY_O, glfwGetKeyScancode(GLFW_KEY_O), GLFW_RELEASE, 0);
	inputMng.setupKey("Set New Music 1", GLFW_KEY_P, glfwGetKeyScancode(GLFW_KEY_P), GLFW_RELEASE, 0);
	inputMng.setupKey("Set New Music 2", GLFW_KEY_L, glfwGetKeyScancode(GLFW_KEY_L), GLFW_RELEASE, 0);
	
	inputMng.setupEvent<Audio::SetAudioState>("Play Audio", "BGM");
	inputMng.setupEvent<Audio::SetNewAudio>("Set New Music 1", "Close_To_The_End");
	inputMng.setupEvent<Audio::SetNewAudio>("Set New Music 2", "Across_The_Horizon");

	// Sound Effects
	// Intended to be played regardless of any background music playing at the time
	inputMng.setupKey("Play SFX1", GLFW_KEY_A, glfwGetKeyScancode(GLFW_KEY_A), GLFW_RELEASE, 0);
	inputMng.setupKey("Play SFX2", GLFW_KEY_S, glfwGetKeyScancode(GLFW_KEY_S), GLFW_RELEASE, 0);
	inputMng.setupKey("Play SFX3", GLFW_KEY_D, glfwGetKeyScancode(GLFW_KEY_D), GLFW_RELEASE, 0);
	
	inputMng.setupEvent<Audio::PlayInstant>("Play SFX1", "SFX", "Flask");
	inputMng.setupEvent<Audio::PlayInstant>("Play SFX2", "SFX", "Stone_Impact");
	inputMng.setupEvent<Audio::PlayInstant>("Play SFX3", "SFX", "Water_Drip");
}

void GameApplication::setupTreeTesting()
{
	auto& inputMng = engine_.getInputManager();

	inputMng.setupKey("Test Editor Tree", GLFW_KEY_Q, glfwGetKeyScancode(GLFW_KEY_Q), GLFW_RELEASE, 0);
	inputMng.setupEvent<Editor::TestTree>("Test Editor Tree");
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
	std::cout << "vector : " << vec.x << ", " << vec.y << std::endl;

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
	playerWalkTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Walk-Sheet");

	if (!backgroundTexture_ || !backgroundTexture_->isValid())
	{
		std::cerr << "Background texture not loaded properly" << std::endl;
	}

	if (!playerTexture_ || !playerTexture_->isValid())
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

	auto playerWalkAnim = Animation::createSpriteSheetAnimation(
		96,    // Frame width
		96,    // Frame height
		384,  // Sheet width
		96,    // Sheet height
		0,     // Start frame
		4,    // Frame count
		0.08f  // Faster duration for walking
	);
	playerWalkAnim.setLooping(true);
	playerAnimController_.addAnimation("walk", playerWalkAnim);

	playerAnimController_.setAnimation("idle");

	std::cout << "Player animation setup complete!" << std::endl;
}

void GameApplication::setupImGui() {
	GLFWwindow* window = engine_.getWindowManager().getWindow();
	imGuiInit(window);
	extern GameApplication* g_game;
	g_game = this;
}

void GameApplication::updateGameLogic() {
    // This is called every frame (replaces your while loop in eno_engine.cpp run())
	SystemTimer totalTimePerFrame("totalTimePerFrame", systemTimes_);

    // Calculate delta time
    double deltaTime = engine_.getDeltaTime();

	// Get window dimensions from config
	auto windowDim = engine_.getWindowManager().getWindowSize();
	Math::eVec2<float> windowSize{
		static_cast<float>(windowDim.first),
		static_cast<float>(windowDim.second)
	};

	// ===== UPDATE PLAYER ANIMATION =====
	if (playerTexture_ && playerSpawned_)
	{
		playerAnimController_.update(static_cast<float>(deltaTime));
	}

    updateEntities();
    drawShapes();
	drawPlayerAndIndicator();
	engine_.getGraphicsManager().flush();
	updateImGui();
	drawText(windowSize);
}

void GameApplication::updateEntities() {
	SystemTimer EntityTimer("Entities" , systemTimes_);

	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;
	auto& inputManager = engine_.getInputManager();
	auto& graphics = engine_.getGraphicsManager();
	auto windowDim = engine_.getConfigManager().getWindowDim();
	Math::eVec2<float> windowSize{
			static_cast<float>(windowDim.first),
			static_cast<float>(windowDim.second)
	};

	// Draw background
	graphics.drawTexturedQuadUV(
		Math::eVec2<float>{0.0f, 0.0f},           // Position at origin
		windowSize,                                // Full screen size from config
		backgroundTexture_,
		Math::eVec2<float>{0.0f, 0.0f},           // UV start
		Math::eVec2<float>{1.0f, 1.0f},           // UV end
		Graphics::Color::White
	);

	

	// REBUILD CACHE ONLY WHEN DIRTY (optimization)
	if (entitiesDirty_) {
		cachedDrawableEntities_ = engine_.getEntityManager().findEntity<ECSComponents::DrawInfoComponent>();
		cachedSceneTwoEntities_ = engine_.getEntityManager().findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));
		entitiesDirty_ = false;
	}

	// Check if player is moving and switch animation accordingly
	if (playerSystem_ && playerSpawned_) {
		// Check for WASD input
		bool isMoving = 
			inputManager.isKeyPressed(GLFW_KEY_W) ||
			inputManager.isKeyPressed(GLFW_KEY_A) ||
			inputManager.isKeyPressed(GLFW_KEY_S) ||
			inputManager.isKeyPressed(GLFW_KEY_D);

		// Update facing direction based on A/D keys
		if (inputManager.isKeyPressed(GLFW_KEY_D)) {
			playerFacingDirection_ = -1;  // Face right
		}
		else if (inputManager.isKeyPressed(GLFW_KEY_A)) {
			playerFacingDirection_ = 1;  // Face left
		}

		// Switch animation based on movement
		if (isMoving) {
			if (playerAnimController_.getCurrentAnimationName() != "walk") {
				playerAnimController_.setAnimation("walk");
			}
		}
		else {
			if (playerAnimController_.getCurrentAnimationName() != "idle") {
				playerAnimController_.setAnimation("idle");
			}
		}

		// Update player movement
		playerSystem_->Update(engine_.getEntityManager(), inputManager, deltaTime);
	}

	// Update collision system
	if (collisionSystem_) {
		collisionSystem_->Update(engine_.getEntityManager());
	}
	
	static bool frameUpdate = false;
	static bool keyFrameMode = false;

	if (inputManager.isKeyDown(GLFW_KEY_M))
	{
		// Get list of entities which I want to store current pos
		auto& ent = engine_.getEntityManager();
		std::vector<ECS::Entity> entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));

		// This toggles keyframe mode
		keyFrameMode = !keyFrameMode;

		// If true, store all the positions
		if (keyFrameMode)
		{

			if (!entityList.empty())
			{
				for (auto& e : entityList)
				{
					e.get<ECSComponents::TransformComponent>().resetPos = e.get<ECSComponents::TransformComponent>().pos;
					std::cout << "Reset pos: " << e.get<ECSComponents::TransformComponent>().resetPos << std::endl;
				}

			}

		}
		else
		{
			std::cout << "keyframe off! \n";
			// Reset all object positions
			for (auto& e : entityList)
			{
				std::cout << "Before reset : " << e.get<ECSComponents::TransformComponent>().pos << std::endl;
				e.get<ECSComponents::TransformComponent>().pos = e.get<ECSComponents::TransformComponent>().resetPos;
				std::cout << "After reset : " << e.get<ECSComponents::TransformComponent>().resetPos << std::endl;
			}
			
		}
	}
	// In key frame mode, wait for trigger to update
	if (keyFrameMode)
	{

		if (inputManager.isKeyDown(GLFW_KEY_N))
		{
			// Frame update 
			frameUpdate = true;
		}

		if (frameUpdate)
		{
			// Update one time
			updateTestObject(engine_.getEntityManager(), engine_);

			// Set it back to false
			frameUpdate = false;
		}
	}
	else
	{
		//not in key frame mode, update normally
		updateTestObject(engine_.getEntityManager(), engine_);
	}
}


void GameApplication::drawText(Math::eVec2<float> windowSize) {
	// DEMO DRAW TEXT
	auto& graphics = engine_.getGraphicsManager();
	auto& input = engine_.getInputManager();
	std::string text;
	if (input.isMousePressed(GLFW_MOUSE_BUTTON_LEFT) && input.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		text = "Left and Right Mouse Button pressed!";
	}
	else if (input.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
		text = "Left Mouse Button pressed!";
	}
	else if (input.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		text = "Right Mouse Button pressed!";
	}
	else {
		text = "No Mouse Button pressed!";
	}
	std::stringstream fpsSs;
	fpsSs.precision(2);      // 2 significant digits
	fpsSs << std::fixed;     // fixed-point notation
	fpsSs << "FPS: " << engine_.getFPS();
	graphics.drawText(timesFont_, text, { 25.f , 50.f }, 1.f, Graphics::Color::Black);
	graphics.drawText(timesFont_, fpsSs.str(), {windowSize.x - 125.f , 25.f}, 0.5f, Graphics::Color::Red);
	graphics.drawText(pixelifyFont_, "Copyright reserved RuneHolders", { windowSize.x / 2.f, windowSize.y - 25.f }, 0.75f, Graphics::Color::Blue);
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

				// Get the current animation frame
				auto &frame = playerAnimController_.getCurrentFrame();

				// Select the correct texture based on current animation
				std::shared_ptr<Graphics::EnoTexture> currentTexture;
				if (playerAnimController_.getCurrentAnimationName() == "walk") {
					currentTexture = playerWalkTexture_;
				}
				else {
					currentTexture = playerTexture_;
				}

				// Flip UV coordinates based on facing direction
				Math::eVec2<float> uvStart = frame.uvStart;
				Math::eVec2<float> uvEnd = frame.uvEnd;

				// Get player transform
				auto& playerTransform = playerSystem_->GetTransform(it->second);
				
				if (engine_.getInputManager().isKeyPressed(GLFW_KEY_1)) {
					// I am placing it here first as a 4.0 scale speed * DT
					scaleSize += static_cast<float>(3.0 * 0.01667);
					scaleSize = std::clamp(scaleSize, 1.0f, 100.0f);
					playerTransform.scale.x = scaleSize;
					playerTransform.scale.y = scaleSize;
					std::cout << "Scaling by: " << scaleSize << '\n';				
				}

				if (engine_.getInputManager().isKeyPressed(GLFW_KEY_2)) {
					scaleSize -= static_cast < float>(3.0 * 0.01667);
					scaleSize = std::clamp(scaleSize, 1.0f, 100.0f);
					playerTransform.scale.x = scaleSize;
					playerTransform.scale.y = scaleSize;
					std::cout << "Scaling by: " << scaleSize << '\n';
				}

				if (engine_.getInputManager().isKeyPressed(GLFW_KEY_3)) {
					rotationRadians += static_cast < float>(3.0f * 0.0174533 * 0.01667);
					//playerTransform.rot.z = rotationRadians;
					std::cout << "rotation in Radians:" << rotationRadians << '\n';
				}

				if (engine_.getInputManager().isKeyPressed(GLFW_KEY_4)) {
					rotationRadians -= static_cast < float>(3.0f * 0.0174533 * 0.01667);
					//playerTransform.rot.z = rotationRadians;
					std::cout << "rotation in Radians:" << rotationRadians << '\n';
				}


				if (playerFacingDirection_ == -1) {
					// Swap horizontal UV coordinates to flip the sprite
					std::swap(uvStart.x, uvEnd.x);
				}

				// Draw with the correct texture
				if (currentTexture && currentTexture->isValid()) {
					
					// calculate scaled dimensions (match the collider size)
					float scaledWidth = (Player::PLAYER_SPRITE_WIDTH * scaleSize);
					float scaledHeight = (Player::PLAYER_SPRITE_HEIGHT * scaleSize);
					
					// convert center position to top-left for drawing
					Math::eVec2<float> topLeft = {
						transform.pos.x - (scaledWidth * 0.5f),
						transform.pos.y - (scaledHeight * 0.5f)
					};
					
					graphics.drawTexturedQuadUV(
						topLeft,
						Math::eVec2<float>{scaledWidth, scaledHeight},
						currentTexture,
						uvStart,  // Use potentially flipped UV coords
						uvEnd,    // Use potentially flipped UV coords
						Graphics::Color::White,
						rotationRadians
					);
				}
				else {
					// Fallback to colored rect if texture invalid
					graphics.drawRect(
						Math::eVec2<float>{transform.pos.x, transform.pos.y},
						Math::eVec2<float>{10 + scaleSize, 10 +scaleSize},
						Graphics::Color::Blue,
						true,
						rotationRadians
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
	SystemTimer ImGuiTimer("ImGui", systemTimes_);
	imGuiDisplay(engine_);
}

void GameApplication::drawShapes() {
	SystemTimer GraphicsTimer("Graphics", systemTimes_);

	auto& graphics = engine_.getGraphicsManager();
	auto& entityManager = engine_.getEntityManager();

	using drawInfo = ECSComponents::DrawInfoComponent;

	// USE CACHED ENTITIES with validation
	for (ECS::Entity& entity : cachedDrawableEntities_) {
		// Skip if entity was destroyed
		try {
			if (entity.has<ECSComponents::DrawInfoComponent>()) {
				graphics.drawRect(
					Math::eVec2<float>{entity.get<drawInfo>().xPos, entity.get<drawInfo>().yPos},
					Math::eVec2<float>{entity.get<drawInfo>().width, entity.get<drawInfo>().height},
					entity.get<drawInfo>().color,
					true
				);
			}
		}
		catch (...) {
			// Entity was destroyed, rebuild cache next frame
			entitiesDirty_ = true;
			continue;
		}
	}

	// (kim) visualize colliders if enabled
	if (showColliders_ && collisionSystem_) {
		collisionSystem_->VisualizeColliders(graphics, entityManager);
	}

}


void GameApplication::setPlayerTexture(std::shared_ptr<Graphics::EnoTexture> newTex) {
	playerTexture_ = newTex;
	playerWalkTexture_ = newTex;
}