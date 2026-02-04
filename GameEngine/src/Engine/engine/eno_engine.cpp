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
		configMng_.retrieveSettings();
		auto [w, h] = configMng_.getWindowDim();
		bool fullscreen = configMng_.getFullscreen();
		bool vsync = configMng_.getVSync();

		window_.initialize("RuneHolders", w, h, fullscreen, vsync);

		auto [tmW, tmH, tmCell] = configMng_.getTilemapConfig();
		tilemapSystem_ = std::make_unique<Map::TileMapSystem>(bus_, tmW, tmH, tmCell);
		pathfinder_ = std::make_unique<AI::PathfindingSystem>(bus_, *tilemapSystem_);

		// Systems init
		inputMng_.initialize();
		physicsSys_ = std::make_unique<Physics::PhysicsSystem>(bus_);
		stateMachineSys_ = std::make_unique<FSM::StateMachineSystem>();
		collisionSys_ = std::make_unique<Collision::CollisionSystem>(bus_);
		graphicsMng_.initialize(window_);
		audioMng_.init();

		// Initialize editor after tilemap is created
		editor_ = std::make_unique<Editor::EnoEditor>(bus_, entityMng_, coreHost_, *tilemapSystem_);

		Bridge::Bridge_Init(this);
		setupResourceLoading();
	}

	// Thats why i added a callback version
	void EnoEngine::run(std::function<void()> gameUpdateCallback)
	{
		while (!window_.shouldClose())
		{
			updateTime(1.0);
			window_.pollEvents();
			coreHost_.areScriptsModified();

			float deltaTime = static_cast<float>(dt_);

			// Update physics system (gravity, forces, velocity integration)
			// Only update physics when the game is playing (not paused/stopped)
			if (physicsSys_ && isPlaying_) {
				physicsSys_->Update(entityMng_, deltaTime);
			}

			// Update all state machines
			if (isPlaying_) stateMachineSys_->update(editor_->getSceneTree(), entityMng_, deltaTime);

			// Process any state change requests from scripts
			if (isPlaying_) stateMachineSys_->processStateChanges(editor_->getSceneTree(), entityMng_);

			// Update collision system (collision detection and response)
			// Only update collision when the game is playing (not paused/stopped)
			if (collisionSys_ && isPlaying_) {
				collisionSys_->Update(entityMng_);
			}

			// Begin graphics frame
			graphicsMng_.beginFrame(Graphics::Color::Black);  // Clear to black

			// Update audio system
			if (isPlaying_) audioMng_.process();

			// Call game logic if provided
			if (gameUpdateCallback) {
				gameUpdateCallback();
			}

			// End graphics frame and present
			graphicsMng_.endFrame(window_);

			inputMng_.endFrame();
		}

	}

	void EnoEngine::shutDown() {
		// Clean up graphics system resources
		graphicsMng_.shutdown();
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