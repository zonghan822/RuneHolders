/*============================================================================================
* All content ? 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         game_logic.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @author		Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         05-11-2025
* @brief        Game application layer that handles game-specific logic, entity management,
*               rendering, and ImGui integration. Systems are now managed by EnoEngine.
*//*========================================================================================*/

#include "game_logic.hpp"
#include <GLFW/glfw3.h>
#include "imGui.hpp"
#include "math/math.hpp"


void GameApplication::initialize() {
	// Setup engine systems first (this now includes game systems)
	engine_.setup();

	// Initialize player system
	playerSystem_ = std::make_unique<Player::PlayerSystem>();

	// Setup all game content
	setupPrefabs();
	setupGameSystems();
	setupGameAudio();
	setupTreeTesting();
	setupGameEvents();
	setupGameMath();
	setupImGui();
	setupAnimations();
	setupScripting();
	setupFont();
	setupCombatSystem();

	auto& tileMap = engine_.getTileMap();

	// Set up tile map editor (To be removed after tile map node is set up)
	mapEditor_ = std::make_unique<MapEditor>(tileMap);
	Math::eVec2<float> worldSize = tileMap.getWorldSize();

	worldWidth_ = worldSize.x;
	worldHeight_ = worldSize.y;

	gameFramebuffer_ = std::make_unique<Graphics::EnoFramebuffer>();
	gameFramebuffer_->create(static_cast<int>(worldWidth_), static_cast<int>(worldHeight_)); // Fixed virtual resolution

	editorFramebuffer_ = std::make_unique<Graphics::EnoFramebuffer>();
	editorFramebuffer_->create(EDITOR_WIDTH, EDITOR_HEIGHT); // Zoomed in editor view

	// Center editor camera on the middle of the game world initially
	editorCameraX_ = (worldWidth_ / 2.0f) - static_cast<float>(EDITOR_WIDTH) / 2.f;
	editorCameraY_ = (worldHeight_ / 2.0f) - static_cast<float>(EDITOR_HEIGHT) / 2.f;

	// Subscribe to delete selected entity event (using GameApplication's scene tree)
	engine_.getEventBus().subscribe<Editor::DeleteSelectedEntity>([this](const Editor::DeleteSelectedEntity&)
		{
			extern void deleteSelectedEntity(Editor::EnoSceneTree&, ECS::EntityManager&, Editor::UndoManager&);
			//deleteSelectedEntity(*sceneTree_, engine_.getEntityManager(), engine_.getEnoEditor().getUndoManager());
			deleteSelectedEntity(engine_.getEnoEditor().getSceneTree(), engine_.getEntityManager(), engine_.getEnoEditor().getUndoManager());
		});

	// Subscribe to scene cleared event to reset UI state
	engine_.getEventBus().subscribe<Editor::SceneCleared>([this](const Editor::SceneCleared&)
		{
			extern void clearSelectedEntity();
			clearSelectedEntity();
			// Clear entity caches immediately to prevent accessing destroyed entities
			cachedDrawableEntities_.clear();
			cachedSceneTwoEntities_.clear();
			entitiesDirty_ = true;
			std::cout << "[GameApplication] Scene cleared, reset selected entity and cleared caches" << std::endl;
		});

	std::cout << "Game initialized successfully!" << std::endl;
}

void GameApplication::run() {
	std::cout << "Starting game..." << std::endl;

	// Start the engine with game update callback
	engine_.run([this]() {
		updateGameLogic();  // This gets called every frame
		});

}

void GameApplication::shutdown() {
	std::cout << "Terminating game..." << std::endl;
	engine_.shutDown();
	imGuiTerminate();
}

void GameApplication::setupPrefabs() {
	auto prefabManager = engine_.getPrefabManager();
	prefabManager.registerPresets();

	ECSComponents::Preset::saveAll(prefabManager);
	ECSComponents::Preset::loadAll(prefabManager);
}

void GameApplication::setupGameSystems() {
	

}

void GameApplication::setupFont() {
	pixelifyFont_ = engine_.getResourceLoader().getFont("PixelifySans");
	timesFont_ = engine_.getResourceLoader().getFont("times");
}

void GameApplication::setupCombatSystem() {
	auto& entityManager = engine_.getEntityManager();

	std::cout << "[Editor] Setting up combat system..." << std::endl;

	// Find player entity and add HealthComponent
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	if (!playerEntities.empty()) {
		ECS::Entity& playerEntity = playerEntities[0];
		if (!playerEntity.has<ECSComponents::HealthComponent>(entityManager)) {
			playerEntity.add<ECSComponents::HealthComponent>(entityManager, 100.0f);
			std::cout << "[CombatSystem] Added HealthComponent to player (100 HP)" << std::endl;
		}
	}

	// Find all enemies with AIComponent and add health/damage
	std::vector<ECS::Entity> enemies = entityManager.findEntity<ECSComponents::AIComponent>();
	for (auto& enemy : enemies) {
		// Add or reset HealthComponent
		if (!enemy.has<ECSComponents::HealthComponent>(entityManager)) {
			enemy.add<ECSComponents::HealthComponent>(entityManager, 50.0f);
			std::cout << "[CombatSystem] Added HealthComponent to enemy (50 HP)" << std::endl;
		} else {
			// Reset health to max if component already exists
			auto& health = enemy.get<ECSComponents::HealthComponent>(entityManager);
			health.currentHealth = 50.0f;
			health.maxHealth = 50.0f;
			health.isDead = false;
			health.invulnerable = false;
			health.invulnerabilityTimer = 0.0f;
			std::cout << "[CombatSystem] Reset enemy health to 50 HP" << std::endl;
		}

		// Add DamageComponent if not already present
		if (!enemy.has<ECSComponents::DamageComponent>(entityManager)) {
			enemy.add<ECSComponents::DamageComponent>(entityManager, 10.0f);
			std::cout << "[CombatSystem] Added DamageComponent to enemy (10 damage)" << std::endl;
		}
	}

	std::cout << "[Editor] Combat system setup complete!" << std::endl;
}

void GameApplication::setupScripting() {
	// Scripting premade calls
	engine_.getCoreHost().HRSetup();
	engine_.getCoreHost().defineBegin();
	engine_.getCoreHost().defineUpdate();
	engine_.getCoreHost().defineSMD();
}


void GameApplication::setupGameAudio() {
	// setup audio
	SystemTimer totalTimePerFrame("Audio", systemTimes_);
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
	inputMng.setupKey("Play SFX1", GLFW_KEY_Z, glfwGetKeyScancode(GLFW_KEY_Z), GLFW_RELEASE, 0);
	inputMng.setupKey("Play SFX2", GLFW_KEY_X, glfwGetKeyScancode(GLFW_KEY_X), GLFW_RELEASE, 0);
	inputMng.setupKey("Play SFX3", GLFW_KEY_C, glfwGetKeyScancode(GLFW_KEY_C), GLFW_RELEASE, 0);

	inputMng.setupEvent<Audio::PlayInstant>("Play SFX1", "SFX", "Flask");
	inputMng.setupEvent<Audio::PlayInstant>("Play SFX2", "SFX", "Stone_Impact");
	inputMng.setupEvent<Audio::PlayInstant>("Play SFX3", "SFX", "Water_Drip");

	// Undo/Redo System
	// Ctrl+Z for undo
	inputMng.setupKey("Undo", GLFW_KEY_Z, glfwGetKeyScancode(GLFW_KEY_Z), GLFW_RELEASE, GLFW_MOD_CONTROL);
	inputMng.setupEvent<Editor::UndoAction>("Undo");

	// Ctrl+Shift+Z for redo
	inputMng.setupKey("Redo", GLFW_KEY_Z, glfwGetKeyScancode(GLFW_KEY_Z), GLFW_RELEASE, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT);
	inputMng.setupEvent<Editor::RedoAction>("Redo");
}

void GameApplication::setupTreeTesting()
{
	auto& inputMng = engine_.getInputManager();

	inputMng.setupKey("Test Editor Tree", GLFW_KEY_Q, glfwGetKeyScancode(GLFW_KEY_Q), GLFW_RELEASE, 0);
	inputMng.setupEvent<Editor::TestTree>("Test Editor Tree");

	// Setup Delete key for entity deletion
	inputMng.setupKey("Delete Selected Entity", GLFW_KEY_DELETE, glfwGetKeyScancode(GLFW_KEY_DELETE), GLFW_RELEASE, 0);
	inputMng.setupEvent<Editor::DeleteSelectedEntity>("Delete Selected Entity");
}

