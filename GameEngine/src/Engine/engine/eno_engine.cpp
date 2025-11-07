/*============================================================================================
* @file         eno_engine.cpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author		Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief		Implementation of the core engine systems and main game loop
*				
*				This file contains the implementation of the EnoEngine class which serves as
*				the central coordinator for all engine subsystems. It manages the initialization,
*				main game loop execution, resource loading, time management, and shutdown procedures.
*				The engine follows a component-based architecture where each system (graphics, audio,
*				input, etc.) is managed independently but coordinated through the main engine class.
*//*========================================================================================*/

#include "eno_engine.hpp"

namespace Engine
{
	void EnoEngine::setup()
	{
		window_.initialize("RuneHolders", configMng_.getWindowDim().first, configMng_.getWindowDim().second);

		// Systems init
		inputMng_.initialize();
		graphicsMng_.initialize(window_);
		audioMng_.init();

		setupResourceLoading();

		// Initialize game systems
		setupGameSystems();
	}

	// Thats why i added a callback version
	void EnoEngine::run(std::function<void()> gameUpdateCallback)
	{
		while (!window_.shouldClose())
		{
			updateTime(1.0);
			window_.pollEvents();
			coreHost_.areScriptsModified();

			// Handle window resize for graphics system
			auto fbSize = window_.getFramebufferSize();
			int width = fbSize.first, height = fbSize.second;
			if (width > 0 && height > 0) {
				graphicsMng_.setProjectionMatrix(width, height);
			}

			// Begin graphics frame
			graphicsMng_.beginFrame(Graphics::Color::Black);  // Clear to black

			// Update game systems first (physics, collision, player)
			updateGameSystems(static_cast<float>(dt_), isPlaying_);

			// Call game logic if provided
			if (gameUpdateCallback) {
				gameUpdateCallback();
			}
			// Update AI State
			aiMng_.update(entityMng_, dt_);

			audioMng_.process();

			// End graphics frame and present
			graphicsMng_.endFrame(window_);

			inputMng_.endFrame();
		}

	}

	void EnoEngine::shutDown() {

	}

	// Loader
	void EnoEngine::setupResourceLoading()
	{
		// Loading assets from directories
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getMusicPath());
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getSFXPath());

		// Loading shaders
		loader_.loadShaders(loader_.getShaderPath());

		// Loading images
		loader_.loadImages(loader_.getImagePath());

		// Loading fonts
		loader_.loadFonts(loader_.getFontPath(), 48);
	}

	/*============================================================================================
	* \brief		Initializes game systems (physics, collision, player)
	*//*========================================================================================*/
	void EnoEngine::setupGameSystems()
	{
		// Initialize physics system
		physicsSys_ = std::make_unique<Physics::PhysicsSystem>();

		// Initialize collision system (needs event bus reference)
		collisionSys_ = std::make_unique<Collision::CollisionSystem>(bus_);

		// Initialize player system
		playerSys_ = std::make_unique<Player::PlayerSystem>();

		std::cout << "[EnoEngine] Game systems initialized successfully" << std::endl;
	}

	/*============================================================================================
	* \brief		Updates all game systems in proper order
	* \param[in]	deltaTime - Time elapsed since last frame
	* \param[in]	isPlaying - Whether the game is in playing mode (not editor mode)
	*//*========================================================================================*/
	void EnoEngine::updateGameSystems(float deltaTime, bool isPlaying)
	{
		// Update systems in correct order:
		// 1. Player input/movement
		// 2. Physics simulation
		// 3. Collision detection

		// Update player system (handles input and player movement)
		if (playerSys_) {
			playerSys_->Update(entityMng_, inputMng_, deltaTime, isPlaying);
		}

		// Update physics system (gravity, forces, velocity integration)
		// Only update physics when the game is playing (not paused/stopped)
		if (physicsSys_ && isPlaying) {
			physicsSys_->Update(entityMng_, deltaTime);
		}

		// Update collision system (collision detection and response)
		// Only update collision when the game is playing (not paused/stopped)
		if (collisionSys_ && isPlaying) {
			collisionSys_->Update(entityMng_);
		}
	}

	void EnoEngine::updateTime(double fps_calc_interval) {
		// get elapsed time (in seconds) between previous and current frames
		static double prev_time = glfwGetTime();
		double curr_time = glfwGetTime();
		dt_ = curr_time - prev_time;
		prev_time = curr_time;

		// fps calculations
		static double count = 0.0; // number of game loop iterations
		static double start_time = glfwGetTime();
		// get elapsed time since very beginning (in seconds) ...
		double elapsed_time = curr_time - start_time;

		++count;

		// update fps at least every 10 seconds ...
		fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
		fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
		if (elapsed_time > fps_calc_interval) {
			fps_ = count / elapsed_time;
			start_time = curr_time;
			count = 0.0;
		}
	}
}