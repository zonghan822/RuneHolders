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
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "math/math.hpp"

float scaleSize = 1.0f;
float rotationRadians = 0.f;


void GameApplication::initialize() {
	// Setup engine systems first (this now includes game systems)
	engine_.setup();

	// Set RuneWeavers to fullscreen mode (Editor stays windowed)
	engine_.getWindowManager().setFullscreen(true);

	// Initialize player system
	playerSystem_ = std::make_unique<Player::PlayerSystem>();

	// Setup all game content
	setupPrefabs();
	setupGameSystems();
	setupGameAudio();
	setupTreeTesting();
	setupGameEvents();
	setupGameMath();
	setupAnimations();
	setupScripting();
	setupFont();
	setupCamera();

	auto& tileMap = engine_.getTileMap();

	Math::eVec2<float> worldSize = tileMap.getWorldSize();

	worldWidth_ = worldSize.x;
	worldHeight_ = worldSize.y;

	std::cout << "Game initialized successfully!" << std::endl;

	// ========== SCENE DEBUG INFO ==========
	auto& sceneTree = engine_.getEnoEditor().getSceneTree();
	auto& entityMng = engine_.getEntityManager();

	std::cout << "\n========== LOADED SCENE DEBUG ==========\n";
	//std::cout << "Scene Name: " << sceneTree.getSceneName() << "\n";
	std::cout << "Total Scene Nodes: " << sceneTree.getAllNodes().size() << "\n\n";

	int nodeCount = 0;
	for (const auto& node : sceneTree.getAllNodes()) {
		std::cout << "Node " << nodeCount++ << ":\n";
		std::cout << "  - Scene Object ID: " << node->sceneObjectID_ << "\n";
		std::cout << "  - Entity ID: " << node->entity_.id() << "\n";
		std::cout << "  - Enabled: " << (node->enabled_ ? "true" : "false") << "\n";

		if (node->entity_.has<ECSComponents::NameComponent>(entityMng)) {
			auto& name = node->entity_.get<ECSComponents::NameComponent>(entityMng);
			std::cout << "  - Name: " << name.name << "\n";
		}

		if (node->entity_.has<ECSComponents::TransformComponent>(entityMng)) {
			auto& transform = node->entity_.get<ECSComponents::TransformComponent>(entityMng);
			std::cout << "  - Position: (" << transform.pos.x << ", " << transform.pos.y << ")\n";
		}

		if (node->entity_.has<ECSComponents::RenderComponent>(entityMng)) {
			auto& render = node->entity_.get<ECSComponents::RenderComponent>(entityMng);
			std::cout << "  - Has RenderComponent: YES\n";
			std::cout << "  - Size: " << render.width << "x" << render.height << "\n";
			std::cout << "  - Color: (" << render.color.r << ", " << render.color.g
					  << ", " << render.color.b << ", " << render.color.a << ")\n";
			std::cout << "  - Shape: " << render.shape << "\n";
			if (strlen(render.texturePath) > 0) {
				std::cout << "  - Texture Path: " << render.texturePath << "\n";
			}
		} else {
			std::cout << "  - Has RenderComponent: NO\n";
		}

		std::cout << "\n";
	}

	// Check how many entities have DrawInfoComponent
	auto drawableEntities = entityMng.findEntity<ECSComponents::DrawInfoComponent>();
	std::cout << "Total entities with DrawInfoComponent: " << drawableEntities.size() << "\n";
	std::cout << "========================================\n\n";
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
}

void GameApplication::setupPrefabs() {
	auto prefabManager = engine_.getPrefabManager();
	prefabManager.registerPresets();

	ECSComponents::Preset::saveAll(prefabManager);
	ECSComponents::Preset::loadAll(prefabManager);
}

void GameApplication::setupGameSystems() {
	// Check if a player entity exists in the scene
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();

	if (!playerEntities.empty()) {
		// Found a player entity
		playerEntityID_ = playerEntities[0].id();
		playerSpawned_ = true;
		std::cout << "[RuneWeavers] Player entity found (ID: " << playerEntityID_ << ")" << std::endl;
	}
	else {
		std::cout << "[RuneWeavers] No player entity found in scene" << std::endl;
	}
}

void GameApplication::setupFont() {
	pixelifyFont_ = engine_.getResourceLoader().getFont("PixelifySans");
	timesFont_ = engine_.getResourceLoader().getFont("times");
}

