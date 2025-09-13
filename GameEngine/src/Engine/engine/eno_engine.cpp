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
		ECS::Entity enemy = ent.create<ECSComponents::VisibilityComponent, ECSComponents::NameComponent>(
			ECSComponents::VisibilityComponent{ 5, true }, "Selestia");

		// add and remove
		enemy.add<ECSComponents::TagComponent>("Human");
		enemy.remove<ECSComponents::NameComponent>();

		auto& enemyZIdx = enemy.get<ECSComponents::VisibilityComponent>();
		std::cout << "enemy's z_layer: " << enemyZIdx.z_index << std::endl;
		std::cout << std::boolalpha << "enemy visible?: " << enemyZIdx.visible << std::endl;

		std::cout << std::endl;

		// set and get
		enemy.set<ECSComponents::VisibilityComponent>({ 7, false });
		std::cout << "enemy's z_layer: " << enemyZIdx.z_index << std::endl;
		std::cout << std::boolalpha << "enemy visible?: " << enemyZIdx.visible << std::endl;

		// checking if the entity has a specific component
		bool hasName = enemy.hasComponent<ECSComponents::NameComponent>();
		bool hasTag = enemy.hasComponent<ECSComponents::TagComponent>();

		std::cout << "Does enemy have a name: " << hasName << std::endl;
		std::cout << "Does enemy have a tag: " << hasTag << std::endl;
		std::cout << std::endl;

		// Clone the enemy into the hero and the hero
		// will receive the same components and component values
		ECS::Entity hero = enemy.clone();
		ECS::Entity widow = enemy.clone();
		
		enemy.print();
		hero.print();
		widow.print();

		//checking tag addition
		auto& heroVisibility = hero.get<ECSComponents::VisibilityComponent>();
		std::cout << "hero's tag: " << heroVisibility.z_index << std::endl;

		auto& enemyVisibility = enemy.get<ECSComponents::VisibilityComponent>();
		std::cout << "enemy's tag: " << enemyVisibility.z_index << std::endl;

		// Creating an entity with presets
		//ECS::Entity villager = ent.presetCreate();

		// checking if an entity is destroyed properly
		enemy.destroy();

		// Save entities which are still alive
		ent.save();

		// creating a signal
		Events::Signal<int> signal;
		signal.connect([](int a)
			{
				std::cout << "Damage: " << a << std::endl;
			});
		signal.emit(50);

		Events::Signal<> signal2;
		signal2.connect([]()
			{
				std::cout << "No params passed" << std::endl;
			});
		signal2.emit();

		// Audio
		audioMng_.setChannelGroup("BGM");

		// Modifying the audio
		audioMng_.modifyChannelGroup(
			"BGM", 
			Audio::SoundParameters{ 
				0.01f, 1.f, 0.5f, 
			Audio::Reverb{ FMOD_DSP_TYPE_SFXREVERB, 1.f, 1.f, 1.f, 1.f },
			Audio::EQ{ FMOD_DSP_TYPE_PARAMEQ, 1.f, 1.f, 1.f }
			});

		// Math demonstration
		Math::eVec3 vec{ 1, 2, 3 };
		Math::eVec3 vec2{ 4, 5, 6 };
		vec += vec2;
		std::cout << "vec v1: " << vec.v1 << std::endl;
		std::cout << "vec v2: " << vec.v2 << std::endl;
		std::cout << "vec v3: " << vec.v3 << std::endl;

		// vec += 3.5f; not allowed!! because vec is made out of ints
		vec += 3;

		std::cout << "vec v1: " << vec.v1 << std::endl;
		std::cout << "vec v2: " << vec.v2 << std::endl;
		std::cout << "vec v3: " << vec.v3 << std::endl;

		// Event bus demonstration
		// 
		// Explanation:
		// [&](const Audio::RequestSound& event
		// is a lambda, which is just a callable object
		// This lambda (callable object) is being passed in to subscribe.
		// 
		// The subscribe param is of type std::function<void(const Event&)>
		// Here, we pass an object of std::function<void(const Audio::RequestSound&)>
		// which would later be wrapped into a type-erased lambda
		// so the object stored in the map is a std::function<void(const void*)>
		// 
		// [&] is a capture list, which we use a simple & to capture loader_ for
		// the onSoundRequest to call when we publish the request from audio manager
		bus_.subscribe<Audio::RequestSound>
		(
			[&](const Audio::RequestSound& event){loader_.onSoundRequest(event);}
		);

		bus_.subscribe<Audio::LoadSound>([&](const Audio::LoadSound& event)
			{
				audioMng_.onSoundReceived(event);
			});

		// Publish the sound request for these two sounds and 
		// process them in the audioMng_
		audioMng_.publishSoundRequest("Close_To_The_End");
		audioMng_.publishSoundRequest("Simulacrum");

		// Window loop
		while (!glfwWindowShouldClose(window_.getWindow()))
		{
			glfwPollEvents();

			audioMng_.playByGroup("Close_To_The_End", "BGM");
		}

		//enemy.set<ECSComponents::NameComponent>({ "Cynthia" });

		//enemyName = enemy.get<ECSComponents::NameComponent>();
		//std::cout << "enemy2's new name: " << enemyName.name << std::endl;

		//enemy.set<ECSComponents::NameComponent>({ "Iusia" });

		//enemyName = enemy.get<ECSComponents::NameComponent>();
		//std::cout << "enemy2's new name: " << enemyName.name << std::endl;

		//ECS::Entity player = ent.presetCreate(ECS::Preset::DEFAULT{});
		//ECS::Entity player2 = ent.presetCreate(ECS::Preset::DEFAULT{});
		//ECS::Entity npc = ent.create<Component::NameComponent>({ "HI" });

		//auto& enemyName = enemy.get<ECSComponents::NameComponent>();
		//std::cout << "player's name is " << enemyName.name << std::endl;

		// Setting values with .set and getting them with .get
		//enemy.set<ECSComponents::NameComponent>({ "Cynthia" });
		//enemy2.set<ECSComponents::NameComponent>({ "Isea" });
		//player.set<ECSComponents::NameComponent>({ "Mirast" });
		//player2.set<ECSComponents::NameComponent>({ "Syrus" });
		
		//auto& playerName = player.get<ECSComponents::NameComponent>();
		//std::cout << "player's name is " << playerName.name << std::endl;
		// 
		//auto& player2Name = player2.get<ECSComponents::NameComponent>();
		//std::cout << "player2's name is " << player2Name.name << std::endl;

		//player2.set<ECSComponents::NameComponent>({ "Melody" });

		//player2Name = player2.get<ECSComponents::NameComponent>();
		//std::cout << "player2's new name is " << player2Name.name << std::endl;

		//auto& enemyName = enemy.get<ECSComponents::NameComponent>();
		//std::cout << "enemy's name is " << enemyName.name << std::endl;

		// Adding component
		//player.add<ECSComponents::ModulationComponent, ECSComponents::LayerComponent>();


		std::cout << std::endl;
		std::cout << std::endl;


		// Loop
		//while (!glfwWindowShouldClose(window_.getWindow()))
		//{
		//	glfwPollEvents();

		//	device_.drawFrame();
		//}

		//vkDeviceWaitIdle(device_.getDevice());
	}
}