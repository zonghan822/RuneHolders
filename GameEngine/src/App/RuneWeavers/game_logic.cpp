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
	setupCombatSystem();

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

	// Auto-play BGM when game starts (RuneWeavers is a standalone game, not editor)
	std::cout << "[GameApplication] Auto-starting BGM for RuneWeavers" << std::endl;
	playSceneBGM();
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

void GameApplication::setupCombatSystem() {
	auto& entityManager = engine_.getEntityManager();

	std::cout << "[RuneWeavers] Setting up combat system..." << std::endl;

	// Add HealthComponent to player if it exists
	if (playerSpawned_) {
		ECS::Entity playerEntity(playerEntityID_);
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

	std::cout << "[RuneWeavers] Combat system setup complete!" << std::endl;
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

	// Rune hover audio
	audioMng.publishSoundRequest("Rune-GalaxyBellPad-1");
	audioMng.publishSoundRequest("Rune-GalaxyBellPad-2");	
	
	// BGM
	audioMng.publishSoundRequest("BGM_01_Layer2_loop");
	audioMng.publishSoundRequest("Ambience_Creaking_Wood");

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

void GameApplication::playSceneBGM()
{
	auto& audioMng = engine_.getAudioManager();
	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
	std::string sceneName = currentScene.getSceneName();

	std::cout << "[GameApplication] Playing BGM for scene: " << sceneName << std::endl;

	// Determine which BGM to play based on scene name
	std::string bgmName;
	if (sceneName == "MainMenu.txt")
	{
		bgmName = "BGM_01_Layer2_loop";
	}
	else
	{
		// All other scenes play the ambient creaking wood
		bgmName = "Ambience_Creaking_Wood";
	}

	// Check if the sound is loaded before trying to play it
	auto& sounds = audioMng.getSounds();
	if (sounds.find(bgmName) == sounds.end())
	{
		std::cerr << "[GameApplication] Warning: BGM sound '" << bgmName << "' not loaded yet, skipping playback" << std::endl;
		return;
	}

	// Find or create entity with AudioComponent for BGM
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> audioEntities = entityManager.findEntity<ECSComponents::AudioComponent>();

	// Look for existing BGM audio entity
	ECS::Entity bgmEntity;
	bool foundBGMEntity = false;

	for (auto& entity : audioEntities)
	{
		auto& audioComp = entity.get<ECSComponents::AudioComponent>(entityManager);
		// Check if this is a BGM entity (belongs to BGM channel group)
		if (std::string(audioComp.audioMod_.chGroup_) == "BGM")
		{
			bgmEntity = entity;
			foundBGMEntity = true;
			break;
		}
	}

	// If no BGM entity exists, create one
	if (!foundBGMEntity)
	{
		bgmEntity = entityManager.create<ECSComponents::AudioComponent>();
		std::cout << "[GameApplication] Created new BGM audio entity" << std::endl;
	}

	// Configure the audio component
	auto& audioComp = bgmEntity.get<ECSComponents::AudioComponent>(entityManager);

	// Use strncpy to copy strings to char arrays
	std::string fullPath = "assets/sound/music/" + bgmName + ".wav";
	strncpy(audioComp.audioMod_.name_, bgmName.c_str(), sizeof(audioComp.audioMod_.name_) - 1);
	audioComp.audioMod_.name_[sizeof(audioComp.audioMod_.name_) - 1] = '\0';  // Ensure null termination

	strncpy(audioComp.audioMod_.path_, fullPath.c_str(), sizeof(audioComp.audioMod_.path_) - 1);
	audioComp.audioMod_.path_[sizeof(audioComp.audioMod_.path_) - 1] = '\0';  // Ensure null termination

	strncpy(audioComp.audioMod_.chGroup_, "BGM", sizeof(audioComp.audioMod_.chGroup_) - 1);
	audioComp.audioMod_.chGroup_[sizeof(audioComp.audioMod_.chGroup_) - 1] = '\0';  // Ensure null termination

	audioComp.audioMod_.loop_ = Audio::EnoAudioModule::LoopMode::Forward;  // Loop the BGM
	audioComp.volume_ = 30.0;  // Set volume to 30%
	audioComp.isAutoplay_ = true;
	audioComp.isPlaying_ = false;  // Reset playing state

	// Prepare and play the audio
	try
	{
		audioMng.prepareMod(audioComp);
		audioMng.play(audioComp);
		std::cout << "[GameApplication] Started playing BGM: " << bgmName << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[GameApplication] Failed to play BGM: " << e.what() << std::endl;
	}
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
	playerConfig_.LoadFromFile(FileSystem::GetAssetsDir().string() + "/saves/player/player_config.txt");
	std::cout << "[GameLogic] Player config loaded" << std::endl;
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

	// Create player idle animation - 1 row, 17 columns, 96x96 per frame
	auto playerIdleAnim = Animation::createSpriteSheetAnimation(
		96,    // Frame width
		96,    // Frame height
		1632,  // Sheet width (96 * 17 = 1632)
		96,    // Sheet height
		0,     // Start frame
		17,    // Frame count
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

	// ========== PAUSE MENU AND SPELLBOOK INPUT HANDLING ==========
	// Only handle pause/spellbook input when player exists
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	bool playerExists = !playerEntities.empty();

	if (playerExists) {
		// Handle F key - Toggle Spellbook
		if (inputMng.isKeyDown(GLFW_KEY_F)) {
			if (!showSpellbook_) {
				// Open spellbook and pause game
				showSpellbook_ = true;
				isPaused_ = true;
				hidePlayer();
				spawnSpellbook();
				engine_.setIsPlaying(false);
				std::cout << "[Input] Spellbook opened (F key pressed)" << std::endl;
			}
			else {
				// Close spellbook and resume game
				showSpellbook_ = false;
				isPaused_ = false;
				destroySpellbook();
				showPlayer();
				engine_.setIsPlaying(true);
				std::cout << "[Input] Spellbook closed (F key pressed)" << std::endl;
			}
		}

		// Handle ESC key - Toggle Pause Menu or close Spellbook
		if (inputMng.isKeyDown(GLFW_KEY_ESCAPE)) {
			if (showSpellbook_) {
				// If spellbook is open, close it and resume game
				showSpellbook_ = false;
				isPaused_ = false;
				destroySpellbook();
				showPlayer();
				engine_.setIsPlaying(true);
				std::cout << "[Input] Spellbook closed (ESC key pressed)" << std::endl;
			}
			else if (!isPaused_) {
				// If not paused and not in spellbook, open pause menu
				isPaused_ = true;
				hidePlayer();
				spawnPauseMenu();
				engine_.setIsPlaying(false);
				std::cout << "[Input] Pause menu opened (ESC key pressed)" << std::endl;
			}
			else {
				// If pause menu is open, close it and resume game
				isPaused_ = false;
				destroyPauseMenu();
				showPlayer();
				engine_.setIsPlaying(true);
				std::cout << "[Input] Pause menu closed (ESC key pressed)" << std::endl;
			}
		}
	}

	// Only update game systems when not paused
	if (!isPaused_) {
		// Always update player movement (no editor state checks for game)
		playerSystem_->Update(entityManager, inputMng, deltaTime);

		// Update rune system
		updateRuneSystem();

		// Update combat system (damage dealing and health)
		updateCombatSystem(deltaTime);
	}

	// Update camera system (replaces the old fixed projection)
	updateCamera(deltaTime);

	// Animation state (must be outside the pause check so currentFrame is accessible later)
	static float animationTimer = 0.0f;
	static int currentFrame = 0;
	const int totalFrames = 4;  // Protagonist-Walk-Sheet has 4 frames
	const float frameDuration = 0.1f;  // 0.1 seconds per frame

	// Only update animations when not paused
	if (!isPaused_) {
		if (playerTexture_ && playerSpawned_) {
			playerAnimController_.update(deltaTime);
		}

		// Update all animated entities (spritesheet animation and rotation)
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
	} // End of if (!isPaused_)

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
			if (entity.has<ECSComponents::AnimationControllerComponent>(engine_.getEntityManager())) {
				auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(engine_.getEntityManager());

				// Handle old format (backward compatibility) - single animation with Frame Width/Height/Rows/Cols
				if (animComp.animationCount == 0 && animComp.frameWidth > 0 && animComp.frameHeight > 0) {
					if (entity.has<ECSComponents::DrawInfoComponent>(engine_.getEntityManager())) {
						auto& drawComp = entity.get<ECSComponents::DrawInfoComponent>(engine_.getEntityManager());

						// Migrate old format to new array format
						const char* animName = (strlen(animComp.currentAnimationName) > 0) ? animComp.currentAnimationName : "Default";
						animComp.setAnimationData(
							animName,
							drawComp.texturePath,
							animComp.frameWidth,
							animComp.frameHeight,
							animComp.rows,
							animComp.cols,
							animComp.frameDuration,
							animComp.looping
						);

						std::cout << "[GameLogic] Migrated old format animation '" << animName << "' to new format" << std::endl;
					}
				}

				// Restore all animations from the animations array
				if (animComp.animationCount > 0) {
					// Check if controller exists in registry, if not create a new one
					Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);
					if (!ctrl) {
						// Controller doesn't exist (registry was cleared), create new one
						animComp.controllerID = engine_.getAnimationRegistry().createController();
						ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);
						std::cout << "[GameLogic] Created new controller ID: " << animComp.controllerID << std::endl;
					} else {
						// Controller exists, clear it before restoring animations
						ctrl->animations_.clear();
						ctrl->textures_.clear();
						ctrl->currentAnimation_ = nullptr;
						ctrl->currentTexture_ = nullptr;
						ctrl->currentAnimationName_.clear();
						std::cout << "[GameLogic] Reusing existing controller ID: " << animComp.controllerID << std::endl;
					}

					if (ctrl) {
						// Recreate all animations from saved configuration
						for (int i = 0; i < animComp.animationCount; ++i) {
							const auto& animData = animComp.animations[i];

							// Skip if no valid data
							if (animData.frameWidth <= 0 || animData.frameHeight <= 0) {
								continue;
							}

							// Load texture for this animation
							std::shared_ptr<Graphics::EnoTexture> texture;
							if (strlen(animData.texturePath) > 0) {
								auto& textures = engine_.getResourceLoader().getAllTextures();
								auto it = textures.find(animData.texturePath);
								if (it != textures.end()) {
									texture = it->second;
								}
							}

							if (texture) {
								// Recreate the animation using Frame Width/Height/Rows/Cols
								Animation::EnoAnimation anim = Animation::createSpriteSheetAnimation(
									animData.frameWidth,
									animData.frameHeight,
									texture->getWidth(),
									texture->getHeight(),
									0,  // start frame
									animData.rows * animData.cols,  // total frames
									animData.frameDuration
								);

								anim.setLooping(animData.looping);
								ctrl->addAnimation(animData.name, anim, texture);

								std::cout << "[GameLogic] Restored animation '" << animData.name
									<< "' (FrameW: " << animData.frameWidth << ", FrameH: " << animData.frameHeight
									<< ", Rows: " << animData.rows << ", Cols: " << animData.cols << ")" << std::endl;
							}
						}

						// Set the current animation if specified
						if (strlen(animComp.currentAnimationName) > 0) {
							ctrl->setAnimation(animComp.currentAnimationName);
							std::cout << "[GameLogic] Set current animation to '" << animComp.currentAnimationName << "'" << std::endl;
						}

						// Sync the current texture to DrawInfoComponent
						// This is crucial for the renderer to use drawTexturedQuadUV
						if (entity.has<ECSComponents::DrawInfoComponent>(engine_.getEntityManager()) && ctrl->currentTexture_) {
							auto& drawComp = entity.get<ECSComponents::DrawInfoComponent>(engine_.getEntityManager());
							drawComp.texture = ctrl->currentTexture_;

							// Also update the texture path in DrawInfoComponent to match
							if (!ctrl->currentAnimationName_.empty()) {
								for (int i = 0; i < animComp.animationCount; ++i) {
									if (strcmp(animComp.animations[i].name, ctrl->currentAnimationName_.c_str()) == 0) {
										strncpy_s(drawComp.texturePath, animComp.animations[i].texturePath, sizeof(drawComp.texturePath) - 1);
										drawComp.texturePath[sizeof(drawComp.texturePath) - 1] = '\0';
										break;
									}
								}
							}
							std::cout << "[GameLogic] Synced texture to DrawInfoComponent for animation '" << ctrl->currentAnimationName_ << "'" << std::endl;
						}
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

							// Check if entity is a fireball and flip it horizontally
							if (entity.has<ECSComponents::FireballComponent>(entityManager)) {
								std::swap(uvStart.x, uvEnd.x);
							}

							// Check if entity is an enemy with AI and flip based on movement direction
							if (entity.has<ECSComponents::AIComponent>(entityManager) &&
								entity.has<ECSComponents::PhysicsComponent>(entityManager)) {
								auto& physics = entity.get<ECSComponents::PhysicsComponent>(entityManager);
								// Flip sprite if moving right (positive x velocity)
								if (physics.velocity.x > 0.1f) {
									std::swap(uvStart.x, uvEnd.x);
								}
							}
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
	// Only allow rune system when player exists
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	if (playerEntities.empty()) {
		return;  // No player, don't allow spell casting
	}

	// Initialize if not done yet
	if (!runeSystemInitialized_) {
		initializeRuneSystem();
	}

	auto& inputManager = engine_.getInputManager();

	// Get rune system entity
	ECS::Entity runeSystemEntity(runeSystemEntityID_);

	// Check if rune system entity still exists
	if (!runeSystemEntity.has<ECSComponents::RuneSystemStateComponent>(entityManager)) {
		return;
	}

	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Get camera position for world space conversion
	std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();
	float cameraX = 0.0f;
	float cameraY = 0.0f;
	float viewportWidth = 1920.0f;
	float viewportHeight = 1080.0f;

	if (!cameraEntities.empty()) {
		auto& camera = cameraEntities[0].get<ECSComponents::CameraComponent>(entityManager);
		cameraX = camera.position.x - camera.viewportWidth * 0.5f;
		cameraY = camera.position.y - camera.viewportHeight * 0.5f;
		viewportWidth = camera.viewportWidth;
		viewportHeight = camera.viewportHeight;
	}

	// Get window size for fullscreen conversion
	auto windowSize = engine_.getWindowManager().getWindowSize();

	// Get mouse position and convert to world space
	auto mouseScreenPos = inputManager.getMousePosition();
	Math::eVec2<float> mouseWorldPos = Graphics::screenToWorld(
		{ static_cast<float>(mouseScreenPos.first), static_cast<float>(mouseScreenPos.second) },
		{ 0.0f, 0.0f },  // Screen min (fullscreen starts at 0,0)
		{ static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) },  // Screen max
		viewportWidth,
		viewportHeight,
		cameraX,
		cameraY
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
				// Play left rune hover sound
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-1" });
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
			bool wasRightHovered = rightRune.isHovered;
			rightRune.isHovered = (distToRightRune <= rightRune.radius);

			// Play right rune hover sound when first hovering
			if (rightRune.isHovered && !wasRightHovered) {
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-2" });
			}
		}
	}
}

void GameApplication::drawRuneConnectionLine()
{
	auto& entityManager = engine_.getEntityManager();
	auto& graphics = engine_.getGraphicsManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// In RuneWeavers, positions are already in screen space
	Math::eVec2<float> screenStart = runeState.lineStart;
	Math::eVec2<float> screenEnd = runeState.lineEnd;

	// Draw line using graphics system
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
	if (playerSpawned_) {
		ECS::Entity playerEntity(playerEntityID_);
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

	// If no enemy found, use mouse position as target
	if (!foundEnemy) {
		auto& inputManager = engine_.getInputManager();
		auto mouseScreenPos = inputManager.getMousePosition();

		// Get camera for world space conversion
		std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();
		float cameraX = 0.0f;
		float cameraY = 0.0f;
		float viewportWidth = 1920.0f;
		float viewportHeight = 1080.0f;

		if (!cameraEntities.empty()) {
			auto& camera = cameraEntities[0].get<ECSComponents::CameraComponent>(entityManager);
			cameraX = camera.position.x - camera.viewportWidth * 0.5f;
			cameraY = camera.position.y - camera.viewportHeight * 0.5f;
			viewportWidth = camera.viewportWidth;
			viewportHeight = camera.viewportHeight;
		}

		// Get window size for conversion
		auto windowSize = engine_.getWindowManager().getWindowSize();

		// Convert mouse position to world space
		targetPos = Graphics::screenToWorld(
			{ static_cast<float>(mouseScreenPos.first), static_cast<float>(mouseScreenPos.second) },
			{ 0.0f, 0.0f },
			{ static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) },
			viewportWidth,
			viewportHeight,
			cameraX,
			cameraY
		);
	}

	// Calculate direction to target
	Math::eVec2<float> direction = { targetPos.x - playerPos.x, targetPos.y - playerPos.y };
	float length = Math::eVec2Length(direction);
	if (length > 0.0f) {
		direction.x /= length;  // Normalize
		direction.y /= length;
	}

	// Calculate rotation angle for fireball orientation
	// atan2 gives angle in radians, fireball sprite points left by default (-180 degrees or PI radians)
	// So we need to add PI to make it point in the direction of movement
	float fireballRotation = std::atan2(direction.y, direction.x);

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
	transform.rot.x = fireballRotation;  // Set rotation to face direction of travel

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

	// Setup animation configuration - fireball spritesheet is 1 row, 4 columns, 90x48 per frame
	auto& animComp = fireball.get<ECSComponents::AnimationControllerComponent>(entityManager);
	if (fireballTexture && fireballTexture->isValid()) {
		animComp.setAnimationData(
			"fly",                              // Animation name
			"Spell-Fireball-Flying-Sheet",      // Texture path (match resource loader name)
			90,                                 // Frame width
			48,                                 // Frame height
			1,                                  // Rows
			4,                                  // Cols
			0.1f,                               // Frame duration
			true                                // Looping
		);
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

			// Get half dimensions - NOW ACCOUNTING FOR TRANSFORM SCALE
			float aHalfW = (collA.data.rectangle.width * transA.scale.x) * 0.5f;
			float aHalfH = (collA.data.rectangle.height * transA.scale.y) * 0.5f;
			float bHalfW = (collB.data.rectangle.width * transB.scale.x) * 0.5f;
			float bHalfH = (collB.data.rectangle.height * transB.scale.y) * 0.5f;

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

				std::cout << "[Combat] Fireball hit enemy! Dealt " << damage << " damage. Enemy HP: "
					<< enemyHealth.currentHealth << "/" << enemyHealth.maxHealth << std::endl;

				if (died) {
					std::cout << "[Combat] Enemy defeated!" << std::endl;
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
	if (playerSpawned_) {
		ECS::Entity playerEntity(playerEntityID_);
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
				if (checkAABBCollision(playerTransform, playerCollision, enemyTransform, enemyCollision)) {
					// Deal damage to player
					float damage = enemyDamage.getTotalDamage();
					bool died = playerHealth.takeDamage(damage);

					std::cout << "[Combat] Enemy hit player! Dealt " << damage << " damage. Player HP: "
						<< playerHealth.currentHealth << "/" << playerHealth.maxHealth << std::endl;

					if (died) {
						std::cout << "[Combat] Player defeated!" << std::endl;
					}

					// Give player temporary invulnerability after being hit
					playerHealth.invulnerable = true;
					playerHealth.invulnerabilityTimer = 0.5f;
				}
			}
		}
	}
}

// ========== PAUSE MENU AND SPELLBOOK SYSTEM ==========

void GameApplication::spawnSpellbook() {
	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Load spellbook texture
	auto spellbookTexture = resourceLoader.getTexture("SpellBook-HowToCast_Fireball");

	if (!spellbookTexture || !spellbookTexture->isValid()) {
		std::cerr << "[Spellbook] Failed to load spellbook texture!" << std::endl;
		return;
	}

	// Create spellbook entity
	ECS::Entity spellbookEntity = entityManager.create();
	spellbookEntityID_ = spellbookEntity.id();

	// Add components
	spellbookEntity.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("Spellbook UI"));
	spellbookEntity.add<ECSComponents::TransformComponent>(entityManager);
	spellbookEntity.add<ECSComponents::DrawInfoComponent>(entityManager);

	// Configure transform - centered at 1024, 1024
	auto& transform = spellbookEntity.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = { 1024.0f, 1024.0f };
	transform.scale = { 1.0f, 1.0f };
	transform.rot = { 0.0f, 0.0f };

	// Configure rendering - 1537x961 texture
	auto& draw = spellbookEntity.get<ECSComponents::DrawInfoComponent>(entityManager);
	draw.width = 1537.0f;
	draw.height = 961.0f;
	draw.color = Graphics::Color::White;
	strcpy_s(draw.shape, "Rectangle");
	draw.texture = spellbookTexture;

	// Mark entities dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[Spellbook] Spellbook UI spawned at (1024, 1024)" << std::endl;
}

void GameApplication::destroySpellbook() {
	if (spellbookEntityID_ != 0) {
		auto& entityManager = engine_.getEntityManager();
		entityManager.destroyEntity(spellbookEntityID_);
		spellbookEntityID_ = 0;
		entitiesDirty_ = true;
		std::cout << "[Spellbook] Spellbook UI destroyed" << std::endl;
	}
}

void GameApplication::spawnPauseMenu() {
	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Load pause menu texture
	auto pauseMenuTexture = resourceLoader.getTexture("PauseMenu-Popup");

	if (!pauseMenuTexture || !pauseMenuTexture->isValid()) {
		std::cerr << "[PauseMenu] Failed to load pause menu texture!" << std::endl;
		return;
	}

	// Create pause menu entity
	ECS::Entity pauseMenuEntity = entityManager.create();
	pauseMenuEntityID_ = pauseMenuEntity.id();

	// Add components
	pauseMenuEntity.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("Pause Menu UI"));
	pauseMenuEntity.add<ECSComponents::TransformComponent>(entityManager);
	pauseMenuEntity.add<ECSComponents::DrawInfoComponent>(entityManager);

	// Configure transform - centered at 1024, 1024
	auto& transform = pauseMenuEntity.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = { 1024.0f, 1024.0f };
	transform.scale = { 1.0f, 1.0f };
	transform.rot = { 0.0f, 0.0f };

	// Configure rendering - 1537x961 texture
	auto& draw = pauseMenuEntity.get<ECSComponents::DrawInfoComponent>(entityManager);
	draw.width = 1537.0f;
	draw.height = 961.0f;
	draw.color = Graphics::Color::White;
	strcpy_s(draw.shape, "Rectangle");
	draw.texture = pauseMenuTexture;

	// Mark entities dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[PauseMenu] Pause menu UI spawned at (1024, 1024)" << std::endl;
}