void GameApplication::setupGameEvents() {
	// Subscribe to collision events for trigger tracking
	auto& eventBus = engine_.getEventBus();

	eventBus.subscribe<Collision::CollisionEvent>(
		[this](const Collision::CollisionEvent& event) {
			// Check if one of the entities is the player and the other is a trigger
			uint32_t triggerID = 0;
			bool isPlayerInvolved = false;

			// Check if entityA is trigger and entityB is player
			if (event.isEntityA_Trigger && event.entityB_ID == playerEntityID_) {
				triggerID = event.entityA_ID;
				isPlayerInvolved = true;
			}
			// Check if entityB is trigger and entityA is player
			else if (event.isEntityB_Trigger && event.entityA_ID == playerEntityID_) {
				triggerID = event.entityB_ID;
				isPlayerInvolved = true;
			}

			// If player is colliding with a trigger, add it to current frame's set
			if (isPlayerInvolved && triggerID != 0) {
				currentTriggersPlayerIsIn_.insert(triggerID);
			}
		}
	);

	// Subscribe to EntityDestroyed to clean up tracking
	eventBus.subscribe<ECS::EntityDestroyed>(
		[this](const ECS::EntityDestroyed& event) {
			// Remove destroyed entity from both tracking sets
			currentTriggersPlayerIsIn_.erase(event.entityID);
			previousTriggersPlayerWasIn_.erase(event.entityID);
		}
	);
}

