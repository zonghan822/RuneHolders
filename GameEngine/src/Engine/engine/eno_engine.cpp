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
		setupEventSystem();
		setupGraphicsSystem();
		loader_.loadImages(loader_.getImagePath());
		// For debugging
		compReg_.DisplayReg();

		// Removed all your previous game codes like math and shi
		
	}

	// Simpler run not sure how to build on but yeah
	void EnoEngine::run()
	{
		run(nullptr);
	}

	// Thats why i added a callback version
	void EnoEngine::run(std::function<void()> gameUpdateCallback)
	{
		while (!window_.shouldClose())
		{
			glfwPollEvents();

			update_time(1.0);

			// Handle window resize for graphics system
			auto fbSize= window_.getFramebufferSize();
			int width = fbSize.first, height = fbSize.second;
			if (width > 0 && height > 0) 
			{
				graphicsMng_.setProjectionMatrix(width, height);
			}

			// Begin graphics frame
			graphicsMng_.beginFrame(Graphics::Color::Black);  // Clear to black
			
			// Call game logic if provided
			if (gameUpdateCallback) {
				gameUpdateCallback();
			}

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

	// Function added for future use**
	void EnoEngine::setupEventSystem()
	{
		// Setup core engine event handlers
		bus_.subscribe<Audio::RequestSound>([&](const Audio::RequestSound& event) 
			{
				loader_.onSoundRequest(event);}
			);

		bus_.subscribe<Audio::LoadSound>([&](const Audio::LoadSound& event)
			{
				audioMng_.onSoundReceived(event);
			});

		bus_.subscribe<Graphics::Events::WindowResized>(
			[&](const Graphics::Events::WindowResized& event) {
				if (graphicsMng_ && graphicsMng_->isInitialized())
				{
					graphicsMng_->setProjectionMatrix(event.width, event.height);
				}
			});
	}

	void EnoEngine::setupGraphicsSystem()
	{
		// Create graphics manager
		graphicsMng_ = std::make_unique<Graphics::EnoGraphicsManager>(bus_, window_.getWindow());

		// Initialize the graphics system
		if (!graphicsMng_->initialize())
		{
			throw std::runtime_error("Failed to initialize graphics system!");
		}

		std::cout << "Graphics system setup complete!" << std::endl;
	}

	// Create a game instance
	void EnoEngine::createGameInstance()
	{
		// Initialization of game instance
		gameInstance_ = std::make_unique<GameInstance::EnoGameInstance>(bus_);
	}

	// Function added for future use**
	void EnoEngine::updateEngineSystems()
	{
		// Update engine systems each frame
		// Add any per-frame engine updates here
		static bool scheduled{ false };
		if (!scheduled)
		{
			//soundPlaying();
			scheduled = true;
		}

		audioMng_.process();
		audioMng_.Update();

		// Graphics update is handled in the main loop (beginFrame/endFrame)
		// but we call Update() here for consistency with the system interface
		if (graphicsMng_)
		{
			graphicsMng_->Update();
		}
	}

	void EnoEngine::update_time(double fps_calc_interval) {
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