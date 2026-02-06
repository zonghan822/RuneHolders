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
#include <sstream>
#include <iomanip>
#include <random>

#include "math/math.hpp"

// Random number generator for dialogue typing sounds
static std::mt19937 rng(std::random_device{}());

// Helper function for random float generation
float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

float scaleSize = 1.0f;
float rotationRadians = 0.f;


void GameApplication::initialize() {
	// Setup engine systems first (this now includes game systems)
	engine_.setup();

	// FORCE LOAD MAIN MENU FOR RUNEWEAVERS
	// This ensures RuneWeavers always starts at the main menu regardless of LastSavedScene.txt
	engine_.getEnoEditor().selectScene("MainMenu.txt");

	// Setup all game content
	setupPrefabs();
	setupGameSystems();
	setupGameAudio();
	setupGameEvents();
	setupAnimations();
	setupScripting();
	setupFont();
	setupCamera();
	setupCombatSystem();
	setupMainMenuButtons();
	/*setupSceneTransitions();*/

	auto& tileMap = engine_.getTileMap();

	Math::eVec2<float> worldSize = tileMap.getWorldSize();

	worldWidth_ = worldSize.x;
	worldHeight_ = worldSize.y;


	// ========== SCENE DEBUG INFO ==========
	auto& sceneTree = engine_.getEnoEditor().getSceneTree();
	auto& entityMng = engine_.getEntityManager();


	// int nodeCount = 0;  // Unused - commented out to remove warning
	for (const auto& node : sceneTree.getAllNodes()) {

		if (node->entity_.has<ECSComponents::NameComponent>(entityMng)) {
			// auto& name = node->entity_.get<ECSComponents::NameComponent>(entityMng);  // Unused - commented out
		}

		if (node->entity_.has<ECSComponents::TransformComponent>(entityMng)) {
			// auto& transform = node->entity_.get<ECSComponents::TransformComponent>(entityMng);  // Unused - commented out
		}

		if (node->entity_.has<ECSComponents::RenderComponent>(entityMng)) {
			auto& render = node->entity_.get<ECSComponents::RenderComponent>(entityMng);
			if (strlen(render.texturePath) > 0) {
			}
		} else {
		}

	}

	// Check how many entities have DrawInfoComponent
	auto drawableEntities = entityMng.findEntity<ECSComponents::DrawInfoComponent>();

	// Auto-play BGM when game starts (RuneWeavers is a standalone game, not editor)
	playSceneBGM();
}

void GameApplication::run() {

	// Start the engine with game update callback
	engine_.run([this]() {
		updateGameLogic();  // This gets called every frame
		});

}

void GameApplication::shutdown() {
	engine_.shutDown();
}

void GameApplication::setupPrefabs() {
	auto prefabManager = engine_.getPrefabManager();
	prefabManager.registerPresets();

	ECSComponents::Preset::saveAll(prefabManager);
	ECSComponents::Preset::loadAll(prefabManager);
}

void GameApplication::setupGameSystems() {

	// Initialize player system
	playerSystem_ = std::make_unique<Player::PlayerSystem>();

	// Load player config
	playerConfig_.LoadFromFile(FileSystem::GetAssetsDir().string() + "/saves/player/player_config.txt");

	// Check if a player entity exists in the scene
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();

	if (!playerEntities.empty()) {
		// Found a player entity
		playerEntityID_ = playerEntities[0].id();
		playerSpawned_ = true;
	}
}

void GameApplication::setupFont() {
	pixelifyFont_ = engine_.getResourceLoader().getFont("PixelifySans");
	timesFont_ = engine_.getResourceLoader().getFont("times");
	philosopherBoldFont = engine_.getResourceLoader().getFont("Philosopher-Bold");
	philosopherRegularFont = engine_.getResourceLoader().getFont("Philosopher-Regular");
}

void GameApplication::setupCamera() {
	auto& entityManager = engine_.getEntityManager();

	// Check if a camera already exists in the scene
	std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();

	if (cameraEntities.empty()) {
		// No camera found, create one

		// Create camera component with default viewport
		ECSComponents::CameraComponent camera;
		camera.viewportWidth = 1920.0f;
		camera.viewportHeight = 1080.0f;
		camera.position = Math::eVec2<float>{ camera.viewportWidth * 0.5f, camera.viewportHeight * 0.5f };
		camera.isActive = true;

		ECSComponents::NameComponent nameComp("Main Camera");
		ECS::Entity cameraEntity = entityManager.create<ECSComponents::NameComponent, ECSComponents::CameraComponent>(nameComp, camera);
	}
}

void GameApplication::setupCombatSystem() {
	auto& entityManager = engine_.getEntityManager();


	// Add HealthComponent to player if it exists
	if (playerSpawned_) {
		ECS::Entity playerEntity(playerEntityID_);
		if (!playerEntity.has<ECSComponents::HealthComponent>(entityManager)) {
			playerEntity.add<ECSComponents::HealthComponent>(entityManager, 100.0f);
		}
	}

	// Find all enemies with AIComponent and add health/damage
	std::vector<ECS::Entity> enemies = entityManager.findEntity<ECSComponents::AIComponent>();
	for (auto& enemy : enemies) {
		// Add or reset HealthComponent
		if (!enemy.has<ECSComponents::HealthComponent>(entityManager)) {
			enemy.add<ECSComponents::HealthComponent>(entityManager, 50.0f);
		} else {
			// Reset health to max if component already exists
			auto& health = enemy.get<ECSComponents::HealthComponent>(entityManager);
			health.currentHealth = 50.0f;
			health.maxHealth = 50.0f;
			health.isDead = false;
			health.invulnerable = false;
			health.invulnerabilityTimer = 0.0f;
		}

		// Add DamageComponent if not already present
		if (!enemy.has<ECSComponents::DamageComponent>(entityManager)) {
			enemy.add<ECSComponents::DamageComponent>(entityManager, 10.0f);
		}
	}

}

void GameApplication::setupScripting() {
	// Scripting premade calls
	engine_.getCoreHost().HRSetup();
	engine_.getCoreHost().defineBegin();
	engine_.getCoreHost().defineUpdate();
	engine_.getCoreHost().defineSMD();

	// Re-register all scripts that were loaded before the scripting system was initialized
	// Scene loading happens before setupScripting(), so rpm_ was null during initial registration
	engine_.getEnoEditor().getSceneTree().registerAllScripts();
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

	// Rune hover audio
	audioMng.publishSoundRequest("Rune-GalaxyBellPad-1");
	audioMng.publishSoundRequest("Rune-GalaxyBellPad-2");

	// Spellbook audio
	audioMng.publishSoundRequest("Spellbook_Open_01");
	audioMng.publishSoundRequest("Spellbook_Close_01");

	// Fireball audio
	audioMng.publishSoundRequest("Fireball-Cast");
	audioMng.publishSoundRequest("Fireball-Explosion-Small");

	// Typing sounds for dialogue
	for (int i = 1; i <= 21; ++i) {
		std::string soundName = "Typing_" + std::string(i < 10 ? "0" : "") + std::to_string(i);
		audioMng.publishSoundRequest(soundName);
	}

	// BGM
	audioMng.publishSoundRequest("BGM_01_Layer1_loop");
	audioMng.publishSoundRequest("Ambience_Creaking_Wood");
}


void GameApplication::playSceneBGM()
{
	auto& audioMng = engine_.getAudioManager();
	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
	std::string sceneName = currentScene.getSceneName();


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
	audioComp.volume_ = 100.0;  // Set volume to 100%
	audioComp.isAutoplay_ = true;
	audioComp.isPlaying_ = false;  // Reset playing state

	// Prepare and play the audio
	try
	{
		audioMng.prepareMod(audioComp);
		audioMng.play(audioComp);
	}
	catch (const std::exception&)
	{
		// Exception caught and ignored
	}
}