void GameApplication::setupGameMath() {

	// Math lib and maybe collision
	//std::cout << "Calling math function\n";
	//Math::eVec2 vec{ 1.1f, 1.2f };
	//Math::eVec2 vec2{ 1.3f, 2.f };
	//vec += vec2;
	//std::cout << "vector : " << vec.x << ", " << vec.y << std::endl;

	Collision::RunBasicCollisionTest();

	// NOTE: Systems are now initialized in eno_engine.cpp::setupGameSystems()
	// No need to initialize them here anymore

	// Load player config
	playerConfig_.LoadFromFile(APP_SRC_PATH "player/player_config.txt");
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

//void GameApplication::createTestObjectsAndAddToSceneTree() {
//	auto& entityManager = engine_.getEntityManager();
//
//	// Create the test objects (boxes)
//	createTestObject(entityManager);
//
//
//	// Now add all entities with NameComponent "testcaven" to the scene tree
//	std::vector<ECS::Entity> testEntities = entityManager.findEntity<ECSComponents::NameComponent>(
//		ECSComponents::NameComponent("testcaven")
//	);
//
//	std::cout << "[GameLogic] Adding " << testEntities.size() << " test objects to scene tree..." << std::endl;
//
//	for (auto& entity : testEntities) {
//		// Add each entity to the scene tree as a root node
//		engine_.getEnoEditor().getSceneTree().createFromEntity(entity, nullptr);
//		std::cout << "[GameLogic] Added entity " << entity.id() << " to scene tree" << std::endl;
//	}
//
//	std::cout << "[GameLogic] Test objects added to scene tree successfully!" << std::endl;
//}

void GameApplication::setupImGui() {
	GLFWwindow* window = engine_.getWindowManager().getWindow();
	imGuiInit(window);
	extern GameApplication* g_game;
	g_game = this;
}

void GameApplication::updateGameLogic() {
	auto& entityManager = engine_.getEntityManager();
	auto& graphics_ = engine_.getGraphicsManager();
	auto& inputMng = engine_.getInputManager();
	auto& tileMap = engine_.getTileMap();
	auto& loader = engine_.getResourceLoader();

	Math::eVec2<float> worldSize = tileMap.getWorldSize();
	worldWidth_ = worldSize.x, worldHeight_ = worldSize.y;

	SystemTimer totalTimePerFrame("totalTimePerFrame", systemTimes_);
	float deltaTime = static_cast<float>(engine_.getDeltaTime());

	// Only update player movement when in play mode
	if (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing || engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame) {
		playerSystem_->Update(entityManager, inputMng, deltaTime);
	}

	// Update rune system
	updateRuneSystem();

	// Update combat system (damage dealing and health)
	updateCombatSystem(deltaTime);

	// Update engine playing state based on editor state
	engine_.setIsPlaying(engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing || engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame);

	if (gameFramebuffer_) {

		// CRITICAL: Set projection matrix to fixed virtual resolution
		engine_.getGraphicsManager().setProjectionMatrix(static_cast<int>(worldWidth_), static_cast<int>(worldHeight_));

		// Bind framebuffer for rendering
		gameFramebuffer_->bind();
	}

	// Clear the framebuffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (playerTexture_ && playerSpawned_) {
		playerAnimController_.update(deltaTime);
	}

	// Update all animated entities (spritesheet animation and rotation)
	static float animationTimer = 0.0f;
	static int currentFrame = 0;
	const int totalFrames = 4;  // Protagonist-Walk-Sheet has 4 frames
	const float frameDuration = 0.1f;  // 0.1 seconds per frame

	animationTimer += deltaTime;
	if (animationTimer >= frameDuration) {
		animationTimer -= frameDuration;
		currentFrame = (currentFrame + 1) % totalFrames;
	}
	
	// ========= Audio =========
	// CRITICAL FIX: Only handle autoplay on scene start, not every frame
	// Manual play is triggered via the "Play" button in ImGui

	// Track scene state to detect when we enter play mode
	static Editor::State lastSceneState = Editor::State::Stopped;
	Editor::State currentSceneState = engine_.getEnoEditor().getCurrentScene().getState();

	// Check if we just entered play mode (scene state changed to Playing)
	bool justStartedPlaying = (lastSceneState != Editor::State::Playing &&
	                           currentSceneState == Editor::State::Playing);

	if (justStartedPlaying)
	{
		// Scene just started - handle autoplay for all audio components
		auto sceneTreeNodes = engine_.getEnoEditor().getSceneTree().getAllNodes();
		for (const auto& node : sceneTreeNodes)
		{
			bool in = node->entity_.has<ECSComponents::AudioComponent>(entityManager);

			// Ensure that node has audio component and is enabled
			if (in && node->enabled_)
			{
				auto& audioComp = node->entity_.get<ECSComponents::AudioComponent>(entityManager);

				// Auto-start audio if autoplay is enabled
				// This only happens once when scene starts, not every frame
				if (audioComp.isAutoplay_)
				{
					engine_.getAudioManager().play(audioComp);
				}
			}
		}
	}

	// Update last scene state for next frame
	lastSceneState = currentSceneState;

	// =========================

	// Update all animated entities with "AnimatedEntity" in their name
	// Use cached entities for better performance
	for (ECS::Entity& entity : cachedDrawableEntities_) {
		try {
			// Check if this is an animated entity
			if (entity.has<ECSComponents::NameComponent>(entityManager)) {
				auto& nameComp = entity.get<ECSComponents::NameComponent>(entityManager);
				std::string name(nameComp.name);

				if (name.find("AnimatedEntity") != std::string::npos) {
					// Update animation frame
					if (entity.has<ECSComponents::DrawInfoComponent>(entityManager)) {
						auto& renderComp = entity.get<ECSComponents::DrawInfoComponent>(entityManager);
						renderComp.texID = currentFrame;
					}

					// Update rotation (stored in TransformComponent.rot)
					// rot.x = current angle, rot.y = rotation speed
					if (entity.has<ECSComponents::TransformComponent>(entityManager)) {
						auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);
						transform.rot.x += transform.rot.y * deltaTime;
					}
				}
			}
		}
		catch (...) {
			continue;
		}
	}

	// Load textures for entities (if needed)
	loadEntityTextures();

	updateEntities();
	// THERE ARE 2 DRAWCALLS. if want to use drawshapes pls remove bottom part of drawcalls. Regards, YunQi
	//drawShapes();
	drawPlayerAndIndicator();
	graphics_.flush();

	// Unbind game framebuffer
	if (gameFramebuffer_) {
		gameFramebuffer_->unbind();
	}

	// ==== CAMERA PANNING INPUT (ONLY IN PLAY MODE) ====
	if (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing) {
		auto& inputManager = engine_.getInputManager();
		// Trigger camera pan upward when UP arrow is pressed
		if (inputManager.isKeyDown(GLFW_KEY_UP) && !isPanning_) {
			// Pan up by one screen height (360 pixels = EDITOR_HEIGHT)
			float targetY = editorCameraY_ - static_cast<float>(EDITOR_HEIGHT);
			startCameraPan(targetY, 1.0f); // 1 second duration
		}
		// Optional: Add other directions
		if (inputManager.isKeyDown(GLFW_KEY_DOWN) && !isPanning_) {
			float targetY = editorCameraY_ + static_cast<float>(EDITOR_HEIGHT);
			startCameraPan(targetY, 1.0f);
		}
	}

	// ==== RENDER EDITOR VIEW (ZOOMED IN WITH CAMERA) ====
	if (editorFramebuffer_) {
		// Update camera panning for room transitions
		if (isPanning_) {
			panTimer_ += deltaTime;
			float progress = Math::min(panTimer_ / panDuration_, 1.0f);

			// Smooth easing function (ease-in-out)
			float easedProgress = progress < 0.5f
				? 2.0f * progress * progress
				: 1.0f - Math::pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;

			// Interpolate camera position
			editorCameraY_ = panStartY_ + (panTargetY_ - panStartY_) * easedProgress;

			// End panning when complete
			if (progress >= 1.0f) {
				isPanning_ = false;
				editorCameraY_ = panTargetY_;
			}
		}
		// Manual camera control (only when not panning and in edit mode)
		else if (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Stopped) {
			// NOTE: WASD controls BOTH player movement AND editor camera
			// To move ONLY the camera, set the editor state to stopped (edit) mode
			auto& inputManager = engine_.getInputManager();
			float cameraDelta = editorCameraSpeed_ * deltaTime;
			if (inputManager.isKeyPressed(GLFW_KEY_W)) editorCameraY_ -= cameraDelta;
			if (inputManager.isKeyPressed(GLFW_KEY_S)) editorCameraY_ += cameraDelta;
			if (inputManager.isKeyPressed(GLFW_KEY_A)) editorCameraX_ -= cameraDelta;
			if (inputManager.isKeyPressed(GLFW_KEY_D)) editorCameraX_ += cameraDelta;
		}

		// Calculate viewport size with zoom (zoom is now limited to prevent exceeding world bounds)
		int editorFbSizeX = getEditorWidth();
		int editorFbSizeY = getEditorHeight();

		// Clamp camera to world bounds (accounting for zoom)
		editorCameraX_ = std::clamp(editorCameraX_, 0.0f, worldWidth_ - static_cast<float>(editorFbSizeX));
		editorCameraY_ = std::clamp(editorCameraY_, 0.0f, worldHeight_ - static_cast<float>(editorFbSizeY));

		// Only resize framebuffer when size changes (expensive operation)
		if (editorFbSizeX != lastEditorFbWidth_ || editorFbSizeY != lastEditorFbHeight_) {
			editorFramebuffer_->create(editorFbSizeX, editorFbSizeY);
			lastEditorFbWidth_ = editorFbSizeX;
			lastEditorFbHeight_ = editorFbSizeY;
		}

		// Set projection for editor view (uses zoomed size to enable zoom functionality)
		engine_.getGraphicsManager().setProjectionMatrix(editorFbSizeX, editorFbSizeY);
		editorFramebuffer_->bind();

		// Clear editor framebuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ============================================================
		// DRAW TILEMAP (all layers from lowest index to highest)
		// ============================================================
		if (!mapEditor_->isEditMode()) {
			auto& layers = tileMap.layers();

			int mapW = tileMap.width();
			int mapH = tileMap.height();
			float cellSize = tileMap.cellSize();

			// For every layer (0 = background, higher = foreground)
			for (size_t layerIndex = 0; layerIndex < layers.size(); ++layerIndex) {
				const auto& layer = layers[layerIndex];

				for (int y = 0; y < mapH; ++y) {
					for (int x = 0; x < mapW; ++x) {
						const auto& tile = layer.tiles()[y][x];

						// Skip empty tile
						if (!tile.hasTexture)
							continue;

						// Convert tile coord to world position
						float worldX = x * cellSize;
						float worldY = y * cellSize;

						// Convert to screen space (apply camera offset)
						float screenX = worldX - editorCameraX_;
						float screenY = worldY - editorCameraY_;

						// Visibility check (skips drawing offscreen tiles)
						if (screenX + cellSize < 0 || screenX > EDITOR_WIDTH ||
							screenY + cellSize < 0 || screenY > EDITOR_HEIGHT)
							continue;

						// Fetch the texture from loader (textureName stored inside Tile struct)
						std::shared_ptr<Graphics::EnoTexture> tex = loader.getTexture(tile.textureName);

						if (!tex || !tex->isValid())
							continue;

						// Draw the tile quad using its UV coordinates
						engine_.getGraphicsManager().drawTexturedQuadUV(
							Math::eVec2<float>{screenX, screenY},
							Math::eVec2<float>{cellSize, cellSize},
							tex,
							tile.uv0,
							tile.uv1,
							Graphics::Color::White
						);
					}
				}
			}
		}

		// Draw all entities (with camera offset - convert world space to screen space)
		for (ECS::Entity& entity : cachedDrawableEntities_) {
			try {
				if (entity.has<ECSComponents::DrawInfoComponent>(entityManager)) {
					if (entity.has<ECSComponents::PlayerComponent>(entityManager)) {
						continue; // Skip player, drawn separately
					}

					auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);
					auto& renderInfo = entity.get<ECSComponents::DrawInfoComponent>(entityManager);

					// Apply scale to width and height
					float scaledWidth = renderInfo.width * transform.scale.x;
					float scaledHeight = renderInfo.height * transform.scale.y;

					// Get rotation from transform component
					float rotation = transform.rot.x;

					// Convert world position to screen position (relative to camera)
					float screenX = transform.pos.x - editorCameraX_;
					float screenY = transform.pos.y - editorCameraY_;

					// Calculate half dimensions for rendering
					float halfW = scaledWidth * 0.5f;
					float halfH = scaledHeight * 0.5f;

					// Culling disabled - draw all entities
					{
						// Check if entity has a texture
						if (renderInfo.texture && renderInfo.texture->isValid()) {
							// Draw textured quad (top-left position)
							float topLeftX = screenX - halfW;
							float topLeftY = screenY - halfH;

							// Check if entity has animation controller
							if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager)) {
								auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
								Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);

								if (ctrl && ctrl->getCurrentAnimation()) {
									// Update animation and get current frame
									const Animation::AnimationFrame& frame = ctrl->update(static_cast<float>(deltaTime));

									// Draw with animation UV coordinates
									engine_.getGraphicsManager().drawTexturedQuadUV(
										Math::eVec2<float>{topLeftX, topLeftY},
										Math::eVec2<float>{scaledWidth, scaledHeight},
										renderInfo.texture,
										frame.uvStart,
										frame.uvEnd,
										renderInfo.color,
										rotation
									);
								}
								else {
									// No active animation, draw full texture
									engine_.getGraphicsManager().drawTexturedQuad(
										Math::eVec2<float>{topLeftX, topLeftY},
										Math::eVec2<float>{scaledWidth, scaledHeight},
										renderInfo.texture,
										renderInfo.color
									);
								}
							}
							else {
								// No animation controller, draw full texture
								engine_.getGraphicsManager().drawTexturedQuad(
									Math::eVec2<float>{topLeftX, topLeftY},
									Math::eVec2<float>{scaledWidth, scaledHeight},
									renderInfo.texture,
									renderInfo.color
								);
							}
						}
						// Check shape type and draw accordingly
						else if (strcmp(renderInfo.shape, "Circle") == 0) {
							// Draw circle at screen position (center)
							engine_.getGraphicsManager().drawCircle(
								Math::eVec2<float>{screenX, screenY},
								scaledWidth * 0.5f,  // Use scaled width/2 as radius
								renderInfo.color,
								true,  // Filled
								32     // Segments
							);
						}
						else {
							// Draw rectangle (top-left position)
							float topLeftX = screenX - halfW;
							float topLeftY = screenY - halfH;

							engine_.getGraphicsManager().drawRect(
								Math::eVec2<float>{topLeftX, topLeftY},
								Math::eVec2<float>{scaledWidth, scaledHeight},
								renderInfo.color,
								true,
								rotation
							);
						}
						// Draw text for button component
						//if (entity.has<ECSComponents::ButtonComponent>(entityManager))
						//{
						//	auto& b = entity.get<ECSComponents::ButtonComponent>(entityManager);
						//	graphics_.drawText(timesFont_, b.getName(), transform.pos, 0.5f, Color::Red);
						//}
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
					// Get player system from engine (no longer a global)
					if (playerSystem_) {
						auto& transform = playerSystem_->GetTransform(it->second);
						auto& frame = playerAnimController_.getCurrentFrame();

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
							float scaledWidth = (playerConfig_.spriteWidth * transform.scale.x);
							float scaledHeight = (playerConfig_.spriteHeight * transform.scale.y);

							// Apply camera offset to player position
							float screenX = transform.pos.x - editorCameraX_;
							float screenY = transform.pos.y - editorCameraY_;
							Math::eVec2<float> topLeft = {
								screenX - (scaledWidth * 0.5f),
								screenY - (scaledHeight * 0.5f)
							};

							graphics_.drawTexturedQuadUV(
								topLeft,
								Math::eVec2<float>{scaledWidth, scaledHeight},
								currentTexture,
								uvStart,
								uvEnd,
								Graphics::Color::White,
								transform.rot.x
							);
						}
					}
				}
			}
			catch (...) {}
		}

		// Draw colliders in editor view (with camera offset)
		if (showColliders_) {
			// Get collision system from engine (no longer a global)
			auto* collisionSystem = engine_.getCollisionSystem();
			if (collisionSystem) {
				// Visualize colliders with camera offset applied
				Math::eVec2<float> cameraOffset = { editorCameraX_, editorCameraY_ };
				collisionSystem->VisualizeColliders(graphics_, engine_.getEntityManager(), cameraOffset);
			}
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

	// Update engine playing state for NEXT frame based on current editor state
	// This must happen BEFORE we transition NextKeyFrame->Paused
	bool shouldPlayNextFrame = (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing || engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame);
	engine_.setIsPlaying(shouldPlayNextFrame);

	// Handle NextKeyFrame state transition at the END of the frame
	// This ensures physics/collision will have already updated with isPlaying=true in the next frame
	if (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame) {
		engine_.getEnoEditor().getCurrentScene().pause();
	}
}

