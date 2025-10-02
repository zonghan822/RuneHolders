#pragma once

#include "movementTest.hpp"

void createTestObject(ECS::EntityManager& ent) {

	//test circle initial config
	const int numCircles = 3;
	Math::eVec2<float> pos[numCircles] = {{ 900.f, 400.f }, { 800.0f, 350.f }, {950.0f, 450.0f}};
	const char* objName = { "testcaven" };
	const char* shapeName = { "Circle" };
	float radius = 15.f;
	int color = 1; // red

	static float speed = 250.f;

	Math::eVec2<float> vel = { cos(45.0f * 3.1415926f / 180.f) * speed, sin(-45.0f * 3.1415926f / 180.f) * speed };
	std::cout << vel;

	//now i want to create an entity with 3 different components, transform, drawinfo and name
	for (int i = 0; i < numCircles; ++i)
	{
		ECSComponents::DrawInfoComponent DIC{ pos[i].v1, pos[i].v2, radius, radius, shapeName, color};
		ECSComponents::NameComponent NC{ objName };
		ECSComponents::TransformComponent TC{};
		TC.pos.x = pos[i].v1;
		TC.pos.y = pos[i].v2;
		TC.pos.z = 0.0f;
		TC.rot = { 0.0f, 0.0f, 0.0f };
		TC.scale = { 1.0f, 1.0f, 1.0f };
		TC.vel = { vel.v1, vel.v2 };
		-vel;

		Collision::CollisionComponent cc{ radius };

		ECS::Entity circle = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::TransformComponent>(DIC, NC, TC);
		circle.add<Collision::CollisionComponent>(cc);
		//std::cout << circle.get<ECSComponents::DrawInfoComponent>();
	}


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
	double dt = eng.GetDeltaTime();
	
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
				if (collision.type != Collision::ColliderType::CIRCLE) continue;

				auto& transform = entity.get<ECSComponents::TransformComponent>();
				auto& drawInfo = entity.get<ECSComponents::DrawInfoComponent>();
			
				//std::cout << "object found: " << entity.get<ECSComponents::NameComponent>() << std::endl;
				//update movement here
				Math::eVec2<float> nextPos;
				nextPos.v1 = transform.pos.x + transform.vel.x * dt;
				nextPos.v2 = transform.pos.y + transform.vel.y * dt;

				Math::eVec2<float> oldPos = { transform.pos.x, transform.pos.y };
 
				// Track earliest collision
				bool hadCollision = false;
				float earliestTime = 2.0f;	// Impossible value for collision within current frame
				Math::eVec2<float> earliestColPoint;
				Math::eVec2<float> earliestNormal;
				ECS::Entity* collidedWith = nullptr;

				for (auto& other : entityList)
				{
					// skip same entity
					if (&entity == &other) continue;

					auto& otherCollision = other.get<Collision::CollisionComponent>();
					auto& otherTransform = other.get<ECSComponents::TransformComponent>();
					
					// Variables for this specific collision check
					Math::eVec2<float> colPoint;
					Math::eVec2<float> normalAtCol;
					float colTi;
					bool collided = false;

					//check collision with line
					if (otherCollision.type == Collision::ColliderType::LINE)
					{
						collided = Collision::CollisionDetection::CheckDynamicCollision(
							collision,
							transform,
							otherCollision,
							otherTransform,
							nextPos,
							colPoint,
							normalAtCol,
							colTi);
							
					}
					else if (otherCollision.type == Collision::ColliderType::CIRCLE)
					{
						// check circle to circle collision
						collided = (Collision::CollisionDetection::CheckDynamicCollision(
							collision, transform, otherCollision, otherTransform,
							nextPos, colPoint, normalAtCol, colTi, dt)); // dt to calculate moving distance between the circles

						
					}

					// Update earliest collision 
					if (collided && colTi >= 0.0f && colTi < earliestTime)
					{
						earliestTime = colTi;
						earliestColPoint = colPoint;
						earliestNormal = normalAtCol;
						hadCollision = true;
						collidedWith = &other;
					}
				}
				// Apply collision response
				if (hadCollision)
				{
					if (collidedWith->get<Collision::CollisionComponent>().type == Collision::ColliderType::CIRCLE)
					{
						// This circle handles velocity reflection for BOTH circles
						auto& otherTransform = collidedWith->get<ECSComponents::TransformComponent>();

						// Move to collision point
						const float separationEpsilon = 0.5f;
						transform.pos.x = earliestColPoint.v1 + earliestNormal.v1 * separationEpsilon;
						transform.pos.y = earliestColPoint.v2 + earliestNormal.v2 * separationEpsilon;

						// Reflect velocity
						Math::eVec2<float> velocityVec = { transform.vel.x, transform.vel.y };
						float dotProduct = velocityVec.v1 * earliestNormal.v1 + velocityVec.v2 * earliestNormal.v2;

						Math::eVec2<float> reflectedVel;
						reflectedVel.v1 = velocityVec.v1 - 2.0f * dotProduct * earliestNormal.v1;
						reflectedVel.v2 = velocityVec.v2 - 2.0f * dotProduct * earliestNormal.v2;

						const float restitution = 0.95f;
						transform.vel.x = reflectedVel.v1 * restitution;
						transform.vel.y = reflectedVel.v2 * restitution;
						
					}

					// For LINE collisions: Apply normal reflection
					else if (collidedWith->get<Collision::CollisionComponent>().type == Collision::ColliderType::LINE)
					{
						const float separationEpsilon = 0.5f;
						transform.pos.x = earliestColPoint.v1 + earliestNormal.v1 * separationEpsilon;
						transform.pos.y = earliestColPoint.v2 + earliestNormal.v2 * separationEpsilon;

						Math::eVec2<float> velocityVec = { transform.vel.x, transform.vel.y };
						Math::eVec2<float> reflectedVel;
						Collision::CollisionDetection::ReflectVelocity(velocityVec, earliestNormal, reflectedVel);

						const float restitution = 0.99f;
						transform.vel.x = reflectedVel.v1 * restitution;
						transform.vel.y = reflectedVel.v2 * restitution;
					}

				}
				else
				{
					// No collision - move normally
					transform.pos.x = nextPos.v1;
					transform.pos.y = nextPos.v2;
				}
				
				//update draw info component for the new pos
				entity.get<ECSComponents::DrawInfoComponent>().xPos = entity.get<ECSComponents::TransformComponent>().pos.x;
				entity.get<ECSComponents::DrawInfoComponent>().yPos = entity.get<ECSComponents::TransformComponent>().pos.y;
			}
		}

}