void GameApplication::setupGameEvents() {
	// Subscribe to window focus events (ALT-TAB, etc.)
	engine_.getEventBus().subscribe<Window::WindowFocusEvent>([this](const Window::WindowFocusEvent& event) {

		windowFocused_ = event.focused;

		if (!event.focused) {
			// Lost focus - pause game, clear input

			// Don't spawn pause menu if we're in the main menu
			auto& currentScene = engine_.getEnoEditor().getCurrentScene();
			std::string sceneName = currentScene.getSceneName();
			if (sceneName == "MainMenu.txt") {
				return; // Skip pause menu logic for main menu
			}

			// Pause the game if not already paused
			if (!isPaused_) {
				isPaused_ = true;
				engine_.setIsPlaying(false);
			}

			// If spellbook is open, close it before opening pause menu
			if (showSpellbook_) {
				destroySpellbook();
				showSpellbook_ = false;
			}

			// Hide player if not already hidden
			hidePlayer();

			// Ensure pause menu is displayed (spawn if it doesn't exist)
			if (pauseMenuEntityID_ == 0) {
				spawnPauseMenu();
			}

			// Clear input states to prevent stuck keys
			engine_.getInputManager().clearAllKeyStates();

			// Destroy any active runes
			if (runeSystemInitialized_) {
				ECS::Entity runeSystemEntity(runeSystemEntityID_);
				auto& entityManager = engine_.getEntityManager();
				if (runeSystemEntity.has<ECSComponents::RuneSystemStateComponent>(entityManager)) {
					destroyRunes();
				}
			}
		}
		else {
			// Regained focus - stay paused until player presses ESC 
		}
	});

	// Subscribe to window iconify events (minimize/restore)
	engine_.getEventBus().subscribe<Window::WindowIconifyEvent>([this](const Window::WindowIconifyEvent& event) {
		windowMinimized_ = event.iconified;

		if (event.iconified) {

			// Don't spawn pause menu if we're in the main menu
			auto& currentScene = engine_.getEnoEditor().getCurrentScene();
			std::string sceneName = currentScene.getSceneName();
			if (sceneName == "MainMenu.txt") {
				return; // Skip pause menu logic for main menu
			}

			// Window minimized, pause game
			// Pause the game if not already paused
			if (!isPaused_) {
				isPaused_ = true;
				engine_.setIsPlaying(false);
			}

			// If spellbook is open, close it before opening pause menu
			if (showSpellbook_) {
				destroySpellbook();
				showSpellbook_ = false;
			}

			// Hide player
			hidePlayer();

			// Ensure pause menu is displayed (spawn if it doesn't exist)
			if (pauseMenuEntityID_ == 0) {
				spawnPauseMenu();
			}
		}
		else {
			// Window restored - stay paused until player manually resumes
		}
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
}

void GameApplication::setupAnimations() {
	// Load the sprite sheet texture
	backgroundTexture_ = engine_.getResourceLoader().getTexture("Background");
	playerTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Idle-Sheet");
	playerWalkTexture_ = engine_.getResourceLoader().getTexture("Protagonist-Walk-Sheet");

	if (!backgroundTexture_ || !backgroundTexture_->isValid())
	{
	}

	if (!playerTexture_ || !playerTexture_->isValid())
	{
		return;
	}

	// Create player idle animation
	auto playerIdleAnim = Animation::createSpriteSheetAnimation(
		128,    // Frame width
		128,    // Frame height
		768,    // Sheet width
		384,    // Sheet height
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

}

void GameApplication::updateCamera() {
	auto& entityManager = engine_.getEntityManager();

	// Find the active camera
	std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();

	for (auto& cameraEntity : cameraEntities) {
		auto& camera = cameraEntity.get<ECSComponents::CameraComponent>(entityManager);

		if (!camera.isActive) continue;

		// Sync script camera: pick up changes from C# bridge
		if (engine_.consumeScriptCameraDirty()) {
			auto camPos = engine_.getScriptCameraPos();
			camera.position.x = camPos.x;
			camera.position.y = camPos.y;
		}
		// Keep script camera in sync so GetCameraPosition returns current values
		engine_.syncScriptCameraPos(camera.position.x, camera.position.y);

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
	auto& windowMng = engine_.getWindowManager();

	Math::eVec2<float> worldSize = tileMap.getWorldSize();
	worldWidth_ = worldSize.x, worldHeight_ = worldSize.y;
	float deltaTime = static_cast<float>(engine_.getDeltaTime());

	// ========== WINDOW SIZE HANDLING BY SHORTCUT KEY ==========
	bool ctrl = inputMng.isModifierActive(GLFW_MOD_CONTROL);

	if (ctrl && inputMng.isKeyDown(GLFW_KEY_KP_ADD))       // Numpad +
		windowMng.increaseResolution();

	if (ctrl && inputMng.isKeyDown(GLFW_KEY_EQUAL))        // + / =
		windowMng.increaseResolution();

	if (ctrl && inputMng.isKeyDown(GLFW_KEY_KP_SUBTRACT))  // Numpad -
		windowMng.decreaseResolution();

	if (ctrl && inputMng.isKeyDown(GLFW_KEY_MINUS))        // '-' key
		windowMng.decreaseResolution();
	
	if (ctrl && inputMng.isKeyDown(GLFW_KEY_0))		       //  0
		windowMng.toggleFullscreen();

	if (ctrl && inputMng.isKeyDown(GLFW_KEY_KP_0))		   //  0
		windowMng.toggleFullscreen();

	if (ctrl && inputMng.isKeyDown(GLFW_KEY_9))
		isDisplayFPS_ = !isDisplayFPS_;


	// ========== PAUSE MENU AND SPELLBOOK INPUT HANDLING ==========
	// Only handle pause/spellbook input when player exists
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
	bool playerExists = !playerEntities.empty();

	if (playerExists) {
		// Handle F key - Toggle Spellbook
		if (inputMng.isKeyDown(GLFW_KEY_F) && !isPaused_) {
			if (!showSpellbook_) {
				// Open spellbook and pause game
				showSpellbook_ = true;
				isPaused_ = true;
				hidePlayer();
				spawnSpellbook();
				engine_.setIsPlaying(false);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Open_01" });
			}
			else {
				// Close spellbook and resume game
				showSpellbook_ = false;
				isPaused_ = false;
				destroySpellbook();
				showPlayer();
				engine_.setIsPlaying(true);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
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
			}
			else if (!isPaused_) {
				// If not paused and not in spellbook, open pause menu
				isPaused_ = true;
				hidePlayer();
				spawnPauseMenu();
				engine_.setIsPlaying(false);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Open_01" });
			}
			else {
				// If pause menu is open, close it and resume game
				isPaused_ = false;
				destroyPauseMenu();
				showPlayer();
				engine_.setIsPlaying(true);
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
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
				// Check if player is colliding with UI-PopUp-E entity
				if (playerSpawned_ && playerEntityID_ != 0) {
					ECS::Entity playerEntity(playerEntityID_);
					if (playerEntity.has<ECSComponents::TransformComponent>(entityManager) &&
						playerEntity.has<ECSComponents::CollisionComponent>(entityManager)) {

						auto& playerTransform = playerEntity.get<ECSComponents::TransformComponent>(entityManager);
						auto& playerCollision = playerEntity.get<ECSComponents::CollisionComponent>(entityManager);

						// Find all UI-PopUp-E entities
						auto scrolls = entityManager.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("UI-PopUp-E"));
						for (auto& scroll : scrolls) {
							if (scroll.has<ECSComponents::TransformComponent>(entityManager) &&
								scroll.has<ECSComponents::CollisionComponent>(entityManager)) {

								auto& scrollTransform = scroll.get<ECSComponents::TransformComponent>(entityManager);
								auto& scrollCollision = scroll.get<ECSComponents::CollisionComponent>(entityManager);

								// Check AABB collision
								if (scrollCollision.type == ECSComponents::ColliderType::RECTANGLE &&
									playerCollision.type == ECSComponents::ColliderType::RECTANGLE) {

									// Get positions with offsets
									float playerX = playerTransform.pos.x + playerCollision.offset.x;
									float playerY = playerTransform.pos.y + playerCollision.offset.y;
									float scrollX = scrollTransform.pos.x + scrollCollision.offset.x;
									float scrollY = scrollTransform.pos.y + scrollCollision.offset.y;

									// Get half dimensions
									float playerHalfW = (playerCollision.data.rectangle.width * playerTransform.scale.x) * 0.5f;
									float playerHalfH = (playerCollision.data.rectangle.height * playerTransform.scale.y) * 0.5f;
									float scrollHalfW = (scrollCollision.data.rectangle.width * scrollTransform.scale.x) * 0.5f;
									float scrollHalfH = (scrollCollision.data.rectangle.height * scrollTransform.scale.y) * 0.5f;

									// AABB collision test
									if ((std::abs(playerX - scrollX) <= (playerHalfW + scrollHalfW)) &&
										(std::abs(playerY - scrollY) <= (playerHalfH + scrollHalfH))) {
										startDialogue();
										break; // Only trigger once
									}
								}
							}
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
			//Play audio here for keyclicks here: Yun Qi
			int typingSoundValue = static_cast<int>(randomFloat(1, 22)); // 1-21 inclusive
			std::string number = (typingSoundValue < 10 ? "0" : "") + std::to_string(typingSoundValue);
			std::string typeSound = "Typing_" + number;
			engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", typeSound });

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

	// Update main menu buttons (outside pause check - buttons work even when paused)
	updateMainMenuButtons();

	// Update scene transitions (check for player collision with transition triggers)
	//updateSceneTransitions();

	// Only update game systems when not paused
	if (!isPaused_) {
		// Always update player movement (no editor state checks for game)
		playerSystem_->Update(entityManager, inputMng, deltaTime);

		// Update enemy AI (pathfinding and movement toward player)
		updateEnemyAI(deltaTime);

		// Update rune system
		updateRuneSystem();

		// Update combat system (damage dealing and health)
		updateCombatSystem(deltaTime);

		// Update floating texts (rise and fade)
		updateFloatingTexts(deltaTime);

		// Update rupture mark spells (state machine, enemy detection, damage)
		updateRuptureMarks(deltaTime);
	}

	// Update camera system (replaces the old fixed projection)
	updateCamera();

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

	// Render particles (with camera offset)
	auto* particleSystem = engine_.getParticleSystem();
	if (particleSystem) {
		Math::eVec2<float> cameraOffset = { 0.0f, 0.0f };
		auto cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();
		if (!cameraEntities.empty()) {
			auto& cam = cameraEntities[0].get<ECSComponents::CameraComponent>(entityManager);
			cameraOffset = { cam.position.x - cam.viewportWidth * 0.5f, cam.position.y - cam.viewportHeight * 0.5f };
		}
		particleSystem->Render(graphics_, engine_.getEntityManager(), cameraOffset);
	}

	// Draw rune connection lines ON TOP of everything else
	if (runeSystemInitialized_) {
		ECS::Entity runeSystemEntity(runeSystemEntityID_);
		if (runeSystemEntity.has<ECSComponents::RuneSystemStateComponent>(entityManager)) {
			auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);
			if (runeState.isDrawingLine) {
				drawRuneConnectionLine();
			}
		}
	}

	graphics_.flush();
	displayFPS();
}

void GameApplication::loadEntityTextures() {
	// Load textures for all entities that have a texture path but no texture loaded
	auto& entityManager = engine_.getEntityManager();
	auto& loader = engine_.getResourceLoader();

	auto renderEntities = entityManager.findEntity<ECSComponents::RenderComponent>();
	for (auto& entity : renderEntities) {
		auto& renderComp = entity.get<ECSComponents::RenderComponent>(entityManager);

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

		// DEBUG: Count runes in cache
		int runeCount = 0;
		for (auto& ent : cachedDrawableEntities_) {
			if (ent.has<ECSComponents::RuneComponent>(engine_.getEntityManager())) {
				runeCount++;
			}
		}
		if (runeCount > 0) {
			std::cout << "[DEBUG] Cache rebuilt: " << runeCount << " runes found in cache" << std::endl;
		}

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
					} else {
						// Controller exists, clear it before restoring animations
						ctrl->animations_.clear();
						ctrl->textures_.clear();
						ctrl->currentAnimation_ = nullptr;
						ctrl->currentTexture_ = nullptr;
						ctrl->currentAnimationName_.clear();
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
				ctrl->update(static_cast<float>(engine_.getDeltaTime()));
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
					std::wcout << "origin of call: " << org << " for entity " << node->entity_.id() << std::endl;
					coreHost.begin(org, node->sceneObjectID_);
					//coreHost.begin(org, node->entity_.id());  // FIX: Use entity_.id() instead of sceneObjectID_
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
				// coreHost.update(org, node->entity_.id());  // FIX: Use entity_.id() instead of sceneObjectID_
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

					// Skip drawing if player is hidden (scale set to 0 by hidePlayer())
					if (transform.scale.x == 0.0f || transform.scale.y == 0.0f) {
						return;
					}

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
					}

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_2)) {
						scaleSize -= static_cast <float>(3.0 * 0.01667);
						scaleSize = std::clamp(scaleSize, 1.0f, 100.0f);
						transform.scale.x = scaleSize;
						transform.scale.y = scaleSize;
					}

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_3)) {
						rotationRadians += static_cast <float>(3.0f * 0.0174533 * 0.01667);

						auto& playerCollider = playerSystem_->GetCollision(it->second);
						playerCollider.rotation = rotationRadians;
					}

					if (engine_.getInputManager().isKeyPressed(GLFW_KEY_4)) {
						rotationRadians -= static_cast <float>(3.0f * 0.0174533 * 0.01667);
						auto& playerCollider = playerSystem_->GetCollision(it->second);
						playerCollider.rotation = rotationRadians;
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

	// CRITICAL: Flush the player batch immediately to ensure it renders with correct projection
	graphics.flush();
}


void GameApplication::drawShapes() {

	auto& graphics = engine_.getGraphicsManager();
	auto& entityManager = engine_.getEntityManager();

	using drawInfo = ECSComponents::DrawInfoComponent;

	static bool firstRun = true;
	if (firstRun) {
		firstRun = false;
	}

	// Collect and sort cached entities by layer
	ECSComponents::collectObjectsByLayer(engine_, cachedDrawableEntities_);

	// Draw all entities by layer order (lowest z_index first = background)
	for (auto& [layer, entities] : ECSComponents::sortedObjectLayers) 
	{
		for (ECS::Entity* entityPtr : entities) 
		{
			ECS::Entity& entity = *entityPtr;
			try {
				if (entity.has<ECSComponents::DrawInfoComponent>(entityManager)) 
				{
					if (entity.has<ECSComponents::PlayerComponent>(entityManager)) {
						continue; // Skip player, drawn separately
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

						// Custom rendering for runes (spritesheet with UV)
						if (entity.has<ECSComponents::RuneComponent>(entityManager)) {
							auto& runeComp = entity.get<ECSComponents::RuneComponent>(entityManager);

							// DEBUG: Log rune rendering
							// std::cout << "[DEBUG] Rendering rune " << runeComp.runeNumber << " (entity " << entity.id() << ")" << std::endl;

							// Calculate UV coordinates based on rune number
							ECSComponents::RuneAssets::getRuneUV(runeComp.runeNumber, uvStart, uvEnd);
						}
						else if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager))
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

						//	<< renderInfo.texture->getWidth() << " x "
						//	<< renderInfo.texture->getHeight() << std::endl;
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
			catch (const std::exception& e) {
				// Log the actual error to help debug missing runes
				std::cerr << "[Rendering] Exception for entity " << entity.id()
				          << ": " << e.what() << std::endl;
				entitiesDirty_ = true;
				continue;
			}
			catch (...) {
				// Unknown exception
				std::cerr << "[Rendering] Unknown exception for entity " << entity.id() << std::endl;
				entitiesDirty_ = true;
				continue;
			}
		}
	}

	// Flush pending entity batch before text drawing (drawTextScreen unbinds defaultShader)
	graphics.flush();

	// Get active camera for offset calculation and viewport size
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

	// Draw TextComponent entities (world space, multiline support)
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

				// Use transform.pos directly (same as sprite rendering - no parent accumulation)
				Math::eVec2<float> worldPos = transform.pos;

				// Convert world position to camera-relative position for rendering
				Math::eVec2<float> screenPos = {
					worldPos.x - cameraX,
					worldPos.y - cameraY
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
						graphics.drawTextScreen(
							font, line,
							Math::eVec2<float>{ screenPos.x, screenPos.y + yOffset },
							textComp.fontSize,
							Graphics::Color::White,
							viewportWidth,
							viewportHeight
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

void GameApplication::displayFPS() {
	if (!isDisplayFPS_) return;

	auto& graphics = engine_.getGraphicsManager();
	auto& window = engine_.getWindowManager();
	
	std::ostringstream ss;
	ss << "FPS: " <<  std::fixed << std::setprecision(2) << engine_.getFPS();
	std::string fpsText = ss.str();

	auto [w, h] = window.getFramebufferSize();

	// Compute top-right position
	Math::eVec2<float> pos{ 
		static_cast<float>(w) - 150.f,   // 150px margin from right
		40.f							 // 40px from top
	};

	// Draw text
	graphics.drawTextScreen(
		timesFont_,
		fpsText,
		pos,
		0.5f,
		Graphics::Color::White,
		static_cast<float>(w),
		static_cast<float>(h)
	);
}

void GameApplication::applyTextureToPlayer(const Graphics::TextureData& texData) {
	if (!texData.texture) return;

	playerTexture_ = texData.texture;
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
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Pre-load and cache all rune textures
	auto& resourceLoader = engine_.getResourceLoader();
	runeTexture_ = resourceLoader.getTexture("Runes_White_Sheet_Large");
	runeTextureGrey_ = resourceLoader.getTexture("Runes_Grey_Sheet_Small");
	runeTextureGreen_ = resourceLoader.getTexture("Runes_Green_Sheet_Large");
	runeTextureRed_ = resourceLoader.getTexture("Runes_Red_Sheet_Small");

	if (!runeTexture_ || !runeTexture_->isValid()) {
		std::cerr << "[RuneSystem] WARNING: Failed to pre-load white rune texture!" << std::endl;
	}
	if (!runeTextureGrey_ || !runeTextureGrey_->isValid()) {
		std::cerr << "[RuneSystem] WARNING: Failed to pre-load grey rune texture!" << std::endl;
	}
	if (!runeTextureGreen_ || !runeTextureGreen_->isValid()) {
		std::cerr << "[RuneSystem] WARNING: Failed to pre-load green rune texture!" << std::endl;
	}
	if (!runeTextureRed_ || !runeTextureRed_->isValid()) {
		std::cerr << "[RuneSystem] WARNING: Failed to pre-load red rune texture!" << std::endl;
	}

	// Load line texture
	runeLineTexture_ = resourceLoader.getTexture("Runes-LineRender");
	if (!runeLineTexture_ || !runeLineTexture_->isValid()) {
		std::cerr << "[RuneSystem] WARNING: Failed to pre-load line texture!" << std::endl;
	}

	// Create all 6 runes once (pooled) - position off-screen
	constexpr float OFF_SCREEN_POS = -9999.0f;
	const int runeNumbers[6] = { 6, 1, 2, 3, 4, 5 };

	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i)
	{
		int runeNumber = runeNumbers[i];

		// Create rune entity
		ECS::Entity rune = entityManager.create();
		uint32_t runeId = rune.id();

		// Add all components
		rune.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent(("Rune_" + std::to_string(runeNumber)).c_str()));
		rune.add<ECSComponents::TransformComponent>(entityManager);
		rune.add<ECSComponents::DrawInfoComponent>(entityManager);
		rune.add<ECSComponents::RuneComponent>(entityManager, ECSComponents::RuneComponent(i, runeNumber, 30.0f));
		rune.add<ECSComponents::VisibilityComponent>(entityManager);

		// Configure transform - position off-screen initially
		auto& transform = rune.get<ECSComponents::TransformComponent>(entityManager);
		transform.pos = { OFF_SCREEN_POS, OFF_SCREEN_POS };
		transform.scale = { 1.0f, 1.0f };

		// Configure visibility - high z_index to draw on top of everything
		auto& visibility = rune.get<ECSComponents::VisibilityComponent>(entityManager);
		visibility.z_index = 15;  // High value within MAX_LAYERS (20) = drawn on top
		visibility.visible = true;

		// Configure draw info
		auto& draw = rune.get<ECSComponents::DrawInfoComponent>(entityManager);
		draw.width = 56.0f;
		draw.height = 56.0f;
		draw.color = Graphics::Color::White;
		strcpy_s(draw.shape, "Rectangle");
		draw.texture = runeTexture_;
		draw.texID = runeNumber - 1;

		// Store entity ID in the rune system state
		runeState.runeEntityIDs[i] = runeId;

		std::cout << "[RuneSystem] Created pooled rune " << runeNumber << " with entity ID: " << runeId << std::endl;
	}

	runeSystemInitialized_ = true;
	entitiesDirty_ = true;
	std::cout << "[RuneSystem] Initialized with 6 pooled runes" << std::endl;
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

	// Handle invalid state timer
	if (runeState.isInvalidState)
	{
		float deltaTime = static_cast<float>(engine_.getDeltaTime());
		runeState.invalidStateTimer += deltaTime;

		std::cout << "[RuneSystem] In invalid state, timer: " << runeState.invalidStateTimer << std::endl;

		// Keep updating visual states to ensure red textures are applied
		updateRuneVisualStates();

		// After duration, reset runes
		if (runeState.invalidStateTimer >= ECSComponents::RuneSystemStateComponent::INVALID_STATE_DURATION)
		{
			std::cout << "[RuneSystem] Invalid state timer expired, destroying runes" << std::endl;
			destroyRunes();
			runeState.runesActive = false;
			runeState.isLeftClickHeld = false;
			runeState.isDrawingLine = false;
			// resetConnection() is called in destroyRunes(), which resets isInvalidState
		}

		// Lines will be drawn at end of render pipeline
		// While in invalid state, don't process normal input
		return;
	}

	// Handle success state timer (green flash after valid spell)
	if (runeState.isSuccessState)
	{
		float deltaTime = static_cast<float>(engine_.getDeltaTime());
		runeState.successStateTimer += deltaTime;

		std::cout << "[RuneSystem] In success state, timer: " << runeState.successStateTimer << std::endl;

		// Lines will be drawn at end of render pipeline
		// After duration, destroy runes
		if (runeState.successStateTimer >= ECSComponents::RuneSystemStateComponent::SUCCESS_STATE_DURATION)
		{
			std::cout << "[RuneSystem] Success state timer expired, destroying runes" << std::endl;
			destroyRunes();
			runeState.runesActive = false;
			runeState.isLeftClickHeld = false;
			runeState.isDrawingLine = false;
		}

		// While in success state, don't process normal input
		return;
	}

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

	// ACTIVATE RUNES when left click is first pressed (move from off-screen to cursor)
	if (leftClickPressed && !runeState.runesActive) {
		// Reset connection state
		runeState.resetConnection();

		// Play rune spawn sound
		engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-1" });

		// Move pooled runes to cursor position
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

	// Lines are now drawn at the end of the render pipeline in updateGameLogic()

	// Update fireballs - check lifetime and destroy expired ones
	float deltaTime = static_cast<float>(engine_.getDeltaTime());
	std::vector<uint32_t> fireballsToDestroy;

	auto fireballs = entityManager.findEntity<ECSComponents::FireballComponent>();
	for (auto& entity : fireballs) {
		auto& fireballComp = entity.get<ECSComponents::FireballComponent>(entityManager);
		fireballComp.elapsedTime += deltaTime;

		// Check if fireball has exceeded its lifetime
		if (fireballComp.elapsedTime >= fireballComp.lifetime) {
			fireballsToDestroy.push_back(entity.id());
		}
	}

	// Destroy expired fireballs
	for (uint32_t id : fireballsToDestroy) {
		entityManager.destroyEntity(id);
		entitiesDirty_ = true;
		std::cout << "[RuneSystem] Fireball expired and destroyed" << std::endl;
	}
}

void GameApplication::updateRuneVisualStates()
{
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	int validNextRunes = runeState.getValidNextRunes();

	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i)
	{
		if (runeState.runeEntityIDs[i] == UINT32_MAX) continue;

		ECS::Entity runeEntity(runeState.runeEntityIDs[i]);
		if (!runeEntity.has<ECSComponents::RuneComponent>(entityManager) || !runeEntity.has<ECSComponents::DrawInfoComponent>(entityManager)) continue;

		auto& rune = runeEntity.get<ECSComponents::RuneComponent>(entityManager);
		auto& draw = runeEntity.get<ECSComponents::DrawInfoComponent>(entityManager);

		// If in invalid state, ALL runes show red
		if (runeState.isInvalidState)
		{
			rune.visualState = ECSComponents::RuneVisualState::INVALID;
		}
		// Otherwise determine visual state normally
		else if (rune.isConnected)
		{
			rune.visualState = ECSComponents::RuneVisualState::SELECTED;
		}
		else if (validNextRunes & (1 << rune.runeNumber))
		{
			if (runeState.connectionCount == 0)
				rune.visualState = ECSComponents::RuneVisualState::AVAILABLE_START;
			else
				rune.visualState = ECSComponents::RuneVisualState::NEXT_VALID;
		}
		else
		{
			rune.visualState = ECSComponents::RuneVisualState::UNAVAILABLE;
		}

		// Apply texture and size based on visual state
		switch (rune.visualState)
		{
		case ECSComponents::RuneVisualState::UNAVAILABLE:
			std::cout << "[DEBUG] Rune " << rune.runeNumber << " -> UNAVAILABLE (grey)" << std::endl;
			draw.texture = runeTextureGrey_;
			draw.width = 40.0f;   // Small size
			draw.height = 40.0f;
			break;

		case ECSComponents::RuneVisualState::SELECTED:
			std::cout << "[DEBUG] Rune " << rune.runeNumber << " -> SELECTED (green)" << std::endl;
			draw.texture = runeTextureGreen_;
			draw.width = 56.0f;   // Large size
			draw.height = 56.0f;
			break;

		case ECSComponents::RuneVisualState::AVAILABLE_START:
		case ECSComponents::RuneVisualState::NEXT_VALID:
			std::cout << "[DEBUG] Rune " << rune.runeNumber << " -> AVAILABLE (white)" << std::endl;
			draw.texture = runeTexture_;  // White Large
			draw.width = 56.0f;
			draw.height = 56.0f;
			break;

		case ECSComponents::RuneVisualState::INVALID:
			std::cout << "[DEBUG] Rune " << rune.runeNumber << " -> INVALID (red), runeTextureRed_ is "
			          << (runeTextureRed_ && runeTextureRed_->isValid() ? "VALID" : "NULL/INVALID") << std::endl;
			draw.texture = runeTextureRed_;
			draw.width = 40.0f;   // Small size
			draw.height = 40.0f;
			break;
		}

		// Debug: verify texture was applied
		std::cout << "[DEBUG] Rune " << rune.runeNumber << " texture after apply: "
		          << (draw.texture && draw.texture->isValid() ? "VALID" : "NULL/INVALID") << std::endl;
	}
}


void GameApplication::spawnRunes(const Math::eVec2<float>& mouseWorldPos)
{
	// POOLING APPROACH: Move existing runes to cursor position (no entity creation)
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	// Store circle center for spell spawning
	runeState.circleCenter = mouseWorldPos;

	// Reset connection state
	runeState.resetConnection();

	// Move all 6 pooled runes to circle pattern around cursor
	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i)
	{
		if (runeState.runeEntityIDs[i] == UINT32_MAX) {
			std::cerr << "[RuneSystem] ERROR: Pooled rune " << i << " has invalid ID!" << std::endl;
			continue;
		}

		ECS::Entity runeEntity(runeState.runeEntityIDs[i]);
		if (!runeEntity.has<ECSComponents::TransformComponent>(entityManager) ||
			!runeEntity.has<ECSComponents::RuneComponent>(entityManager)) {
			std::cerr << "[RuneSystem] ERROR: Pooled rune " << i << " missing components!" << std::endl;
			continue;
		}

		// Calculate position in circle
		float angle = (-120.0f + i * 60.0f) * (PI / 180.0f);
		float x = mouseWorldPos.x + runeState.runeCircleRadius * std::cos(angle);
		float y = mouseWorldPos.y + runeState.runeCircleRadius * std::sin(angle);

		// Move rune to position
		auto& transform = runeEntity.get<ECSComponents::TransformComponent>(entityManager);
		transform.pos = { x, y };

		// Reset rune state
		auto& rune = runeEntity.get<ECSComponents::RuneComponent>(entityManager);
		rune.isConnected = false;
		rune.isHovered = false;
	}

	// Set initial visual states (starting runes highlighted)
	updateRuneVisualStates();
}

void GameApplication::destroyRunes()
{
	// POOLING APPROACH: Move runes off-screen (no entity destruction)
	auto& entityManager = engine_.getEntityManager();
	ECS::Entity runeSystemEntity(runeSystemEntityID_);
	auto& runeState = runeSystemEntity.get<ECSComponents::RuneSystemStateComponent>(entityManager);

	constexpr float OFF_SCREEN_POS = -9999.0f;

	// Move all 6 runes off-screen
	for (int i = 0; i < ECSComponents::RUNE_COUNT; ++i) {
		if (runeState.runeEntityIDs[i] == UINT32_MAX) continue;

		ECS::Entity runeEntity(runeState.runeEntityIDs[i]);
		if (runeEntity.has<ECSComponents::TransformComponent>(entityManager)) {
			auto& transform = runeEntity.get<ECSComponents::TransformComponent>(entityManager);
			transform.pos = { OFF_SCREEN_POS, OFF_SCREEN_POS };
		}

		// Reset rune state
		if (runeEntity.has<ECSComponents::RuneComponent>(entityManager)) {
			auto& rune = runeEntity.get<ECSComponents::RuneComponent>(entityManager);
			rune.isConnected = false;
			rune.isHovered = false;
		}
	}

	// Reset rune system state
	runeState.resetConnection();
	runeState.runesActive = false;
	runeState.isLeftClickHeld = false;
	runeState.isDrawingLine = false;
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
		// Also check visual state - only allow selecting AVAILABLE_START or NEXT_VALID runes
		bool isSelectable = (rune.visualState == ECSComponents::RuneVisualState::AVAILABLE_START ||
		                     rune.visualState == ECSComponents::RuneVisualState::NEXT_VALID);
		if (rune.isHovered && !wasHovered && !rune.isConnected && !rune.isBanned && !runeState.spellCasted && !runeState.isInvalidState
			&& isSelectable)
		{
			// Debug: Check validity
			int validMask = runeState.getValidNextRunes();
			bool isValid = runeState.isValidNextRune(rune.runeNumber);
			std::cout << "[RuneSystem] Hover rune " << rune.runeNumber
			          << ", connectionCount=" << runeState.connectionCount
			          << ", validMask=0x" << std::hex << validMask << std::dec
			          << ", isValid=" << (isValid ? "true" : "false") << std::endl;

			// Check if this rune is a valid next choice
			if (!isValid)
			{
				// Invalid selection! Only trigger if we have at least 1 rune connected
				if (runeState.connectionCount > 0)
				{
					std::cout << "[RuneSystem] INVALID selection: Rune " << rune.runeNumber
					          << " is not valid after sequence of " << runeState.connectionCount << " runes" << std::endl;

					// Enter invalid state
					runeState.isInvalidState = true;
					runeState.invalidStateTimer = 0.0f;
					runeState.lineColorState = ECSComponents::LineColorState::INVALID;

					// Play invalid sound
					engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Rune-GalaxyBellPad-1" });  // TODO: use a different sound for invalid

					// Update visuals to show all red
					updateRuneVisualStates();
					return;  // Stop processing hover
				}
				// If no runes connected yet, just ignore the invalid selection
				continue;
			}

			// Try to add this rune to the sequence
			if (runeState.addRuneToSequence(rune.runeNumber, transform.pos))
			{
				rune.isConnected = true;
				updateRuneVisualStates();	// Update visual states to show next valid runes
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
						// Valid spell! Enter success state with green lines
						runeState.lineColorState = ECSComponents::LineColorState::SUCCESS;
						runeState.isSuccessState = true;
						runeState.successStateTimer = 0.0f;

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
						// Invalid 4-rune sequence - enter invalid state with red lines
						runeState.lineColorState = ECSComponents::LineColorState::INVALID;
						runeState.isInvalidState = true;
						runeState.invalidStateTimer = 0.0f;

						std::cout << "[RuneSystem] Invalid spell sequence: "
						          << runeState.connectedRuneSequence[0] << "-"
						          << runeState.connectedRuneSequence[1] << "-"
						          << runeState.connectedRuneSequence[2] << "-"
						          << runeState.connectedRuneSequence[3] << std::endl;
					}

					runeState.spellCasted = true;
					// Don't destroy immediately - let the success/invalid state timer handle it
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

	// Determine line color based on state
	Graphics::Color lineColor;
	switch (runeState.lineColorState)
	{
	case ECSComponents::LineColorState::SELECTING:
		lineColor = Graphics::Color(1.0f, 1.0f, 0.0f, 0.9f);  // Yellow
		break;
	case ECSComponents::LineColorState::SUCCESS:
		lineColor = Graphics::Color(0.0f, 1.0f, 0.0f, 0.9f);  // Green
		break;
	case ECSComponents::LineColorState::INVALID:
		lineColor = Graphics::Color(1.0f, 0.0f, 0.0f, 0.9f);  // Red
		break;
	default:
		lineColor = Graphics::Color(1.0f, 1.0f, 0.0f, 0.9f);  // Default yellow
		break;
	}

	constexpr float LINE_WIDTH = 8.0f;  // Line thickness in pixels

	std::cout << "[DEBUG] drawRuneConnectionLine called, connectionCount=" << runeState.connectionCount
	          << ", lineEnd=(" << runeState.lineEnd.x << "," << runeState.lineEnd.y << ")" << std::endl;

	// Draw lines between connected runes (multi-segment line)
	for (int i = 0; i < runeState.connectionCount; ++i)
	{
		Math::eVec2<float> screenStart = runeState.connectedPositions[i];
		Math::eVec2<float> screenEnd;

		// If this is the last connected rune, draw to mouse position
		if (i == runeState.connectionCount - 1)
		{
			screenEnd = runeState.lineEnd;
		}
		else
		{
			// Draw to next connected rune
			screenEnd = runeState.connectedPositions[i + 1];
		}

		// Draw line segment
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

			// drawTexturedQuad expects top-left corner, not center
			// Calculate top-left position so the rectangle is centered at midpoint
			Math::eVec2<float> topLeft = {
				midpoint.x - length / 2.0f,
				midpoint.y - LINE_WIDTH / 2.0f
			};

			std::cout << "[DEBUG] Drawing line segment " << i << ": from (" << screenStart.x << "," << screenStart.y
			          << ") to (" << screenEnd.x << "," << screenEnd.y << "), length=" << length
			          << ", topLeft=(" << topLeft.x << "," << topLeft.y << ")" << std::endl;

			// TEMP: Force fallback rect to test if drawing works at all
			// TODO: Re-enable texture once rect is confirmed working
			bool useTexture = false;  // Set to true to use texture, false to test with rect

			if (useTexture && runeLineTexture_ && runeLineTexture_->isValid())
			{
				std::cout << "[DEBUG] Using textured line" << std::endl;
				graphics.drawTexturedQuad(
					topLeft,
					Math::eVec2<float>{length, LINE_WIDTH},
					runeLineTexture_,
					lineColor,
					angle
				);
			}
			else
			{
				std::cout << "[DEBUG] Using fallback rect" << std::endl;
				// Fallback to colored rectangle
				graphics.drawRect(
					topLeft,
					Math::eVec2<float>{length, LINE_WIDTH},
					lineColor,
					true,
					angle
				);
			}
		}
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
	ruptureMark.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("RuptureMark"));
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

	// Get all rupture marks and enemies once at the start
	auto ruptureMarks = entityManager.findEntity<ECSComponents::RuptureMarkComponent, ECSComponents::TransformComponent>();
	auto enemies = entityManager.findEntity<ECSComponents::AIComponent, ECSComponents::TransformComponent>();

	for (auto& entity : ruptureMarks)
	{

		auto& mark = entity.get<ECSComponents::RuptureMarkComponent>(entityManager);
		auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);

		mark.stateTimer += deltaTime;

		switch (mark.state)
		{
		case ECSComponents::RuptureMarkState::SPAWNING:
		{
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

			for (auto& enemyEntity : enemies)
			{
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

				if (!isTargetEnemy)
					continue;

				auto& enemyTransform = enemyEntity.get<ECSComponents::TransformComponent>(entityManager);
				float dist = Math::eVec2Distance(transform.pos, enemyTransform.pos);

				if (dist <= mark.detectionRadius)
				{
					// Enemy triggered the mark! Switch to windup animation
					mark.state = ECSComponents::RuptureMarkState::WINDUP;
					mark.stateTimer = 0.0f;
					mark.triggeredByEntityID = enemyEntity.id();

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

					//std::cout << "[RuptureMark] Triggered by enemy (ID: " << enemyId << ")! Entering WINDUP" << std::endl;
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
			marksToDestroy.push_back(entity.id());
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

	// Get camera position for world-to-screen conversion
	std::vector<ECS::Entity> cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();
	float cameraX = 0.0f;
	float cameraY = 0.0f;

	if (!cameraEntities.empty()) {
		auto& camera = cameraEntities[0].get<ECSComponents::CameraComponent>(entityManager);
		cameraX = camera.position.x - camera.viewportWidth * 0.5f;
		cameraY = camera.position.y - camera.viewportHeight * 0.5f;
	}

	auto floatingTexts = entityManager.findEntity<ECSComponents::FloatingTextComponent, ECSComponents::TransformComponent>();
	for (auto& entity : floatingTexts) {
		auto& floatText = entity.get<ECSComponents::FloatingTextComponent>(entityManager);
		auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);

		// Update elapsed time
		floatText.elapsedTime += deltaTime;

		// Move text upward
		transform.pos.y -= floatText.riseSpeed * deltaTime;

		// Check if expired
		if (floatText.elapsedTime >= floatText.lifetime) {
			textsToDestroy.push_back(entity.id());
		}
		else {
			// Render the floating text
			float alpha = floatText.getCurrentAlpha();

			// Convert world position to screen position
			Math::eVec2<float> screenPos = {
				transform.pos.x - cameraX,
				transform.pos.y - cameraY
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

				// Play fireball hit sound effect
				engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Fireball-Explosion-Small" });

				if (died) {
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

	// Check if we're in Scene2 and all enemies will be dead after destruction -> return to main menu
	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
	std::string sceneName = currentScene.getSceneName();
	bool shouldReturnToMenu = false;

	if (sceneName == "Scene2.txt" && !enemiesToDestroy.empty()) {
		// Count total enemies (including those about to be destroyed)
		auto allEnemies = entityManager.findEntity<ECSComponents::AIComponent, ECSComponents::HealthComponent>();
		// If the number of enemies to destroy equals total enemies, all will be dead
		if (enemiesToDestroy.size() >= allEnemies.size()) {
			std::cout << "[Scene2] All enemies defeated! Returning to main menu..." << std::endl;
			shouldReturnToMenu = true;
		}
	}

	// If returning to menu, clear all entity IDs first to prevent crashes
	if (shouldReturnToMenu) {
		// Destroy pause menu and spellbook if they exist
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

		// Change scene (this will destroy all entities including enemies and fireballs)
		engine_.getEnoEditor().selectScene("MainMenu.txt");
		entitiesDirty_ = true;

		// Recreate main menu buttons after scene load
		setupMainMenuButtons();

		return; // Exit early - don't try to destroy entities since scene change already did it
	}

	// Only destroy entities if we're NOT changing scenes
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


					if (died) {
					}

					// Give player temporary invulnerability after being hit
					playerHealth.invulnerable = true;
					playerHealth.invulnerabilityTimer = 0.5f;
				}
			}
		}
	}
}

// ========== ENEMY AI SYSTEM ==========

void GameApplication::updateEnemyAI(float dt)
{
	auto& entityManager = engine_.getEntityManager();

	// Need player to chase
	if (!playerSpawned_) return;

	ECS::Entity playerEntity(playerEntityID_);
	if (!playerEntity.has<ECSComponents::TransformComponent>(entityManager)) return;
	if (!playerEntity.has<ECSComponents::CollisionComponent>(entityManager)) return;

	auto& playerTransform = playerEntity.get<ECSComponents::TransformComponent>(entityManager);
	auto& playerCollision = playerEntity.get<ECSComponents::CollisionComponent>(entityManager);
	Math::eVec2<float> playerPos = playerTransform.pos;

	// Get player collision radius (for circle) or half-size (for rect)
	float playerRadius = 0.0f;
	if (playerCollision.type == ECSComponents::ColliderType::CIRCLE) {
		playerRadius = playerCollision.data.circle.radius * playerTransform.scale.x;
	} else if (playerCollision.type == ECSComponents::ColliderType::RECTANGLE) {
		playerRadius = std::max(playerCollision.data.rectangle.width, playerCollision.data.rectangle.height) * 0.5f * playerTransform.scale.x;
	}

	// Get all enemies with AI component
	auto enemies = entityManager.findEntity<ECSComponents::AIComponent, ECSComponents::TransformComponent, ECSComponents::CollisionComponent>();

	for (auto& enemy : enemies) {
		auto& ai = enemy.get<ECSComponents::AIComponent>(entityManager);
		auto& transform = enemy.get<ECSComponents::TransformComponent>(entityManager);
		auto& collision = enemy.get<ECSComponents::CollisionComponent>(entityManager);

		// Get enemy collision size
		float enemyRadius = 0.0f;
		if (collision.type == ECSComponents::ColliderType::CIRCLE) {
			enemyRadius = collision.data.circle.radius * transform.scale.x;
		} else if (collision.type == ECSComponents::ColliderType::RECTANGLE) {
			enemyRadius = std::max(collision.data.rectangle.width, collision.data.rectangle.height) * 0.5f * transform.scale.x;
		}

		// Minimum distance to avoid overlap
		float minDistance = playerRadius + enemyRadius + 2.0f; // Small buffer

		// Calculate distance to player
		float distToPlayer = Math::eVec2Distance(transform.pos, playerPos);

		// Only chase if player is within awareness radius
		if (distToPlayer > ai.awarenessRadius) continue;

		// Don't move if already at attack range OR too close (would overlap)
		if (distToPlayer <= ai.attackRange || distToPlayer <= minDistance) continue;

		// Simple direct chase - move toward player
		Math::eVec2<float> direction = { playerPos.x - transform.pos.x, playerPos.y - transform.pos.y };
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

		if (length > 0.1f) {
			// Normalize and apply speed
			direction.x /= length;
			direction.y /= length;

			// Calculate movement amount
			float moveAmount = ai.chaseSpeed * dt;

			// Don't move closer than minDistance
			float maxMove = distToPlayer - minDistance;
			if (moveAmount > maxMove) {
				moveAmount = maxMove;
			}

			if (moveAmount > 0.0f) {
				transform.pos.x += direction.x * moveAmount;
				transform.pos.y += direction.y * moveAmount;
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
			// Only save current scale if player is not already hidden
			// This prevents multiple hidePlayer() calls from overwriting the saved scale
			if (transform.scale.x != 0.0f || transform.scale.y != 0.0f) {
				savedPlayerScale_ = transform.scale;
			}
			// Set scale to 0 to hide
			transform.scale = { 0.0f, 0.0f };
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
		}
	}
}

// ============================================================================
// MAIN MENU BUTTON SYSTEM IMPLEMENTATION
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
	//playBtn.setTargetScene("Scene1.txt");
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

}

void GameApplication::updateMainMenuButtons() {
	auto& entityManager = engine_.getEntityManager();
	auto& inputMng = engine_.getInputManager();

	// Get current scene name to check if we're in main menu
	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
	std::string sceneName = currentScene.getSceneName();

	// Only process main menu buttons if we're in MainMenu scene
	bool inMainMenu = (sceneName == "MainMenu.txt");

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

	// Get mouse position (keep in screen space)
	auto mouseScreenPos = inputMng.getMousePosition();
	Math::eVec2<float> mouseWorldPos = Graphics::screenToWorld(
		{ static_cast<float>(mouseScreenPos.first), static_cast<float>(mouseScreenPos.second) },
		{ 0.0f, 0.0f },
		{ static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) },
		viewportWidth,
		viewportHeight,
		cameraX,
		cameraY
	);

	//std::vector<ECS::Entity> entityList = entityManager.findEntity<ECSComponents::ButtonComponent>();

	//for (auto& ent : entityList)
	//{
	//	auto& c = ent.get<ECSComponents::CollisionComponent>(engine_.getEntityManager());
	//	auto& t = ent.get<ECSComponents::TransformComponent>(engine_.getEntityManager());
	//	auto& d = ent.get<ECSComponents::DrawInfoComponent>(engine_.getEntityManager());
	//	auto& b = ent.get<ECSComponents::ButtonComponent>(engine_.getEntityManager());
	//	if (inputMng.isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
	//	{
	//		// Check if button is active
	//		if (b.getActive())
	//		{
	//			// Convert button position from world space to screen space
	//			Math::eVec2<float> buttonScreenPos = Graphics::worldToScreen(
	//				t.pos,
	//				{ 0.0f, 0.0f },
	//				{ static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) },
	//				viewportWidth,
	//				viewportHeight,
	//				cameraX,
	//				cameraY
	//			);

	//			// Create transform with screen-space position
	//			ECSComponents::TransformComponent screenTransform = t;
	//			screenTransform.pos = buttonScreenPos;

	//			// Mouse position in screen space
	//			Math::eVec2<float> mousePos = {
	//				static_cast<float>(mouseScreenPos.first),
	//				static_cast<float>(mouseScreenPos.second)
	//			};

	//			// Check if mouse pos in clickable area (both now in screen space)
	//			if (b.onClick(mousePos, screenTransform, d, c))
	//			{

	//				// Emit signal
	//				auto signalIt = b.signals_.find("onClick");
	//				if (signalIt != b.signals_.end())
	//				{
	//					signalIt->second.emit();
	//				}
	//			}
	//		}
	//	}
	//}

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
					return;
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
	engine_.getEnoEditor().selectScene("Level1.txt");
	entitiesDirty_ = true;

	// Re-scan for player entity in the new scene
	auto& entityManager = engine_.getEntityManager();
	std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();

	if (!playerEntities.empty()) {
		playerEntityID_ = playerEntities[0].id();
		playerSpawned_ = true;

		// Re-add health component to player if needed
		ECS::Entity playerEntity(playerEntityID_);
		if (!playerEntity.has<ECSComponents::HealthComponent>(entityManager)) {
			playerEntity.add<ECSComponents::HealthComponent>(entityManager, 100.0f);
		}
	} else {
		playerSpawned_ = false;
		playerEntityID_ = 0;
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

	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Load controls texture
	auto controlsTexture = resourceLoader.getTexture("UI-HowToPlay");

	if (!controlsTexture || !controlsTexture->isValid()) {
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

	auto& entityManager = engine_.getEntityManager();
	auto& resourceLoader = engine_.getResourceLoader();

	// Load quit confirm texture
	auto quitTexture = resourceLoader.getTexture("MainMenu-QuitConfirm");

	if (!quitTexture || !quitTexture->isValid()) {
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

	// Request window close to properly shut down the engine
	engine_.getWindowManager().setShouldClose(true);
}


// ============================================================================
// SCENE TRANSITION SYSTEM IMPLEMENTATION
// ============================================================================

//void GameApplication::setupSceneTransitions() {
//	auto& entityManager = engine_.getEntityManager();
//
//	// Create invisible trigger hitbox for Scene1 -> Scene2 transition
//	// Position: (1023, 656), Size: 75x50, Rotation: 0 degrees
//	ECS::Entity sceneTransitionTrigger = entityManager.create();
//	scene1ToScene2TriggerID_ = sceneTransitionTrigger.id();
//
//	sceneTransitionTrigger.add<ECSComponents::NameComponent>(entityManager, ECSComponents::NameComponent("Scene1ToScene2Trigger"));
//	sceneTransitionTrigger.add<ECSComponents::TransformComponent>(entityManager);
//	sceneTransitionTrigger.add<ECSComponents::CollisionComponent>(entityManager);
//
//	auto& transform = sceneTransitionTrigger.get<ECSComponents::TransformComponent>(entityManager);
//	transform.pos = { 1023.0f, 656.0f };
//	transform.scale = { 1.0f, 1.0f };
//
//	auto& collision = sceneTransitionTrigger.get<ECSComponents::CollisionComponent>(entityManager);
//	collision.type = ECSComponents::ColliderType::RECTANGLE;
//	collision.data.rectangle.width = 75.0f;
//	collision.data.rectangle.height = 50.0f;
//	collision.rotation = 0.0f;
//	collision.isTrigger = true;
//	collision.isActive = false; // Start inactive, will activate only in Scene1

//}

//void GameApplication::updateSceneTransitions() {
//	auto& entityManager = engine_.getEntityManager();
//	auto& currentScene = engine_.getEnoEditor().getCurrentScene();
//	std::string sceneName = currentScene.getSceneName();
//
//	// Only check for transitions in Scene1
//	if (sceneName != "Scene1.txt") {
//		// Deactivate trigger if not in Scene1
//		if (scene1ToScene2TriggerID_ != 0) {
//			ECS::Entity trigger(scene1ToScene2TriggerID_);
//			if (trigger.has<ECSComponents::CollisionComponent>(entityManager)) {
//				auto& collision = trigger.get<ECSComponents::CollisionComponent>(entityManager);
//				collision.isActive = false;
//			}
//		}
//		return;
//	}
//
//	// Activate trigger in Scene1
//	if (scene1ToScene2TriggerID_ != 0) {
//		ECS::Entity trigger(scene1ToScene2TriggerID_);
//		if (trigger.has<ECSComponents::CollisionComponent>(entityManager)) {
//			auto& collision = trigger.get<ECSComponents::CollisionComponent>(entityManager);
//			collision.isActive = true;
//		}
//	}
//
//	// Check if player exists
//	if (!playerSpawned_ || playerEntityID_ == 0) {
//		return;
//	}
//
//	// Get player entity
//	ECS::Entity player(playerEntityID_);
//	if (!player.has<ECSComponents::TransformComponent>(entityManager) ||
//		!player.has<ECSComponents::CollisionComponent>(entityManager)) {
//		return;
//	}
//
//	// Get player components
//	auto& playerTransform = player.get<ECSComponents::TransformComponent>(entityManager);
//	auto& playerCollision = player.get<ECSComponents::CollisionComponent>(entityManager);
//
//	// Get trigger entity
//	ECS::Entity trigger(scene1ToScene2TriggerID_);
//	if (!trigger.has<ECSComponents::TransformComponent>(entityManager) ||
//		!trigger.has<ECSComponents::CollisionComponent>(entityManager)) {
//		return;
//	}
//
//	auto& triggerTransform = trigger.get<ECSComponents::TransformComponent>(entityManager);
//	auto& triggerCollision = trigger.get<ECSComponents::CollisionComponent>(entityManager);
//
//	// Simple AABB collision check (rectangle-to-rectangle)
//	float playerLeft = playerTransform.pos.x - (playerCollision.data.rectangle.width * 0.5f);
//	float playerRight = playerTransform.pos.x + (playerCollision.data.rectangle.width * 0.5f);
//	float playerTop = playerTransform.pos.y - (playerCollision.data.rectangle.height * 0.5f);
//	float playerBottom = playerTransform.pos.y + (playerCollision.data.rectangle.height * 0.5f);
//
//	float triggerLeft = triggerTransform.pos.x - (triggerCollision.data.rectangle.width * 0.5f);
//	float triggerRight = triggerTransform.pos.x + (triggerCollision.data.rectangle.width * 0.5f);
//	float triggerTop = triggerTransform.pos.y - (triggerCollision.data.rectangle.height * 0.5f);
//	float triggerBottom = triggerTransform.pos.y + (triggerCollision.data.rectangle.height * 0.5f);
//
//	// Check for overlap
//	bool colliding = !(playerRight < triggerLeft || playerLeft > triggerRight ||
//	                   playerBottom < triggerTop || playerTop > triggerBottom);
//
//	if (colliding) {
//		engine_.getEnoEditor().selectScene("Scene2.txt");
//		entitiesDirty_ = true;
//
//		// Re-scan for player in new scene
//		std::vector<ECS::Entity> playerEntities = entityManager.findEntity<ECSComponents::PlayerComponent>();
//		if (!playerEntities.empty()) {
//			playerEntityID_ = playerEntities[0].id();
//			playerSpawned_ = true;
//		} else {
//			playerSpawned_ = false;
//			playerEntityID_ = 0;
//		}
//	}
//}

// ============================================================================
// PAUSE MENU BUTTON SYSTEM IMPLEMENTATION
// ============================================================================

void GameApplication::setupPauseMenuButtons() {
	auto& entityManager = engine_.getEntityManager();

	// Resume Button at (716, 965) with size (100x100) rotated 45 degrees
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
	resumeTransform.rot.x = 0.785398f; // 45 degrees in radians

	auto& resumeDraw = resumeButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	resumeDraw.width = 100.0f;
	resumeDraw.height = 100.0f;
	resumeDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(resumeDraw.shape, "Rectangle");

	auto& resumeCollision = resumeButton.get<ECSComponents::CollisionComponent>(entityManager);
	resumeCollision.type = ECSComponents::ColliderType::RECTANGLE;
	resumeCollision.data.rectangle.width = 100.0f;
	resumeCollision.data.rectangle.height = 100.0f;
	resumeCollision.rotation = 0.785398f; // 45 degrees in radians
	resumeCollision.isTrigger = true;

	auto& resumeBtn = resumeButton.get<ECSComponents::ButtonComponent>(entityManager);
	resumeBtn.getActive() = true;

	// Main Menu Button at (557, 1141) with size (56x56) rotated 45 degrees
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
	menuTransform.rot.x = 0.785398f; // 45 degrees in radians

	auto& menuDraw = mainMenuButton.get<ECSComponents::DrawInfoComponent>(entityManager);
	menuDraw.width = 56.0f;
	menuDraw.height = 56.0f;
	menuDraw.color = Graphics::Color(0.0f, 0.0f, 0.0f, 0.0f); // Invisible
	strcpy_s(menuDraw.shape, "Rectangle");

	auto& menuCollision = mainMenuButton.get<ECSComponents::CollisionComponent>(entityManager);
	menuCollision.type = ECSComponents::ColliderType::RECTANGLE;
	menuCollision.data.rectangle.width = 56.0f;
	menuCollision.data.rectangle.height = 56.0f;
	menuCollision.rotation = 0.785398f; // 45 degrees in radians
	menuCollision.isTrigger = true;

	auto& menuBtn = mainMenuButton.get<ECSComponents::ButtonComponent>(entityManager);
	menuBtn.getActive() = true;

}

void GameApplication::handleResumeButton() {

	// Close pause menu and resume game
	isPaused_ = false;
	destroyPauseMenu();
	showPlayer();
	engine_.setIsPlaying(true);
	engine_.getEventBus().publish(Audio::PlayInstant{ "SFX", "Spellbook_Close_01" });
}

void GameApplication::handleMainMenuButton() {

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