void GameApplication::destroyPauseMenu() {
	if (pauseMenuEntityID_ != 0) {
		auto& entityManager = engine_.getEntityManager();
		entityManager.destroyEntity(pauseMenuEntityID_);
		pauseMenuEntityID_ = 0;
		entitiesDirty_ = true;
		std::cout << "[PauseMenu] Pause menu UI destroyed" << std::endl;
	}
}

void GameApplication::hidePlayer() {
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();

	if (!playerEntities.empty()) {
		ECS::Entity& player = playerEntities[0];
		if (player.has<ECSComponents::TransformComponent>(entityManager)) {
			auto& transform = player.get<ECSComponents::TransformComponent>(entityManager);
			// Save current scale
			savedPlayerScale_ = transform.scale;
			// Set scale to 0 to hide
			transform.scale = { 0.0f, 0.0f };
			std::cout << "[Player] Hidden (scale set to 0)" << std::endl;
		}
	}
}

void GameApplication::showPlayer() {
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();

	if (!playerEntities.empty()) {
		ECS::Entity& player = playerEntities[0];
		if (player.has<ECSComponents::TransformComponent>(entityManager)) {
			auto& transform = player.get<ECSComponents::TransformComponent>(entityManager);
			// Restore saved scale
			transform.scale = savedPlayerScale_;
			std::cout << "[Player] Shown (scale restored to " << savedPlayerScale_.x << ", " << savedPlayerScale_.y << ")" << std::endl;
		}
	}
}

