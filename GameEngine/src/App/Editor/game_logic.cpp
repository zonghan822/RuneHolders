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
    setupAnimations();
	pixelifyFont_ = engine_.getResourceLoader().getFont("PixelifySans");
	timesFont_ = engine_.getResourceLoader().getFont("times");

    // This is for dynamic collision test object
    createTestObject(engine_.getEntityManager());

	gameFramebuffer_ = std::make_unique<Graphics::EnoFramebuffer>();
	gameFramebuffer_->create(VIRTUAL_WIDTH, VIRTUAL_HEIGHT); // Fixed virtual resolution

	editorFramebuffer_ = std::make_unique<Graphics::EnoFramebuffer>();
	editorFramebuffer_->create(EDITOR_WIDTH, EDITOR_HEIGHT); // Zoomed in editor view

	// Center editor camera on the middle of the game world initially
	editorCameraX_ = (VIRTUAL_WIDTH - EDITOR_WIDTH) / 2.0f;
	editorCameraY_ = (VIRTUAL_HEIGHT - EDITOR_HEIGHT) / 2.0f;

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
	
	// Math lib and maybe collision
	//std::cout << "Calling math function\n";
	//Math::eVec2 vec{ 1.1f, 1.2f };
	//Math::eVec2 vec2{ 1.3f, 2.f };
	//vec += vec2;
	//std::cout << "vector : " << vec.x << ", " << vec.y << std::endl;

	Collision::RunBasicCollisionTest();

	// Initialize collision system
	collisionSystem_ = std::make_unique<Collision::CollisionSystem>(engine_.getEventBus());

	// Initialize player system
	playerSystem_ = std::make_unique<Player::PlayerSystem>();

	// Load player config
	playerConfig_.LoadFromFile(CONFIG_DIR_PATH "player/player_config.txt");
	std::cout << "[GameLogic] Player configg loaded" << std::endl;
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
	SystemTimer totalTimePerFrame("totalTimePerFrame", systemTimes_);
	double deltaTime = engine_.getDeltaTime();

	// Use FIXED virtual resolution for consistent rendering
	Math::eVec2<float> renderSize{
		static_cast<float>(VIRTUAL_WIDTH),
		static_cast<float>(VIRTUAL_HEIGHT)
	};

	if (gameFramebuffer_) {
		// CRITICAL: Set projection matrix to fixed virtual resolution
		engine_.getGraphicsManager().setProjectionMatrix(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

		// Bind framebuffer for rendering
		gameFramebuffer_->bind();
	}

	// Clear the framebuffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (playerTexture_ && playerSpawned_) {
		playerAnimController_.update(static_cast<float>(deltaTime));
	}

	updateEntities();
	drawShapes();
	drawPlayerAndIndicator();
	engine_.getGraphicsManager().flush();
	drawText(renderSize);

	// Unbind game framebuffer
	if (gameFramebuffer_) {
		gameFramebuffer_->unbind();
	}

	// ==== RENDER EDITOR VIEW (ZOOMED IN WITH CAMERA) ====
	if (editorFramebuffer_) {
		// NOTE: WASD controls BOTH player movement AND editor camera
		// This is intentional - allows you to navigate while keeping player in view
		// To move ONLY the camera, pause the game or use arrow keys for indicator instead
		auto& inputManager = engine_.getInputManager();
		float cameraDelta = editorCameraSpeed_ * static_cast<float>(deltaTime);
		if (inputManager.isKeyPressed(GLFW_KEY_W)) editorCameraY_ -= cameraDelta;
		if (inputManager.isKeyPressed(GLFW_KEY_S)) editorCameraY_ += cameraDelta;
		if (inputManager.isKeyPressed(GLFW_KEY_A)) editorCameraX_ -= cameraDelta;
		if (inputManager.isKeyPressed(GLFW_KEY_D)) editorCameraX_ += cameraDelta;

		// Clamp camera to world bounds
		editorCameraX_ = std::clamp(editorCameraX_, 0.0f, static_cast<float>(VIRTUAL_WIDTH - EDITOR_WIDTH));
		editorCameraY_ = std::clamp(editorCameraY_, 0.0f, static_cast<float>(VIRTUAL_HEIGHT - EDITOR_HEIGHT));

		// Set projection for editor view
		engine_.getGraphicsManager().setProjectionMatrix(EDITOR_WIDTH, EDITOR_HEIGHT);
		editorFramebuffer_->bind();

		// Clear editor framebuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw background - crop to visible camera area using UV coordinates
		// Calculate what portion of the background texture is visible
		float uvStartX = editorCameraX_ / static_cast<float>(VIRTUAL_WIDTH);
		float uvStartY = editorCameraY_ / static_cast<float>(VIRTUAL_HEIGHT);
		float uvEndX = (editorCameraX_ + EDITOR_WIDTH) / static_cast<float>(VIRTUAL_WIDTH);
		float uvEndY = (editorCameraY_ + EDITOR_HEIGHT) / static_cast<float>(VIRTUAL_HEIGHT);

		// Draw only the visible portion of the background at origin
		engine_.getGraphicsManager().drawTexturedQuadUV(
			Math::eVec2<float>{0.0f, 0.0f},  // Draw at origin (camera offset handled by UV)
			Math::eVec2<float>{static_cast<float>(EDITOR_WIDTH), static_cast<float>(EDITOR_HEIGHT)},
			backgroundTexture_,
			Math::eVec2<float>{uvStartX, uvStartY},  // Crop texture start
			Math::eVec2<float>{uvEndX, uvEndY},      // Crop texture end
			Graphics::Color::White
		);

		// Draw all entities (with camera offset - convert world space to screen space)
		for (ECS::Entity& entity : cachedDrawableEntities_) {
			try {
				if (entity.has<ECSComponents::DrawInfoComponent>()) {
					if (entity.has<ECSComponents::PlayerComponent>()) {
						continue; // Skip player, drawn separately
					}

					auto& transform = entity.get<ECSComponents::TransformComponent>();
					auto& renderInfo = entity.get<ECSComponents::DrawInfoComponent>();

					// Convert world position to screen position (relative to camera)
					float screenX = transform.pos.x - editorCameraX_;
					float screenY = transform.pos.y - editorCameraY_;

					// Only draw if entity is within visible camera bounds
					float halfW = renderInfo.width * 0.5f;
					float halfH = renderInfo.height * 0.5f;
					if (screenX + halfW >= 0 && screenX - halfW < EDITOR_WIDTH &&
						screenY + halfH >= 0 && screenY - halfH < EDITOR_HEIGHT) {

						float topLeftX = screenX - halfW;
						float topLeftY = screenY - halfH;

						engine_.getGraphicsManager().drawRect(
							Math::eVec2<float>{topLeftX, topLeftY},
							Math::eVec2<float>{renderInfo.width, renderInfo.height},
							renderInfo.color,
							true
						);
					}
				}
			}
			catch (...) {
				entitiesDirty_ = true;
				continue;
			}
		}

		// Draw player in editor view (with camera offset - convert to screen space) - NO RED CURSOR
		if (playerSpawned_) {
			try {
				auto& entityMap = engine_.getEntityManager().getMap();
				auto it = entityMap.find(playerEntityID_);
				if (it != entityMap.end()) {
					auto& transform = playerSystem_->GetTransform(it->second);
					auto &frame = playerAnimController_.getCurrentFrame();

					std::shared_ptr<Graphics::EnoTexture> currentTexture;
					if (playerAnimController_.getCurrentAnimationName() == "walk") {
						currentTexture = playerWalkTexture_;
					}
					else {
						currentTexture = playerTexture_;
					}

					Math::eVec2<float> uvStart = frame.uvStart;
					Math::eVec2<float> uvEnd = frame.uvEnd;

					if (playerFacingDirection_ == -1) {
						std::swap(uvStart.x, uvEnd.x);
					}

					if (currentTexture && currentTexture->isValid()) {
						float scaledWidth = (playerConfig_.spriteWidth * scaleSize);
						float scaledHeight = (playerConfig_.spriteHeight * scaleSize);

						// Convert world position to screen position (relative to camera)
						float screenX = transform.pos.x - editorCameraX_;
						float screenY = transform.pos.y - editorCameraY_;

						Math::eVec2<float> topLeft = {
							screenX - (scaledWidth * 0.5f),
							screenY - (scaledHeight * 0.5f)
						};

						engine_.getGraphicsManager().drawTexturedQuadUV(
							topLeft,
							Math::eVec2<float>{scaledWidth, scaledHeight},
							currentTexture,
							uvStart,
							uvEnd,
							Graphics::Color::White,
							rotationRadians
						);
					}
				}
			}
			catch (...) {}
		}

		// Draw colliders in editor view (with camera offset)
		if (showColliders_ && collisionSystem_) {
			// TODO: This would need camera offset support in visualization
			// For now, skip or implement camera-aware collider drawing
		}

		engine_.getGraphicsManager().flush();
		editorFramebuffer_->unbind();
	}

	// CRITICAL: Restore viewport to window size for ImGui rendering
	auto windowSize = engine_.getWindowManager().getFramebufferSize();
	glViewport(0, 0, windowSize.first, windowSize.second);

	// Restore projection matrix to window size
	engine_.getGraphicsManager().setProjectionMatrix(windowSize.first, windowSize.second);

	// Now render ImGui (which will display the framebuffer textures)
	updateImGui();
}

