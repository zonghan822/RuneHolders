#include "eno_engine.hpp"

namespace Engine
{
	void EnoEngine::setup()
	{
		// Engine init
		setupResourceLoading();
		setupEventSystem();

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
		ECS::Entity f = getEntityManager().create<ECSComponents::NameComponent>("Circle");
		f.add<ECSComponents::VisibilityComponent>(ECSComponents::VisibilityComponent{5, true});
		//f.add<ECSComponents::VisibilityComponent>();

		auto& index = f.get<ECSComponents::VisibilityComponent>().z_index;
		std::cout << "Index: " << index << std::endl;

		while (!glfwWindowShouldClose(window_.getWindow()))
		{
			glfwPollEvents();

			// Call game logic if provided
			if (gameUpdateCallback) {
				gameUpdateCallback();
			}

			// Update engine systems
			updateEngineSystems();
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
		bus_.subscribe<Audio::RequestSound>
			(
				[&](const Audio::RequestSound& event) {loader_.onSoundRequest(event);}
			);

		bus_.subscribe<Audio::LoadSound>([&](const Audio::LoadSound& event)
			{
				audioMng_.onSoundReceived(event);
			});
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
	}

	// Not sure if we need it
	ECS::EntityManager& EnoEngine::getEntityManager()
	{
		static ECS::EntityManager manager(compReg_);
		return manager;
	}
}