void GameApplication::loadEntityTextures() {
	// Load textures for all entities that have a texture path but no texture loaded
	auto& entityManager = engine_.getEntityManager();
	auto& loader = engine_.getResourceLoader();

	for (auto& [entityID, entityLoc] : entityManager.getMap()) {
		// Check if entity has RenderComponent
		bool hasRender = false;
		for (const auto& type : entityLoc.archetype->types_) {
			if (type == std::type_index(typeid(ECSComponents::RenderComponent))) {
				hasRender = true;
				break;
			}
		}

		if (!hasRender) continue;

		// Get RenderComponent
		size_t renderIndex = entityLoc.archetype->findComponentIndex<ECSComponents::RenderComponent>();
		auto& renderComp = entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::RenderComponent>(renderIndex, entityLoc.entityIndexInChunk);

		// If texture path is set but texture is not loaded, load it
		if (strlen(renderComp.texturePath) > 0 && (!renderComp.texture || !renderComp.texture->isValid())) {
			// Extract filename from path (e.g., "assets/art/mainmenu.png" -> "mainmenu")
			std::string path(renderComp.texturePath);
			size_t lastSlash = path.find_last_of("/\\");
			std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
			size_t lastDot = filename.find_last_of('.');
			std::string textureName = (lastDot != std::string::npos) ? filename.substr(0, lastDot) : filename;

			// Get texture from resource loader
			renderComp.texture = loader.getTexture(textureName);

			if (renderComp.texture && renderComp.texture->isValid()) {
				std::cout << "[GameLogic] Loaded texture '" << textureName << "' for entity " << entityID << std::endl;
			} else {
				std::cerr << "[GameLogic] Failed to load texture '" << textureName << "' from path: " << renderComp.texturePath << std::endl;
			}
		}
	}
}

void GameApplication::updateEntities() {
	SystemTimer EntityTimer("Entities", systemTimes_);

	//ImGuiIO& io = ImGui::GetIO();
	//float deltaTime = io.DeltaTime;
	auto& inputManager = engine_.getInputManager();
	auto& graphics = engine_.getGraphicsManager();

	// Detect trigger entries (new triggers in current set that weren't in previous set)
	for (uint32_t triggerID : currentTriggersPlayerIsIn_) {
		if (previousTriggersPlayerWasIn_.find(triggerID) == previousTriggersPlayerWasIn_.end()) {
			std::cout << "[Trigger] Player (ID:" << playerEntityID_ << ") ENTERED trigger (ID:" << triggerID << ")" << std::endl;
		}
	}

	// Detect trigger exits (triggers in previous set that aren't in current set)
	for (uint32_t triggerID : previousTriggersPlayerWasIn_) {
		if (currentTriggersPlayerIsIn_.find(triggerID) == currentTriggersPlayerIsIn_.end()) {
			std::cout << "[Trigger] Player (ID:" << playerEntityID_ << ") EXITED trigger (ID:" << triggerID << ")" << std::endl;
		}
	}

	// Save current frame's triggers as previous for next frame
	previousTriggersPlayerWasIn_ = currentTriggersPlayerIsIn_;

	// Clear current set - it will be rebuilt by collision events this frame
	currentTriggersPlayerIsIn_.clear();

	// Draw background
	graphics.drawTexturedQuadUV(
		Math::eVec2<float>{0.0f, 0.0f},           // Position at origin
		{worldWidth_, worldHeight_},			  // World Size
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

		// Restore animations from serialized data after scene load
		for (auto& entity : cachedDrawableEntities_) {
			if (entity.has<ECSComponents::AnimationControllerComponent>(engine_.getEntityManager()) &&
				entity.has<ECSComponents::DrawInfoComponent>(engine_.getEntityManager())) {
				auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(engine_.getEntityManager());
				auto& drawComp = entity.get<ECSComponents::DrawInfoComponent>(engine_.getEntityManager());

				// Check if animation data exists but controller is not initialized
				if (animComp.frameWidth > 0 && animComp.frameHeight > 0 && animComp.controllerID == 0) {
					// Create new controller
					animComp.controllerID = engine_.getAnimationRegistry().createController();
					Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);

					if (ctrl && drawComp.texture) {
						// Recreate the animation from saved configuration
						Animation::EnoAnimation anim = Animation::createSpriteSheetAnimation(
							animComp.frameWidth,
							animComp.frameHeight,
							drawComp.texture->getWidth(),
							drawComp.texture->getHeight(),
							0,  // start frame
							animComp.rows * animComp.cols,  // total frames
							animComp.frameDuration
						);

						anim.setLooping(animComp.looping);
						ctrl->addAnimation(animComp.currentAnimationName, anim);
						ctrl->setAnimation(animComp.currentAnimationName);

						std::cout << "[GameLogic] Restored animation '" << animComp.currentAnimationName
							<< "' for entity (frames: " << animComp.frameWidth << "x" << animComp.frameHeight
							<< ", grid: " << animComp.rows << "x" << animComp.cols << ")" << std::endl;
					}
				}
			}
		}
	}


	// Update all entities with AnimationControllerComponent. Can use this to debug later to see if updating or not. 
	for (auto& entity : cachedDrawableEntities_) {
		if (entity.has<ECSComponents::AnimationControllerComponent>(engine_.getEntityManager())) {
			auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(engine_.getEntityManager());
			Animation::AnimationController* ctrl =	engine_.getAnimationRegistry().get(animComp.controllerID);

			if (ctrl)
				ctrl->update(static_cast<float>(engine_.getDeltaTime()));
		}
		

	}

	// Check if editor state is in play and player is moving and switch animation accordingly
	if (playerSystem_ && playerSpawned_ && engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing) {
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
		// NOTE: Player system update is now done in eno_engine.cpp::updateGameSystems()
		// No need to call playerSystem_->Update() here anymore
	}

	// NOTE: Collision system update is now done in eno_engine.cpp::updateGameSystems()
	// No need to call collisionSystem->Update() here anymore

	// NOTE: Physics system update is now done in eno_engine.cpp::updateGameSystems()
	// No need to call physicsSystem.Update() here anymore

	static bool keyFrameMode = false;
	static bool playAfterStop = true;
	static bool hasBegun = false;
	auto& coreHost = engine_.getCoreHost();

	// Editor state = play game
	switch (engine_.getEnoEditor().getCurrentScene().getState())
	{
	case Editor::State::Playing:
		{
			// Check that the last frame editor state was 'Stopped'
			if (playAfterStop)
			{
				// Toggle flag to make it a one time event
				playAfterStop = false;
			}

			if (!hasBegun)
			{
				for (const auto& node : engine_.getEnoEditor().getSceneTree().getAllNodes())
				{
					if (node->script_)
					{
						for (const auto& org : node->script_->origin())
						{
							std::wcout << "origin of call: " << org << std::endl;
							coreHost.begin(org, node->sceneObjectID_);
						}
					}
				}

				//for (auto* node : sceneTree_->getAllNodes())
				//{
				//	if (!node->script_)continue;
				//	std::wstring className = node->script_->extrClassName(node->script_->name_.data());
				//	std::wcout << L"[DEBUG] Entity ID " << node->entity_.id()
				//		<< L" running Begin for class: " << node->script_->extrClassName(node->script_->name_.data())
				//		<< std::endl;
				//	coreHost.runScriptMethod(className, L"Begin");



				//}
				hasBegun = true;
			}

			//for (auto* node : sceneTree_->getAllNodes())
			//{
			//	if (!node->script_) continue;

			//	std::wstring className = node->script_->extrClassName(node->script_->name_.data());
			//	coreHost.runScriptMethod(className, L"Update");
			//}
			
			
			//for (const auto& node : sceneTree_->getAllNodes())
			for (const auto& node : engine_.getEnoEditor().getSceneTree().getAllNodes())
			{
				if (node->script_)
				{
					for (const auto& org : node->script_->origin())
					{
						coreHost.update(org, node->sceneObjectID_);
					}
				}
			}
			updateTestObject(engine_.getEntityManager(), engine_);

			// loop to check for UI button entities 
			std::vector<ECS::Entity> entityList = engine_.getEntityManager().findEntity<ECSComponents::ButtonComponent>();

			for (auto& ent : entityList)
			{
				auto& c = ent.get<ECSComponents::CollisionComponent>(engine_.getEntityManager());
				auto& t = ent.get<ECSComponents::TransformComponent>(engine_.getEntityManager());
				auto& d = ent.get<ECSComponents::DrawInfoComponent>(engine_.getEntityManager());
				auto& b = ent.get<ECSComponents::ButtonComponent>(engine_.getEntityManager());

				if (inputManager.isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
				{
					// Get mouse to screen pos
					std::pair<double, double> gMousePos = inputManager.getMousePosition();
					Math::eVec2<float> mousePos = Graphics::screenToWorld
					({ static_cast<float>(gMousePos.first),static_cast<float>(gMousePos.second) },
						getEditorViewMin(),
						getEditorViewMax(),
						static_cast<float>(getEditorWidth()),
						static_cast<float>(getEditorHeight()),
						getEditorCameraX(),
						getEditorCameraY());

					if (b.onClick(mousePos, t, d, c))
					{

						std::cout << "CLICK!" << std::endl;
						b.executeSignal(ECSComponents::signalType::onClick, engine_);
					}
					
				}
			}



			break;

		}
		case Editor::State::Paused:
		{
			// This toggles keyframe mode
			keyFrameMode = true;
			break;
		}
		case Editor::State::NextFrame:
		{
			// Check if key frame mode is true
			if (keyFrameMode)
			{
				// Update one time
				updateTestObject(engine_.getEntityManager(), engine_);

				// Reset variables - transition back to Paused for next frame
				// NOTE: Don't set state back to Paused here! It happens too early in the frame.
				// The state will be reset to Paused at the end of this function after rendering.
			}
			engine_.getEnoEditor().getCurrentScene().pause();
			break;
		}
		case Editor::State::Stopped:
		{
			// This condition ensures resetting of pos comes after play
			if (!playAfterStop)
			{

				// Mark entities as dirty to refresh the rendering cache
				entitiesDirty_ = true;
				// Toggle flag
				playAfterStop = true;
				hasBegun = false;
			}
			break;
		}
	}


	

}


