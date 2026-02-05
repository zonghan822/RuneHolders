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
#include <algorithm>
#include "imGui.hpp"
#include "math/math.hpp"
#include "particle/particle_presets.hpp"
#include "particle/particle_component.hpp"
#include <typeindex>

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
	setupMainMenuButtons();
	setupSceneTransitions();

	auto& tileMap = engine_.getTileMap();

	// Set up tile map editor (To be removed after tile map node is set up)
	mapEditor_ = std::make_unique<MapEditor>(tileMap);
	Math::eVec2<float> worldSize = tileMap.getWorldSize();

	stateMachineEditor_ = std::make_unique<Editor::StateMachineEditor>(engine_.getEnoEditor());

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

	// Subscribe to PlayScene event to start BGM when game starts
	engine_.getEventBus().subscribe<Editor::PlayScene>([this](const Editor::PlayScene&)
		{
			std::cout << "[GameApplication] Play button pressed - starting BGM" << std::endl;
			playSceneBGM();
		});

	// Subscribe to StopScene event to stop BGM when game stops
	engine_.getEventBus().subscribe<Editor::StopScene>([this](const Editor::StopScene&)
		{
			std::cout << "[GameApplication] Stop button pressed - stopping BGM" << std::endl;
			engine_.getEventBus().publish(Audio::StopBGM{});
		});

	// Dialogue texts
	dialogueTexts_ = {	"OH, WHAT'S THIS?",
						"A SCROLL FULL OF SPELLS!",
						"HEHEHEHE...",
						"DID I JUST HEAR SOMETHING?",
						"WH-WHO GOES THERE?", 
						"...",
						"I GUESS I WAS JUST HEARING THINGS THEN.",
						"LET ME RECORD THE SPELLS IN MY SPELLBOOK." };

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
	philosopherBoldFont = engine_.getResourceLoader().getFont("Philosopher-Bold");
	philosopherRegularFont = engine_.getResourceLoader().getFont("Philosopher-Regular");
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

	// Re-register all scripts that were loaded before the scripting system was initialized
	// Scene loading happens in EnoEditor constructor before setupScripting(), so rpm_ was null
	// during initial registration. This ensures scripts are properly registered.
	engine_.getEnoEditor().getSceneTree().registerAllScripts();
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

	audioMng.publishSoundRequest("Rune-GalaxyBellPad-1");
	audioMng.publishSoundRequest("Rune-GalaxyBellPad-2");

	// Spellbook audio
	audioMng.publishSoundRequest("Spellbook_Open_01");
	audioMng.publishSoundRequest("Spellbook_Close_01");

	// Fireball audio
	audioMng.publishSoundRequest("Fireball-Cast");  // Add your fireball spawn sound file name here
	audioMng.publishSoundRequest("Fireball-Explosion-Small");    // Add your fireball hit sound file name here

	// Scene BGM audio
	audioMng.publishSoundRequest("BGM_01_Layer1_loop");  // Main menu BGM
	audioMng.publishSoundRequest("Ambience_Creaking_Wood");  // General scene ambience

	// setup input to play sound
	auto& inputMng = engine_.getInputManager();


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
		bgmName = "BGM_01_Layer1_loop";
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
	audioComp.volume_ = 70.0;  // Set volume to 70%
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
	playerConfig_.LoadFromFile(FileSystem::GetAssetsDir().string() + "/saves/player/player_config.txt");
	//std::cout << "[GameLogic] Player config loaded" << std::endl;
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

	// Test particle system
	testParticleSystem();

	// Check if we're in play mode
	bool isPlayMode = (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing ||
	                   engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame);

	// Update main menu buttons (works in both play mode and edit mode)
	updateMainMenuButtons();
	updateSceneTransitions();

	// ========== PAUSE MENU AND SPELLBOOK INPUT HANDLING ==========
	// Only handle pause/spellbook input when in play mode AND player exists
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	bool playerExists = !playerEntities.empty();

	if (isPlayMode && playerExists) {
		// Handle F key - Toggle Spellbook
		if (inputMng.isKeyDown(GLFW_KEY_F)) {
			if (!showSpellbook_) {
				// Open spellbook and pause game
				showSpellbook_ = true;
				isPaused_ = true;
				hidePlayer();
				spawnSpellbook();
				engine_.setIsPlaying(false);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Open_01" });
				std::cout << "[Input] Spellbook opened (F key pressed)" << std::endl;
			}
			else {
				// Close spellbook and resume game
				showSpellbook_ = false;
				isPaused_ = false;
				destroySpellbook();
				showPlayer();
				engine_.setIsPlaying(true);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
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
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
				std::cout << "[Input] Spellbook closed (ESC key pressed)" << std::endl;
			}
			else if (!isPaused_) {
				// If not paused and not in spellbook, open pause menu
				isPaused_ = true;
				hidePlayer();
				spawnPauseMenu();
				engine_.setIsPlaying(false);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Open_01" });
				std::cout << "[Input] Pause menu opened (ESC key pressed)" << std::endl;
			}
			else {
				// If pause menu is open, close it and resume game
				isPaused_ = false;
				destroyPauseMenu();
				showPlayer();
				engine_.setIsPlaying(true);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
				std::cout << "[Input] Pause menu closed (ESC key pressed)" << std::endl;
			}
		}

		// Handle E key - Interact with Collectible-Scroll or advance dialogue
		if (inputMng.isKeyDown(GLFW_KEY_E)) {
			if (dialogueActive_) {
				if (dialogueTyping_) {
					// Finish current text animation instantly
					dialogueTyping_ = false;
					dialogueCurrentText_ = dialogueTargetText_;
					auto bodyText = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("body text"));
					if (!bodyText.empty() && bodyText[0].has<ECSComponents::TextComponent>(entityManager)) {
						auto& textComp = bodyText[0].get<ECSComponents::TextComponent>(entityManager);
						strncpy_s(textComp.text, dialogueCurrentText_.c_str(), sizeof(textComp.text) - 1);
						textComp.text[sizeof(textComp.text) - 1] = '\0';
					}
				}
				else {
					advanceDialogue();
				}
			}
			else if (!isPaused_) {
				// Find scroll entity
				auto scrolls = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("Collectible-Scroll"));
				if (!scrolls.empty()) {
					for (auto& it : scrolls)
					{
						if (currentTriggersPlayerIsIn_.count(it.id()))
						{
							startDialogue();
						}
					}
				}
			}
		}
	}

	// Dialogue typewriter animation
	if (dialogueTyping_) {
		// Start timer
		dialogueTypeTimer_ += deltaTime;
		while (dialogueTypeTimer_ >= DIALOGUE_TYPE_SPEED && dialogueCurrentText_.size() < dialogueTargetText_.size()) {
			// Add next letter, reset timer
			dialogueCurrentText_ += dialogueTargetText_[dialogueCurrentText_.size()];
			dialogueTypeTimer_ -= DIALOGUE_TYPE_SPEED;
		}
		auto bodyText = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("body text"));
		if (!bodyText.empty() && bodyText[0].has<ECSComponents::TextComponent>(entityManager)) {
			auto& textComp = bodyText[0].get<ECSComponents::TextComponent>(entityManager);
			strncpy_s(textComp.text, dialogueCurrentText_.c_str(), sizeof(textComp.text) - 1);
			textComp.text[sizeof(textComp.text) - 1] = '\0';
		}
		if (dialogueCurrentText_.size() >= dialogueTargetText_.size()) {
			dialogueTyping_ = false;
		}
	}

	// Only update player movement when in play mode and not paused
	if (isPlayMode && !isPaused_) {
		playerSystem_->Update(entityManager, inputMng, deltaTime);

		// Update rune system
		updateRuneSystem();

		// Update combat system (damage dealing and health)
		updateCombatSystem(deltaTime);

		// Update floating texts (rise and fade)
		updateFloatingTexts(deltaTime);

		// Update rupture mark spells (state machine, enemy detection, damage)
		updateRuptureMarks(deltaTime);
	}

	// Update engine playing state based on editor state (but override if paused)
	if (!isPaused_) {
		engine_.setIsPlaying(engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing || engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame);
	}

	if (gameFramebuffer_) {

		// CRITICAL: Set projection matrix to fixed virtual resolution
		engine_.getGraphicsManager().setProjectionMatrix(static_cast<int>(worldWidth_), static_cast<int>(worldHeight_));

		// Bind framebuffer for rendering
		gameFramebuffer_->bind();
	}

	// Clear the framebuffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	// Check if we just stopped play mode (scene state changed to Stopped)
	bool justStopped = (lastSceneState == Editor::State::Playing &&
	                    currentSceneState == Editor::State::Stopped);

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

				// Prepare the audio module (load sound pointer if needed)
				// This is necessary after deserializing from a saved scene
				if (audioComp.audioMod_.name_[0] != '\0')
				{
					engine_.getAudioManager().prepareMod(audioComp);
				}

				// Auto-start audio if autoplay is enabled
				// This only happens once when scene starts, not every frame
				if (audioComp.isAutoplay_)
				{
					engine_.getAudioManager().play(audioComp);
				}
			}
		}
	}

	// Clean up pause menu and spellbook entities when stopping the scene
	if (justStopped)
	{
		if (showSpellbook_) {
			destroySpellbook();
			showSpellbook_ = false;
		}
		if (isPaused_) {
			destroyPauseMenu();
			isPaused_ = false;
		}
		// Restore player visibility if they were hidden
		showPlayer();
		std::cout << "[Editor] Scene stopped - cleaned up pause/spellbook entities" << std::endl;
	}

	// Update last scene state for next frame
	lastSceneState = currentSceneState;

	// =========================

	// Only update animated entities when not paused
	if (!isPaused_) {
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

		//// Clamp camera to world bounds (accounting for zoom)
		//editorCameraX_ = std::clamp(editorCameraX_, 0.0f, worldWidth_ - static_cast<float>(editorFbSizeX));
		//editorCameraY_ = std::clamp(editorCameraY_, 0.0f, worldHeight_ - static_cast<float>(editorFbSizeY));

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

		// Collect and sort cached entities by layer
		ECSComponents::collectObjectsByLayer(engine_, cachedDrawableEntities_);

		// Draw all entities by layer order (lowest z_index first = background)
		for (auto& [layer, entities] : ECSComponents::sortedObjectLayers) {
			for (ECS::Entity* entityPtr : entities) {
				ECS::Entity& entity = *entityPtr;
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

							// Custom rendering for runes (spritesheet with UV)
							if (entity.has<ECSComponents::RuneComponent>(entityManager)) {
								auto& runeComp = entity.get<ECSComponents::RuneComponent>(entityManager);

								// Calculate UV coordinates based on rune number
								Math::eVec2<float> uvStart, uvEnd;
								ECSComponents::RuneAssets::getRuneUV(runeComp.runeNumber, uvStart, uvEnd);

								engine_.getGraphicsManager().drawTexturedQuadUV(
									Math::eVec2<float>{topLeftX, topLeftY},
									Math::eVec2<float>{scaledWidth, scaledHeight},
									renderInfo.texture,
									uvStart,
									uvEnd,
									renderInfo.color,
									rotation
								);
							}
							// Check if entity has animation controller
							else if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager)) {
								auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
								Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);

								if (ctrl && ctrl->getCurrentAnimation()) {
									// Update animation and get current frame
									const Animation::AnimationFrame& frame = ctrl->update(static_cast<float>(deltaTime));

									// Get UV coordinates
									Math::eVec2<float> uvStart = frame.uvStart;
									Math::eVec2<float> uvEnd = frame.uvEnd;

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

									// Draw with animation UV coordinates
									engine_.getGraphicsManager().drawTexturedQuadUV(
										Math::eVec2<float>{topLeftX, topLeftY},
										Math::eVec2<float>{scaledWidth, scaledHeight},
										renderInfo.texture,
										uvStart,
										uvEnd,
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

		// Flush pending entity batch before text drawing (drawTextScreen unbinds defaultShader)
		graphics_.flush();

		// Draw TextComponent entities (screen space, multiline support)
		{
			auto textEntities = engine_.getEntityManager().findEntity<ECSComponents::TextComponent>();
			for (auto& textEntity : textEntities) {
				if (textEntity.has<ECSComponents::VisibilityComponent>(entityManager)) {
					if (!textEntity.get<ECSComponents::VisibilityComponent>(entityManager).visible)
						continue;
				}
				if (!ECSComponents::isAncestorVisible(textEntity.id(), engine_.getEnoEditor().getSceneTree(), entityManager))
					continue;
				if (textEntity.has<ECSComponents::TransformComponent>(entityManager)) {
					auto& textComp  = textEntity.get<ECSComponents::TextComponent>(entityManager);
					auto& transform = textEntity.get<ECSComponents::TransformComponent>(entityManager);
					if (strlen(textComp.text) == 0) continue;

					auto font = engine_.getResourceLoader().getFont(std::string(textComp.fontName));
					if (!font) continue;

					Math::eVec2<float> screenPos = {
						transform.pos.x - editorCameraX_,
						transform.pos.y - editorCameraY_
					};

					// Draw each line separately to support newlines
					std::string fullText(textComp.text);
					float lineHeight = 60.0f * textComp.fontSize;
					float yOffset = 0.0f;
					size_t start = 0;
					while (start <= fullText.size()) {
						size_t pos = fullText.find('\n', start);
						std::string line = (pos != std::string::npos)
							? fullText.substr(start, pos - start)
							: fullText.substr(start);
						if (!line.empty()) {
							graphics_.drawTextScreen(
								font, line,
								Math::eVec2<float>{ screenPos.x, screenPos.y + yOffset },
								textComp.fontSize,
								Graphics::Color::White,
								static_cast<float>(editorFbSizeX),
								static_cast<float>(editorFbSizeY)
							);
						}
						if (pos == std::string::npos) break;
						yOffset += lineHeight;
						start = pos + 1;
					}
				}
			}
		}

		// Restore defaultShader after text drawing
		graphics_.rebindDefaultShader();

		// Render particles (with camera offset)
		auto* particleSystem = engine_.getParticleSystem();
		if (particleSystem) {
			Math::eVec2<float> cameraOffset = { editorCameraX_, editorCameraY_ };
			particleSystem->Render(graphics_, engine_.getEntityManager(), cameraOffset);
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
				//std::cout << "[GameLogic] Loaded texture '" << textureName << "' for entity " << entityID << std::endl;
			} else {
				//std::cerr << "[GameLogic] Failed to load texture '" << textureName << "' from path: " << renderComp.texturePath << std::endl;
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

						//std::cout << "[GameLogic] Migrated old format animation '" << animName << "' to new format" << std::endl;
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
						//std::cout << "[GameLogic] Created new controller ID: " << animComp.controllerID << std::endl;
					} else {
						// Controller exists, clear it before restoring animations
						ctrl->animations_.clear();
						ctrl->textures_.clear();
						ctrl->currentAnimation_ = nullptr;
						ctrl->currentTexture_ = nullptr;
						ctrl->currentAnimationName_.clear();
						//std::cout << "[GameLogic] Reusing existing controller ID: " << animComp.controllerID << std::endl;
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

							}
						}

						// Set the current animation if specified
						if (strlen(animComp.currentAnimationName) > 0) {
							ctrl->setAnimation(animComp.currentAnimationName);
							//std::cout << "[GameLogic] Set current animation to '" << animComp.currentAnimationName << "'" << std::endl;
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
							//std::cout << "[GameLogic] Synced texture to DrawInfoComponent for animation '" << ctrl->currentAnimationName_ << "'" << std::endl;
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
				coreHost.hotReload();
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
							//coreHost.begin(org, node->entity_.id());  
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
						//coreHost.update(org, node->entity_.id());  
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

					//std::cout << "is button active " << b.getActive() << std::endl;

					// Check if button is active
					if (b.getActive())
					{
						// Check if mouse pos in clickable area
						if (b.onClick(mousePos, t, d, c))
						{

							//std::cout << "CLICK!" << std::endl;
							//b.executeSignal(ECSComponents::signalType::onClick, engine_);
							// Emit signal
							/*auto signalIt = b.getSignalMap().find("started");
							if (signalIt != b.getSignalMap().end())
							{
								signalIt->second.emit();
							}*/

							// Emit signal
							auto signalIt = b.signals_.find("onClick");
							if (signalIt != b.signals_.end())
							{
								signalIt->second.emit();
							}
						}
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
				if (entity.has<ECSComponents::VisibilityComponent>(entityManager))
				{
					if (!entity.get<ECSComponents::VisibilityComponent>(entityManager).visible)
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
				
			}
		}
		catch (...) {
			// Entity was destroyed, rebuild cache next frame
			entitiesDirty_ = true;
			continue;
		}
	}

	// Flush pending entity batch before text drawing (drawTextScreen unbinds defaultShader)
	graphics.flush();

	// Draw TextComponent entities (screen space, multiline support)
	{
		auto textEntities = engine_.getEntityManager().findEntity<ECSComponents::TextComponent>();
		auto [w, h] = engine_.getWindowManager().getFramebufferSize();
		for (auto& textEntity : textEntities) {
			if (textEntity.has<ECSComponents::VisibilityComponent>(entityManager)) {
				if (!textEntity.get<ECSComponents::VisibilityComponent>(entityManager).visible)
					continue;
			}
			if (!ECSComponents::isAncestorVisible(textEntity.id(), engine_.getEnoEditor().getSceneTree(), entityManager))
				continue;
			if (textEntity.has<ECSComponents::TransformComponent>(entityManager)) {
				auto& textComp  = textEntity.get<ECSComponents::TextComponent>(entityManager);
				auto& transform = textEntity.get<ECSComponents::TransformComponent>(entityManager);
				if (strlen(textComp.text) == 0) continue;

				auto font = engine_.getResourceLoader().getFont(std::string(textComp.fontName));
				if (!font) continue;

				// Draw each line separately to support newlines
				std::string fullText(textComp.text);
				float lineHeight = 60.0f * textComp.fontSize;
				float yOffset = 0.0f;
				size_t start = 0;
				while (start <= fullText.size()) {
					size_t pos = fullText.find('\n', start);
					std::string line = (pos != std::string::npos)
						? fullText.substr(start, pos - start)
						: fullText.substr(start);
					if (!line.empty()) {
						graphics.drawTextScreen(
							font, line,
							Math::eVec2<float>{ transform.pos.x, transform.pos.y + yOffset },
							textComp.fontSize,
							Graphics::Color::White,
							static_cast<float>(w),
							static_cast<float>(h)
						);
					}
					if (pos == std::string::npos) break;
					yOffset += lineHeight;
					start = pos + 1;
				}
			}
		}
	}

	// Restore defaultShader after text drawing
	graphics.rebindDefaultShader();
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
		// Play right rune hover sound when first hovering
		engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-1" });
		spawnRunes(mouseWorldPos);
		runeState.runesActive = true;
		runeState.isLeftClickHeld = true;
	}

	// UPDATE RUNES while left click is held
	if (leftClickHeld && runeState.runesActive) {
		updateRuneHover(mouseWorldPos);
	}

	// DESTROY RUNES when left click is released
	if (leftClickReleased && runeState.runesActive) {
		destroyRunes();
		runeState.runesActive = false;
		runeState.isLeftClickHeld = false;
		runeState.isDrawingLine = false;
	}

	// Draw connection line if active
	if (runeState.isDrawingLine) {
		drawRuneConnectionLine();
	}

	// Update fireballs - check lifetime and destroy expired ones
	float deltaTime = static_cast<float>(engine_.getDeltaTime());
	std::vector<uint32_t> fireballsToDestroy;

	//Rework: Entity loop might need rework. Also destroying fireball is in update_runesystem.
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

	auto& resourceLoader = engine_.getResourceLoader();

	// Load the rune spritesheet
	auto runeTexture = resourceLoader.getTexture("Runes_White_Sheet_Large");

	constexpr float PI = 3.14159265359f;
	
	// Rune numbers at each position (0=TL, 1=TR, 2=MR, 3=BR, 4=BL, 5=ML)
	const int runeNumbers[6] = { 6, 1, 2, 3, 4, 5 };

	// Store circle center for spell spawning
	runeState.circleCenter = mouseWorldPos;

	// Spawn 6 runes in a circle pattern
	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i) 
	{
		// Calculate angles
		float angle = (-120.0f + i * 60.0f) * (PI / 180.0f);
		float x = mouseWorldPos.x + runeState.runeCircleRadius * std::cos(angle);
		float y = mouseWorldPos.y + runeState.runeCircleRadius * std::sin(angle);

		// Get rune number for this position
		int runeNumber = runeNumbers[i];

		// Create rune entity
		ECS::Entity rune = entityManager.create();

		std::string runeName = "Rune_" + std::to_string(runeNumber);
		rune.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent(runeName.c_str()));
		rune.add<ECSComponents::TransformComponent>(entityManager);
		rune.add<ECSComponents::DrawInfoComponent>(entityManager);
		rune.add<ECSComponents::RuneComponent>(entityManager, ECSComponents::RuneComponent(i, runeNumber, 30.0f));

		auto& transform = rune.get<ECSComponents::TransformComponent>(entityManager);
		transform.pos = { x, y };
		transform.scale = { 1.0f, 1.0f };

		auto& draw = rune.get<ECSComponents::DrawInfoComponent>(entityManager);
		draw.width = 56.0f;		// Spritesheet frame size
		draw.height = 56.0f;
		draw.color = Graphics::Color::White;
		strcpy_s(draw.shape, "Rectangle");
		draw.texture = runeTexture;

		// Store entity ID
		runeState.runeEntityIDs[i] = rune.id();

	}

	// Reset connection state
	runeState.resetConnection();

	entitiesDirty_ = true;
}