void GameApplication::updateEntities() {
	SystemTimer EntityTimer("Entities" , systemTimes_);

	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;
	auto& inputManager = engine_.getInputManager();
	auto& graphics = engine_.getGraphicsManager();

	// Use fixed virtual resolution for consistent rendering
	Math::eVec2<float> renderSize{
		static_cast<float>(VIRTUAL_WIDTH),
		static_cast<float>(VIRTUAL_HEIGHT)
	};

	// Draw background
	graphics.drawTexturedQuadUV(
		Math::eVec2<float>{0.0f, 0.0f},           // Position at origin
		renderSize,                                // Use fixed virtual resolution
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
	
	static bool keyFrameMode = false;
	static bool playAfterStop = true;

	//std::cout << "outside Editor state: " << _editorState << std::endl;
	// Editor state = play game
	switch (getEditorState())
	{
	case Playing:
		{
			std::cout << "Editor state: " << getEditorState() << std::endl;
			// Get list of entities to store current pos to reset later
			// Check that the last frame editor state was 'Stopped'
			if (playAfterStop)
			{
				UpdateResetPos();
				playAfterStop = false;
			}

			// Update dynamic objects
			updateTestObject(engine_.getEntityManager(), engine_);
			break;
		}
		case Paused:
		{
			// This toggles keyframe mode
			keyFrameMode = true;
			break;
		}
		case NextKeyFrame:
		{
			// Check if key frame mode is true
			if (keyFrameMode)
			{
				// Update one time
				updateTestObject(engine_.getEntityManager(), engine_);

				// Reset variables
				setEditorState(Paused);
			}
			break;
		}
		case Stopped:
		{
			// Reset all object positions
			// This condition ensures resetting of pos comes after play
			if (!playAfterStop)
			{
				UpdateResetPos();
				playAfterStop = true;
			}
			break;
		}
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
					
					auto& playerCollider = playerSystem_->GetCollision(it->second);
					playerCollider.rotation = rotationRadians;

					std::cout << "rotation in Radians:" << rotationRadians << '\n';
				}

				if (engine_.getInputManager().isKeyPressed(GLFW_KEY_4)) {
					rotationRadians -= static_cast < float>(3.0f * 0.0174533 * 0.01667);

					auto& playerCollider = playerSystem_->GetCollision(it->second);
					playerCollider.rotation = rotationRadians;
					std::cout << "rotation in Radians:" << rotationRadians << '\n';
				}


				if (playerFacingDirection_ == -1) {
					// Swap horizontal UV coordinates to flip the sprite
					std::swap(uvStart.x, uvEnd.x);
				}

				// Draw with the correct texture
				if (currentTexture && currentTexture->isValid()) {
					
					// calculate scaled dimensions (match the collider size)
					float scaledWidth = (playerConfig_.spriteWidth * scaleSize);
					float scaledHeight = (playerConfig_.spriteHeight * scaleSize);
					
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
				// Skip rendering for player entities (they're drawn separately with sprite/animation)
				if (entity.has<ECSComponents::PlayerComponent>()) {
					continue;
				}

				// Get position from TransformComponent (spatial data)
				// Get rendering info from DrawInfoComponent (size, color, etc.)
				auto& transform = entity.get<ECSComponents::TransformComponent>();
				auto& renderInfo = entity.get<drawInfo>();

				// TransformComponent.pos is the CENTER, but drawRect expects TOP-LEFT
				// Convert from center to top-left
				float topLeftX = transform.pos.x - (renderInfo.width * 0.5f);
				float topLeftY = transform.pos.y - (renderInfo.height * 0.5f);

				graphics.drawRect(
					Math::eVec2<float>{topLeftX, topLeftY},  // Top-left position for drawRect
					Math::eVec2<float>{renderInfo.width, renderInfo.height},
					renderInfo.color,
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

void GameApplication::applyTextureToPlayer(const Graphics::TextureData& texData) {
	if (!texData.texture) return;

	playerTexture_ = texData.texture;
	std::cout << "Applied texture: " << texData.texture->getWidth()
		<< "x" << texData.texture->getHeight()
		<< " | Frame: " << texData.frameWidth << "x" << texData.frameHeight << '\n';

	// You could use texData.frameWidth/Height to rebuild the animation
	auto playerIdleAnim = Animation::createSpriteSheetAnimation(
		texData.frameWidth,
		texData.frameHeight,
		texData.texture->getWidth(),
		texData.texture->getHeight(),
		0,
		texData.rows * texData.cols,
		0.1f
	);
	playerIdleAnim.setLooping(true);

	//auto playerWalkAnim = Animation::createSpriteSheetAnimation(
	//	texData.frameWidth,
	//	texData.frameHeight,
	//	texData.texture->getWidth(),
	//	texData.texture->getHeight(),
	//	0,
	//	texData.rows * texData.cols,
	//	0.8f
	//);
	//playerWalkAnim.setLooping(true);


	playerAnimController_.addAnimation("idle", playerIdleAnim);
	//playerAnimController_.addAnimation("walk", playerWalkAnim);

	// Keep the same current state
	if (playerAnimController_.getCurrentAnimationName().empty())
		playerAnimController_.setAnimation("idle");
}

// Eventually i would want to restore scene state through serialization
void GameApplication::UpdateResetPos()
{
	// Get list of entities to store current pos to reset later
	auto& ent = engine_.getEntityManager();
	static std::vector<ECS::Entity> entityList;
	// If empty, store pos, else reset
	if (entityList.empty())
	{
		entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven")); // This will change to all dynamic entities in future
		for (auto& e : entityList)
		{
			// Store reset pos
			e.get<ECSComponents::TransformComponent>().resetPos = e.get<ECSComponents::TransformComponent>().pos;

			// Store reset velocity if entity has physics component
			if (e.has<ECSComponents::PhysicsComponent>())
			{
				// Store current velocity for reset (you may want to add resetVelocity to PhysicsComponent)
				// For now we'll just reset to zero velocity
			}

			//std::cout << "Reset pos: " << e.get<ECSComponents::TransformComponent>().resetPos << std::endl;
		}
	}
	else
	{
		for (auto& e : entityList)
		{
			//std::cout << "Before reset : " << e.get<ECSComponents::TransformComponent>().pos << std::endl;
			e.get<ECSComponents::TransformComponent>().pos = e.get<ECSComponents::TransformComponent>().resetPos;

			// Reset velocity if entity has physics component
			if (e.has<ECSComponents::PhysicsComponent>())
			{
				e.get<ECSComponents::PhysicsComponent>().velocity = { 0.0f, 0.0f };
			}
			//std::cout << "After reset : " << e.get<ECSComponents::TransformComponent>().resetPos << std::endl;
		}
		// Clear list
		entityList.clear();
	}
	
}

// Accessor and mutators to access editor state
EditorState GameApplication::getEditorState() const
{
	return g_editorState;
}
EditorState GameApplication::getEditorState()
{
	return g_editorState;
}

void GameApplication::setEditorState(EditorState state)
{
	g_editorState = state;
}