void GameApplication::setupCamera() {
	auto& entityManager = engine_.getEntityManager();

	// Check if a camera already exists in the scene
	std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();

	if (cameraEntities.empty()) {
		// No camera found, create one
		std::cout << "[RuneWeavers] No camera found, creating default camera" << std::endl;

		// Create camera component with default viewport
		ECSComponents::CameraComponent camera;
		camera.viewportWidth = 1920.0f;
		camera.viewportHeight = 1080.0f;
		camera.position = Math::eVec2<float>{ camera.viewportWidth * 0.5f, camera.viewportHeight * 0.5f };
		camera.isActive = true;

		ECSComponents::NameComponent nameComp("Main Camera");
		ECS::Entity cameraEntity = entityManager.create<ECSComponents::NameComponent, ECSComponents::CameraComponent>(nameComp, camera);

		std::cout << "[RuneWeavers] Camera created at position ("
				  << camera.position.x << ", " << camera.position.y << ")" << std::endl;
	} else {
		std::cout << "[RuneWeavers] Camera found in scene" << std::endl;
	}
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

void GameApplication::updateCamera(float deltaTime) {
	auto& entityManager = engine_.getEntityManager();
	auto& inputMng = engine_.getInputManager();

	// Find the active camera
	std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();

	for (auto& cameraEntity : cameraEntities) {
		auto& camera = cameraEntity.get<ECSComponents::CameraComponent>(entityManager);

		if (!camera.isActive) continue;

		// Camera is static - position and viewport are set in editor only
		// Set projection matrix based on camera
		// The camera position is the center of the viewport
		float left = camera.position.x - camera.viewportWidth * 0.5f;
		float right = camera.position.x + camera.viewportWidth * 0.5f;
		float top = camera.position.y - camera.viewportHeight * 0.5f;
		float bottom = camera.position.y + camera.viewportHeight * 0.5f;

		// Create orthographic projection matrix
		std::array<float, 16> projectionMatrix;
		projectionMatrix.fill(0.0f);
		projectionMatrix[0] = 2.0f / (right - left);
		projectionMatrix[5] = 2.0f / (top - bottom);
		projectionMatrix[10] = -2.0f / (1.0f - (-1.0f));
		projectionMatrix[12] = -(right + left) / (right - left);
		projectionMatrix[13] = -(top + bottom) / (top - bottom);
		projectionMatrix[14] = -(1.0f + (-1.0f)) / (1.0f - (-1.0f));
		projectionMatrix[15] = 1.0f;

		engine_.getGraphicsManager().setProjectionMatrix(projectionMatrix.data());

		// Set OpenGL viewport to match window size
		auto [windowWidth, windowHeight] = engine_.getWindowManager().getFramebufferSize();
		glViewport(0, 0, windowWidth, windowHeight);

		// Only use the first active camera
		break;
	}
}

void GameApplication::updateGameLogic() {
	auto& entityManager = engine_.getEntityManager();
	auto& graphics_ = engine_.getGraphicsManager();
	auto& inputMng = engine_.getInputManager();
	auto& tileMap = engine_.getTileMap();
	auto& loader = engine_.getResourceLoader();

	Math::eVec2<float> worldSize = tileMap.getWorldSize();
	worldWidth_ = worldSize.x, worldHeight_ = worldSize.y;
	float deltaTime = static_cast<float>(engine_.getDeltaTime());

	// Always update player movement (no editor state checks for game)
	playerSystem_->Update(entityManager, inputMng, deltaTime);

	// Game is always playing
	engine_.setIsPlaying(true);

	// Update camera system (replaces the old fixed projection)
	updateCamera(deltaTime);

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
	// Handle autoplay at game start (only once)
	if (!musicStarted_)
	{
		auto sceneTreeNodes = engine_.getEnoEditor().getSceneTree().getAllNodes();
		for (const auto& node : sceneTreeNodes)
		{
			bool in = node->entity_.has<ECSComponents::AudioComponent>(entityManager);

			// Ensure that node has audio component and is enabled
			if (in && node->enabled_)
			{
				auto& audioComp = node->entity_.get<ECSComponents::AudioComponent>(entityManager);

				// Auto-start audio if autoplay is enabled
				if (audioComp.isAutoplay_)
				{
					engine_.getAudioManager().play(audioComp);
				}
			}
		}
		musicStarted_ = true;
	}

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
	// Draw all entities
	drawShapes();
	drawPlayerAndIndicator();
	graphics_.flush();
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
			}
			else {
				std::cerr << "[GameLogic] Failed to load texture '" << textureName << "' from path: " << renderComp.texturePath << std::endl;
			}
		}
	}
}