void GameApplication::destroyRunes()
{
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Destory all 6 runes
	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i) {
		if (runeState.runeEntityIDs[i] != UINT32_MAX)
		{
			ECS::Entity runeEntity(runeState.runeEntityIDs[i]);
			if (runeEntity.has<ECSComponents::RuneComponent>(entityManager))
			{
				entityManager.destroyEntity(runeState.runeEntityIDs[i]);
			}
			runeState.resetConnection();
			entitiesDirty_ = true;
		}
	}

	runeState.resetConnection();
	// Mark entities as dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[RuneSystem] Destroyed runes" << std::endl;
}

void GameApplication::updateRuneHover(const Math::eVec2<float>& mouseWorldPos)
{
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Check hover for all runes
	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i)
	{
		if (runeState.runeEntityIDs[i] == UINT32_MAX) continue;

		ECS::Entity runeEntity(runeState.runeEntityIDs[i]);
		if (!runeEntity.has<ECSComponents::TransformComponent>(entityManager) ||
			!runeEntity.has<ECSComponents::RuneComponent>(entityManager))
			continue;

		auto& transform = runeEntity.get<ECSComponents::TransformComponent>(entityManager);
		auto& rune = runeEntity.get<ECSComponents::RuneComponent>(entityManager);

		float dist = Math::eVec2Distance(mouseWorldPos, transform.pos);
		bool wasHovered = rune.isHovered;
		rune.isHovered = (dist <= rune.radius);

		// Rune hover - add to sequence if not already connected and not banned
		if (rune.isHovered && !wasHovered && !rune.isConnected && !rune.isBanned && !runeState.spellCasted)
		{
			// Try to add this rune to the sequence
			if (runeState.addRuneToSequence(rune.runeNumber, transform.pos))
			{
				rune.isConnected = true;
				runeState.isDrawingLine = true;

				// Play sound
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-1" });
				std::cout << "[RuneSystem] Added rune " << rune.runeNumber
				          << " to sequence (count: " << runeState.connectionCount << ")" << std::endl;

				// Check if we've connected 4 runes
				if (runeState.connectionCount == ECSComponents::RuneSystemStateComponent::SPELL_SEQUENCE_LENGTH)
				{
					// Check spell pattern
					ECSComponents::SpellType spellType = runeState.checkSpellPattern();

					if (spellType != ECSComponents::SpellType::NONE)
					{
						// Valid spell!
						std::cout << "[RuneSystem] Valid spell cast: ";
						switch (spellType) {
							case ECSComponents::SpellType::ICE:
								std::cout << "ICE (3-2-5-4)" << std::endl;
								spawnRuptureMark(runeState.circleCenter, spellType);
								break;
							case ECSComponents::SpellType::FIRE:
								std::cout << "FIRE (6-1-5-2)" << std::endl;
								spawnRuptureMark(runeState.circleCenter, spellType);
								break;
							case ECSComponents::SpellType::LIGHT:
								std::cout << "LIGHT (4-1-3-6)" << std::endl;
								spawnRuptureMark(runeState.circleCenter, spellType);
								break;
							default:
								break;
						}
						engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-2" });
					}
					else
					{
						// Invalid sequence
						std::cout << "[RuneSystem] Invalid spell sequence: "
						          << runeState.connectedRuneSequence[0] << "-"
						          << runeState.connectedRuneSequence[1] << "-"
						          << runeState.connectedRuneSequence[2] << "-"
						          << runeState.connectedRuneSequence[3] << std::endl;
					}

					runeState.spellCasted = true;

					// Destroy runes after spell attempt
					destroyRunes();
					runeState.runesActive = false;
					runeState.isDrawingLine = false;
					return;
				}
			}
		}
	}

	// Update line end position while drawing
	if (runeState.isDrawingLine && !runeState.spellCasted)
	{
		runeState.lineEnd = mouseWorldPos;
	}
}