void GameApplication::drawPlayerAndIndicator() {
	auto& graphics = engine_.getGraphicsManager();

	// Draw player sprite
	if (playerSpawned_) {
		try {
			auto& entityMap = engine_.getEntityManager().getMap();
			auto it = entityMap.find(playerEntityID_);
			if (it != entityMap.end())
			{
				// Get player system from engine (no longer a global)
				if (playerSystem_) {
					auto& transform = playerSystem_->GetTransform(it->second);
					auto& frame = playerAnimController_.getCurrentFrame();

					std::shared_ptr<Graphics::EnoTexture> currentTexture;
					if (playerAnimController_.getCurrentAnimationName() == "walk") {
						currentTexture = playerWalkTexture_;
					}
					else {
						currentTexture = playerTexture_;
					}

					// Calculate UV coordinates for current frame
					Math::eVec2<float> uvStart = frame.uvStart;
					Math::eVec2<float> uvEnd = frame.uvEnd;


					if (playerFacingDirection_ == -1) {
						// Swap horizontal UV coordinates to flip the sprite
						std::swap(uvStart.x, uvEnd.x);
					}
					// Draw with the correct texture
					if (currentTexture && currentTexture->isValid()) {

						// calculate scaled dimensions (match the collider size)
						float scaledWidth = (playerConfig_.spriteWidth * transform.scale.x); //48.0 for playerSpriteW
						float scaledHeight = (playerConfig_.spriteHeight * transform.scale.y); //48.0 for playerspriteH

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
							transform.rot.x
						);
					}
					else {
						// Fallback to colored rect if texture invalid
						graphics.drawRect(
							Math::eVec2<float>{transform.pos.x, transform.pos.y},
							Math::eVec2<float>{10 + transform.scale.x, 10 + transform.scale.y},
							Graphics::Color::Blue,
							true,
							transform.rot.x
						);
					}
				}

			}
		}
		catch (...) {}
	}
}

void GameApplication::updateImGui() {
	SystemTimer ImGuiTimer("ImGui", systemTimes_);
	imGuiDisplay(engine_, engine_.getEnoEditor().getSceneTree(), engine_.getEnoEditor().getUndoManager());
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
			if (entity.has<ECSComponents::DrawInfoComponent>(entityManager)) {
				// Skip rendering for player entities (they're drawn separately with sprite/animation)
				if (entity.has<ECSComponents::PlayerComponent>(entityManager)) {
					continue;
				}

				// Get position from TransformComponent (spatial data)
				// Get rendering info from DrawInfoComponent (size, color, etc.)
				auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);
				auto& renderInfo = entity.get<drawInfo>(entityManager);


				// Apply scale to width and height
				float scaledWidth = renderInfo.width * transform.scale.x;
				float scaledHeight = renderInfo.height * transform.scale.y;

				// Get rotation from transform component
				float rotation = transform.rot.x;

				// Check if entity has a texture (sprite)
				if (renderInfo.texture && renderInfo.texture->isValid())
				{

					Math::eVec2<float> topLeft = {
						transform.pos.x - (scaledWidth * 0.5f),
						transform.pos.y - (scaledHeight * 0.5f)
					};

					// Default UVs (full texture)
					Math::eVec2<float> uvStart{ 0.0f, 0.0f };
					Math::eVec2<float> uvEnd{ 1.0f, 1.0f };


					if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager))
					{
						auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
						Animation::AnimationController* ctrl =
							engine_.getAnimationRegistry().get(animComp.controllerID);

						if (ctrl)
						{
							const auto& frame = ctrl->getCurrentFrame();
							uvStart = frame.uvStart;
							uvEnd   = frame.uvEnd;
						}
					}
					//std::cout << "Middle Values: "    << topLeft.x << " || " << topLeft.y << "\n";

					// Draw textured quad with UV coordinates, rotation and scale
					graphics.drawTexturedQuadUV(
						topLeft,
						Math::eVec2<float>{scaledWidth, scaledHeight},
						renderInfo.texture,
						uvStart,
						uvEnd,
						renderInfo.color,
						rotation
					);
					//IT IS SOMEHOW IGNORING THE FACT THAT I HAVE A DRAW UV OF 0.25??
					//graphics.drawTexturedQuadUV(
					//	topLeft,
					//	Math::eVec2<float>{scaledWidth, scaledHeight},
					//	renderInfo.texture,
					//	{ 0,0 },
					//	{ 0, 0.25 },
					//	renderInfo.color,
					//	rotation
					//);
					//std::cout << "Entity being drawn: " << entity.get<ECSComponents::NameComponent>(entityManager).name << "\n";
					//std::cout << "Entities UV start X and Y: " << uvStart.x << " " << uvStart.y << "|| UV end X and Y: " << uvEnd.x << " " << uvEnd.y << "\n";

					//std::cout << "Texture Dimensions: "
					//	<< renderInfo.texture->getWidth() << " x "
					//	<< renderInfo.texture->getHeight() << std::endl;
					//std::cout << "Frame Dimensions: "
					//	<< renderInfo.width << " x "
					//	<< renderInfo.height << std::endl;

				}
				// Check shape type and draw accordingly
				else if (strcmp(renderInfo.shape, "Circle") == 0) {
					// Draw circle at center position with scale applied to radius
					float scaledRadius = (scaledWidth * 0.5f);
					graphics.drawCircle(
						transform.pos,
						scaledRadius,
						renderInfo.color,
						true,
						32
					);
				}
				else {
					// Draw rectangle with rotation and scale applied
					float topLeftX = transform.pos.x - (scaledWidth * 0.5f);
					float topLeftY = transform.pos.y - (scaledHeight * 0.5f);

					graphics.drawRect(
						Math::eVec2<float>{topLeftX, topLeftY},
						Math::eVec2<float>{scaledWidth, scaledHeight},
						renderInfo.color,
						true,
						rotation
					);
				}
				//// Draw text for button component
				//if (entity.has<ECSComponents::ButtonComponent>(entityManager))
				//{
				//	auto& b = entity.get<ECSComponents::ButtonComponent>(entityManager);
				//	graphics.drawText(timesFont_, b.getName(), transform.pos, 0.5f, Color::Red);
				//}

			}
		}
		catch (...) {
			// Entity was destroyed, rebuild cache next frame
			entitiesDirty_ = true;
			continue;
		}
	}
}