void GameApplication::updateEntities() {

	//ImGuiIO& io = ImGui::GetIO();
	//float deltaTime = io.DeltaTime;
	auto& inputManager = engine_.getInputManager();
	auto& graphics = engine_.getGraphicsManager();

	// Draw background
	graphics.drawTexturedQuadUV(
		Math::eVec2<float>{0.0f, 0.0f},           // Position at origin
		{ worldWidth_, worldHeight_ },			  // World Size
		backgroundTexture_,
		Math::eVec2<float>{0.0f, 0.0f},           // UV start
		Math::eVec2<float>{1.0f, 1.0f},           // UV end
		Graphics::Color::White
	);


	// REBUILD CACHE ONLY WHEN DIRTY (optimization)
	if (entitiesDirty_) {
		cachedDrawableEntities_ = engine_.getEntityManager().findEntity<ECSComponents::DrawInfoComponent>();
		cachedSceneTwoEntities_ = engine_.getEntityManager().findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));

		std::cout << "[DEBUG] Entity cache rebuilt! Found " << cachedDrawableEntities_.size() << " drawable entities\n";

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
			Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);

			if (ctrl)
				ctrl->update(engine_.getDeltaTime());
		}


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
		// NOTE: Player system update is now done in eno_engine.cpp::updateGameSystems()
		// No need to call playerSystem_->Update() here anymore
	}

	// NOTE: Collision system update is now done in eno_engine.cpp::updateGameSystems()
	// No need to call collisionSystem->Update() here anymore

	// NOTE: Physics system update is now done in eno_engine.cpp::updateGameSystems()
	// No need to call physicsSystem.Update() here anymore	

	static bool hasBegun = false;
	auto& coreHost = engine_.getCoreHost();

	// Game is always playing - run game logic
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
		hasBegun = true;
	}

	// Update running scripts
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

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_1)) {
						// I am placing it here first as a 4.0 scale speed * DT
						scaleSize += static_cast<float>(3.0 * 0.01667);
						scaleSize = std::clamp(scaleSize, 1.0f, 100.0f);
						transform.scale.x = scaleSize;
						transform.scale.y = scaleSize;
						std::cout << "Scaling by: " << scaleSize << '\n';
					}

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_2)) {
						scaleSize -= static_cast <float>(3.0 * 0.01667);
						scaleSize = std::clamp(scaleSize, 1.0f, 100.0f);
						transform.scale.x = scaleSize;
						transform.scale.y = scaleSize;
						std::cout << "Scaling by: " << scaleSize << '\n';
					}

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_3)) {
						rotationRadians += static_cast <float>(3.0f * 0.0174533 * 0.01667);

						auto& playerCollider = playerSystem_->GetCollision(it->second);
						playerCollider.rotation = rotationRadians;
						std::cout << "rotation in Radians:" << rotationRadians << '\n';
					}

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_4)) {
						rotationRadians -= static_cast <float>(3.0f * 0.0174533 * 0.01667);
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
						float scaledWidth = (playerConfig_.spriteWidth * scaleSize); //48.0 for playerSpriteW
						float scaledHeight = (playerConfig_.spriteHeight * scaleSize); //48.0 for playerspriteH

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
							Math::eVec2<float>{10 + scaleSize, 10 + scaleSize},
							Graphics::Color::Blue,
							true,
							rotationRadians
						);
					}
				}

			}
		}
		catch (...) {}
	}
}


void GameApplication::drawShapes() {

	auto& graphics = engine_.getGraphicsManager();
	auto& entityManager = engine_.getEntityManager();

	using drawInfo = ECSComponents::DrawInfoComponent;

	static bool firstRun = true;
	if (firstRun) {
		std::cout << "[DEBUG] drawShapes() called! Total cached entities: " << cachedDrawableEntities_.size() << "\n";
		firstRun = false;
	}

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
							uvEnd = frame.uvEnd;
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