void GameApplication::drawRuneConnectionLine()
{
	auto& entityManager = engine_.getEntityManager();
	auto& graphics = engine_.getGraphicsManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	if (runeState.connectionCount == 0) return;

	// Draw lines between all connected runes
	for (int i = 0; i < runeState.connectionCount; ++i)
	{
		Math::eVec2<float> startPos, endPos;

		// Get start position (current connected rune)
		startPos = {
			runeState.connectedPositions[i].x - editorCameraX_,
			runeState.connectedPositions[i].y - editorCameraY_
		};

		// Get end position (next connected rune, or mouse if this is the last one)
		if (i < runeState.connectionCount - 1)
		{
			endPos = {
				runeState.connectedPositions[i + 1].x - editorCameraX_,
				runeState.connectedPositions[i + 1].y - editorCameraY_
			};
		}
		else
		{
			// Last connected rune - draw to mouse position
			endPos = {
				runeState.lineEnd.x - editorCameraX_,
				runeState.lineEnd.y - editorCameraY_
			};
		}

		// Draw line segment
		Math::eVec2<float> direction = {
			endPos.x - startPos.x,
			endPos.y - startPos.y
		};
		float length = Math::eVec2Length(direction);

		if (length > 0.1f) {
			float angle = std::atan2(direction.y, direction.x);
			Math::eVec2<float> midpoint = {
				startPos.x + direction.x / 2.0f,
				startPos.y + direction.y / 2.0f
			};

			graphics.drawRect(
				midpoint,
				Math::eVec2<float>{length, 3.0f},
				Graphics::Color(1.0f, 1.0f, 0.0f, 0.8f),  // Yellow
				true,
				angle
			);
		}
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

	// Play fireball spawn sound effect
	engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Fireball-Cast" });

	std::cout << "[RuneSystem] Spawned fireball at position (" << pos.x << ", " << pos.y << ")" << std::endl;
}

void GameApplication::spawnRuptureMark(const Math::eVec2<float>& pos, ECSComponents::SpellType spellType)
{
	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Determine spritesheets based on spell type
	const char* spawnSheetName = "";
	const char* windupSheetName = "";
	const char* explosionSheetName = "";

	switch (spellType)
	{
	case ECSComponents::SpellType::ICE:
		spawnSheetName = "RuptureMark-Ice-Spawn-Sheet";
		windupSheetName = "RuptureMark-Ice-Windup-Sheet";
		explosionSheetName = "RuptureMark-Ice-Explosion-Sheet";
		break;
	case ECSComponents::SpellType::FIRE:
		spawnSheetName = "RuptureMark-Fire-Spawn-Sheet";
		windupSheetName = "RuptureMark-Fire-Windup-Sheet";
		explosionSheetName = "RuptureMark-Fire-Explosion-Sheet";
		break;
	case ECSComponents::SpellType::LIGHT:
		spawnSheetName = "RuptureMark-Light-Spawn-Sheet";
		windupSheetName = "RuptureMark-Light-Windup-Sheet";
		explosionSheetName = "RuptureMark-Light-Explosion-Sheet";
		break;
	default:
		return;
	}

	auto texture = resourceLoader.getTexture(spawnSheetName);

	// Create rupture mark entity
	ECS::Entity ruptureMark = entityManager.create();
	ruptureMark.add<ECSComponents::NameComponent>(entityManager,ECSComponents::NameComponent("RuptureMark"));
	ruptureMark.add<ECSComponents::TransformComponent>(entityManager);
	ruptureMark.add<ECSComponents::DrawInfoComponent>(entityManager);
	ruptureMark.add<ECSComponents::AnimationControllerComponent>(entityManager);
	ruptureMark.add<ECSComponents::RuptureMarkComponent>(entityManager, ECSComponents::RuptureMarkComponent(spellType, 20.0f, 180.0f));


	auto& transform = ruptureMark.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = pos;
	transform.scale = { 1.0f, 1.0f };

	auto& draw = ruptureMark.get<ECSComponents::DrawInfoComponent>(entityManager);
	draw.width = 128.0f;
	draw.height = 128.0f;
	draw.color = Graphics::Color::White;
	strcpy_s(draw.shape, "Rectangle");
	draw.texture = texture;

	// Setup all 3 animations
	auto& animComp = ruptureMark.get<ECSComponents::AnimationControllerComponent>(entityManager);

	// Spawn animation (1024x128 sheet = 8 frames of 128x128)
	animComp.setAnimationData(
		"spawn",
		spawnSheetName,
		128, 128,  // frame size
		1, 8,      // 1 row, 8 columns
		0.1f,      // frame duration
		false      // not looping
	);

	// Windup animation (640x128 sheet = 5 frames of 128x128)
	animComp.setAnimationData(
		"windup",
		windupSheetName,
		128, 128,  // frame size
		1, 5,      // 1 row, 5 columns
		0.1f,      // frame duration
		false      // not looping
	);

	// Explosion animation (512x128 sheet = 4 frames of 128x128)
	animComp.setAnimationData(
		"explosion",
		explosionSheetName,
		128, 128,  // frame size
		1, 4,      // 1 row, 4 columns
		0.1f,      // frame duration
		false      // not looping
	);

	strcpy_s(animComp.currentAnimationName, "spawn");

	entitiesDirty_ = true;

	std::cout << "[RuneSystem] Spawned RuptureMark at (" << pos.x << ", " << pos.y << ")" << std::endl;
}

void GameApplication::updateRuptureMarks(float deltaTime)
{
	auto& entityManager = engine_.getEntityManager();
	auto& animRegistry = engine_.getAnimationRegistry();
	auto& resourceLoader = engine_.getResourceLoader();
	std::vector<uint32_t> marksToDestroy;
	std::vector<uint32_t> enemiesToDestroy;

	// Debug: Count how many RuptureMarks exist
	static float countTimer = 0.0f;
	countTimer += deltaTime;
	if (countTimer >= 2.0f) {
		countTimer = 0.0f;
		int ruptureCount = 0;
		for (const auto& [id, loc] : entityManager.getMap()) {
			ECS::Entity e(id);
			if (e.has<ECSComponents::RuptureMarkComponent>(entityManager))
				ruptureCount++;
		}
		if (ruptureCount > 0)
			std::cout << "[RuptureMark DEBUG] Total RuptureMarks in scene: " << ruptureCount << std::endl;
	}

	for (const auto& [id, loc] : entityManager.getMap())
	{
		ECS::Entity entity(id);
		if (!entity.has<ECSComponents::RuptureMarkComponent>(entityManager) || !entity.has<ECSComponents::TransformComponent>(entityManager))
			continue;

		auto& mark = entity.get<ECSComponents::RuptureMarkComponent>(entityManager);
		auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);

		mark.stateTimer += deltaTime;

		switch (mark.state)
		{
		case ECSComponents::RuptureMarkState::SPAWNING:
		{
			//std::cout << "[RuptureMark DEBUG] SPAWNING - timer: " << mark.stateTimer << "/" << mark.spawnDuration << std::endl;
			if (mark.stateTimer >= mark.spawnDuration)
			{
				mark.state = ECSComponents::RuptureMarkState::IDLE;
				mark.stateTimer = 0.0f;
				std::cout << "[RuptureMark] Entered IDLE state" << std::endl;
			}
			break;
		}

		case ECSComponents::RuptureMarkState::IDLE:
		{
			// Check for enemies in range
			const char* targetEnemy = mark.getTargetEnemyName();

			// Debug: Log once per second to avoid spam
			static float debugTimer = 0.0f;
			debugTimer += deltaTime;
			bool shouldLog = debugTimer >= 1.0f;
			if (shouldLog) {
				debugTimer = 0.0f;
				std::cout << "[RuptureMark DEBUG] Looking for target: '" << targetEnemy << "'" << std::endl;
				std::cout << "[RuptureMark DEBUG] Mark position: (" << transform.pos.x << ", " << transform.pos.y << "), radius: " << mark.detectionRadius << std::endl;
			}

			for (const auto& [enemyId, enemyLoc] : entityManager.getMap())
			{
				ECS::Entity enemyEntity(enemyId);
				if (!enemyEntity.has<ECSComponents::AIComponent>(entityManager) ||
					!enemyEntity.has<ECSComponents::TransformComponent>(entityManager))
					continue;

				// Debug: Found an AI entity
				const char* debugName = "???";
				const char* debugTexture = "???";
				if (enemyEntity.has<ECSComponents::NameComponent>(entityManager))
					debugName = enemyEntity.get<ECSComponents::NameComponent>(entityManager).name;
				if (enemyEntity.has<ECSComponents::DrawInfoComponent>(entityManager))
					debugTexture = enemyEntity.get<ECSComponents::DrawInfoComponent>(entityManager).texturePath;

				// Check if this is the target enemy type by name OR texture path
				bool isTargetEnemy = false;

				// Check entity name
				if (enemyEntity.has<ECSComponents::NameComponent>(entityManager))
				{
					auto& enemyName = enemyEntity.get<ECSComponents::NameComponent>(entityManager);
					if (strstr(enemyName.name, targetEnemy) != nullptr)
						isTargetEnemy = true;
				}

				// Also check texture path (e.g., "Enemy-Slug-Idle-Sheet" contains "Slug")
				if (!isTargetEnemy && enemyEntity.has<ECSComponents::DrawInfoComponent>(entityManager))
				{
					auto& enemyDraw = enemyEntity.get<ECSComponents::DrawInfoComponent>(entityManager);
					if (strstr(enemyDraw.texturePath, targetEnemy) != nullptr)
						isTargetEnemy = true;
				}

				auto& enemyTransform = enemyEntity.get<ECSComponents::TransformComponent>(entityManager);
				float dist = Math::eVec2Distance(transform.pos, enemyTransform.pos);

				if (shouldLog) {
					std::cout << "[RuptureMark DEBUG] Found AI entity - Name: '" << debugName
							  << "', Texture: '" << debugTexture
							  << "', IsTarget: " << (isTargetEnemy ? "YES" : "NO")
							  << ", Distance: " << dist << std::endl;
				}

				if (!isTargetEnemy)
					continue;

				if (dist <= mark.detectionRadius)
				{
					// Enemy triggered the mark! Switch to windup animation
					mark.state = ECSComponents::RuptureMarkState::WINDUP;
					mark.stateTimer = 0.0f;
					mark.triggeredByEntityID = enemyId;

					// Switch to windup animation
					if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager) &&
						entity.has<ECSComponents::DrawInfoComponent>(entityManager))
					{
						auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
						auto& drawComp = entity.get<ECSComponents::DrawInfoComponent>(entityManager);

						strcpy_s(animComp.currentAnimationName, "windup");

						// Get the windup texture and update draw component
						for (int i = 0; i < animComp.animationCount; ++i)
						{
							if (strcmp(animComp.animations[i].name, "windup") == 0)
							{
								auto windupTex = resourceLoader.getTexture(animComp.animations[i].texturePath);
								if (windupTex)
								{
									drawComp.texture = windupTex;
									strncpy_s(drawComp.texturePath, animComp.animations[i].texturePath, sizeof(drawComp.texturePath) - 1);
								}

								// Reset the animation controller if exists
								auto* ctrl = animRegistry.get(animComp.controllerID);
								if (ctrl)
								{
									ctrl->setAnimation("windup");
								}
								break;
							}
						}
						entitiesDirty_ = true;
					}

					std::cout << "[RuptureMark] Triggered by enemy (ID: " << enemyId << ")! Entering WINDUP" << std::endl;
					break;
				}
			}
			break;
		}

		case ECSComponents::RuptureMarkState::WINDUP:
		{
			if (mark.stateTimer >= mark.windupDuration)
			{
				mark.state = ECSComponents::RuptureMarkState::EXPLODING;
				mark.stateTimer = 0.0f;

				// Switch to explosion animation
				if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager) &&
					entity.has<ECSComponents::DrawInfoComponent>(entityManager))
				{
					auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
					auto& drawComp = entity.get<ECSComponents::DrawInfoComponent>(entityManager);

					strcpy_s(animComp.currentAnimationName, "explosion");

					// Get the explosion texture and update draw component
					for (int i = 0; i < animComp.animationCount; ++i)
					{
						if (strcmp(animComp.animations[i].name, "explosion") == 0)
						{
							auto explosionTex = resourceLoader.getTexture(animComp.animations[i].texturePath);
							if (explosionTex)
							{
								drawComp.texture = explosionTex;
								strncpy_s(drawComp.texturePath, animComp.animations[i].texturePath, sizeof(drawComp.texturePath) - 1);
							}

							// Reset the animation controller if exists
							auto* ctrl = animRegistry.get(animComp.controllerID);
							if (ctrl)
							{
								ctrl->setAnimation("explosion");
							}
							break;
						}
					}
					entitiesDirty_ = true;
				}

				// Deal damage to the triggering enemy
				if (mark.triggeredByEntityID != UINT32_MAX)
				{
					ECS::Entity enemyEntity(mark.triggeredByEntityID);

					// Use HealthComponent for proper damage handling
					if (enemyEntity.has<ECSComponents::HealthComponent>(entityManager))
					{
						auto& health = enemyEntity.get<ECSComponents::HealthComponent>(entityManager);
						bool died = health.takeDamage(mark.damage);
						std::cout << "[RuptureMark] EXPLODING! Dealt " << mark.damage << " damage. Enemy HP: " << health.currentHealth << "/" << health.maxHealth << std::endl;

						// Also update AIComponent health to stay in sync
						if (enemyEntity.has<ECSComponents::AIComponent>(entityManager))
						{
							auto& ai = enemyEntity.get<ECSComponents::AIComponent>(entityManager);
							ai.health = static_cast<int>(health.currentHealth);
						}

						// Trigger state change through StateMachineSystem
						if (!died)
						{
							engine_.getStateMachineSys().requestStateChange(mark.triggeredByEntityID, "Hurt");
						}
						else
						{
							engine_.getStateMachineSys().requestStateChange(mark.triggeredByEntityID, "Death");
							enemiesToDestroy.push_back(mark.triggeredByEntityID);
						}
					}
					// Fallback to AIComponent if no HealthComponent
					else if (enemyEntity.has<ECSComponents::AIComponent>(entityManager))
					{
						auto& ai = enemyEntity.get<ECSComponents::AIComponent>(entityManager);
						ai.health -= static_cast<int>(mark.damage);
						std::cout << "[RuptureMark] EXPLODING! Dealt " << mark.damage << " damage. Enemy HP: " << ai.health << std::endl;
					}
				}
			}
			break;
		}

		case ECSComponents::RuptureMarkState::EXPLODING:
		{
			if (mark.stateTimer >= mark.explosionDuration)
			{
				mark.state = ECSComponents::RuptureMarkState::FINISHED;
				std::cout << "[RuptureMark] Finished, marking for destruction" << std::endl;
			}
			break;
		}

		case ECSComponents::RuptureMarkState::FINISHED:
		{
			marksToDestroy.push_back(id);
			break;
		}
		}
	}

	// Destroy finished marks
	for (uint32_t id : marksToDestroy)
	{
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
	}

	// Destroy dead enemies
	for (uint32_t id : enemiesToDestroy)
	{
		std::cout << "[RuptureMark] Destroying dead enemy (ID: " << id << ")" << std::endl;
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
	}
}

