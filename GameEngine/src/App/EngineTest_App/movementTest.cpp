/*============================================================================================
* @file         movementTest.cpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         03-10-2025
* @brief        This file creates objects using ECS to then test for various dynamic collision
*				scenarios, such as moving circle and line, circle to circle and square to square
*//*========================================================================================*/
#pragma once
#include "movementTest.hpp"



float min(float a, float b) { return a < b ? a : b; }
float max(float a, float b) { return a < b ? a : b; }

bool CheckDynamicSquare(
	const Collision::CollisionComponent& colA,
	const ECSComponents::TransformComponent& transformA,
	const Collision::CollisionComponent& colB,
	const ECSComponents::TransformComponent& transformB,
	float& colTi,                 // time of collision (output)
	Math::eVec2<float>& normal,
	const float &dt
)
{
	
	AABB aabb1{ transformA, colA };
	AABB aabb2{ transformB, colB };

	float tFirstX = 0.f;
	float tLastX = dt;

	float tFirstY = 0.f;
	float tLastY = dt;

	// Calculate relative velocity between the two objects
	Math::eVec2<float> relVel{ 
		transformA.vel.x - transformB.vel.x,
		transformA.vel.y - transformB.vel.y 
	};

	std::cout << "rev vel" << relVel << std::endl;

	// X-axis collision checks
	if (relVel.v1 < 0.f)
	{
		// Object A moving left relative to B
		if (aabb1.min.v1 > aabb2.max.v1) 
		{
			return false; // Already separated, moving away
		}

		if (aabb1.max.v1 < aabb2.min.v1) 
		{
			// Calculate collision time window on X-axis
			tFirstX = max((aabb1.max.v1 - aabb2.min.v1) / relVel.v1*dt, tFirstX);
			tLastX = min((aabb1.min.v1 - aabb2.max.v1) / relVel.v1, tLastX);
		}
	}
	else if (relVel.v1 > 0.f)
	{
		// Object A moving right relative to B
		if (aabb1.min.v1 > aabb2.max.v1) 
		{
			tFirstX = max((aabb1.min.v1 - aabb2.max.v1) / relVel.v1, tFirstX);
			tLastX = min((aabb1.max.v1 - aabb2.min.v1) / relVel.v1, tLastX);		
		}

		if (aabb1.max.v1 < aabb2.min.v1)
		{
			return false; // Already separated, moving away
		}
	}
	else if (relVel.v1 == 0.f) 
	{
		// No relative motion on X-axis - check for overlap
		if (aabb1.max.v1 < aabb2.min.v1 || aabb1.min.v1 > aabb2.max.v1) 
		{
			return false; // Not overlapping on X-axis
		}
	}

	// Y-axis collision checks
	if (relVel.v2 < 0.f) 
	{
		// Object A moving up relative to B
		if (aabb1.min.v2 > aabb2.max.v2) 
		{
			return false; // Already separated, moving away
		}

		if (aabb1.max.v2 < aabb2.min.v2) 
		{
			// Calculate collision time window on Y-axis
			tFirstY = max((aabb1.max.v2 - aabb2.min.v2) / relVel.v2, tFirstY);
			tLastY = min((aabb1.min.v2 - aabb2.max.v2) / relVel.v2, tLastY);		
		}
	}
	else if (relVel.v2 > 0.f) 
	{
		// Object A moving down relative to B
		if (aabb1.min.v2 > aabb2.max.v2) 
		{
			tFirstY = max((aabb1.min.v2 - aabb2.max.v2) / relVel.v2, tFirstY);
			tLastY = min((aabb1.max.v2 - aabb2.min.v2) / relVel.v2, tLastY);		
		}

		if (aabb1.max.v2 < aabb2.min.v2) 
		{
			return false; // Already separated, moving away
		}
	}
	else if (relVel.v2 == 0.f) 
	{
		// No relative motion on Y-axis - check for overlap
		if (aabb1.max.v2 < aabb2.min.v2 || aabb1.min.v2 > aabb2.max.v2) 
		{
			return false; // Not overlapping on Y-axis
		}
	}

	// Check if collision time windows overlap
	if (tFirstX > tLastY || tFirstY > tLastX) 
	{
		return false; // No intersection
	}

	// Collision detected - calculate time and normal
	colTi = max(tFirstX, tFirstY);

	// Determine collision normal based on which axis had first contact
	if (tFirstX > tFirstY) 
	{
		// X-axis collision
		normal.v1 = (relVel.v1 > 0.f) ? -1.f : 1.f;
		std::cout << "normal V1: " << normal.v1 << std::endl;
		normal.v2 = 0.f;
	}
	else if (tFirstY > tFirstX) 
	{
		// Y-axis collision
		normal.v1 = 0.f;
		normal.v2 = (relVel.v2 > 0.f) ? -1.f : 1.f;
		std::cout << "normal V2: " << normal.v2 << std::endl;
	}
	else 
	{
		std::cout << "tFirstX: " << tFirstX << ", tFirstY: " << tFirstY << std::endl;
		std::cout << "relVel: (" << relVel.v1 << ", " << relVel.v2 << ")" << std::endl;

		// Corner collision - normalize diagonal normal
		normal.v1 = (relVel.v1 > 0.f) ? -1.f : (relVel.v1 < 0.f ? 1.f : 0.f);
		normal.v2 = (relVel.v2 > 0.f) ? -1.f : (relVel.v2 < 0.f ? 1.f : 0.f);
		std::cout << "normalv1v2:: " << normal << std::endl;
		//float length = sqrt(normal.v1 * normal.v1 + normal.v2 * normal.v2);
		//if (length > 0.f) 
		//{
		//	normal.v1 /= length;
		//	normal.v2 /= length;
		//}
		std::cout << "Normal v1 v2:" << normal << std::endl;
	}

	// Only report collision if it happens within this frame
	if (colTi >= 0.f && colTi <= 1.f) 
	{
		return true;
	}

	return false;
}






