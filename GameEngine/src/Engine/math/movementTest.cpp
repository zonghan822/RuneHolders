#pragma once

#include "movementTest.hpp"

void createTestObject(ECS::EntityManager& ent) {

	//test rect initial config
	Math::eVec2<float> pos{ 500.f, 300.f };
	const char* objName = { "testRect" };
	const char* shapeName = { "Rectangle" };
	float width = 100.f;
	float height = 100.f;
	int color = 0; // green

	//now i want to create an entity with 3 different components, transform, drawinfo and name
	ECSComponents::DrawInfoComponent DIC{ pos.v1, pos.v2, width, height, shapeName, color };
	ECSComponents::NameComponent NC{ objName };
	ECSComponents::eVec2TransformComponent TC{ pos.v1, pos.v2, width, height };

	ECS::Entity rect = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::eVec2TransformComponent>(DIC, NC, TC);
	std::cout << rect.get<ECSComponents::DrawInfoComponent>();

}

void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng)
{

	static float speed = 80.f;
	//look for the entity that i created
	std::vector<ECS::Entity> rect = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testRect"));
	if (!rect.empty())
	{
		for (auto& entity : rect)
		{
			//std::cout << "object found: " << entity.get<ECSComponents::NameComponent>() << std::endl;
			//i have yet to decide where i want to store this value 
			//(transform component? what if some objects have transform component but no movement?)

			//call generic movement function which takes in a transform component of the entity, as well as that entity's speed
			testMovement(eng, entity.get<ECSComponents::eVec2TransformComponent>(), speed);

			//update draw info component for the new pos
			entity.get<ECSComponents::DrawInfoComponent>().xPos = entity.get<ECSComponents::eVec2TransformComponent>().pos.v1;
			entity.get<ECSComponents::DrawInfoComponent>().yPos = entity.get<ECSComponents::eVec2TransformComponent>().pos.v2;

		}
		
	}

}

//call this function to help me to test movement features
void testMovement(const Engine::EnoEngine& eng, ECSComponents::eVec2TransformComponent& transform, float speed) {

	//dt
	double dt = eng.getDeltaTime();

	//update movement here
	//for now, i am doing linear movement without any velocity
	//since we have no key inputs defined, i shall randomize movement with time intervals
	static int randDir = 1;
	static double startTime = glfwGetTime();
	double currTime = glfwGetTime();
	double elaspedTime = currTime - startTime;

	if (elaspedTime > 1.0f)
	{
		srand(currTime);
		randDir = rand() % 4 + 1; //randomize between 1 - 4
		//reset interval
		startTime = currTime;

	}

	Math::eVec2<float> nextPos{ transform.pos };
	//determine which direction to move
	switch (randDir)
	{
	case 1:

		//move right
		nextPos.v1 = transform.pos.v1 + speed * dt;
		break;

	case 2:
		//move left
		nextPos.v1 = transform.pos.v1 - speed * dt;
		break;

	case 3:
		//move down
		nextPos.v2 = transform.pos.v2 + speed * dt;
		break;

	case 4:
		//move up
		nextPos.v2 = transform.pos.v2 - speed * dt;
		break;

	default:
		break;
	}

	//i can check for collision here...


	//if no collision, update new position in transform component
	transform.pos = nextPos;


}