void GameApplication::spawnFloatingText(const Math::eVec2<float>& pos, const char* text)
{
	auto& entityManager = engine_.getEntityManager();

	// Create floating text entity
	ECS::Entity floatingText = entityManager.create();
	floatingText.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("FloatingText"));
	floatingText.add<ECSComponents::TransformComponent>(entityManager);
	floatingText.add<ECSComponents::FloatingTextComponent>(entityManager, ECSComponents::FloatingTextComponent(text, 1.5f, 50.0f));

	auto& transform = floatingText.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = pos;
	transform.scale = { 1.0f, 1.0f };

	entitiesDirty_ = true;

	std::cout << "[RuneSystem] Spawned floating text: " << text << std::endl;
}

void GameApplication::updateFloatingTexts(float deltaTime)
{
	auto& entityManager = engine_.getEntityManager();
	auto& graphics = engine_.getGraphicsManager();
	std::vector<uint32_t> textsToDestroy;

	for (const auto& [id, loc] : entityManager.getMap()) {
		ECS::Entity entity(id);
		if (entity.has<ECSComponents::FloatingTextComponent>(entityManager) &&
		    entity.has<ECSComponents::TransformComponent>(entityManager)) {

			auto& floatText = entity.get<ECSComponents::FloatingTextComponent>(entityManager);
			auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);

			// Update elapsed time
			floatText.elapsedTime += deltaTime;

			// Move text upward
			transform.pos.y -= floatText.riseSpeed * deltaTime;

			// Check if expired
			if (floatText.elapsedTime >= floatText.lifetime) {
				textsToDestroy.push_back(id);
			}
			else {
				// Render the floating text
				float alpha = floatText.getCurrentAlpha();

				// Convert world position to screen position
				Math::eVec2<float> screenPos = {
					transform.pos.x - editorCameraX_,
					transform.pos.y - editorCameraY_
				};

				// Draw text with fading alpha
				graphics.drawText(
					pixelifyFont_,
					std::string(floatText.text),
					screenPos,
					1.0f,  // scale
					Graphics::Color(1.0f, 0.3f, 0.3f, alpha)  // Red color with fading alpha
				);
			}
		}
	}

	// Destroy expired floating texts
	for (uint32_t id : textsToDestroy) {
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
	}
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

				// Play fireball hit sound effect
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Fireball-Explosion-Small" });

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
	// Setup pause menu buttons (Resume and Main Menu)
	setupPauseMenuButtons();
	entitiesDirty_ = true;

	std::cout << "[PauseMenu] Pause menu UI spawned at (1024, 1024)" << std::endl;
}

