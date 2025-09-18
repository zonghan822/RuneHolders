#include "eno_engine.hpp"


namespace Engine
{
	void EnoEngine::setup()
	{
		// Loading sound assets from two directories
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getMusicPath());
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getSFXPath());

		// Loading image assets
		
		
		// Loading shader assets
		loader_.loadShaders(loader_.getShaderPath());

		// Getting Shaders
		auto vert = loader_.getVertShader("shader");
		auto frag = loader_.getFragShader("shader");

		// Print all registered component IDs
		compReg_.PrintIDs();

		run();
	}

	void EnoEngine::run()
	{
		std::cout << std::endl;
		std::cout << std::endl;
		ECS::EntityManager ent{ compReg_ };

		// ECS Demonstration
		
		// EXAMPLES
		
		// Entity creation
		// Default initialization and set new values
		ECS::Entity ui = ent.presetCreate<ECSComponents::Preset::DEFAULT>();
		ECS::Entity io = ent.create<ECSComponents::NameComponent>();

		auto& uiName = ui.get<ECSComponents::NameComponent>();
		auto& ioName = io.get<ECSComponents::NameComponent>();
		std::cout << "ui's name: " << uiName << std::endl;
		std::cout << "io's name: " << ioName << std::endl;

		ui.set<ECSComponents::NameComponent>("ui");
		io.set<ECSComponents::NameComponent>("io");
		std::cout << "ui's name: " << uiName << std::endl;
		std::cout << "io's name: " << ioName << std::endl;

		// -----------------------------------------------------

		// Initialization with values
		ECS::Entity villager = ent.presetCreate<ECSComponents::Preset::DEFAULT>(
			"Villager A", ECSComponents::VisibilityComponent{ 7, true }, "Lava layer");

		auto& villagerName = villager.get<ECSComponents::NameComponent>().name;
		auto& villagerLayer = villager.get<ECSComponents::LayerComponent>().layer;
		auto& villagerZ = villager.get<ECSComponents::VisibilityComponent>().z_index;
		auto& villagerVisible = villager.get<ECSComponents::VisibilityComponent>().visible;
		std::cout << "Villager name is " << villagerName << std::endl;
		std::cout << "Villager layer is " << villagerLayer << std::endl;
		std::cout << "Villager Z is " << villagerZ << std::endl;
		std::cout << "Villager visible is " << villagerVisible << std::endl;

		std::cout << std::endl;
		// Events
		
		// Loading Sound
		// Audio && Loader
		bus_.subscribe<Audio::RequestSound>([&](const Audio::RequestSound& event) //1
			{
				loader_.onSoundRequest(event);
			});

		bus_.subscribe<Audio::LoadSound>([&](const Audio::LoadSound& event)
			{
				audioMng_.onSoundReceived(event);
			});

		audioMng_.publishSoundRequest("Close_To_The_End");   //2
		audioMng_.publishSoundRequest("Simulacrum");

		// Registering tags
		bus_.subscribe<Tag::RecordTag>([&](const Tag::RecordTag& event)
			{
				tagReg_.registerTag(event);
			});

		// Scene Playing
		// Game Instance and Window manager
		uint32_t id = bus_.subscribe<GameInstance::PlayScene>([&](const GameInstance::PlayScene& event) 
			{
				gameInstance_.get()->playScene();
			});

		bus_.subscribe<GameInstance::PauseScene>([&](const GameInstance::PauseScene& event)
			{
				gameInstance_.get()->pauseScene();
			});

		bus_.subscribe<GameInstance::StopScene>([&](const GameInstance::StopScene& event)
			{
				gameInstance_.get()->stopScene();
			});

		bus_.subscribe<GameInstance::CreateGameWindow>([&](const GameInstance::CreateGameWindow& event)
			{
				window_.gameWindowInit();
			});

		bus_.subscribe<GameInstance::DestroyGameWindow>([&](const GameInstance::DestroyGameWindow& event)
			{
				window_.destroyGameWindow();
			});

		// Unsubcribe using the id assigned through the subscribe
		//bus_.unsubscribe<GameInstance::PlayScene>(id);

		// Audio
		// Setting a channel group
		audioMng_.setChannelGroup("BGM");

		// Modifying the audio
		Audio::SoundParameters sound;
		Audio::ParamEQ paramEQ;
		Audio::SFXReverb sfxReverb;
		paramEQ = Audio::ParamEQ{ 3.f, 1000.f, 1.f };
		// *Important: create an empty Reverb/EQ object before setting values
		// then assign the values such as type or the any chosen EQ
		sound.volume = 0.1f;
		sound.eq = Audio::EQ{};
		sound.eq->type = FMOD_DSP_TYPE_PARAMEQ;
		sound.eq->paramEQ = paramEQ;
		sound.reverb = Audio::Reverb{};
		sound.reverb->type = FMOD_DSP_TYPE_SFXREVERB;
		sound.reverb->sfx = sfxReverb;
		audioMng_.modifyChannelGroup("BGM", sound);

		// 1
		// Connecting an input to a name
		// Setting up input
		input_.setupKey("Play Scene", GLFW_KEY_F5, glfwGetKeyScancode(GLFW_KEY_F5), GLFW_RELEASE, 0);
		input_.setupKey("Pause Scene", GLFW_KEY_F7, glfwGetKeyScancode(GLFW_KEY_F7), GLFW_RELEASE, 0);
		input_.setupKey("Stop Scene", GLFW_KEY_F8, glfwGetKeyScancode(GLFW_KEY_F8), GLFW_RELEASE, 0);

		input_.setupEvent<GameInstance::PlayScene>("Play Scene");
		input_.setupEvent<GameInstance::PauseScene>("Pause Scene");
		input_.setupEvent<GameInstance::StopScene>("Stop Scene");

		// 2
		// Connecting an input key to doing something
		// When I click on the K key, I want to stop the audio
		// So input manager has to listen for the K key,
		// Figure out the mods, action and all that
		// Then form subscription during runtime with
		// an engine component in this case being audio manager
		// and tell it to stop the audio

		// Initializing a game instance
		createGameInstance();

		// Test added tag in game instance
		tagReg_.print();

		// Window loop
		bool scheduled{ false };
		while (!glfwWindowShouldClose(window_.getWindow()))
		{
			glfwPollEvents();

			// Setting up fade and playing the sound
			if (!scheduled)
			{
				FMOD::Channel* channel = audioMng_.playByGroup("Close_To_The_End", "BGM", true);

				audioMng_.scheduleGroupFade("BGM", 5.0f, 5.0f, audioMng_.getSoundDurSec("Close_To_The_End"));

				//audioMng_.togglePauseGroup("BGM", false);
				audioMng_.setPauseChannel(channel, false);

				scheduled = true;
			}

			audioMng_.process();
		}

		std::cout << std::endl;
	}

	// Create a game instance
	void EnoEngine::createGameInstance()
	{
		// Initialization of game instance
		gameInstance_ = std::make_unique<GameInstance::EnoGameInstance>(bus_);
	}
}