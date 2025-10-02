#include "eno_engine.hpp"

namespace Engine
{
	void EnoEngine::setup()
	{
		// Engine init
		setupResourceLoading();
		setupEventSystem();
		setupGraphicsSystem();
		loader_.loadImages(loader_.getImagePath());

		// Temporary(?) Audio-Input setup
		setupAudioInput();

		soundSetup();

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
		while (!glfwWindowShouldClose(window_.getWindow()))
		{
			glfwPollEvents();

			update_time(1.0);

			// Handle window resize for graphics system
			int width, height;
			glfwGetFramebufferSize(window_.getWindow(), &width, &height);
			if (width > 0 && height > 0) 
			{
				graphicsMng_->setProjectionMatrix(width, height);
			}

			// Begin graphics frame
			if (graphicsMng_ && graphicsMng_->isInitialized())
			{
				graphicsMng_->beginFrame(Graphics::Color::Black);  // Clear to black
			}

			// Call game logic if provided
			if (gameUpdateCallback) {
				gameUpdateCallback();
			}

			// Update engine systems
			updateEngineSystems();

			// End graphics frame and present
			if (graphicsMng_ && graphicsMng_->isInitialized())
			{
				graphicsMng_->endFrame();
			}
		}

	}

	// Function added for future use**
	void EnoEngine::setupResourceLoading()
	{
		// Loading assets from directories
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getMusicPath());
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getSFXPath());
		loader_.loadShaders(loader_.getShaderPath());

		// Shaders for future graphics use
		auto vert = loader_.getVertShader("shader");//might need to change the name on this
		auto frag = loader_.getFragShader("shader");
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

	// *** For audio button demonstration purposes
	// *** maybe to be removed!!!
	void EnoEngine::setupAudioInput()
	{
		input_.setupKey("Play Audio", GLFW_KEY_O, glfwGetKeyScancode(GLFW_KEY_O), GLFW_RELEASE, 0);
		input_.setupKey("Set New Audio", GLFW_KEY_P, glfwGetKeyScancode(GLFW_KEY_P), GLFW_RELEASE, 0);
		input_.setupKey("Set New Audio2", GLFW_KEY_L, glfwGetKeyScancode(GLFW_KEY_L), GLFW_RELEASE, 0);
		input_.setupEvent<Audio::SetAudioState>("Play Audio", "BGM");
		input_.setupEvent<Audio::SetNewAudio>("Set New Audio", "Close_To_The_End");
		input_.setupEvent<Audio::SetNewAudio>("Set New Audio2", "Simulacrum");

		bus_.subscribe<Audio::SetAudioState>([&](const Audio::SetAudioState& event)
			{
				audioMng_.onTogglePlay(event);
			});

		bus_.subscribe<Audio::SetNewAudio>([&](const Audio::SetNewAudio& event)
			{
				audioMng_.onAudioChange(event);
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

		audioMng_.process();

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
		dt = curr_time - prev_time;
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
			fps = count / elapsed_time;
			start_time = curr_time;
			count = 0.0;
		}
		input_.endFrame();
	}

	// Not sure if we need it
	ECS::EntityManager& EnoEngine::getEntityManager()
	{
		static ECS::EntityManager manager(compReg_);
		return manager;
	}


	/// <summary>
	///  TBD: for sound demoing!!!
	/// </summary>
	void EnoEngine::soundSetup()
	{
		audioMng_.setChannelGroup("BGM");

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
		audioMng_.modifyChannelGroup("BGM", sound);

		audioMng_.publishSoundRequest("Close_To_The_End");
		audioMng_.publishSoundRequest("Simulacrum");
	}
}