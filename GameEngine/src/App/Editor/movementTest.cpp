/*============================================================================================
* @file         movementTest.cpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         03-10-2025
* @brief        This file creates objects using ECS to then test for various dynamic collision
*				scenarios, such as moving circle and line, circle to circle and square to square
* 
* * All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/
#pragma once

#include "movementTest.hpp"



//double min(double a, double b) { return a < b ? a : b; }
//double max(double a, double b) { return a > b ? a : b; }

AABB::AABB(ECSComponents::TransformComponent transform, ECSComponents::CollisionComponent coll)
{
	float minx = transform.pos.x - coll.data.rectangle.width / 2.0f;
	float miny = transform.pos.y - coll.data.rectangle.height / 2.0f;

	float maxx = transform.pos.x + coll.data.rectangle.width / 2.0f;
	float maxy = transform.pos.y + coll.data.rectangle.height / 2.0f;

	min = { minx, miny };
	max = { maxx, maxy };
}


bool CheckDynamicSquare(
	const ECSComponents::CollisionComponent& colA,
	const ECSComponents::TransformComponent& transformA,
	const ECSComponents::CollisionComponent& colB,
	const ECSComponents::TransformComponent& transformB,
	float& colTi,                 // Time of collision (output)
	Math::eVec2<float>& normal,
	const double& dt
)
{

	AABB aabb1{ transformA, colA };
	AABB aabb2{ transformB, colB };

	double tFirstX = 0.f;
	double tLastX = dt;

	double tFirstY = 0.f;
	double tLastY = dt;

	// Calculate relative velocity between the two objects
	Math::eVec2<float> relVel{
		transformA.vel.x - transformB.vel.x,
		transformA.vel.y - transformB.vel.y
	};

	//std::cout << "rev vel" << relVel << std::endl;

	// X-axis collision checks
	if (relVel.x < 0.f)
	{
		// Object A moving left relative to B
		if (aabb1.min.x > aabb2.max.x)
		{
			return false; // Already separated, moving away
		}

		if (aabb1.max.x < aabb2.min.x)
		{
			// Calculate collision time window on X-axis
			tFirstX = Math::max(static_cast<double>((aabb1.max.x - aabb2.min.x) / relVel.x), tFirstX);
			tLastX = Math::min(static_cast<double>((aabb1.min.x - aabb2.max.x) / relVel.x), tLastX);
		}
	}
	else if (relVel.x > 0.f)
	{
		// Object A moving right relative to B
		if (aabb1.min.x > aabb2.max.x)
		{
			tFirstX = Math::max(static_cast<double>((aabb1.min.x - aabb2.max.x) / relVel.x), tFirstX);
			tLastX = Math::min(static_cast<double>((aabb1.max.x - aabb2.min.x) / relVel.x), tLastX);
		}

		if (aabb1.max.x < aabb2.min.x)
		{
			return false; // Already separated, moving away
		}
	}
	else if (relVel.x == 0.f)
	{
		// No relative motion on X-axis - check for overlap
		if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x)
		{
			return false; // Not overlapping on X-axis
		}
	}

	// Y-axis collision checks
	if (relVel.y < 0.f)
	{
		// Object A moving up relative to B
		if (aabb1.min.y > aabb2.max.y)
		{
			return false; // Already separated, moving away
		}

		if (aabb1.max.y < aabb2.min.y)
		{
			// Calculate collision time window on Y-axis
			tFirstY = Math::max(static_cast<double>((aabb1.max.y - aabb2.min.y) / relVel.y), tFirstY);
			tLastY = Math::min(static_cast<double>((aabb1.min.y - aabb2.max.y) / relVel.y), tLastY);
		}
	}
	else if (relVel.y > 0.f)
	{
		// Object A moving down relative to B
		if (aabb1.min.y > aabb2.max.y)
		{
			tFirstY = Math::max(static_cast<double>((aabb1.min.y - aabb2.max.y) / relVel.y), tFirstY);
			tLastY = Math::min(static_cast<double>((aabb1.max.y - aabb2.min.y) / relVel.y), tLastY);
		}

		if (aabb1.max.y < aabb2.min.y)
		{
			return false; // Already separated, moving away
		}
	}
	else if (relVel.y == 0.f)
	{
		// No relative motion on Y-axis - check for overlap
		if (aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y)
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
	colTi = static_cast<float>(Math::max(tFirstX, tFirstY));

	// Determine collision normal based on which axis had first contact
	if (tFirstX > tFirstY)
	{
		// X-axis collision
		normal.x = (relVel.x > 0.f) ? -1.f : 1.f;
		//std::cout << "normal V1: " << normal.x << std::endl;
		normal.y = 0.f;
	}
	else if (tFirstY > tFirstX)
	{
		// Y-axis collision
		normal.x = 0.f;
		normal.y = (relVel.y > 0.f) ? -1.f : 1.f;
		//std::cout << "normal V2: " << normal.y << std::endl;
	}
	else
	{
		//std::cout << "tFirstX: " << tFirstX << ", tFirstY: " << tFirstY << std::endl;
		//std::cout << "relVel: (" << relVel.x << ", " << relVel.y << ")" << std::endl;

		// Corner collision - normalize diagonal normal
		normal.x = (relVel.x > 0.f) ? -1.f : (relVel.x < 0.f ? 1.f : 0.f);
		normal.y = (relVel.y > 0.f) ? -1.f : (relVel.y < 0.f ? 1.f : 0.f);
		//std::cout << "normalv1v2:: " << normal << std::endl;
		//float length = sqrt(normal.x * normal.x + normal.y * normal.y);

		//std::cout << "Normal x y:" << normal << std::endl;
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
	// Test circle initial config
	const int numCircles = 3;
	Math::eVec2<float> pos[numCircles] = {{ 900.f, 400.f }, { 800.0f, 350.f }, {950.0f, 450.0f}};
	const char* objName = { "testcaven" };
	const char* shapeName = { "Circle" };
	float radius = 15.f;
	int color = 1; // red

	static float speed = 400.f;

	Math::eVec2<float> vel = { cos(45.0f * 3.1415926f / 180.f) * speed, sin(-45.0f * 3.1415926f / 180.f) * speed };
	std::cout << vel;

	// Now i want to create an entity with 3 different components, transform, drawinfo and name
	for (int i = 0; i < numCircles; ++i)
	{
		ECSComponents::DrawInfoComponent DIC{ pos[i].x, pos[i].y, radius, radius, shapeName, color};
		ECSComponents::NameComponent NC{ objName };
		ECSComponents::TransformComponent TC{};
		TC.pos.x = pos[i].x;
		TC.pos.y = pos[i].y;
		TC.pos.z = 0.0f;
		TC.rot = { 0.0f, 0.0f, 0.0f };
		TC.scale = { 1.0f, 1.0f, 1.0f };
		TC.vel = { vel.x, vel.y };
		-vel;

		ECSComponents::CollisionComponent cc{ radius };

		ECS::Entity circle = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::TransformComponent>(DIC, NC, TC);
		circle.add<ECSComponents::CollisionComponent>(cc);
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
		lineTC.pos.x = linePos[i].first.x + ((linePos[i].first.x - linePos[i].second.x) / 2.f);
		lineTC.pos.y = linePos[i].first.y + ((linePos[i].first.y - linePos[i].second.y) / 2.f);
		lineTC.pos.z = 0.0f;
		lineTC.rot = { 0.0f, 0.0f, 0.0f };
		lineTC.scale = { 1.0f, 1.0f, 1.0f };
		
		// Name componenet
		ECSComponents::NameComponent lineNC{ "testcaven"};
		// Create collision component
		ECSComponents::CollisionComponent lineCC{ linePos[i].first, linePos[i].second};
		ECS::Entity wall = ent.create<ECSComponents::TransformComponent, ECSComponents::NameComponent>(lineTC, lineNC);
		wall.add<ECSComponents::CollisionComponent>(lineCC);
		// Debugging
		//std::cout << wall.get<ECSComponents::CollisionComponent>() << std::endl;
	}
#endif

	// Square config
	const int numSquares = 2;
	Math::eVec2<float> squarePos[numSquares] = { {200.0f, 650.0f}, {1000.0f, 650.0f} };
	Math::eVec2<float> squareVel[numSquares] = { {100.0f, 0.0f}, { -100.0f, 0.0f } };
	const char* objName = { "testcaven" };
	const char* shapeName = { "Rectangle" };
	float width = 100.f;
	float height = 100.f;
	int color = 1; // Red

	static float squareSpeed = 200.0f;

	// Now i want to create an entity with 3 different components, transform, drawinfo and name
	for (int i = 0; i < numSquares; ++i)
	{
		ECSComponents::DrawInfoComponent DIC{ squarePos[i].x, squarePos[i].y, width, height, shapeName, color };
		ECSComponents::NameComponent NC{ objName };
		ECSComponents::TransformComponent TC{};
		TC.pos.x = squarePos[i].x;
		TC.pos.y = squarePos[i].y;
		TC.rot = { 0.0f, 0.0f };
		TC.scale = { 1.0f, 1.0f };
		TC.vel = { squareVel[i].x, squareVel[i].y };

		ECSComponents::CollisionComponent SC{ width, height };

		ECS::Entity square = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::TransformComponent>(DIC, NC, TC);
		square.add<ECSComponents::CollisionComponent>(SC);
	}
}

void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng)
{

	// DT
	double dt = eng.getDeltaTime();


#if 0
	// Look for the entity that i created
	std::vector<ECS::Entity> entityList = ent.findEntity<ECSComponents::NameComponent>(ECSComponents::NameComponent("testcaven"));
	if (!entityList.empty())
	{

		for (auto& entity : entityList)
		{
			auto& collision = entity.get<ECSComponents::CollisionComponent>();
			// Skip non circle objects
			if (collision.type != ECSComponents::ColliderType::CIRCLE) continue;

			auto& transform = entity.get<ECSComponents::TransformComponent>();
			auto& drawInfo = entity.get<ECSComponents::DrawInfoComponent>();

			//std::cout << "object found: " << entity.get<ECSComponents::NameComponent>() << std::endl;
			// Update movement here
			Math::eVec2<float> nextPos;
			nextPos.x = transform.pos.x + transform.vel.x * dt;
			nextPos.y = transform.pos.y + transform.vel.y * dt;

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
				// Skip same entity
				if (&entity == &other) continue;

				auto& otherCollision = other.get<ECSComponents::CollisionComponent>();
				auto& otherTransform = other.get<ECSComponents::TransformComponent>();


				// Check collision with line
				if (otherCollision.type == ECSComponents::ColliderType::LINE)
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

						transform.vel.x = reflectedVec.x * 100.f * 0.99f;
						transform.vel.y = reflectedVec.y * 100.f * 0.99f;
						hadCollision = true;

					}

				}
				else if (otherCollision.type == ECSComponents::ColliderType::CIRCLE)
				{

					// Check circle to circle collision
					if (Collision::CollisionDetection::CheckDynamicCollision(
						collision, transform, otherCollision, otherTransform,
						nextPos, BeA, BeB, colTi))
					{

						transform.pos.x = BeA.x;
						transform.pos.y = BeA.y;

						otherTransform.pos.x = BeB.x;
						otherTransform.pos.y = BeB.y;

						std::cout << "bia: " << BeB << std::endl;
						hadCollision = true;

					}	

				}

			}	
			
			
			if (!hadCollision)
			{
				transform.pos.x = nextPos.x;
				transform.pos.y = nextPos.y;
			}

			// Update draw info component for the new pos
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

			auto& collision = entity.get<ECSComponents::CollisionComponent>();
			auto& transform = entity.get<ECSComponents::TransformComponent>();
			auto& drawInfo = entity.get<ECSComponents::DrawInfoComponent>();

			Math::eVec2<float> nextPos;
			nextPos.x = static_cast<float>(transform.pos.x + transform.vel.x * dt);
			nextPos.y = static_cast<float>(transform.pos.y + transform.vel.y * dt);
			float colTi;
			Math::eVec2<float> normalAtCol;
			bool hasCollided = false;

			// Loop through other objects
			for (auto& other : entityList)
			{
				// Skip self
				if (&entity == &other) continue;

				auto& otherCollision = other.get<ECSComponents::CollisionComponent>();
				auto& otherTransform = other.get<ECSComponents::TransformComponent>();

				// Check type of collision
				switch (collision.type)
				{
					case ECSComponents::ColliderType::RECTANGLE:
					{
						switch (otherCollision.type)
						{
							case ECSComponents::ColliderType::RECTANGLE:
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
									float velAlongNormal = relVel.x * normalAtCol.x +
										relVel.y * normalAtCol.y;

									// Don't resolve if velocities are separating
									if (velAlongNormal > 0)
									continue;

									// Subtract the component along that direction
									transform.vel.x -= velAlongNormal * normalAtCol.x;
									transform.vel.y -= velAlongNormal * normalAtCol.y;

									//transform.pos.x = static_cast<float>(transform.pos.x + transform.vel.x * dt);
									//transform.pos.y = static_cast<float>(transform.pos.y + transform.vel.y * dt);

									//transform.pos.x = static_cast<float>(transform.pos.x + transform.vel.x );
									//transform.pos.y = static_cast<float>(transform.pos.y + transform.vel.y );

									otherTransform.vel.x += velAlongNormal * normalAtCol.x;
									otherTransform.vel.y += velAlongNormal * normalAtCol.y;

									hasCollided = true;


								}
								break;
							}
						} // End of switch case for other entity
					}

				} // End of switch case for first entity

			} // End of entity loop

			if (!hasCollided)
			{
				transform.pos.x = nextPos.x;
				transform.pos.y = nextPos.y;
			}

			// Update draw info component for the new pos
			drawInfo.xPos = transform.pos.x;
			drawInfo.yPos = transform.pos.y;
		}

	}

}