void GameApplication::applyTextureToPlayer(const Graphics::TextureData& texData) {
	if (!texData.texture) return;

	playerTexture_ = texData.texture;
	//std::cout << "Applied texture: " << texData.texture->getWidth()
	//	<< "x" << texData.texture->getHeight()
	//	<< " | Frame: " << texData.frameWidth << "x" << texData.frameHeight << '\n';

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

#if 0
	auto playerWalkAnim = Animation::createSpriteSheetAnimation(
		texData.frameWidth,
		texData.frameHeight,
		texData.texture->getWidth(),
		texData.texture->getHeight(),
		0,
		texData.rows * texData.cols,
		0.8f
	);
	playerWalkAnim.setLooping(true);
#endif

	playerAnimController_.addAnimation("idle", playerIdleAnim);
	//playerAnimController_.addAnimation("walk", playerWalkAnim);

	// Keep the same current state
	if (playerAnimController_.getCurrentAnimationName().empty())
		playerAnimController_.setAnimation("idle");
}

// Camera panning for room transitions
void GameApplication::startCameraPan(float targetY, float duration)
{
	isPanning_ = true;
	panStartY_ = editorCameraY_;
	panTargetY_ = targetY;
	panDuration_ = duration;
	panTimer_ = 0.0f;
}

// ============================================================================
// RUNE SYSTEM IMPLEMENTATION
// ============================================================================

void GameApplication::initializeRuneSystem()
{
	auto& entityManager = engine_.getEntityManager();

	// Create a dedicated entity to hold the rune system state
	ECS::Entity runeSystemEntity = entityManager.create();
	runeSystemEntity.add<ECSComponents::RuneSystemStateComponent>(entityManager);
	runeSystemEntity.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("RuneSystem"));

	runeSystemEntityID_ = runeSystemEntity.id();

	runeSystemInitialized_ = true;
	std::cout << "[RuneSystem] Initialized rune system entity" << std::endl;
}

void GameApplication::updateRuneSystem()
{
	// Only update when in play mode
	if (engine_.getEnoEditor().getCurrentScene().getState() != Editor::State::Playing) {
		return;
	}

	// Initialize if not done yet
	if (!runeSystemInitialized_) {
		initializeRuneSystem();
	}

	auto& entityManager = engine_.getEntityManager();
	auto& inputManager = engine_.getInputManager();

	// Get rune system entity
	ECS::Entity runeSystemEntity(runeSystemEntityID_);

	// Check if rune system entity still exists
	if (!runeSystemEntity.has<ECSComponents::RuneSystemStateComponent>(entityManager)) {
		return;
	}

	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Get mouse position and convert to world space
	auto mouseScreenPos = inputManager.getMousePosition();
	Math::eVec2<float> mouseWorldPos = Graphics::screenToWorld(
		{ static_cast<float>(mouseScreenPos.first), static_cast<float>(mouseScreenPos.second) },
		getEditorViewMin(),
		getEditorViewMax(),
		static_cast<float>(getEditorWidth()),
		static_cast<float>(getEditorHeight()),
		getEditorCameraX(),
		getEditorCameraY()
	);

	// Check if left mouse button is pressed this frame
	bool leftClickPressed = inputManager.isMouseDown(GLFW_MOUSE_BUTTON_LEFT);
	bool leftClickHeld = inputManager.isMousePressed(GLFW_MOUSE_BUTTON_LEFT);
	bool leftClickReleased = inputManager.isMouseUp(GLFW_MOUSE_BUTTON_LEFT);

	// SPAWN RUNES when left click is first pressed
	if (leftClickPressed && !runeState.runesActive) {
		spawnRunes(mouseWorldPos);
		runeState.runesActive = true;
		runeState.isLeftClickHeld = true;
		runeState.fireballSpawned = false;  // Reset flag for new rune spawn
	}

	// UPDATE RUNES while left click is held
	if (leftClickHeld && runeState.runesActive) {
		updateRuneHover(mouseWorldPos);

		// Update line drawing
		if (runeState.isDrawingLine && !runeState.fireballSpawned) {
			runeState.lineEnd = mouseWorldPos;

			// Check if line connects to right rune
			ECS::Entity rightRuneEntity(runeState.rightRuneEntityID);
			if (rightRuneEntity.has<ECSComponents::TransformComponent>(entityManager) &&
			    rightRuneEntity.has<ECSComponents::RuneComponent>(entityManager)) {

				auto& rightTransform = rightRuneEntity.get<ECSComponents::TransformComponent>(entityManager);
				auto& rightRune = rightRuneEntity.get<ECSComponents::RuneComponent>(entityManager);

				float distToRightRune = Math::eVec2Distance(mouseWorldPos, rightTransform.pos);

				if (distToRightRune <= rightRune.radius) {
					// Connection successful! Spawn fireball
					Math::eVec2<float> fireballPos = {
						(runeState.lineStart.x + rightTransform.pos.x) / 2.0f,
						(runeState.lineStart.y + rightTransform.pos.y) / 2.0f
					};
					spawnFireball(fireballPos);

					// Mark runes as connected
					ECS::Entity leftRuneEntity(runeState.leftRuneEntityID);
					if (leftRuneEntity.has<ECSComponents::RuneComponent>(entityManager)) {
						leftRuneEntity.get<ECSComponents::RuneComponent>(entityManager).isConnected = true;
					}
					rightRune.isConnected = true;

					// Set flag to prevent multiple fireballs
					runeState.fireballSpawned = true;

					// Destroy runes immediately after successful connection
					destroyRunes();
					runeState.runesActive = false;
					runeState.isDrawingLine = false;

					std::cout << "[RuneSystem] Runes connected! Spawning fireball" << std::endl;
				}
			}
		}
	}

	// DESTROY RUNES when left click is released
	if (leftClickReleased && runeState.runesActive) {
		destroyRunes();
		runeState.runesActive = false;
		runeState.isLeftClickHeld = false;
		runeState.isDrawingLine = false;
		runeState.fireballSpawned = false;  // Reset for next cycle
	}

	// Draw connection line if active
	if (runeState.isDrawingLine) {
		drawRuneConnectionLine();
	}

	// Update fireballs - check lifetime and destroy expired ones
	float deltaTime = static_cast<float>(engine_.getDeltaTime());
	std::vector<uint32_t> fireballsToDestroy;

	for (const auto& [id, loc] : entityManager.getMap()) {
		ECS::Entity entity(id);
		if (entity.has<ECSComponents::FireballComponent>(entityManager)) {
			auto& fireballComp = entity.get<ECSComponents::FireballComponent>(entityManager);
			fireballComp.elapsedTime += deltaTime;

			// Check if fireball has exceeded its lifetime
			if (fireballComp.elapsedTime >= fireballComp.lifetime) {
				fireballsToDestroy.push_back(id);
			}
		}
	}

	// Destroy expired fireballs
	for (uint32_t id : fireballsToDestroy) {
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
		std::cout << "[RuneSystem] Fireball expired and destroyed" << std::endl;
	}
}

