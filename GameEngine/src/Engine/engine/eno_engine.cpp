#include "eno_engine.hpp"

namespace Engine
{
	void EnoEngine::setup()
	{
		// Engine init
		setupResourceLoading();
		setupEventSystem();
		setupGraphicsSystem();

		// For debugging
		compReg_.PrintIDs();

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
		soundSetup();

		//audioMng_.publishSoundRequest("Close_To_The_End");
		//audioMng_.publishSoundRequest("Simulacrum");

		while (!glfwWindowShouldClose(window_.getWindow()))
		{
			glfwPollEvents();

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

		audioMng_.process();
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
		bus_.subscribe<Audio::RequestSound>
			(
				[&](const Audio::RequestSound& event) {loader_.onSoundRequest(event);}
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
		soundPlaying();
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

	void EnoEngine::soundPlaying()
	{
		FMOD::Channel* channel = audioMng_.playByGroup("Close_To_The_End", "BGM", true);
		FMOD::Channel* channel2 = audioMng_.playByGroup("Simulacrum", "BGM", true);

		audioMng_.scheduleGroupFade("BGM", 5.0f, 5.0f, audioMng_.getSoundDurSec("Close_To_The_End"));

		audioMng_.setPauseChannel(channel, false);
		audioMng_.setPauseChannel(channel2, false);
	}
}