void createTestObject(ECS::EntityManager& ent) {

#if 0
	// test circle initial config
	const int numCircles = 3;
	Math::eVec2<float> pos[numCircles] = {{ 900.f, 400.f }, { 800.0f, 350.f }, {950.0f, 450.0f}};
	const char* objName = { "testcaven" };
	const char* shapeName = { "Circle" };
	float radius = 15.f;
	int color = 1; // red

	static float speed = 400.f;

	Math::eVec2<float> vel = { cos(45.0f * 3.1415926f / 180.f) * speed, sin(-45.0f * 3.1415926f / 180.f) * speed };
	std::cout << vel;

	// now i want to create an entity with 3 different components, transform, drawinfo and name
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
		// std::cout << circle.get<ECSComponents::DrawInfoComponent>();
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
		// debugging
		// std::cout << wall.get<Collision::CollisionComponent>() << std::endl;
	}
#endif

	// Square config
	const int numSquares = 2;
	Math::eVec2<float> squarePos[numSquares] = { {700.0f, 400.0f}, {1000.0f, 400.0f} };
	Math::eVec2<float> squareVel[numSquares] = { {60.0f, 0.0f}, { -50.0f, 0.0f } };
	const char* objName = { "testcaven" };
	const char* shapeName = { "Rectangle" };
	float width = 100.f;
	float height = 100.f;
	int color = 1; // red

	static float squareSpeed = 200.0f;

	// now i want to create an entity with 3 different components, transform, drawinfo and name
	for (int i = 0; i < numSquares; ++i)
	{
		ECSComponents::DrawInfoComponent DIC{ squarePos[i].v1, squarePos[i].v2, width, height, shapeName, color };
		ECSComponents::NameComponent NC{ objName };
		ECSComponents::TransformComponent TC{};
		TC.pos.x = squarePos[i].v1;
		TC.pos.y = squarePos[i].v2;
		TC.pos.z = 0.0f;
		TC.rot = { 0.0f, 0.0f, 0.0f };
		TC.scale = { 1.0f, 1.0f, 1.0f };
		TC.vel = { squareVel[i].v1, squareVel[i].v2 };

		Collision::CollisionComponent SC{ width, height };

		ECS::Entity square = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::TransformComponent>(DIC, NC, TC);
		square.add<Collision::CollisionComponent>(SC);
	}
}

void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng)
{

	// dt
	double dt = eng.getDeltaTime();


#if 0
	// look for the entity that i created
	std::vector<ECS::Entity> entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));
	if (!entityList.empty())
	{

		for (auto& entity : entityList)
		{
			auto& collision = entity.get<Collision::CollisionComponent>();
			// skip non circle objects
			if (collision.type != Collision::ColliderType::CIRCLE) continue;

			auto& transform = entity.get<ECSComponents::TransformComponent>();
			auto& drawInfo = entity.get<ECSComponents::DrawInfoComponent>();

			// std::cout << "object found: " << entity.get<ECSComponents::NameComponent>() << std::endl;
			// update movement here
			Math::eVec2<float> nextPos;
			nextPos.v1 = transform.pos.x + transform.vel.x * dt;
			nextPos.v2 = transform.pos.y + transform.vel.y * dt;

			Math::eVec2<float> oldPos = { transform.pos.x, transform.pos.y };
			Math::eVec2<float> BeA;
			Math::eVec2<float> BeB;
			// Track earliest collision
			Math::eVec2<float> colPoint;
			Math::eVec2<float> normalAtCol;
			float colTi;
			bool hadCollision = false;

			for (auto& other : entityList)
			{
				// skip same entity
				if (&entity == &other) continue;

				auto& otherCollision = other.get<Collision::CollisionComponent>();
				auto& otherTransform = other.get<ECSComponents::TransformComponent>();


				//check collision with line
				if (otherCollision.type == Collision::ColliderType::LINE)
				{
					if (Collision::CollisionDetection::CheckDynamicCollision(
						collision,
						transform,
						otherCollision,
						otherTransform,
						nextPos,
						colPoint,
						normalAtCol,
						colTi))
					{
						// Reflection for line collision
						Math::eVec2<float> reflectedVec;

						Collision::CollisionDetection::CollisionReflection(colPoint,
							normalAtCol,
							nextPos,
							reflectedVec);

						transform.vel.x = reflectedVec.v1 * 100.f * 0.99f;
						transform.vel.y = reflectedVec.v2 * 100.f * 0.99f;
						hadCollision = true;

					}

				}
				else if (otherCollision.type == Collision::ColliderType::CIRCLE)
				{

					// check circle to circle collision
					if (Collision::CollisionDetection::CheckDynamicCollision(
						collision, transform, otherCollision, otherTransform,
						nextPos, BeA, BeB, colTi))
					{

						transform.pos.x = BeA.v1;
						transform.pos.y = BeA.v2;

						otherTransform.pos.x = BeB.v1;
						otherTransform.pos.y = BeB.v2;

						std::cout << "bia: " << BeB << std::endl;
						hadCollision = true;

					}	

				}

			}	
			
			
			if (!hadCollision)
			{
				transform.pos.x = nextPos.v1;
				transform.pos.y = nextPos.v2;
			}

			// update draw info component for the new pos
			entity.get<ECSComponents::DrawInfoComponent>().xPos = entity.get<ECSComponents::TransformComponent>().pos.x;
			entity.get<ECSComponents::DrawInfoComponent>().yPos = entity.get<ECSComponents::TransformComponent>().pos.y;
			
		}
	}
