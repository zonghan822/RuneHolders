#pragma once

#include "movementTest.hpp"

void createTestObject(ECS::EntityManager& ent) {

	//test circle initial config
	Math::eVec2<float> pos{ 900.f, 400.f };
	const char* objName = { "testcaven" };
	const char* shapeName = { "Circle" };
	float radius = 15.f;
	int color = 1; // red

	static float speed = 250.f;

	Math::eVec2<float> vel = { cos(45.0f * 3.1415926f / 180.f) * speed, sin(-45.0f * 3.1415926f / 180.f) * speed };
	std::cout << vel;

	//now i want to create an entity with 3 different components, transform, drawinfo and name
	ECSComponents::DrawInfoComponent DIC{ pos.v1, pos.v2, radius, radius, shapeName, color };
	ECSComponents::NameComponent NC{ objName };
	ECSComponents::TransformComponent TC{};
	TC.pos.x = pos.v1;
	TC.pos.y = pos.v2;
	TC.pos.z = 0.0f;
	TC.rot = { 0.0f, 0.0f, 0.0f };
	TC.scale = { 1.0f, 1.0f, 1.0f };
	TC.vel = { vel.v1, vel.v2 };

	Collision::CollisionComponent cc{ radius };

	ECS::Entity circle = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::TransformComponent>(DIC, NC, TC);
	circle.add<Collision::CollisionComponent>(cc);
	//std::cout << circle.get<ECSComponents::DrawInfoComponent>();

	// Now make 3 line collider entities
	const int numLines = 3;
	// Line requires a pair of vecs for start and end point
	std::pair<Math::eVec2<float>, Math::eVec2<float>> linePos[numLines] = { {{900.f, 150.f},{1150.f, 500.f}}, {{ 900.f, 150.f },{650.f, 500.f} }, {{ 650.f, 500.f }, {1150.f, 500.f}} };

	for (int i = 0; i < numLines; ++i)
	{
		ECSComponents::TransformComponent lineTC;
		// Point of origin for line segments will be at the middle point
		lineTC.pos.x = linePos[i].first.v1 + ((linePos[i].first.v1 - linePos[i].second.v1) / 2.f);
		lineTC.pos.y = linePos[i].first.v2 + ((linePos[i].first.v2 - linePos[i].second.v2) / 2.f);
		lineTC.pos.z = 0.0f;
		lineTC.rot = { 0.0f, 0.0f, 0.0f };
		lineTC.scale = { 1.0f, 1.0f, 1.0f };
		
		// Name componenet
		ECSComponents::NameComponent lineNC{ "testcaven"};
		// Create collision component
		Collision::CollisionComponent lineCC{ linePos[i].first, linePos[i].second};
		ECS::Entity wall = ent.create<ECSComponents::TransformComponent, ECSComponents::NameComponent>(lineTC, lineNC);
		wall.add<Collision::CollisionComponent>(lineCC);
		//debugging
		//std::cout << wall.get<Collision::CollisionComponent>() << std::endl;
	}

	//std::vector<ECS::Entity> entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));
	//int counter = 0;
	//for (auto& entity : entityList)
	//{
	//	std::cout << " i am entity " << counter++ << " : " << entity.get<Collision::CollisionComponent>() << std::endl;
	//}

}

void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng)
{

	//dt
	double dt = eng.getDeltaTime();
	
	Math::eVec2<float> colPoint{ 0.0f };
	Math::eVec2<float> normalAtCol{ 0.0f };
	float colTi = 0.0f;

	//look for the entity that i created
	std::vector<ECS::Entity> entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));
	if (!entityList.empty())
	{
		
			for (auto& entity : entityList)
			{
				auto& collision = entity.get<Collision::CollisionComponent>();
				//skip non circle objects
				if (collision.type != Collision::ColliderType::CIRCLE)
				//{
				//	std::cout << " i am entity :" << entity.get<Collision::CollisionComponent>() << std::endl;
					continue;
				//}

				auto& transform = entity.get<ECSComponents::TransformComponent>();
				auto& drawInfo = entity.get<ECSComponents::DrawInfoComponent>();
			
				//std::cout << "object found: " << entity.get<ECSComponents::NameComponent>() << std::endl;
				//update movement here
				Math::eVec2<float> nextPos;
				nextPos.v1 = transform.pos.x + transform.vel.x * dt;
				nextPos.v2 = transform.pos.y + transform.vel.y * dt;

				Math::eVec2<float> oldPos = { transform.pos.x, transform.pos.y };
 
				// Track earliest collision
				//bool hadCollision = false;
				//float earliestTime = 2.0f;
				//Math::eVec2<float> earliestColPoint;
				//Math::eVec2<float> earliestNormal;

				for (auto& other : entityList)
				{
					auto& otherCollision = other.get<Collision::CollisionComponent>();
					//std::cout << " i am here :" << other.get<Collision::CollisionComponent>() << std::endl; 
					//check collision with line
					if (otherCollision.type != Collision::ColliderType::LINE) continue;
				
						if (Collision::CollisionDetection::CheckCollision(
							collision,
							transform,
							otherCollision,
							other.get<ECSComponents::TransformComponent>(),
							nextPos,
							colPoint,
							normalAtCol,
							colTi))
						{
							std::cout << "COLLIDED\n" << otherCollision.data.line.p1 << " : " << otherCollision.data.line.p2 << std::endl;
							std::cout << transform.pos.x << " : " << transform.pos.y << " \\n ";

							Math::eVec2<float> reflectedVec;
							Collision::CollisionDetection::CollisionReflection(colPoint, normalAtCol, nextPos, reflectedVec);

							transform.vel.x = reflectedVec.v1 * 200.f;
							transform.vel.y = reflectedVec.v2 * 200.f;

						}


				}

				entity.get<ECSComponents::TransformComponent>().pos.x = nextPos.v1;
				entity.get<ECSComponents::TransformComponent>().pos.y = nextPos.v2;

				//update draw info component for the new pos
				entity.get<ECSComponents::DrawInfoComponent>().xPos = entity.get<ECSComponents::TransformComponent>().pos.x;
				entity.get<ECSComponents::DrawInfoComponent>().yPos = entity.get<ECSComponents::TransformComponent>().pos.y;
			}
		}

}

