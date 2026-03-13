/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         collision.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)   51%
* @co-author    Caven Koh (caven.koh@digipen.edu)   20%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   11%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   11%
* @date         03-10-2025
* @brief        Implementation of the collision detection system including all collision detection algorithms, collision component methods, and collision system for ECS integration.
*//*========================================================================================*/

#include "collision.hpp"
#include <limits>

namespace Collision {

	// COLLISION RESULT IMPLMENTATION
	CollisionResult::CollisionResult()
		: hasCollision(false)
		, penetrationVector{ 0.0f, 0.0f }
		, penetrationDepth(0.0f)
		, contactPoint{ 0.0f, 0.0f }
	{

	}

	// COLLISION DETECTION IMPLMENTATION
	CollisionResult CollisionDetection::CheckCollision(
		const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA,
		const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB)
	{
		// Early exit: skip collision if either collider is inactive
		if (!colliderA.isActive || !colliderB.isActive)
		{
			return CollisionResult(); // Return default "no collision result"
		}

		// Calculate actual world positiions of the colliders
		// World position = transform position + collider offset
		Math::eVec2<float> posA = { transformA.pos.x + colliderA.offset.x, transformA.pos.y + colliderA.offset.y };
		Math::eVec2<float> posB = { transformB.pos.x + colliderB.offset.x, transformB.pos.y + colliderB.offset.y };

		// Convey to the collision detection function accordingly
		// (BASED ON the types of the 2 colliders)

		// Scenario 1: if both colliders are circles
		if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::CIRCLE)
		{
			// Scale the radii
			float radiusA = colliderA.data.circle.radius * transformA.scale.x;
			float radiusB = colliderB.data.circle.radius * transformB.scale.x;
			return CheckCircleCircle(posA, radiusA, posB, radiusB);
		}

		// Scenario 2: if both colliders are rectangles
		else if (colliderA.type == ColliderType::RECTANGLE && colliderB.type == ColliderType::RECTANGLE)
		{
			// Scale rectangle dimensions
			CollisionComponent::ColliderData::RectangleData scaledRectA = colliderA.data.rectangle;
			scaledRectA.width *= transformA.scale.x;
			scaledRectA.height *= transformA.scale.y;

			CollisionComponent::ColliderData::RectangleData scaledRectB = colliderB.data.rectangle;
			scaledRectB.width *= transformB.scale.x;
			scaledRectB.height *= transformB.scale.y;

			return CheckRectangleRectangleSAT(posA, scaledRectA, transformA.rot.x,
				posB, scaledRectB, transformB.rot.x);
		}