void GameApplication::destroyPauseMenu() {
	auto& entityManager = engine_.getEntityManager();
	auto& entityMap = entityManager.getMap();

	// Destroy pause menu entity (check if it exists first)
	if (pauseMenuEntityID_ != 0) {
		if (entityMap.find(pauseMenuEntityID_) != entityMap.end()) {
			entityManager.destroyEntity(pauseMenuEntityID_);
		}
		pauseMenuEntityID_ = 0;
	}

	// Destroy Resume button (check if it exists first)
	if (resumeButtonEntityID_ != 0) {
		if (entityMap.find(resumeButtonEntityID_) != entityMap.end()) {
			entityManager.destroyEntity(resumeButtonEntityID_);
		}
		resumeButtonEntityID_ = 0;
	}

	// Destroy Main Menu button (check if it exists first)
	if (mainMenuButtonEntityID_ != 0) {
		if (entityMap.find(mainMenuButtonEntityID_) != entityMap.end()) {
			entityManager.destroyEntity(mainMenuButtonEntityID_);
		}
		mainMenuButtonEntityID_ = 0;
	}

	entitiesDirty_ = true;
	std::cout << "[PauseMenu] Pause menu UI and buttons destroyed" << std::endl;
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
// ============================================================================
// MAIN MENU BUTTON SYSTEM IMPLEMENTATION (SAME AS RUNEWEAVERS)
// ============================================================================

void GameApplication::setupMainMenuButtons() {
	auto& entityManager = engine_.getEntityManager();

	// Play Button at (613, 1310) with size (94x94) rotated 45 degrees
	ECS::Entity playButton = entityManager.create();
	playButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("PlayButton"));
	playButton.add<ECSComponents::TransformComponent>(entityManager);
	playButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	playButton.add<ECSComponents::CollisionComponent>(entityManager);
	playButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& playTransform = playButton.get<ECSComponents::TransformComponent>(entityManager);
	playTransform.pos = { 613.0f, 1310.0f };
	playTransform.scale = { 1.0f, 1.0f };
	playTransform.rot.x = 0.785398f; // 45 degrees in radians

	auto& playDraw = playButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	playDraw.width = 94.0f;
	playDraw.height = 94.0f;
	playDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(playDraw.shape, "Rectangle");

	auto& playCollision = playButton.get<ECSComponents::CollisionComponent>(entityManager);
	playCollision.type = ECSComponents::ColliderType::RECTANGLE;
	playCollision.data.rectangle.width = 94.0f;
	playCollision.data.rectangle.height = 94.0f;
	playCollision.rotation = 0.785398f; // 45 degrees in radians
	playCollision.isTrigger = true;

	auto& playBtn = playButton.get<ECSComponents::ButtonComponent>(entityManager);
	playBtn.setTargetScene("Scene1.txt");
	playBtn.getActive() = true;

	// Controls Button at (363, 1263) with size (79x79) rotated 45 degrees
	ECS::Entity controlsButton = entityManager.create();
	controlsButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("ControlsButton"));
	controlsButton.add<ECSComponents::TransformComponent>(entityManager);
	controlsButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	controlsButton.add<ECSComponents::CollisionComponent>(entityManager);
	controlsButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& controlsTransform = controlsButton.get<ECSComponents::TransformComponent>(entityManager);
	controlsTransform.pos = { 363.0f, 1263.0f };
	controlsTransform.scale = { 1.0f, 1.0f };
	controlsTransform.rot.x = 0.785398f; // 45 degrees in radians

	auto& controlsDraw = controlsButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	controlsDraw.width = 79.0f;
	controlsDraw.height = 79.0f;
	controlsDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(controlsDraw.shape, "Rectangle");

	auto& controlsCollision = controlsButton.get<ECSComponents::CollisionComponent>(entityManager);
	controlsCollision.type = ECSComponents::ColliderType::RECTANGLE;
	controlsCollision.data.rectangle.width = 79.0f;
	controlsCollision.data.rectangle.height = 79.0f;
	controlsCollision.rotation = 0.785398f; // 45 degrees in radians
	controlsCollision.isTrigger = true;

	auto& controlsBtn = controlsButton.get<ECSComponents::ButtonComponent>(entityManager);
	controlsBtn.getActive() = true;

	// Quit Button at (863, 1263) with size (79x79) rotated 45 degrees
	ECS::Entity quitButton = entityManager.create();
	quitButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("QuitButton"));
	quitButton.add<ECSComponents::TransformComponent>(entityManager);
	quitButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	quitButton.add<ECSComponents::CollisionComponent>(entityManager);
	quitButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& quitTransform = quitButton.get<ECSComponents::TransformComponent>(entityManager);
	quitTransform.pos = { 863.0f, 1263.0f };
	quitTransform.scale = { 1.0f, 1.0f };
	quitTransform.rot.x = 0.785398f; // 45 degrees in radians

	auto& quitDraw = quitButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	quitDraw.width = 79.0f;
	quitDraw.height = 79.0f;
	quitDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(quitDraw.shape, "Rectangle");

	auto& quitCollision = quitButton.get<ECSComponents::CollisionComponent>(entityManager);
	quitCollision.type = ECSComponents::ColliderType::RECTANGLE;
	quitCollision.data.rectangle.width = 79.0f;
	quitCollision.data.rectangle.height = 79.0f;
	quitCollision.rotation = 0.785398f; // 45 degrees in radians
	quitCollision.isTrigger = true;

	auto& quitBtn = quitButton.get<ECSComponents::ButtonComponent>(entityManager);
	quitBtn.getActive() = true;

	// Mark entities dirty to refresh rendering cache
	entitiesDirty_ = true;

	std::cout << "[MainMenu] Main menu buttons created" << std::endl;
}

