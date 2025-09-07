#include "eno_engine.hpp"


namespace Engine
{
	void EnoEngine::setup()
	{
		// Needs to be changed to remove this block in the future
		compReg_.RegisterComponent<ECSComponents::NameComponent>();
		compReg_.RegisterComponent<ECSComponents::LayerComponent>();
		compReg_.RegisterComponent<ECSComponents::TagComponent>();
		compReg_.RegisterComponent<ECSComponents::TransformComponent>();
		compReg_.RegisterComponent<ECSComponents::ModulationComponent>();
		compReg_.RegisterComponent<ECSComponents::VisibilityComponent>();

		compReg_.printIDs();

		run();
	}

	void EnoEngine::run()
	{
		std::cout << std::endl;
		std::cout << std::endl;
		ECS::EntityManager ent{ compReg_ };

		// Initializing the entities
		
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

		// checking tag addition
		auto& enemyTag = enemy.get<ECSComponents::TagComponent>();
		std::cout << "enemy's tag: " << enemyTag.tag << std::endl;
		
		// checking if the entity has a specific component
		bool hasName = enemy.hasComponent<ECSComponents::NameComponent>();
		bool hasTag = enemy.hasComponent<ECSComponents::TagComponent>();

		std::cout << "Does enemy have a name: " << hasName << std::endl;
		std::cout << "Does enemy have a tag: " << hasTag << std::endl;
		std::cout << std::endl;

		// checking if an entity is destroyed properly
		enemy.destroy();

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

		// audio
		audioMng_.setChannelGroup("BGM");

		// Modifying the audio
		audioMng_.modifyChannelGroup(
			"BGM", 
			Sound::SoundParameters{ 
				0.01f, 1.f, 0.5f, 
			Sound::Reverb{ FMOD_DSP_TYPE_SFXREVERB, 1.f, 1.f, 1.f, 1.f },
			Sound::EQ{ FMOD_DSP_TYPE_PARAMEQ, 1.f, 1.f, 1.f } 
			});

		// This should be the game loop or the window loop
		// something to keep the function executing
		//bool looping{ true };
		//while (looping)
		//{
		//	audioMng_.playByGroup("Close_To_The_End", "BGM");
		//}

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