void GameApplication::spawnRunes(const Math::eVec2<float>& mouseWorldPos)
{
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Load textures if not already loaded
	auto& resourceLoader = engine_.getResourceLoader();
	auto leftRuneTexture = resourceLoader.getTexture("Left_Rune");
	auto rightRuneTexture = resourceLoader.getTexture("Right_Rune");

	// Spawn LEFT rune
	ECS::Entity leftRune = entityManager.create();
	leftRune.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("LeftRune"));
	leftRune.add<ECSComponents::TransformComponent>(entityManager);
	leftRune.add<ECSComponents::DrawInfoComponent>(entityManager);
	leftRune.add<ECSComponents::RuneComponent>(entityManager, ECSComponents::RuneComponent(ECSComponents::RuneType::LEFT, 30.0f));

	auto& leftTransform = leftRune.get<ECSComponents::TransformComponent>(entityManager);
	leftTransform.pos = { mouseWorldPos.x - runeState.runeSpawnOffset, mouseWorldPos.y };
	leftTransform.scale = { 1.0f, 1.0f };

	auto& leftDraw = leftRune.get<ECSComponents::DrawInfoComponent>(entityManager);
	leftDraw.width = 60.0f;
	leftDraw.height = 60.0f;
	leftDraw.color = Graphics::Color::White;
	strcpy_s(leftDraw.shape, "Rectangle");
	leftDraw.texture = leftRuneTexture;

	// Spawn RIGHT rune
	ECS::Entity rightRune = entityManager.create();
	rightRune.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("RightRune"));
	rightRune.add<ECSComponents::TransformComponent>(entityManager);
	rightRune.add<ECSComponents::DrawInfoComponent>(entityManager);
	rightRune.add<ECSComponents::RuneComponent>(entityManager, ECSComponents::RuneComponent(ECSComponents::RuneType::RIGHT, 30.0f));

	auto& rightTransform = rightRune.get<ECSComponents::TransformComponent>(entityManager);
	rightTransform.pos = { mouseWorldPos.x + runeState.runeSpawnOffset, mouseWorldPos.y };
	rightTransform.scale = { 1.0f, 1.0f };

	auto& rightDraw = rightRune.get<ECSComponents::DrawInfoComponent>(entityManager);
	rightDraw.width = 60.0f;
	rightDraw.height = 60.0f;
	rightDraw.color = Graphics::Color::White;
	strcpy_s(rightDraw.shape, "Rectangle");
	rightDraw.texture = rightRuneTexture;

	// Store entity IDs
	runeState.leftRuneEntityID = leftRune.id();
	runeState.rightRuneEntityID = rightRune.id();

	// Mark entities as dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[RuneSystem] Spawned runes at mouse position" << std::endl;
}

void GameApplication::destroyRunes()
{
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Destroy left rune if it exists
	if (runeState.leftRuneEntityID != UINT32_MAX) {
		ECS::Entity leftRuneEntity(runeState.leftRuneEntityID);
		if (leftRuneEntity.has<ECSComponents::RuneComponent>(entityManager)) {
			entityManager.destroyEntity(runeState.leftRuneEntityID);
		}
		runeState.leftRuneEntityID = UINT32_MAX;
	}

	// Destroy right rune if it exists
	if (runeState.rightRuneEntityID != UINT32_MAX) {
		ECS::Entity rightRuneEntity(runeState.rightRuneEntityID);
		if (rightRuneEntity.has<ECSComponents::RuneComponent>(entityManager)) {
			entityManager.destroyEntity(runeState.rightRuneEntityID);
		}
		runeState.rightRuneEntityID = UINT32_MAX;
	}

	// Mark entities as dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[RuneSystem] Destroyed runes" << std::endl;
}

void GameApplication::updateRuneHover(const Math::eVec2<float>& mouseWorldPos)
{
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Check if mouse hovers over left rune
	if (runeState.leftRuneEntityID != UINT32_MAX) {
		ECS::Entity leftRuneEntity(runeState.leftRuneEntityID);
		if (leftRuneEntity.has<ECSComponents::TransformComponent>(entityManager) &&
		    leftRuneEntity.has<ECSComponents::RuneComponent>(entityManager)) {

			auto& leftTransform = leftRuneEntity.get<ECSComponents::TransformComponent>(entityManager);
			auto& leftRune = leftRuneEntity.get<ECSComponents::RuneComponent>(entityManager);

			float distToLeftRune = Math::eVec2Distance(mouseWorldPos, leftTransform.pos);
			bool wasHovered = leftRune.isHovered;
			leftRune.isHovered = (distToLeftRune <= leftRune.radius);

			// Start drawing line when hovering over left rune
			if (leftRune.isHovered && !wasHovered) {
				runeState.isDrawingLine = true;
				runeState.lineStart = leftTransform.pos;
				runeState.lineEnd = mouseWorldPos;
				std::cout << "[RuneSystem] Started drawing line from left rune" << std::endl;
			}
		}
	}

	// Update right rune hover state
	if (runeState.rightRuneEntityID != UINT32_MAX) {
		ECS::Entity rightRuneEntity(runeState.rightRuneEntityID);
		if (rightRuneEntity.has<ECSComponents::TransformComponent>(entityManager) &&
		    rightRuneEntity.has<ECSComponents::RuneComponent>(entityManager)) {

			auto& rightTransform = rightRuneEntity.get<ECSComponents::TransformComponent>(entityManager);
			auto& rightRune = rightRuneEntity.get<ECSComponents::RuneComponent>(entityManager);

			float distToRightRune = Math::eVec2Distance(mouseWorldPos, rightTransform.pos);
			rightRune.isHovered = (distToRightRune <= rightRune.radius);
		}
	}
}

void GameApplication::drawRuneConnectionLine()
{
	auto& entityManager = engine_.getEntityManager();
	auto& graphics = engine_.getGraphicsManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Convert world positions to screen positions for drawing
	Math::eVec2<float> screenStart = {
		runeState.lineStart.x - editorCameraX_,
		runeState.lineStart.y - editorCameraY_
	};

	Math::eVec2<float> screenEnd = {
		runeState.lineEnd.x - editorCameraX_,
		runeState.lineEnd.y - editorCameraY_
	};

	// Draw line using graphics system
	// Note: You may need to implement a line drawing function in your graphics system
	// For now, drawing as a thin rectangle
	Math::eVec2<float> direction = {
		screenEnd.x - screenStart.x,
		screenEnd.y - screenStart.y
	};
	float length = Math::eVec2Length(direction);

	if (length > 0.1f) {
		float angle = std::atan2(direction.y, direction.x);
		Math::eVec2<float> midpoint = {
			screenStart.x + direction.x / 2.0f,
			screenStart.y + direction.y / 2.0f
		};

		graphics.drawRect(
			midpoint,
			Math::eVec2<float>{length, 3.0f},  // 3 pixels wide
			Graphics::Color(1.0f, 1.0f, 0.0f, 0.8f),  // Yellow with transparency
			true,
			angle
		);
	}
}

void GameApplication::spawnFireball(const Math::eVec2<float>& pos)
{
	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Get player position
	Math::eVec2<float> playerPos = pos;
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	if (!playerEntities.empty()) {
		auto& playerEntity = playerEntities[0];
		if (playerEntity.has<ECSComponents::TransformComponent>(entityManager)) {
			playerPos = playerEntity.get<ECSComponents::TransformComponent>(entityManager).pos;
		}
	}

	// Find nearest enemy
	auto enemies = entityManager.findEntity<ECSComponents::AIComponent, ECSComponents::TransformComponent>();
	Math::eVec2<float> targetPos = playerPos; // Default if no enemy found
	float minDistance = FLT_MAX;
	bool foundEnemy = false;

	for (auto& enemy : enemies) {
		auto& enemyTransform = enemy.get<ECSComponents::TransformComponent>(entityManager);
		float distance = Math::eVec2Distance(playerPos, enemyTransform.pos);
		if (distance < minDistance) {
			minDistance = distance;
			targetPos = enemyTransform.pos;
			foundEnemy = true;
		}
	}

	// Calculate direction to target
	Math::eVec2<float> direction = { targetPos.x - playerPos.x, targetPos.y - playerPos.y };
	float length = Math::eVec2Length(direction);
	if (length > 0.0f) {
		direction.x /= length;  // Normalize
		direction.y /= length;
	}

	// Offset spawn position so fireball doesn't collide with player
	float spawnOffset = 100.0f;  // Spawn 100 pixels away from player center
	Math::eVec2<float> spawnPos = {
		playerPos.x + direction.x * spawnOffset,
		playerPos.y + direction.y * spawnOffset
	};

	// Load fireball spritesheet texture
	auto fireballTexture = resourceLoader.getTexture("Spell-Fireball-Flying-Sheet");

	// Create fireball entity
	ECS::Entity fireball = entityManager.create();
	fireball.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("Fireball"));
	fireball.add<ECSComponents::TransformComponent>(entityManager);
	fireball.add<ECSComponents::DrawInfoComponent>(entityManager);
	fireball.add<ECSComponents::AnimationControllerComponent>(entityManager);
	fireball.add<ECSComponents::FireballComponent>(entityManager, ECSComponents::FireballComponent(5.0f));
	fireball.add<ECSComponents::DamageComponent>(entityManager, ECSComponents::DamageComponent(25.0f, true));
	fireball.add<ECSComponents::PhysicsComponent>(entityManager);

	auto& transform = fireball.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = spawnPos;  // Spawn offset from player
	transform.scale = { 1.0f, 1.0f };

	// Set fireball velocity toward target
	auto& physics = fireball.get<ECSComponents::PhysicsComponent>(entityManager);
	float fireballSpeed = 400.0f;  // Pixels per second
	physics.velocity.x = direction.x * fireballSpeed;
	physics.velocity.y = direction.y * fireballSpeed;

	auto& draw = fireball.get<ECSComponents::DrawInfoComponent>(entityManager);
	draw.width = 90.0f;   // Match sprite frame width (360/4 = 90)
	draw.height = 48.0f;  // Match sprite frame height
	draw.color = Graphics::Color::White;
	strcpy_s(draw.shape, "Rectangle");
	draw.texture = fireballTexture;

	// Setup collision box for fireball (rectangle collider)
	ECSComponents::CollisionComponent fireballCollision(
		60.0f,  // width - smaller than visual for better gameplay feel
		40.0f,  // height
		Math::eVec2<float>{0.0f, 0.0f},  // offset
		false   // not a trigger
	);
	fireball.add<ECSComponents::CollisionComponent>(entityManager, fireballCollision);

	// Setup animation configuration - fireball spritesheet is 360x48 with 4 frames (90x48 each)
	auto& animComp = fireball.get<ECSComponents::AnimationControllerComponent>(entityManager);
	if (fireballTexture && fireballTexture->isValid()) {
		animComp.frameWidth = 90;    // Each frame is 90 pixels wide
		animComp.frameHeight = 48;   // Each frame is 48 pixels tall
		animComp.rows = 1;            // 1 row
		animComp.cols = 4;            // 4 columns (frames)
		animComp.frameDuration = 0.1f;  // Frame duration
		animComp.looping = true;      // Loop the animation
		strcpy_s(animComp.currentAnimationName, "fly");
	}

	// Mark entities as dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[RuneSystem] Spawned fireball at position (" << pos.x << ", " << pos.y << ")" << std::endl;
}