void GameApplication::updateMainMenuButtons() {
	auto& entityManager = engine_.getEntityManager();
	auto& inputMng = engine_.getInputManager();

	// Get current scene name to check if we're in main menu
	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
	std::string sceneName = currentScene.getSceneName();

	// Only process main menu buttons if we're in MainMenu scene
	bool inMainMenu = (sceneName == "MainMenu.txt");

	// For Editor, we need to check if mouse is in game viewport
	// Get game framebuffer render info from ImGui
	extern ImVec2 lastGameViewportPos;
	extern ImVec2 lastGameViewportSize;

	// Get mouse position
	auto mouseScreenPos = inputMng.getMousePosition();

	// Check if mouse is within game viewport bounds
	bool mouseInGameView = (mouseScreenPos.first >= lastGameViewportPos.x &&
	                        mouseScreenPos.first <= (lastGameViewportPos.x + lastGameViewportSize.x) &&
	                        mouseScreenPos.second >= lastGameViewportPos.y &&
	                        mouseScreenPos.second <= (lastGameViewportPos.y + lastGameViewportSize.y));

	if (!mouseInGameView) return; // Don't process buttons if mouse is outside game view

	// Convert screen space mouse to viewport-relative coordinates
	float viewportMouseX = static_cast<float>(mouseScreenPos.first) - lastGameViewportPos.x;
	float viewportMouseY = static_cast<float>(mouseScreenPos.second) - lastGameViewportPos.y;

	// Normalize to 0-1 range based on viewport size
	float normalizedX = viewportMouseX / lastGameViewportSize.x;
	float normalizedY = viewportMouseY / lastGameViewportSize.y;

	// Convert to world space (assuming 1920x1080 game resolution)
	Math::eVec2<float> mouseWorldPos = {
		normalizedX * worldWidth_,
		normalizedY * worldHeight_
	};

	// Check for mouse click
	bool mouseClicked = inputMng.isMouseDown(GLFW_MOUSE_BUTTON_LEFT);

	if (!mouseClicked) return;

	// Find all button entities
	auto buttonEntities = entityManager.findEntity<ECSComponents::ButtonComponent>();

	for (auto& button : buttonEntities) {
		if (!button.has<ECSComponents::NameComponent>(entityManager) ||
			!button.has<ECSComponents::TransformComponent>(entityManager) ||
			!button.has<ECSComponents::DrawInfoComponent>(entityManager) ||
			!button.has<ECSComponents::CollisionComponent>(entityManager)) {
			continue;
		}

		auto& name = button.get<ECSComponents::NameComponent>(entityManager);
		auto& transform = button.get<ECSComponents::TransformComponent>(entityManager);
		auto& draw = button.get<ECSComponents::DrawInfoComponent>(entityManager);
		auto& collision = button.get<ECSComponents::CollisionComponent>(entityManager);
		auto& btnComp = button.get<ECSComponents::ButtonComponent>(entityManager);

		std::string buttonName(name.name);

		// Check if click is within button bounds
		if (btnComp.onClick(mouseWorldPos, transform, draw, collision)) {
			std::cout << "[MainMenu] Button clicked: " << buttonName << std::endl;

			// Handle different buttons based on name
			if (inMainMenu) {
				if (buttonName == "PlayButton") {
					handlePlayButton();
					return; // Exit after handling to prevent multiple clicks
				} else if (buttonName == "ControlsButton") {
					handleControlsButton();
					return;
				} else if (buttonName == "QuitButton") {
					handleQuitButton();
					return;
				}
			}

			// Handle controls close button (only when controls display is active)
			if (controlsDisplayEntityID_ != 0 && buttonName == "ControlsCloseButton") {
				handleControlsCloseButton();
				return;
			}

			// Handle quit confirm buttons (only when quit confirm is active)
			if (quitConfirmEntityID_ != 0) {
				if (buttonName == "QuitNoButton") {
					handleQuitNoButton();
				} else if (buttonName == "QuitYesButton") {
					handleQuitYesButton();
					return;
				}
			}
			// Handle pause menu buttons (only when pause menu is active)
			if (pauseMenuEntityID_ != 0) {
				if (buttonName == "ResumeButton") {
					handleResumeButton();
					return;
				} else if (buttonName == "MainMenuButton") {
					handleMainMenuButton();
					return;
				}
			}
		}
	}
}

void GameApplication::handlePlayButton() {
	std::cout << "[MainMenu] Play button pressed - Loading Scene1" << std::endl;
	engine_.getEnoEditor().selectScene("Scene1.txt");
	entitiesDirty_ = true;

	// Re-scan for player entity in the new scene
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();

	if (!playerEntities.empty()) {
		std::cout << "[MainMenu] Player entity found in Scene1 (ID: " << playerEntities[0].id() << ")" << std::endl;

		// Re-add health component to player if needed
		ECS::Entity& playerEntity = playerEntities[0];
		if (!playerEntity.has<ECSComponents::HealthComponent>(entityManager)) {
			playerEntity.add<ECSComponents::HealthComponent>(entityManager, 100.0f);
			std::cout << "[MainMenu] Added HealthComponent to player (100 HP)" << std::endl;
		}
	} else {
		std::cout << "[MainMenu] Warning: No player entity found in Scene1!" << std::endl;
	}

	// Re-setup combat system for enemies in new scene
	std::vector<ECS::Entity> enemies = entityManager.findEntity<ECSComponents::AIComponent>();
	for (auto& enemy : enemies) {
		if (!enemy.has<ECSComponents::HealthComponent>(entityManager)) {
			enemy.add<ECSComponents::HealthComponent>(entityManager, 50.0f);
		} else {
			auto& health = enemy.get<ECSComponents::HealthComponent>(entityManager);
			health.currentHealth = 50.0f;
			health.maxHealth = 50.0f;
			health.isDead = false;
		}

		if (!enemy.has<ECSComponents::DamageComponent>(entityManager)) {
			enemy.add<ECSComponents::DamageComponent>(entityManager, 10.0f);
		}
	}
}

void GameApplication::handleControlsButton() {
	std::cout << "[MainMenu] Controls button pressed - Spawning controls display" << std::endl;

	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Load controls texture
	auto controlsTexture = resourceLoader.getTexture("MainMenu-ControlsDisplayV2");

	if (!controlsTexture || !controlsTexture->isValid()) {
		std::cerr << "[MainMenu] Failed to load controls texture!" << std::endl;
		return;
	}

	// Create controls display entity
	ECS::Entity controlsDisplay = entityManager.create();
	controlsDisplayEntityID_ = controlsDisplay.id();

	controlsDisplay.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("ControlsDisplay"));
	controlsDisplay.add<ECSComponents::TransformComponent>(entityManager);
	controlsDisplay.add<ECSComponents::DrawInfoComponent>(entityManager);

	auto& transform = controlsDisplay.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = { 1024.0f, 1024.0f };
	transform.scale = { 1.0f, 1.0f };

	auto& draw = controlsDisplay.get<ECSComponents::DrawInfoComponent>(entityManager);
	draw.width = 1920.0f;
	draw.height = 1080.0f;
	draw.color = Graphics::Color::White;
	strcpy_s(draw.shape, "Rectangle");
	draw.texture = controlsTexture;

	// Create close button at (313, 591) with size (72x72) rotated 45 degrees
	ECS::Entity closeButton = entityManager.create();
	closeButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("ControlsCloseButton"));
	closeButton.add<ECSComponents::TransformComponent>(entityManager);
	closeButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	closeButton.add<ECSComponents::CollisionComponent>(entityManager);
	closeButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& closeTransform = closeButton.get<ECSComponents::TransformComponent>(entityManager);
	closeTransform.pos = { 313.0f, 591.0f };
	closeTransform.scale = { 1.0f, 1.0f };
	closeTransform.rot.x = 0.785398f; // 45 degrees in radians

	auto& closeDraw = closeButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	closeDraw.width = 72.0f;
	closeDraw.height = 72.0f;
	closeDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(closeDraw.shape, "Rectangle");

	auto& closeCollision = closeButton.get<ECSComponents::CollisionComponent>(entityManager);
	closeCollision.type = ECSComponents::ColliderType::RECTANGLE;
	closeCollision.data.rectangle.width = 72.0f;
	closeCollision.data.rectangle.height = 72.0f;
	closeCollision.rotation = 0.785398f; // 45 degrees in radians
	closeCollision.isTrigger = true;

	auto& closeBtn = closeButton.get<ECSComponents::ButtonComponent>(entityManager);
	closeBtn.getActive() = true;

	entitiesDirty_ = true;
}

void GameApplication::handleControlsCloseButton() {
	std::cout << "[MainMenu] Controls close button pressed - Destroying controls display" << std::endl;

	auto& entityManager = engine_.getEntityManager();

	// Destroy controls display entity
	if (controlsDisplayEntityID_ != 0) {
		entityManager.destroyEntity(controlsDisplayEntityID_);
		controlsDisplayEntityID_ = 0;
	}

	// Destroy close button (find by name)
	auto buttons = entityManager.findEntity<ECSComponents::ButtonComponent>();
	for (auto& button : buttons) {
		if (button.has<ECSComponents::NameComponent>(entityManager)) {
			auto& name = button.get<ECSComponents::NameComponent>(entityManager);
			if (std::string(name.name) == "ControlsCloseButton") {
				entityManager.destroyEntity(button.id());
				break;
			}
		}
	}

	entitiesDirty_ = true;
}