		// Scenario 3: if collider A is a circle, and collider B is a rectangle
		else if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::RECTANGLE)
		{
			float radiusA = colliderA.data.circle.radius * transformA.scale.x;
			CollisionComponent::ColliderData::RectangleData scaledRectB = colliderB.data.rectangle;
			scaledRectB.width *= transformB.scale.x;
			scaledRectB.height *= transformB.scale.y;

			return CheckCircleRectangle(posA, radiusA, posB, scaledRectB);
		}

		// Scenario 4: if collider A is a rectangle, and collider B is a circle
		else if (colliderA.type == ColliderType::RECTANGLE && colliderB.type == ColliderType::CIRCLE)
		{
			float radiusB = colliderB.data.circle.radius * transformB.scale.x;
			CollisionComponent::ColliderData::RectangleData scaledRectA = colliderA.data.rectangle;
			scaledRectA.width *= transformA.scale.x;
			scaledRectA.height *= transformA.scale.y;
			CollisionResult result = CheckCircleRectangle(posB, radiusB, posA, scaledRectA);
			result.penetrationVector = -result.penetrationVector;
			return result;
		}

		// Scenario 5: if collider A is a point
		else if (colliderA.type == ColliderType::POINT)
		{

			return CheckPointCollider(posA, colliderB, posB);
		}

		// Scenario 6: if collider B is a point
		else if (colliderB.type == ColliderType::POINT)
		{
			return CheckPointCollider(posB, colliderA, posA);
		}

		// Any unsupported collision type combination
		return CollisionResult{};
	}

	bool CollisionDetection::CheckDynamicCollision(
		const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA, const ECSComponents::PhysicsComponent& physicsA,
		const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB, const ECSComponents::PhysicsComponent& physicsB,
		const Math::eVec2<float>& endPos, Math::eVec2<float>& colPoint, Math::eVec2<float>& normalAtCol, float& colTi)
	{

		// Early exit: skip collision if either collider is inactive
		if (!colliderA.isActive || !colliderB.isActive)
		{
			return false; // Return "no collision result"
		}

		// Calculate actual world positiions of the colliders
		// World position = transform position + collider offset
		Math::eVec2<float> posA = { transformA.pos.x + colliderA.offset.x, transformA.pos.y + colliderA.offset.y };
		Math::eVec2<float> posB = { transformB.pos.x + colliderB.offset.x, transformB.pos.y + colliderB.offset.y };

		// Convey to the collision detection function accordingly
		// (BASED ON the types of the 2 colliders)
		// Dynamic circle and line collision
		if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::LINE)
		{
			//std::cout << "CIRCLE RADIUS ::: " << colliderA.data.circle.radius << std::endl;
			return CheckCircleLine(posA, colliderA.data.circle.radius, endPos, colliderB, colPoint, normalAtCol, colTi);
		}

		// Dynamic circle and circle collision
		if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::CIRCLE)
		{
			//std::cout << "CIRCLE RADIUS ::: " << colliderA.data.circle.radius << std::endl;
			return CheckDynamicCircleCircle(posA, { physicsA.velocity.x, physicsA.velocity.y }, colliderA.data.circle.radius,
				posB, { physicsB.velocity.x, physicsB.velocity.y }, colliderB.data.circle.radius,
				colPoint, normalAtCol, colTi);
		}
		return false;
	}



	CollisionResult CollisionDetection::CheckCircleCircle(const Math::eVec2<float>& posA, float radiusA, const Math::eVec2<float>& posB, float radiusB) {
		CollisionResult result;

		// Calculate the vector from circle A center to circle B center
		Math::eVec2<float> distance = posB - posA;

		// Use squared distance (avoiding the use of sqrt)
		float distanceSquared = Math::eVec2SquaredLength(distance);
		float combinedRadius = radiusA + radiusB;
		float combinedRadiusSquared = combinedRadius * combinedRadius;

		// Check if circle are overlapping
		// They overlap if the distance between centers < sum of radii
		if (distanceSquared < combinedRadiusSquared)
		{
			result.hasCollision = true;

			// Now need actual distance
			float actualDistance = Math::eVec2Length(distance);

			// Avoid division by zero if circles are at exactly the same position
			if (actualDistance > 0.001f)
			{
				// Calculate penetration depth (how much they overlap)
				result.penetrationDepth = combinedRadius - actualDistance;

				// Calculate penetration vecotr (direction and magnitude to separate them)
				Math::eVec2<float> normalizedDistance = distance / actualDistance;
				result.penetrationVector = normalizedDistance * result.penetrationDepth;

				// Contact point is on the edge of circle A in the direction of circle B
				result.contactPoint = posA + (normalizedDistance * radiusA);

			}
			else {
				// Circles are at the exact same position - push them apart
				result.penetrationDepth = combinedRadius;
				result.penetrationVector = Math::eVec2<float>{ combinedRadius, 0.0f }; // Push along x-axis
				result.contactPoint = posA;
			}
		}

		return result;

	} // Definition for CollisionResult CollisionDetection::CheckCircleCircle

	CollisionResult CollisionDetection::CheckRectangleRectangle(
		const Math::eVec2<float>& posA, const CollisionComponent::ColliderData::RectangleData& rectA,
		const Math::eVec2<float>& posB, const CollisionComponent::ColliderData::RectangleData& rectB)
	{
		CollisionResult result;

		// Calculate half-extents (distance from center to edge) for AABB calculations later
		float halfWidthA = rectA.width * 0.5f;
		float halfHeightA = rectA.height * 0.5f;
		float halfWidthB = rectB.width * 0.5f;
		float halfHeightB = rectB.height * 0.5f;

		// Calculate distance between between centers
		Math::eVec2<float> distance = posB - posA;

		// Calcualte overlap on each axis
		// If rectangles overlap, the overlap = (sum of half-widths) - (distance between centers)
		float overlapX = (halfWidthA + halfWidthB) - std::abs(distance.x);
		float overlapY = (halfHeightA + halfHeightB) - std::abs(distance.y);

		// Rectangles collide only if they overlap both axes!
		if (overlapX > 0 && overlapY > 0) {
			result.hasCollision = true;

			// Separate along the axis with the smallest overlap (separating the axis theorem)
			if (overlapX < overlapY)
			{
				// Separate along X-axis
				result.penetrationDepth = overlapX;
				result.penetrationVector.x = (distance.x < 0) ? -overlapX : overlapX;
				result.penetrationVector.y = 0.0f;

				// Contact point is on the edge of rectangle A
				float contactX = (distance.x < 0) ? posA.x - halfWidthA : posA.x + halfWidthA;
				result.contactPoint = Math::eVec2<float>{ contactX , posA.y };

			}
			else {
				// Separate along y-axis
				result.penetrationDepth = overlapY;
				result.penetrationVector.x = 0.0f;
				result.penetrationVector.y = (distance.y < 0) ? -overlapY : overlapY;

				// Contact point is on the edge of rectangle A
				float contactY = (distance.y < 0) ? posA.y - halfHeightA : posA.y + halfHeightA;
				result.contactPoint = Math::eVec2<float>{ posA.x, contactY };
			}
		}

		return result;

	} // Definition for CheckRectangleRectangle

	CollisionResult CollisionDetection::CheckCircleRectangle(
		const Math::eVec2<float>& circlePos, float radius,
		const Math::eVec2<float>& rectPos, const CollisionComponent::ColliderData::RectangleData& rect)
	{
		CollisionResult result;

		// Calculate rectangle bounds
		float halfWidth = rect.width * 0.5f;
		float halfHeight = rect.height * 0.5f;

		// Find the closest point on the rectangle to the circle center
		// Handles both cases:
		//	1. Circle outside rectangle
		//	2. Circle inside rectangle
		Math::eVec2<float> closestPoint;
		closestPoint.x = std::max(rectPos.x - halfWidth, std::min(circlePos.x, rectPos.x + halfWidth));
		closestPoint.y = std::max(rectPos.y - halfHeight, std::min(circlePos.y, rectPos.y + halfWidth));

		// Calculate distance from circle center to closest point on rectangle
		Math::eVec2<float> distance = closestPoint - circlePos;
		float distanceSquared = Math::eVec2SquaredLength(distance);

		// Check if circle overlaps with rectangle
		// Circle overlaps if distance to closest point < radius
		if (distanceSquared < (radius * radius)) {
			result.hasCollision = true;
			result.contactPoint = closestPoint;

			float actualDistance = Math::eVec2Length(distance);

			if (actualDistance > 0.001f) {

				// Normal case: circle is outside or touching the rectangle edge
				Math::eVec2<float> normalizedDistance = distance / actualDistance;
				result.penetrationDepth = radius - actualDistance;
				result.penetrationVector = -normalizedDistance * result.penetrationDepth; // Negative to push circle awway

			}
			else {
				// Special case: circle center is inside rectangle
				// Need to find the shortest path to push the circle out
				float distToLeft = (circlePos.x - (rectPos.x - halfWidth));
				float distToRight = ((rectPos.x + halfWidth) - circlePos.x);
				float distToTop = (circlePos.y - (rectPos.y - halfHeight));
				float distToBottom = ((rectPos.y + halfHeight) - circlePos.y);

				// Find the shortest distance to and edge
				float minDist = std::min({ distToLeft, distToRight, distToTop, distToBottom });

				// Push circle out through the nearest edge
				if (minDist == distToLeft) {
					result.penetrationVector = Math::eVec2<float>{ -(distToLeft + radius), 0.0f };
				}
				else if (minDist == distToRight) {
					result.penetrationVector = Math::eVec2<float>{ distToRight + radius, 0.0f };
				}
				else if (minDist == distToTop) {
					result.penetrationVector = Math::eVec2<float>{ 0.0f, -(distToTop + radius) };
				}
				else // minDist == distToBottom
				{
					result.penetrationVector = Math::eVec2<float>{ 0.0f, distToBottom + radius };

				}

				result.penetrationDepth = Math::eVec2Length(result.penetrationVector);

			}
		}

		return result;

	} //Definition for CheckCircleRectangle

	CollisionResult CollisionDetection::CheckPointCollider(
		const Math::eVec2<float>& pointPos, const CollisionComponent& collider,
		const Math::eVec2<float>& colliderPos)
	{
		CollisionResult result;

		switch (collider.type)
		{
		case ColliderType::CIRCLE: {
			// Point vs circle: check if point is within circle radius
			Math::eVec2<float> distance = pointPos - colliderPos;
			float distanceSquared = Math::eVec2SquaredLength(distance);
			float radiusSquared = collider.data.circle.radius * collider.data.circle.radius;

			if (distanceSquared <= radiusSquared) {
				result.hasCollision = true;
				result.contactPoint = pointPos;

				// Calculate penetration for point inside circle
				float actualDistance = Math::eVec2Length(distance);
				if (actualDistance > 0.001f) {
					Math::eVec2<float> normalizedDistance = distance / actualDistance;
					result.penetrationDepth = collider.data.circle.radius - actualDistance;
					result.penetrationVector = normalizedDistance * result.penetrationDepth;
				}
			}
			break;
		}

		case ColliderType::RECTANGLE:
		{
			// Point vs rectangle: check if point is within rectangle bounds
			float halfWidth = collider.data.rectangle.width * 0.5f;
			float halfHeight = collider.data.rectangle.height * 0.5f;

			bool insideX = (pointPos.x >= colliderPos.x - halfWidth) && (pointPos.x <= colliderPos.x + halfWidth);
			bool insideY = (pointPos.y >= colliderPos.y - halfHeight) && (pointPos.y <= colliderPos.y + halfHeight);

			if (insideX && insideY) {
				result.hasCollision = true;
				result.contactPoint = pointPos;

				// For point collisions, penetration info is usually not needed
				result.penetrationDepth = 0.0f;
				result.penetrationVector = Math::eVec2<float>{ 0.0f, 0.0f };
			}

			break;

		}

		case ColliderType::POINT:
		{
			// Point vs point: check if they're in the same position
			Math::eVec2<float> distance = pointPos - colliderPos;
			float distanceSquared = Math::eVec2SquaredLength(distance);

			if (distanceSquared < 0.001f)
			{
				result.hasCollision = true;
				result.contactPoint = pointPos;
				result.penetrationDepth = 0.0f;
				result.penetrationVector = Math::eVec2<float>{ 0.0f, 0.0f };

			}

			break;
		}
		}

		return result;

	} // Definition for CheckPointCollider

	bool CollisionDetection::CheckCircleLine(const Math::eVec2<float>& circlePos, const float& radius, const Math::eVec2<float>& endPoint, const CollisionComponent& lineCol,
		Math::eVec2<float>& pointAtCol, Math::eVec2<float>& normalAtCol, float& colTi)
	{


		if ((Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) -
			Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1)) <= -radius)
		{
			// We consider an imaginary line that is -R away, where we test for the oppposite direction of the line's outward normal -> inner plane
			// Check whether the normalized velocity is within the end points of the line segment
			// Velocity V = end pt - start pt
			Math::eVec2<float> vel{ endPoint - circlePos };

			// Normalized vector M = normalized(V.y, -V.x)
			Math::eVec2<float> mVel;
			Math::eVec2<float> tmp{ vel.y, -vel.x };
			Math::eVec2Normalize(mVel, tmp);

			// Calculate end points of imaginary line as p1' and p2'
			// P1' = P1 � R*N and P2' = P2 � R*N 

			Math::eVec2<float> innerP1 = lineCol.data.line.p1 - (lineCol.data.line.pNormal * radius);
			Math::eVec2<float> innerP2 = lineCol.data.line.p2 - (lineCol.data.line.pNormal * radius);

			// Check if the circle is moving towards the imaginary line: M.Bsp0' * M.Bsp1' < 0
			if ((Math::eVec2DotProduct(mVel, (innerP1 - circlePos)) * Math::eVec2DotProduct(mVel, (innerP2 - circlePos))) < 0)
			{
				// Calculate the time of intersection
				// Ti = (N.P0 - N.Bs - R)/(N.V)
				colTi = ((Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1) -
					Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) - radius) /
					Math::eVec2DotProduct(lineCol.data.line.pNormal, vel));

				// If time of intersection occurs within the next frame
				if (0 <= colTi && colTi <= 1.f) {

					//intersection pt = Bs + V*Ti
					pointAtCol = circlePos + (vel * colTi);
					normalAtCol = -lineCol.data.line.pNormal;
					return true;

				}
			}

		}

		else if ((Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) -
			Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1)) >= radius)
		{
			// velocity V = end pt - start pt
			Math::eVec2<float> vel{ endPoint - circlePos };

			// Normalized vector M = normalized(V.y, -V.x)
			Math::eVec2<float> mVel;
			Math::eVec2Normalize(mVel, { vel.y, -(vel.x) });

			// Calculate end points of imaginary line as p1' and p2'
			// P1' = P1 + R*N and P2' = P2 + R*N 
			Math::eVec2<float> outerP1 = lineCol.data.line.p1 + (lineCol.data.line.pNormal * radius);
			Math::eVec2<float> outerP2 = lineCol.data.line.p2 + (lineCol.data.line.pNormal * radius);
			// Check if the circle is moving towards the imaginary line: M.Bsp0' * M.Bsp1' < 0
			if ((Math::eVec2DotProduct(mVel, (outerP1 - circlePos)) * Math::eVec2DotProduct(mVel, (outerP2 - circlePos))) < 0)
			{
				// Calculate the time of intersection
				// Ti = (N.P0 - N.Bs + R)/(N.V)
				colTi = ((Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1) -
					Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) + radius) /
					Math::eVec2DotProduct(lineCol.data.line.pNormal, vel));
				// If time of intersection occurs within the next frame
				if (0 <= colTi && colTi <= 1.f)
				{
					// Intersection pt = Bs + V*Ti
					pointAtCol = circlePos + (vel * colTi);
					normalAtCol = lineCol.data.line.pNormal;
					return true;
				}
			}

		}

		return false;
	}

	void CollisionDetection::CollisionReflection(const Math::eVec2<float>& interPt,
		const Math::eVec2<float>& normal,
		Math::eVec2<float>& nextPos,
		Math::eVec2<float>& reflected)
	{

		// Penetration = BiBe
		Math::eVec2<float> penetration = nextPos - interPt;
		// B'e = Bi + penetration - 2(penetration.normal) * normal
		nextPos = interPt + penetration - (2.0f * (Math::eVec2DotProduct(penetration, normal)) * normal);

		// New position = collision point + reflected penetration
		reflected = nextPos - interPt;

		// Return normalized reflection direction
		Math::eVec2Normalize(reflected, reflected);

	}

	bool CollisionDetection::CheckDynamicCircleCircle(
		const Math::eVec2<float>& posA, const Math::eVec2<float>& velA, float radiusA,
		const Math::eVec2<float>& posB, const Math::eVec2<float>& velB, float radiusB,
		Math::eVec2<float>& BeA, Math::eVec2<float>& BeB, float& colTi)
	{

		const Math::eVec2<float> BaBb = posB - posA;

		// Get relative velocity
		Math::eVec2<float> relativeVel = velA - velB;
		Math::eVec2<float> normalizedRV;
		Math::eVec2Normalize(normalizedRV, relativeVel);

		// Check moving point Bsa with velocity Rv against combined circle radii
		Math::eVec2<float> circleC = posB;
		float circleCRad = radiusA + radiusB;

		// First rejection test
		float m = Math::eVec2DotProduct(BaBb, normalizedRV);

		if (m < 0) return false;

		// Second rejection test

		float dist2 = Math::eVec2SquaredLength(BaBb) - (m * m);

		if (dist2 > radiusA * radiusA)
		{
			return false;
		}
		// Compute s^2 = r^2 - dist^2
		float sSquared = (circleCRad * circleCRad) - dist2;

		// calculate time of collision
		colTi = (m - sqrt(sSquared)) / Math::eVec2Length(relativeVel);

		if (0 <= colTi && colTi <= 1.0f)
		{
			// Apply physical reflection
			Math::eVec2<float> Bia = { posA.x, posA.y };
			Bia += velA * colTi;

			Math::eVec2<float> Bib = { posB.x, posB.y };
			Bib += velB * colTi;

			// Compute normal of reflection
			Math::eVec2<float> normalVec = Bia - Bib;
			Math::eVec2Normalize(normalVec, normalVec);

			// Compute length of Va
			Math::eVec2<float> aVel = velA;
			float aA = Math::eVec2DotProduct(aVel, normalVec);

			// Compute length of Vb
			Math::eVec2<float> bVel = velB;
			float aB = Math::eVec2DotProduct(bVel, normalVec);

			// Reflected vector Va'
			Math::eVec2<float> vAprime = aVel - (2 * (aA - aB) / (2.0f + 5.0f)) * 2.0f * normalVec;

			// Reflected vector Vb
			Math::eVec2<float> vBprime = bVel + (2 * (aA - aB) / (3.0f + 4.0f)) * 3.0f * normalVec;

			// Compute BeA
			Bia += ((1.f - colTi) * vAprime);
			BeA = Bia;

			// Compute Beb
			Bib += (1 - colTi) * vBprime;
			BeB = Bib;
			return true;
		}
		return false;
	}

	// COLLISION EVENTS IMPLEMENTATION

	CollisionEvent::CollisionEvent()
		: entityA_ID(0)
		, entityB_ID(0)
		, isEntityA_Trigger(false)
		, isEntityB_Trigger(false)
	{
	}

	TriggerEnterEvent::TriggerEnterEvent()
		: triggerEntityID(0)
		, otherEntityID(0)
		, contactPoint{ 0.0f, 0.0f }
	{
	}

	TriggerExitEvent::TriggerExitEvent()
		:triggerEntityID(0)
		, otherEntityID(0)
	{
	}

	// COLLISION SYSTEM IMPLEMENTATION

	CollisionSystem::CollisionSystem(Event::EnoEventBus& event_bus) : eventBus_(event_bus)
	{
		// Storing event bus reference
		// System is ready to use 
	}

	void CollisionSystem::Update(ECS::EntityManager& entityManager)
	{
		// ==== BROAD PHASE =====
		// Only dynamic (non-static) entities are inserted each frame.
		// Static entities live in staticHash_, which is rebuilt only when tiles change.
		spatialHash_.Clear();

		auto& entityMap = entityManager.getMap();

		struct DynamicEntry {
			uint32_t id;
			ECS::EntityLoc* loc;
			Math::eVec2<float> pos;
			float hw, hh;
		};
		std::vector<DynamicEntry> dynamicEntities;
		dynamicEntities.reserve(64);

		for (auto& [entityID, entityLoc] : entityMap)
		{
			if (!HasRequiredComponents(entityLoc)) continue;

			auto& collision = GetCollisionComponent(entityLoc);
			if (!collision.isActive || collision.isStatic) continue;

			auto& transform = GetTransformComponent(entityLoc);
			Math::eVec2<float> worldPos = {
				transform.pos.x + collision.offset.x,
				transform.pos.y + collision.offset.y
			};
			float hw, hh;
			GetColliderBounds(collision, transform, hw, hh);

			spatialHash_.Insert(entityID, worldPos, hw, hh);
			dynamicEntities.push_back({ entityID, &entityLoc, worldPos, hw, hh });
		}

		// ==== NARROW PHASE ====

		// Dynamic-dynamic pairs from the dynamic hash
		for (const auto& [idA, idB] : spatialHash_.GetPotentialPairs())
		{
			auto itA = entityMap.find(idA);
			auto itB = entityMap.find(idB);
			if (itA == entityMap.end() || itB == entityMap.end()) continue;
			CheckEntityCollision(idA, itA->second, idB, itB->second);
		}

		// Dynamic-static pairs: query the persistent static hash for each dynamic entity
		for (const auto& dyn : dynamicEntities)
		{
			auto nearbyStatics = staticHash_.Query(dyn.pos, dyn.hw, dyn.hh);

			// Deduplicate (an entity can appear in multiple cells)
			std::sort(nearbyStatics.begin(), nearbyStatics.end());
			nearbyStatics.erase(std::unique(nearbyStatics.begin(), nearbyStatics.end()), nearbyStatics.end());

			for (uint32_t staticID : nearbyStatics)
			{
				auto itStat = entityMap.find(staticID);
				if (itStat == entityMap.end()) continue;
				CheckEntityCollision(dyn.id, *dyn.loc, staticID, itStat->second);
			}
		}
	}

	void CollisionSystem::RebuildStaticEntities(ECS::EntityManager& entityManager)
	{
		staticHash_.Clear();

		auto& entityMap = entityManager.getMap();
		for (auto& [entityID, entityLoc] : entityMap)
		{
			if (!HasRequiredComponents(entityLoc)) continue;

			auto& collision = GetCollisionComponent(entityLoc);
			if (!collision.isActive || !collision.isStatic) continue;

			auto& transform = GetTransformComponent(entityLoc);
			Math::eVec2<float> worldPos = {
				transform.pos.x + collision.offset.x,
				transform.pos.y + collision.offset.y
			};
			float hw, hh;
			GetColliderBounds(collision, transform, hw, hh);

			staticHash_.Insert(entityID, worldPos, hw, hh);
		}
	}

	void CollisionSystem::AddCollisionCallback(std::function<void(const CollisionEvent&)> callback)
	{
		collisionCallbacks_.push_back(callback);
	}

	void CollisionSystem::VisualizeColliders(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager)
	{
		auto& entityMap = entityManager.getMap();

		// Iterate through all entities
		for (auto& [entityID, entityLoc] : entityMap)
		{
			// Check if entity has required components
			if (!HasRequiredComponents(entityLoc))
				continue;

			// Get transform and collision components
			auto& transform = GetTransformComponent(entityLoc);
			auto& collision = GetCollisionComponent(entityLoc);

			// Skip if collider is inactive
			if (!collision.isActive)
				continue;

			// Calculate world position
			Math::eVec2<float> worldPos = {
				transform.pos.x + collision.offset.x,
				transform.pos.y + collision.offset.y
			};

			// Choose color based on trigger status
			Graphics::Color color = collision.isTrigger ? Graphics::Color::Red : Graphics::Color::Red;

			// Draw based on collider type
			switch (collision.type)
			{
			case ColliderType::CIRCLE:graphics.drawCircle(
				worldPos,
				collision.data.circle.radius * transform.scale.x,  // Scale the radius
				color,
				false,
				32
			);
				break;

			case ColliderType::RECTANGLE:
			{

				// Apply transform scale to the visualization
				float scaledWidth = collision.data.rectangle.width * transform.scale.x;
				float scaledHeight = collision.data.rectangle.height * transform.scale.y;


				// Calculate top-left corner (drawRect uses top-left pos), but worldPos is CENTER
				Math::eVec2<float> topLeft = {
					worldPos.x - (scaledWidth * 0.5f),
					worldPos.y - (scaledHeight * 0.5f)
				};


				graphics.drawRect(
					topLeft,
					Math::eVec2<float>{scaledWidth, scaledHeight},
					color,
					false,				// Not filled, just outline
					transform.rot.x		// Use transform rotation
				);

				break;
			}

			case ColliderType::LINE:
			{

				graphics.drawLine(
					collision.data.line.p1,
					collision.data.line.p2,
					color,
					1.0f
				);
				break;
			}

			case ColliderType::POINT:
				graphics.drawPoint(worldPos, color, 5.0f);
				break;

			}

		}
	}

	void CollisionSystem::VisualizeColliders(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager, Math::eVec2<float> cameraOffset)
	{
		// Get entity map
		auto& entityMap = entityManager.getMap();

		// Iterate through all entities
		for (auto& [entityID, entityLoc] : entityMap)
		{
			// Check if entity has required components
			if (!HasRequiredComponents(entityLoc))
				continue;

			// Get transform and collision components
			auto& transform = GetTransformComponent(entityLoc);
			auto& collision = GetCollisionComponent(entityLoc);

			// Skip if collider is inactive
			if (!collision.isActive)
				continue;

			// Calculate world position
			Math::eVec2<float> worldPos = {
				transform.pos.x + collision.offset.x,
				transform.pos.y + collision.offset.y
			};

			// Apply camera offset to convert to screen space
			Math::eVec2<float> screenPos = {
				worldPos.x - cameraOffset.x,
				worldPos.y - cameraOffset.y
			};

			// Choose color based on trigger status
			Graphics::Color color = collision.isTrigger ? Graphics::Color::Red : Graphics::Color::Red;

			// Draw based on collider type
			switch (collision.type)
			{
			case ColliderType::CIRCLE:
				graphics.drawCircle(
					screenPos,
					collision.data.circle.radius * transform.scale.x,  // Scale the radius
					color,
					false,
					32
				);
				break;

			case ColliderType::RECTANGLE:
			{
				// Apply transform scale to the visualization
				float scaledWidth = collision.data.rectangle.width * transform.scale.x;
				float scaledHeight = collision.data.rectangle.height * transform.scale.y;

				// Calculate top-left corner (drawRect uses top-left pos), but screenPos is CENTER
				Math::eVec2<float> topLeft = {
					screenPos.x - (scaledWidth * 0.5f),
					screenPos.y - (scaledHeight * 0.5f)
				};

				graphics.drawRect(
					topLeft,
					Math::eVec2<float>{scaledWidth, scaledHeight},
					color,
					false,				// Not filled, just outline
					transform.rot.x		// Use transform rotation
				);

				break;
			}

			case ColliderType::LINE:
			{
				graphics.drawLine(
					collision.data.line.p1,
					collision.data.line.p2,
					color,
					1.0f
				);
				break;
			}

			case ColliderType::POINT:
				graphics.drawPoint(screenPos, color, 5.0f);
				break;
			}
		}
	}

	void CollisionSystem::ClearCallbacks()
	{
		collisionCallbacks_.clear();
	}

	bool CollisionSystem::HasRequiredComponents(const ECS::EntityLoc& entityLoc) {
		bool hasTransform = false;
		bool hasCollision = false;

		// Check each component type in the entity's archetype 
		for (const auto& type : entityLoc.archetype->types_)
		{
			if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
			{
				hasTransform = true;
			}
			else if (type == std::type_index(typeid(ECSComponents::CollisionComponent)))
			{
				hasCollision = true;
			}
		}

		// Entity must have both components to participate in collision detection
		return hasTransform && hasCollision;
	}

	void CollisionSystem::CheckEntityCollision(
		uint32_t entityA_ID, const ECS::EntityLoc& entityA_Loc,
		uint32_t entityB_ID, const ECS::EntityLoc& entityB_Loc)
	{
		try
		{
			// Get the required components from both entities
			auto& transformA = GetTransformComponent(entityA_Loc);
			auto& transformB = GetTransformComponent(entityB_Loc);
			auto& collisionA = GetCollisionComponent(entityA_Loc);
			auto& collisionB = GetCollisionComponent(entityB_Loc);

			// Perform the actual collision detection
			CollisionResult result = CollisionDetection::CheckCollision(
				collisionA, transformA,
				collisionB, transformB
			);

			// If a collision was detected, handle it
			if (result.hasCollision)
			{
				HandleCollision(entityA_ID, entityB_ID, collisionA, collisionB, result);
			}
		}
		catch (const std::exception& e)
		{
			// Log errors but don't crash the game
			std::cerr << "Error in collision detection between entities "
				<< entityA_ID << " and " << entityB_ID
				<< ": " << e.what() << std::endl;
		}
	}


	void CollisionSystem::HandleCollision(
		uint32_t entityA_ID, uint32_t entityB_ID,
		const CollisionComponent& collisionA, const CollisionComponent& collisionB,
		const CollisionResult& result)
	{
		// Create and populate the collision event
		CollisionEvent event;
		event.entityA_ID = entityA_ID;
		event.entityB_ID = entityB_ID;
		event.result = result;
		event.isEntityA_Trigger = collisionA.isTrigger;
		event.isEntityB_Trigger = collisionB.isTrigger;

		// Publish the event to the event bus for other systems to handle 
		eventBus_.publish(event);

		// Call all registered callbacks
		for (auto& callback : collisionCallbacks_)
		{
			try
			{
				callback(event);
			}
			catch (const std::exception& e) {
				std::cerr << "Error in collision callback: " << e.what() << std::endl;
			}
		}

		// Debug logging (only in debug builds to avoid performance impact)
//#ifdef _DEBUG
//		std::cout << "Collision detected between Entity " << entityA_ID
//			<< " and Entity " << entityB_ID
//			<< " | Penetration Depth: " << result.penetrationDepth
//			<< " | Trigger: " << (event.isEntityA_Trigger || event.isEntityB_Trigger ? "Yes" : "No")
//			<< std::endl;
//#endif	
	}

	ECSComponents::TransformComponent& CollisionSystem::GetTransformComponent(const ECS::EntityLoc& entityLoc)
	{
		// Find the Transform component's index in this entity's archetype
		size_t transformIndex = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();

		// Retrieve the component from the appropriate chunk
		return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
			transformIndex, entityLoc.entityIndexInChunk);
	}

	CollisionComponent& CollisionSystem::GetCollisionComponent(const ECS::EntityLoc& entityLoc)
	{
		size_t collisionIndex = entityLoc.archetype->findComponentIndex<ECSComponents::CollisionComponent>();
		return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::CollisionComponent>(
			collisionIndex, entityLoc.entityIndexInChunk);
	}

	void CollisionSystem::GetColliderBounds(
		const CollisionComponent& collision,
		const ECSComponents::TransformComponent& transform,
		float& halfWidth,
		float& halfHeight
	)
	{
		// Calculate bounding box based on collider type
		switch (collision.type)
		{
			case ColliderType::CIRCLE:
			{
				// Circle: bounding box is a square with side = 2 * radius
				float scaledRadius = collision.data.circle.radius * transform.scale.x;
				halfWidth = scaledRadius;
				halfHeight = scaledRadius;
				break;
			}
			case ColliderType::RECTANGLE:
			{
				// Rectangle: use scaled dimensions
				// For rotated rectangles, we use the AABB that contains the rotated rect
				float scaledWidth = collision.data.rectangle.width * transform.scale.x;
				float scaledHeight = collision.data.rectangle.height * transform.scale.y;

				// If rectangle is rotated, compute the AABB of the rotated rectangle 
				if (std::abs(transform.rot.x) > 0.001f)
				{
					float cosAngle = std::abs(std::cos(transform.rot.x));
					float sinAngle = std::abs(std::sin(transform.rot.x));

					// AABB dimensions of a rotated rectangle
					halfWidth = (scaledWidth * cosAngle + scaledHeight * sinAngle) * 0.5f;
					halfHeight = (scaledWidth * sinAngle + scaledHeight * cosAngle) * 0.5f;
				}
				else
				{
					halfWidth = scaledWidth * 0.5f;
					halfHeight = scaledHeight * 0.5f;
				}
				break;
			}
			case ColliderType::LINE:
			{
				// Line: Bounding box encompasses both endpoints
				// float minX = std::min(collision.data.line.p1.x, collision.data.line.p2.x); this is unused - zhe an
				float maxX = std::max(collision.data.line.p1.x, collision.data.line.p2.x);
				float minY = std::min(collision.data.line.p1.y, collision.data.line.p2.y);
				float maxY = std::max(collision.data.line.p1.y, collision.data.line.p2.y);

				halfWidth = (maxX - minY) * 0.5f + 1.0f; // Add small padding
				halfHeight = (maxY - minY) * 0.5f + 1.0f;
				break;
			}
			case ColliderType::POINT:
			{
				// Point: use a small bounding box
				halfWidth = 1.0f;
				halfHeight = 1.0f;
				break;
			}
			default:
				halfWidth = 1.0f;
				halfHeight = 1.0f;
				break;
		}
	}

	// UTILITY FUNCTIONS IMPLEMENTATION

	void RunBasicCollisionTest()
	{
		std::cout << "=== Basic Collision System Test ===" << std::endl;

		// This is a simple test to verify collision detection works
		// In a real game, you'd create entities through the entity manager

		// Test 1: Circle vs Circle collision
		std::cout << "\nTest 1: Circle vs Circle" << std::endl;
		CollisionComponent circleA(1.0f);  // Radius 1.0 at origin
		CollisionComponent circleB(1.0f);  // Radius 1.0

		ECSComponents::TransformComponent transformA{ {0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f} };
		ECSComponents::TransformComponent transformB{ {1.5f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f} };

		CollisionResult result1 = CollisionDetection::CheckCollision(circleA, transformA, circleB, transformB);
		std::cout << "Collision detected: " << (result1.hasCollision ? "Yes" : "No") << std::endl;
		if (result1.hasCollision) {
			std::cout << "Penetration depth: " << result1.penetrationDepth << std::endl;
		}

		// Test 2: Rectangle vs Rectangle collision
		std::cout << "\nTest 2: Rectangle vs Rectangle" << std::endl;
		CollisionComponent rectA(2.0f, 2.0f);  // 2x2 rectangle
		CollisionComponent rectB(2.0f, 2.0f);  // 2x2 rectangle

		ECSComponents::TransformComponent transformC{ {0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f} };
		ECSComponents::TransformComponent transformD{ {1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f} };

		CollisionResult result2 = CollisionDetection::CheckCollision(rectA, transformC, rectB, transformD);
		std::cout << "Collision detected: " << (result2.hasCollision ? "Yes" : "No") << std::endl;
		if (result2.hasCollision) {
			std::cout << "Penetration depth: " << result2.penetrationDepth << std::endl;
		}

		std::cout << "\n=== Test Complete ===" << std::endl;
	}

	//// ====================== SAT COLLISION IMPLEMENTATION ===========================/
	//
	//// OBB constructor
	//CollisionDetection::OBB::OBB(const ECSComponents::TransformComponent& transform, const CollisionComponent& collision)
	//{
	//	// Set center position (transform position + collision offset)
	//	center.v1 = transform.pos.x + collision.offset.v1;
	//	center.v2 = transform.pos.y + collision.offset.v2;
	//
	//	// Set half extents (scaled by transform scale)
	//	halfExtents.v1 = (collision.data.rectangle.width * transform.scale.x) * 0.5f;
	//	halfExtents.v2 = (collision.data.rectangle.height * transform.scale.y) * 0.5f;
	//
	//	// Calculate rotation angle from transform (assuming Z rotation)
	//	float angle = transform.rot.z;
	//	float cosAngle = std::cos(angle);
	//	float sinAngle = std::sin(angle);
	//
	//	// Set local axes based on rotation
	//	axes[0].v1 = cosAngle;	// X-axis
	//	axes[0].v2 = sinAngle;
	//	axes[1].v1 = -sinAngle;	// Y-axis
	//	axes[1].v2 = cosAngle;
	//}
	//
	//void CollisionDetection::OBB::GetCorners(Math::eVec2<float> corners[4]) const
	//{
	//	// Calculate corner offsets
	//	Math::eVec2<float> xOffset = { axes[0].v1 * halfExtents.v1, axes[0].v2 * halfExtents.v1 };
	//	Math::eVec2<float> yOffset = { axes[1].v1 * halfExtents.v2, axes[1].v2 * halfExtents.v2 };
	//
	//	// Calculate 4 corners
	//	corners[0] = { center.v1 - xOffset.v1 - yOffset.v1, center.v2 - xOffset.v2 - yOffset.v2 };
	//	corners[1] = { center.v1 + xOffset.v1 - yOffset.v1, center.v2 + xOffset.v2 - yOffset.v2 };
	//	corners[2] = { center.v1 + xOffset.v1 + yOffset.v1, center.v2 + xOffset.v2 + yOffset.v2 };
	//	corners[3] = { center.v1 - xOffset.v1 + yOffset.v1, center.v2 - xOffset.v2 + yOffset.v2 };
	//
	//}
	//
	//void CollisionDetection::ProjectOBB(const OBB& obb, const Math::eVec2<float>& axis, float& min, float& max)
	//{
	//	Math::eVec2<float> corners[4];
	//	obb.GetCorners(corners);
	//	
	//	min = max = Math::eVec2DotProduct(corners[0], axis);
	//
	//	for (int i = 1; i < 4; ++i)
	//	{
	//		float projection = Math::eVec2DotProduct(corners[i], axis);
	//		if (projection < min) min = projection;
	//		if (projection > max) max = projection;
	//	}
	//}
	//

	// ====================== SAT COLLISION DETECTION HELPER FUNCTIONS ===========================/

	/*============================================================================================
	* \brief       Get the corners of a rotated rectangle in world space
	* \param[in]   Math::eVec2<float> const& center - Center position of the rectangle
	* \param[in]   float width - Width of the rectangle
	* \param[in]   float height - Height of the rectangle
	* \param[in]   float rotation - Rotation angle in radians
	* \param[out]  Math::eVec2<float> corners[4] - Array to store the 4 corners
	* \return      void
	*===========================================================================================*/
	void GetRotatedRectangleCorners(
		const Math::eVec2<float>& center,
		float width,
		float height,
		float rotation,
		Math::eVec2<float> corners[4])
	{
		// Calculate half extents
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;

		// Pre-calculate sin and cos
		float cosAngle = std::cos(rotation);
		float sinAngle = std::sin(rotation);

		// Local corner positions (before rotation)
		Math::eVec2<float> localCorners[4] = {
			{ -halfWidth, -halfHeight }, // Bottom-left
			{ halfWidth, -halfHeight }, // Bottom-right
			{ halfWidth, halfHeight },   // Top-right
			{ -halfWidth, halfHeight }, // Top-left
		};

		// Rotate and translate each corner
		for (int i = 0; i < 4; ++i) {
			// Rotate the corner
			float rotatedX = localCorners[i].x * cosAngle - localCorners[i].y * sinAngle;
			float rotatedY = localCorners[i].x * sinAngle + localCorners[i].y * cosAngle;

			// Translate to world position
			corners[i].x = center.x + rotatedX;
			corners[i].y = center.y + rotatedY;
		}
	}

	/*============================================================================================
	* \brief       Get the two perpendicular axes for a rotated rectangle (for SAT)
	* \param[in]   float rotation - Rotation angle in radians
	* \param[out]  Math::eVec2<float> axes[2] - Array to store the 2 perpendicular axes
	* \return      void
	*===========================================================================================*/
	void GetRectangleAxes(float rotation, Math::eVec2<float> axes[2])
	{
		// Pre-calculate sin and cos
		float cosAngle = std::cos(rotation);
		float sinAngle = std::sin(rotation);

		// First axis (direction of width)
		axes[0].x = cosAngle;
		axes[0].y = sinAngle;

		// Second axis (direction of height) - perpendicular to first
		axes[1].x = -sinAngle;
		axes[1].y = cosAngle;
	}

	/*============================================================================================
	* \brief       Project a polygon onto an axis and get min/max scalar projections
	* \param[in]   Math::eVec2<float> const corners[] - Array of corner points
	* \param[in]   int numCorners - Number of corners
	* \param[in]   Math::eVec2<float> const& axis - Axis to project onto (must be normalized)
	* \param[out]  float& min - Minimum projection value
	* \param[out]  float& max - Maximum projection value
	* \return      void
	*===========================================================================================*/
	void ProjectPolygonOntoAxis(
		const Math::eVec2<float> corners[],
		int numCorners,
		const Math::eVec2<float>& axis,
		float& min,
		float& max)
	{
		// Project first corner
		min = max = Math::eVec2DotProduct(corners[0], axis);

		// Project remaining corners
		for (int i = 1; i < numCorners; ++i) {
			float projection = Math::eVec2DotProduct(corners[i], axis);
			if (projection < min) min = projection;
			if (projection > max) max = projection;
		}
	}

	/*============================================================================================
	* \brief       Check if two projection ranges overlap on an axis
	* \param[in]   float minA - Minimum projection of shape A
	* \param[in]   float maxA - Maximum projection of shape A
	* \param[in]   float minB - Minimum projection of shape B
	* \param[in]   float maxB - Maximum projection of shape B
	* \return      float - Overlap amount (positive if overlapping, negative if separated)
	*===========================================================================================*/
	float GetProjectionOverlap(float minA, float maxA, float minB, float maxB)
	{
		// Check if there's any overlap
		if (maxA < minB || maxB < minA) {
			return -1.0f; // No overlap - shapres are separated on this axis
		}

		// Calculate overlap amount
		float overlap1 = maxA - minB;	// A's max exceeds B's min
		float overlap2 = maxB - minA;	// B's max exceeds A's min

		// Return the smaller overlap (this is the penetration depth)
		return std::min(overlap1, overlap2);
	}

	/*============================================================================================
	* \brief       SAT collision detection for two rotated rectangles (OBB vs OBB)
	* \param[in]   Math::eVec2<float> const& posA - Center position of rectangle A
	* \param[in]   CollisionComponent::ColliderData::RectangleData const& rectA - Size data of rectangle A
	* \param[in]   float rotationA - Rotation angle of rectangle A in radians
	* \param[in]   Math::eVec2<float> const& posB - Center position of rectangle B
	* \param[in]   CollisionComponent::ColliderData::RectangleData const& rectB - Size data of rectangle B
	* \param[in]   float rotationB - Rotation angle of rectangle B in radians
	* \return      CollisionResult - Contains collision information (hasCollision, penetration vector, etc.)
	*===========================================================================================*/

	CollisionResult CollisionDetection::CheckRectangleRectangleSAT(
		const Math::eVec2<float>& posA,
		const CollisionComponent::ColliderData::RectangleData& rectA,
		float rotationA,
		const Math::eVec2<float>& posB,
		const CollisionComponent::ColliderData::RectangleData& rectB,
		float rotationB)
	{
		CollisionResult result;

		// Get corners of both rectangles in world space
		Math::eVec2<float> cornersA[4];
		Math::eVec2<float> cornersB[4];
		GetRotatedRectangleCorners(posA, rectA.width, rectA.height, rotationA, cornersA);
		GetRotatedRectangleCorners(posB, rectB.width, rectB.height, rotationB, cornersB);

		// Get the axes to test (perpendicular edges of both rectangles)
		Math::eVec2<float> axesA[2];
		Math::eVec2<float> axesB[2];
		GetRectangleAxes(rotationA, axesA);
		GetRectangleAxes(rotationB, axesB);

		// Track the minimum overlap and the axis it occurred on
		float minOverlap = std::numeric_limits<float>::max();
		Math::eVec2<float> separationAxis = { 0.0f, 0.0f };

		// Test all 4 axes (2 from each rectangle)
		Math::eVec2<float> allAxes[4] = { axesA[0], axesA[1], axesB[0], axesB[1] };

		for (int i = 0; i < 4; ++i) {
			// Project both rectangles onto this axis
			float minA, maxA, minB, maxB;
			ProjectPolygonOntoAxis(cornersA, 4, allAxes[i], minA, maxA);
			ProjectPolygonOntoAxis(cornersB, 4, allAxes[i], minB, maxB);

			// Check for overlap on this axis
			float overlap = GetProjectionOverlap(minA, maxA, minB, maxB);

			// If no overlap on any axis, shapes don't collide (SAT theorem)
			if (overlap < 0) {
				result.hasCollision = false;
				return result;
			}

			// Track the axis with minimum overlap (this is the MTV - Minimum Translation Vector)
			if (overlap < minOverlap) {
				minOverlap = overlap;
				separationAxis = allAxes[i];
			}
		}

		// If we get here, all axes overlapped - collision detected!
		result.hasCollision = true;
		result.penetrationDepth = minOverlap;

		// Determine the direction of the separation axis
		// Make sure the penetration vector points from A to B
		Math::eVec2<float> centerToCenter = posB - posA;
		if (Math::eVec2DotProduct(separationAxis, centerToCenter) < 0) {
			// Flip the axis if it points in the wrong direction
			separationAxis.x = -separationAxis.x;
			separationAxis.y = -separationAxis.y;
		}

		// Set the penetration vector (multiply axis by depth)
		result.penetrationVector.x = separationAxis.x * minOverlap;
		result.penetrationVector.y = separationAxis.y * minOverlap;

		// Calculate contact point (approximate as midpoint between centers along separation axis)
		// For a more accurate contact point, we would need to find the actual collision edge
		result.contactPoint.x = (posA.x + posB.x) * 0.5f;
		result.contactPoint.y = (posA.y + posB.y) * 0.5f;

		return result;

	}
} // namespace Collision