void GameApplication::updateCombatSystem(float dt)
{
	auto& entityManager = engine_.getEntityManager();

	// Helper lambda for AABB collision check
	auto checkAABBCollision = [](
		const ECSComponents::TransformComponent& transA, const ECSComponents::CollisionComponent& collA,
		const ECSComponents::TransformComponent& transB, const ECSComponents::CollisionComponent& collB) -> bool
		{
			// Only check rectangle colliders
			if (collA.type != ECSComponents::ColliderType::RECTANGLE || collB.type != ECSComponents::ColliderType::RECTANGLE)
				return false;

			// Get positions with offsets
			float aX = transA.pos.x + collA.offset.x;
			float aY = transA.pos.y + collA.offset.y;
			float bX = transB.pos.x + collB.offset.x;
			float bY = transB.pos.y + collB.offset.y;

			// Get half dimensions
			float aHalfW = collA.data.rectangle.width * 0.5f;
			float aHalfH = collA.data.rectangle.height * 0.5f;
			float bHalfW = collB.data.rectangle.width * 0.5f;
			float bHalfH = collB.data.rectangle.height * 0.5f;

			// AABB collision test - use <= to include touching edges
			return (std::abs(aX - bX) <= (aHalfW + bHalfW)) &&
				   (std::abs(aY - bY) <= (aHalfH + bHalfH));
	};

	// Update invulnerability timers
	auto entitiesWithHealth = entityManager.findEntity<ECSComponents::HealthComponent>();
	for (auto& entity : entitiesWithHealth) {
		auto& health = entity.get<ECSComponents::HealthComponent>(entityManager);
		if (health.invulnerable && health.invulnerabilityTimer > 0.0f) {
			health.invulnerabilityTimer -= dt;
			if (health.invulnerabilityTimer <= 0.0f) {
				health.invulnerable = false;
			}
		}
	}

	// FIREBALL vs ENEMY collision damage
	auto fireballs = entityManager.findEntity<ECSComponents::FireballComponent, ECSComponents::CollisionComponent, ECSComponents::DamageComponent>();
	auto enemies = entityManager.findEntity<ECSComponents::AIComponent, ECSComponents::CollisionComponent, ECSComponents::HealthComponent>();

	// Track entities to destroy (avoid destroying while iterating)
	std::set<uint32_t> fireballsToDestroy;
	std::set<uint32_t> enemiesToDestroy;

	for (auto& fireball : fireballs) {
		// Skip if already marked for destruction
		if (fireballsToDestroy.count(fireball.id()) > 0) continue;
		if (!fireball.has<ECSComponents::TransformComponent>(entityManager)) continue;

		auto& fireballTransform = fireball.get<ECSComponents::TransformComponent>(entityManager);
		auto& fireballCollision = fireball.get<ECSComponents::CollisionComponent>(entityManager);
		auto& fireballDamage = fireball.get<ECSComponents::DamageComponent>(entityManager);

		for (auto& enemy : enemies) {
			// Skip if already marked for destruction
			if (enemiesToDestroy.count(enemy.id()) > 0) continue;
			if (!enemy.has<ECSComponents::TransformComponent>(entityManager)) continue;

			auto& enemyTransform = enemy.get<ECSComponents::TransformComponent>(entityManager);
			auto& enemyCollision = enemy.get<ECSComponents::CollisionComponent>(entityManager);
			auto& enemyHealth = enemy.get<ECSComponents::HealthComponent>(entityManager);

			// Check AABB collision between fireball and enemy
			if (checkAABBCollision(fireballTransform, fireballCollision, enemyTransform, enemyCollision)) {
				// Deal damage to enemy
				float damage = fireballDamage.getTotalDamage();
				bool died = enemyHealth.takeDamage(damage);

				std::cout << "[Combat] Fireball hit enemy! Dealt " << damage << " damage. "
					<< "Enemy HP: " << enemyHealth.currentHealth << "/" << enemyHealth.maxHealth << std::endl;

				if (died) {
					std::cout << "[Combat] Enemy died!" << std::endl;
					enemiesToDestroy.insert(enemy.id());
				}

				// Mark fireball for destruction
				if (fireballDamage.destroyOnHit) {
					fireballsToDestroy.insert(fireball.id());
					break; // Exit enemy loop, this fireball hit something
				}
			}
		}
	}

	// Destroy marked entities after iteration
	for (uint32_t id : fireballsToDestroy) {
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
	}
	for (uint32_t id : enemiesToDestroy) {
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
	}

	// ENEMY vs PLAYER collision damage
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	if (!playerEntities.empty()) {
		ECS::Entity& playerEntity = playerEntities[0];
		if (playerEntity.has<ECSComponents::HealthComponent>(entityManager) &&
			playerEntity.has<ECSComponents::CollisionComponent>(entityManager) &&
			playerEntity.has<ECSComponents::TransformComponent>(entityManager)) {

			auto& playerHealth = playerEntity.get<ECSComponents::HealthComponent>(entityManager);
			auto& playerCollision = playerEntity.get<ECSComponents::CollisionComponent>(entityManager);
			auto& playerTransform = playerEntity.get<ECSComponents::TransformComponent>(entityManager);

			for (auto& enemy : enemies) {
				// Skip enemies that are marked for destruction
				if (enemiesToDestroy.count(enemy.id()) > 0) continue;
				if (!enemy.has<ECSComponents::TransformComponent>(entityManager) ||
					!enemy.has<ECSComponents::DamageComponent>(entityManager)) continue;

				auto& enemyTransform = enemy.get<ECSComponents::TransformComponent>(entityManager);
				auto& enemyCollision = enemy.get<ECSComponents::CollisionComponent>(entityManager);
				auto& enemyDamage = enemy.get<ECSComponents::DamageComponent>(entityManager);

				// Check AABB collision between enemy and player
				if (checkAABBCollision(
					playerTransform, playerCollision,
					enemyTransform, enemyCollision)) {

					// Deal damage to player
					float damage = enemyDamage.getTotalDamage();
					bool died = playerHealth.takeDamage(damage);

					std::cout << "[Combat] Enemy hit player! Dealt " << damage << " damage. "
						<< "Player HP: " << playerHealth.currentHealth << "/" << playerHealth.maxHealth << std::endl;

					if (died) {
						std::cout << "[Combat] Player died!" << std::endl;
						// TODO: Handle player death (game over, respawn, etc.)
					}

					// Give player temporary invulnerability after being hit (0.5 seconds)
					playerHealth.invulnerable = true;
					playerHealth.invulnerabilityTimer = 0.5f;
				}
			}
		}
	}
}