void GameApplication::handleQuitButton() {
	std::cout << "[MainMenu] Quit button pressed - Spawning quit confirm" << std::endl;

	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Load quit confirm texture
	auto quitTexture = resourceLoader.getTexture("MainMenu-QuitConfirm");

	if (!quitTexture || !quitTexture->isValid()) {
		std::cerr << "[MainMenu] Failed to load quit confirm texture!" << std::endl;
		return;
	}

	// Create quit confirm entity
	ECS::Entity quitConfirm = entityManager.create();
	quitConfirmEntityID_ = quitConfirm.id();

	quitConfirm.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("QuitConfirm"));
	quitConfirm.add<ECSComponents::TransformComponent>(entityManager);
	quitConfirm.add<ECSComponents::DrawInfoComponent>(entityManager);

	auto& transform = quitConfirm.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = { 1024.0f, 1024.0f };
	transform.scale = { 1.0f, 1.0f };

	auto& draw = quitConfirm.get<ECSComponents::DrawInfoComponent>(entityManager);
	draw.width = 1920.0f;
	draw.height = 1080.0f;
	draw.color = Graphics::Color::White;
	strcpy_s(draw.shape, "Rectangle");
	draw.texture = quitTexture;

	// Create No button at (869, 1126) with size (74x74)
	ECS::Entity noButton = entityManager.create();
	noButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("QuitNoButton"));
	noButton.add<ECSComponents::TransformComponent>(entityManager);
	noButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	noButton.add<ECSComponents::CollisionComponent>(entityManager);
	noButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& noTransform = noButton.get<ECSComponents::TransformComponent>(entityManager);
	noTransform.pos = { 869.0f, 1126.0f };
	noTransform.scale = { 1.0f, 1.0f };

	auto& noDraw = noButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	noDraw.width = 74.0f;
	noDraw.height = 74.0f;
	noDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(noDraw.shape, "Rectangle");

	auto& noCollision = noButton.get<ECSComponents::CollisionComponent>(entityManager);
	noCollision.type = ECSComponents::ColliderType::RECTANGLE;
	noCollision.data.rectangle.width = 74.0f;
	noCollision.data.rectangle.height = 74.0f;
	noCollision.isTrigger = true;

	auto& noBtn = noButton.get<ECSComponents::ButtonComponent>(entityManager);
	noBtn.getActive() = true;

	// Create Yes button at (1177, 1126) with size (74x74) rotated 35 degrees
	ECS::Entity yesButton = entityManager.create();
	yesButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("QuitYesButton"));
	yesButton.add<ECSComponents::TransformComponent>(entityManager);
	yesButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	yesButton.add<ECSComponents::CollisionComponent>(entityManager);
	yesButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& yesTransform = yesButton.get<ECSComponents::TransformComponent>(entityManager);
	yesTransform.pos = { 1177.0f, 1126.0f };
	yesTransform.scale = { 1.0f, 1.0f };
	yesTransform.rot.x = 0.610865f; // 35 degrees in radians

	auto& yesDraw = yesButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	yesDraw.width = 74.0f;
	yesDraw.height = 74.0f;
	yesDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(yesDraw.shape, "Rectangle");

	auto& yesCollision = yesButton.get<ECSComponents::CollisionComponent>(entityManager);
	yesCollision.type = ECSComponents::ColliderType::RECTANGLE;
	yesCollision.data.rectangle.width = 74.0f;
	yesCollision.data.rectangle.height = 74.0f;
	yesCollision.rotation = 0.610865f; // 35 degrees in radians
	yesCollision.isTrigger = true;

	auto& yesBtn = yesButton.get<ECSComponents::ButtonComponent>(entityManager);
	yesBtn.getActive() = true;

	entitiesDirty_ = true;
}

void GameApplication::handleQuitNoButton() {
	std::cout << "[MainMenu] Quit No button pressed - Closing quit confirm" << std::endl;

	auto& entityManager = engine_.getEntityManager();

	// Destroy quit confirm entity
	if (quitConfirmEntityID_ != 0) {
		entityManager.destroyEntity(quitConfirmEntityID_);
		quitConfirmEntityID_ = 0;
	}

	// Destroy No and Yes buttons (find by name)
	auto buttons = entityManager.findEntity<ECSComponents::ButtonComponent>();
	for (auto& button : buttons) {
		if (button.has<ECSComponents::NameComponent>(entityManager)) {
			auto& name = button.get<ECSComponents::NameComponent>(entityManager);
			std::string buttonName(name.name);
			if (buttonName == "QuitNoButton" || buttonName == "QuitYesButton") {
				entityManager.destroyEntity(button.id());
			}
		}
	}

	entitiesDirty_ = true;
}

void GameApplication::handleQuitYesButton() {
	std::cout << "[MainMenu] Quit Yes button pressed - Shutting down application" << std::endl;

	// Request window close to properly shut down the engine
	engine_.getWindowManager().setShouldClose(true);
}

// ============================================================================
// SCENE TRANSITION SYSTEM IMPLEMENTATION (EDITOR)
// ============================================================================

void GameApplication::setupSceneTransitions() {
	auto& entityManager = engine_.getEntityManager();

	// Create invisible trigger hitbox for Scene1 -> Scene2 transition
	ECS::Entity sceneTransitionTrigger = entityManager.create();
	scene1ToScene2TriggerID_ = sceneTransitionTrigger.id();

	sceneTransitionTrigger.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("Scene1ToScene2Trigger"));
	sceneTransitionTrigger.add<ECSComponents::TransformComponent>(entityManager);
	sceneTransitionTrigger.add<ECSComponents::CollisionComponent>(entityManager);

	auto& transform = sceneTransitionTrigger.get<ECSComponents::TransformComponent>(entityManager);
	transform.pos = { 1023.0f, 656.0f };
	transform.scale = { 1.0f, 1.0f };

	auto& collision = sceneTransitionTrigger.get<ECSComponents::CollisionComponent>(entityManager);
	collision.type = ECSComponents::ColliderType::RECTANGLE;
	collision.data.rectangle.width = 75.0f;
	collision.data.rectangle.height = 50.0f;
	collision.rotation = 0.0f;
	collision.isTrigger = true;
	collision.isActive = false;

	std::cout << "[SceneTransition] Scene transition trigger created" << std::endl;
}

void GameApplication::updateSceneTransitions() {
	auto& entityManager = engine_.getEntityManager();
	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
	std::string sceneName = currentScene.getSceneName();

	// Only check in play mode
	bool isPlayMode = (currentScene.getState() == Editor::State::Playing ||
	                   currentScene.getState() == Editor::State::NextFrame);

	if (!isPlayMode || sceneName != "Scene1.txt") {
		if (scene1ToScene2TriggerID_ != 0) {
			ECS::Entity trigger(scene1ToScene2TriggerID_);
			if (trigger.has<ECSComponents::CollisionComponent>(entityManager)) {
				auto& collision = trigger.get<ECSComponents::CollisionComponent>(entityManager);
				collision.isActive = false;
			}
		}
		return;
	}

	// Activate trigger in Scene1 play mode
	if (scene1ToScene2TriggerID_ != 0) {
		ECS::Entity trigger(scene1ToScene2TriggerID_);
		if (trigger.has<ECSComponents::CollisionComponent>(entityManager)) {
			auto& collision = trigger.get<ECSComponents::CollisionComponent>(entityManager);
			collision.isActive = true;
		}
	}

	// Check for player
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	if (playerEntities.empty()) return;

	ECS::Entity player = playerEntities[0];
	if (!player.has<ECSComponents::TransformComponent>(entityManager) ||
		!player.has<ECSComponents::CollisionComponent>(entityManager)) {
		return;
	}

	auto& playerTransform = player.get<ECSComponents::TransformComponent>(entityManager);
	auto& playerCollision = player.get<ECSComponents::CollisionComponent>(entityManager);

	ECS::Entity trigger(scene1ToScene2TriggerID_);
	if (!trigger.has<ECSComponents::TransformComponent>(entityManager) ||
		!trigger.has<ECSComponents::CollisionComponent>(entityManager)) {
		return;
	}

	auto& triggerTransform = trigger.get<ECSComponents::TransformComponent>(entityManager);
	auto& triggerCollision = trigger.get<ECSComponents::CollisionComponent>(entityManager);

	// AABB collision check
	float playerLeft = playerTransform.pos.x - (playerCollision.data.rectangle.width * 0.5f);
	float playerRight = playerTransform.pos.x + (playerCollision.data.rectangle.width * 0.5f);
	float playerTop = playerTransform.pos.y - (playerCollision.data.rectangle.height * 0.5f);
	float playerBottom = playerTransform.pos.y + (playerCollision.data.rectangle.height * 0.5f);

	float triggerLeft = triggerTransform.pos.x - (triggerCollision.data.rectangle.width * 0.5f);
	float triggerRight = triggerTransform.pos.x + (triggerCollision.data.rectangle.width * 0.5f);
	float triggerTop = triggerTransform.pos.y - (triggerCollision.data.rectangle.height * 0.5f);
	float triggerBottom = triggerTransform.pos.y + (triggerCollision.data.rectangle.height * 0.5f);

	bool colliding = !(playerRight < triggerLeft || playerLeft > triggerRight ||
	                   playerBottom < triggerTop || playerTop > triggerBottom);

	if (colliding) {
		std::cout << "[SceneTransition] Player triggered Scene1->Scene2 transition!" << std::endl;
		engine_.getEnoEditor().selectScene("Scene2.txt");
		entitiesDirty_ = true;
	}
}

// ============================================================================
// PAUSE MENU BUTTON SYSTEM IMPLEMENTATION (EDITOR)
// ============================================================================