#endif

	// Collision check for square objects
	std::vector<ECS::Entity> entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));
	if (!entityList.empty())
	{
		for (auto& entity : entityList)
		{

			auto& collision = entity.get<Collision::CollisionComponent>();
			auto& transform = entity.get<ECSComponents::TransformComponent>();
			auto& drawInfo = entity.get<ECSComponents::DrawInfoComponent>();

			Math::eVec2<float> nextPos;

			nextPos.v1 = transform.pos.x + transform.vel.x * dt;
			nextPos.v2 = transform.pos.y + transform.vel.y * dt;

			//std::cout << "current vel: " << transform.vel.x << std::endl;

			Math::eVec2<float> colPoint;
			Math::eVec2<float> normalAtCol;
			float colTi;
			bool hasCollided = false;

			// Loop through other objects
			for (auto& other : entityList)
			{
				// Skip self
				if (&entity == &other) continue;

				auto& otherCollision = other.get<Collision::CollisionComponent>();
				auto& otherTransform = other.get<ECSComponents::TransformComponent>();

				// Check type of collision
				switch (collision.type)
				{
					case Collision::ColliderType::RECTANGLE:
					{
						switch (otherCollision.type)
						{
							case Collision::ColliderType::RECTANGLE:
							{
								// Call check collision function for dynamic rect rect
								if (CheckDynamicSquare(collision, transform, otherCollision, otherTransform,
														colTi, normalAtCol, dt))
								{
									// Calculate relative velocity
									Math::eVec2<float> relVel{
										transform.vel.x - otherTransform.vel.x,
										transform.vel.y - otherTransform.vel.y
									};

									//std::cout << "relvel = :" << relVel << std::endl;

									// Calculate relative velocity along normal
									float velAlongNormal = relVel.v1 * normalAtCol.v1 +
										relVel.v2 * normalAtCol.v2;

									// Don't resolve if velocities are separating
									//if (velAlongNormal > 0)
									//	continue;

									// Subtract the component along that direction
									transform.vel.x -= velAlongNormal * normalAtCol.v1;
									transform.vel.y -= velAlongNormal * normalAtCol.v2;

									// Restitution (bounciness) - 1.0 = perfectly elastic
									//float restitution = 0.6f;

									// Calculate impulse scalar
									//float impulseMagnitude = -(1.f + restitution) * velAlongNormal;

									// Assume equal mass or adjust based on mass if you have it
									// For equal mass, each gets half the impulse
									//float massA = 0.2f;  // Replace with actual mass if available
									//float massB = 0.1f;  // Replace with actual mass if available
									//float totalMass = massA + massB;

									//impulseMagnitude /= (1.f / massA + 1.f / massB);

									// Apply impulse to velocities
									//Math::eVec2<float> impulse{
									//	impulseMagnitude * normalAtCol.v1,
									//	impulseMagnitude * normalAtCol.v2
									//};


									// Reflect velocity: v' = v - 2(v·n)n
									// Apply impulse to both objects
									//transform.vel.x += impulse.v1 / massA;
									//transform.vel.y += impulse.v2 / massA;

									//otherTransform.vel.x -= impulse.v1 / massB;
									//otherTransform.vel.y -= impulse.v2 / massB;


									hasCollided = true;
									//std::cout << "collided" << "velocity at norm : " << velAlongNormal << std::endl;
									break;

								}
							}
						}
					}

				}

			}

			if (!hasCollided)
			{
				transform.pos.x = nextPos.v1;
				transform.pos.y = nextPos.v2;
			}


		}

	}

}