void GameApplication::setupPauseMenuButtons() {
	auto& entityManager = engine_.getEntityManager();

	// Resume Button
	ECS::Entity resumeButton = entityManager.create();
	resumeButtonEntityID_ = resumeButton.id();

	resumeButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("ResumeButton"));
	resumeButton.add<ECSComponents::TransformComponent>(entityManager);
	resumeButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	resumeButton.add<ECSComponents::CollisionComponent>(entityManager);
	resumeButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& resumeTransform = resumeButton.get<ECSComponents::TransformComponent>(entityManager);
	resumeTransform.pos = { 716.0f, 965.0f };
	resumeTransform.scale = { 1.0f, 1.0f };
	resumeTransform.rot.x = 0.785398f;

	auto& resumeDraw = resumeButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	resumeDraw.width = 100.0f;
	resumeDraw.height = 100.0f;
	resumeDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f);
	strcpy_s(resumeDraw.shape, "Rectangle");

	auto& resumeCollision = resumeButton.get<ECSComponents::CollisionComponent>(entityManager);
	resumeCollision.type = ECSComponents::ColliderType::RECTANGLE;
	resumeCollision.data.rectangle.width = 100.0f;
	resumeCollision.data.rectangle.height = 100.0f;
	resumeCollision.rotation = 0.785398f;
	resumeCollision.isTrigger = true;

	auto& resumeBtn = resumeButton.get<ECSComponents::ButtonComponent>(entityManager);
	resumeBtn.getActive() = true;

	// Main Menu Button
	ECS::Entity mainMenuButton = entityManager.create();
	mainMenuButtonEntityID_ = mainMenuButton.id();

	mainMenuButton.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("MainMenuButton"));
	mainMenuButton.add<ECSComponents::TransformComponent>(entityManager);
	mainMenuButton.add<ECSComponents::DrawInfoComponent>(entityManager);
	mainMenuButton.add<ECSComponents::CollisionComponent>(entityManager);
	mainMenuButton.add<ECSComponents::ButtonComponent>(entityManager);

	auto& menuTransform = mainMenuButton.get<ECSComponents::TransformComponent>(entityManager);
	menuTransform.pos = { 557.0f, 1141.0f };
	menuTransform.scale = { 1.0f, 1.0f };
	menuTransform.rot.x = 0.785398f;

	auto& menuDraw = mainMenuButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	menuDraw.width = 56.0f;
	menuDraw.height = 56.0f;
	menuDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f);
	strcpy_s(menuDraw.shape, "Rectangle");

	auto& menuCollision = mainMenuButton.get<ECSComponents::CollisionComponent>(entityManager);
	menuCollision.type = ECSComponents::ColliderType::RECTANGLE;
	menuCollision.data.rectangle.width = 56.0f;
	menuCollision.data.rectangle.height = 56.0f;
	menuCollision.rotation = 0.785398f;
	menuCollision.isTrigger = true;

	auto& menuBtn = mainMenuButton.get<ECSComponents::ButtonComponent>(entityManager);
	menuBtn.getActive() = true;

	std::cout << "[PauseMenu] Pause menu buttons created" << std::endl;
}

void GameApplication::handleResumeButton() {
	std::cout << "[PauseMenu] Resume button pressed" << std::endl;

	isPaused_ = false;
	destroyPauseMenu();
	showPlayer();
	engine_.setIsPlaying(true);
	engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
}

void GameApplication::handleMainMenuButton() {
	std::cout << "[PauseMenu] Main Menu button pressed" << std::endl;

	// IMPORTANT: Destroy pause menu and spellbook BEFORE changing scenes
	// to prevent them from being saved into the scene file
	destroyPauseMenu();
	if (spellbookEntityID_ != 0) {
		destroySpellbook();
	}
	showPlayer();  // Restore player visibility before scene change

	// Clear all entity IDs to prevent crashes when trying to access them after scene change
	resumeButtonEntityID_ = 0;
	mainMenuButtonEntityID_ = 0;
	pauseMenuEntityID_ = 0;
	spellbookEntityID_ = 0;
	controlsDisplayEntityID_ = 0;
	quitConfirmEntityID_ = 0;
	playerEntityID_ = 0;

	// Reset game state flags
	isPaused_ = false;
	showSpellbook_ = false;
	playerSpawned_ = false;
	engine_.setIsPlaying(true);

	// Change scene (this will destroy all entities including pause menu and buttons)
	engine_.getEnoEditor().selectScene("MainMenu.txt");
	entitiesDirty_ = true;

	// Recreate main menu buttons after scene load (they're not saved in the scene file)
	setupMainMenuButtons();

	std::cout << "[PauseMenu] Returned to MainMenu" << std::endl;
}

void GameApplication::testParticleSystem() {
	auto& entityManager = engine_.getEntityManager();
	auto& input = engine_.getInputManager();

	if (input.isKeyPressed(GLFW_KEY_P)) {
		auto mousePos = input.getMousePosition();
		float worldX = static_cast<float>(mousePos.first) + editorCameraX_;
		float worldY = static_cast<float>(mousePos.second) + editorCameraY_;

		std::cout << "Spawning test particle emitter at (" << worldX << ", " << worldY << ")\n";

		auto particleEntity = entityManager.create<
			ECSComponents::TransformComponent,
			ECSComponents::ParticleEmitterComponent
		>(
			ECSComponents::TransformComponent{{worldX, worldY}, {0,0}, {1,1}},
			Particles::Presets::CreateFireball()
		);

		testParticleEntityID_ = particleEntity.id();
		particleTestSpawned_ = true;

		std::cout << "Particle entity created with ID: " << testParticleEntityID_ << "\n";
	}

	if (particleTestSpawned_ && input.isKeyPressed(GLFW_KEY_O)) {
		auto* particleSystem = engine_.getParticleSystem();
		if (particleSystem) {
			auto it = entityManager.getMap().find(testParticleEntityID_);
			if (it != entityManager.getMap().end()) {
				auto& entityLoc = it->second;

				bool hasEmitter = false;
				for (const auto& type : entityLoc.archetype->types_) {
					if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent))) {
						hasEmitter = true;
						break;
					}
				}

				if (hasEmitter) {
					size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::ParticleEmitterComponent>();
					auto& emitter = entityLoc.archetype->chunks_[entityLoc.chunkIndex]
						->retrieve<ECSComponents::ParticleEmitterComponent>(index, entityLoc.entityIndexInChunk);

					emitter.isEmitting = !emitter.isEmitting;
					std::cout << "Particle emission " << (emitter.isEmitting ? "enabled" : "disabled") << "\n";
				}
			}
		}
	}

	if (particleTestSpawned_ && input.isKeyPressed(GLFW_KEY_B)) {
		auto* particleSystem = engine_.getParticleSystem();
		if (particleSystem) {
			particleSystem->EmitBurst(entityManager, testParticleEntityID_, 50);
			std::cout << "Emitted particle burst!\n";
		}
	}

	if (particleTestSpawned_ && input.isKeyPressed(GLFW_KEY_K)) {
		auto it = entityManager.getMap().find(testParticleEntityID_);
		if (it != entityManager.getMap().end()) {
			// Clean up particle pool first
			auto* particleSystem = engine_.getParticleSystem();
			if (particleSystem) {
				// Access private removePool method through public interface
				// The pool will be cleaned up when entity is destroyed
			}

			// Destroy the entity
			ECS::Entity(testParticleEntityID_).destroy(entityManager);
			particleTestSpawned_ = false;
			testParticleEntityID_ = 0;
			std::cout << "Test particle entity deleted\n";
		}
	}
}

// ========== Dialogue System ==========

void GameApplication::startDialogue()
{
	if (dialogueTexts_.empty()) {
		std::cout << "[Dialogue] No dialogue texts configured – populate dialogueTexts_ first" << std::endl;
		return;
	}

	auto& entityManager = engine_.getEntityManager();

	dialogueActive_ = true;
	dialogueIndex_  = 0;
	isPaused_       = true;
	engine_.setIsPlaying(false);

	// Find dialogue box by name
	auto dialogueBox = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("DialogueBox"));

	// Show the dialogue box
	if (!dialogueBox.empty())
	{
		for (auto& it : dialogueBox)
		{
			if (it.has<ECSComponents::VisibilityComponent>(entityManager)) 
			{
				it.get<ECSComponents::VisibilityComponent>(entityManager).visible = true;
		
			}
		}
	}

	// Start typewriter animation for first dialogue line
	dialogueTargetText_  = dialogueTexts_[0];
	dialogueCurrentText_.clear();
	dialogueTypeTimer_   = 0.0f;
	dialogueTyping_      = true;

	// Clear body text entity to start fresh
	auto bodyText = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("body text"));
	if (!bodyText.empty() && bodyText[0].has<ECSComponents::TextComponent>(entityManager))
	{
		bodyText[0].get<ECSComponents::TextComponent>(entityManager).text[0] = '\0';
	}

	std::cout << "[Dialogue] Started – showing text 0: " << dialogueTexts_[0] << std::endl;
}

void GameApplication::advanceDialogue()
{
	dialogueIndex_++;

	if (dialogueIndex_ >= static_cast<int>(dialogueTexts_.size())) {
		// All texts have been shown – end the dialogue
		dialogueActive_ = false;
		dialogueTyping_      = false;
		isPaused_       = false;
		engine_.setIsPlaying(true);

		auto& entityManager = engine_.getEntityManager();

		// Hide the dialogue box
		auto dialogueBox = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("DialogueBox"));

		// Show the dialogue box
		if (!dialogueBox.empty())
		{
			for (auto& it : dialogueBox)
			{
				if (it.has<ECSComponents::VisibilityComponent>(entityManager))
				{
					it.get<ECSComponents::VisibilityComponent>(entityManager).visible = false;

				}
			}
		}

		std::cout << "[Dialogue] Ended" << std::endl;
	}
	else {
		// Start typewriter animation for next dialogue line
		dialogueTargetText_  = dialogueTexts_[dialogueIndex_];
		dialogueCurrentText_.clear();
		dialogueTypeTimer_   = 0.0f;
		dialogueTyping_      = true;

		// Clear body text entity
		auto& entityManager = engine_.getEntityManager();
		auto bodyText = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("body text"));
		if (!bodyText.empty() && bodyText[0].has<ECSComponents::TextComponent>(entityManager))
		{
			bodyText[0].get<ECSComponents::TextComponent>(entityManager).text[0] = '\0';
		}
		std::cout << "[Dialogue] Showing text " << dialogueIndex_ << ": " << dialogueTexts_[dialogueIndex_] << std::endl;
	}
}
