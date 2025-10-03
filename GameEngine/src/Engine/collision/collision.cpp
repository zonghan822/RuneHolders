
#include "collision.hpp"

namespace Collision {

	// -------------------------------------------------------------------------
	// COLLISION COMPONENT IMPLMENTATION
	// -------------------------------------------------------------------------

	CollisionComponent::CollisionComponent()
		: type(ColliderType::CIRCLE)
		, offset{ 0.0f, 0.0f }
		, isTrigger(false)
		, isActive(true)
	{
		data.circle.radius = 1.0f;
	}

	// circle
	CollisionComponent::CollisionComponent(float radius, Math::eVec2<float> colliderOffset, bool trigger)
		: type(ColliderType::CIRCLE)
		, offset(colliderOffset)
		, isTrigger(trigger)
		, isActive(true)
	{
		data.circle.radius = radius;
	}

	// rectangle
	CollisionComponent::CollisionComponent(float width, float height, Math::eVec2<float> colliderOffset, bool trigger)
		: type(ColliderType::RECTANGLE)
		, offset(colliderOffset)
		, isTrigger(trigger)
		, isActive(true)
	{
		data.rectangle.width = width;
		data.rectangle.height = height;
	}

	// line
	CollisionComponent::CollisionComponent(Math::eVec2<float> point1, Math::eVec2<float> point2, bool trigger)
		: type(ColliderType::LINE)
		, offset{ 0.f, 0.f }
		, isTrigger(trigger)
		, isActive(true)
	{
		data.line.p1 = point1;
		data.line.p2 = point2;
		// Calculate edge vector (from p1 to p2)
		Math::eVec2<float> edge = data.line.p2 - data.line.p1;
		// Calculate perpendicular (rotate 90° counter-clockwise)
		Math::eVec2<float> perpendicular = { -edge.v2, edge.v1 };
		// Normalize to get the outward normal
		Math::eVec2Normalize(data.line.pNormal, perpendicular);
	}

	// point
	CollisionComponent CollisionComponent::CreatePointCollider(Math::eVec2<float> colliderOffset, bool trigger) {
		CollisionComponent comp;
		comp.type = ColliderType::POINT;
		comp.offset = colliderOffset;
		comp.isTrigger = trigger;
		comp.isActive = true;
		return comp;
	}

	// circle
	void CollisionComponent::SetRadius(float radius)
	{
		if (type == ColliderType::CIRCLE)
		{
			data.circle.radius = radius;
		}
	}

	void CollisionComponent::SetRectangleSize(float width, float height)
	{
		if (type == ColliderType::RECTANGLE)
		{
			data.rectangle.width = width;
			data.rectangle.height = height;
		}
	}

	void CollisionComponent::SetOffset(const Math::eVec2<float>& newOffset)
	{
		offset = newOffset;
	}

	void CollisionComponent::SetActive(bool active)
	{
		isActive = active;
	}

	void CollisionComponent::SetTrigger(bool trigger)
	{
		isTrigger = trigger;
	}

	float CollisionComponent::GetColliderArea() const
	{
		switch (type)
		{
		case ColliderType::CIRCLE:
			return 3.14159f * data.circle.radius * data.circle.radius;
		case ColliderType::RECTANGLE:
			return data.rectangle.width * data.rectangle.height;
		case ColliderType::POINT:
			return 0.0f;
		default:
			return 0.0f;
		}
	}

	std::ostream& operator<<(std::ostream& os, const CollisionComponent& collision)
	{
		os << "CollisionComponent: ";
		switch (collision.type)
		{
		case ColliderType::CIRCLE:
			os << "Circle (radius: " << collision.data.circle.radius << ")";
			break;
		case ColliderType::RECTANGLE:
			os << "Rectangle (" << collision.data.rectangle.width << "x" << collision.data.rectangle.height << ")";
			break;
		case ColliderType::LINE:
			os << "Line:" << std::endl << "p1 : " << collision.data.line.p1 << "p2 : " << collision.data.line.p2 << "pNormal : " << collision.data.line.pNormal;
			break;
		case ColliderType::POINT:
			os << "Point";
			break;
		}

		os << " Offset: (" << collision.offset.v1 << ", " << collision.offset.v2 << ")";
		os << " Trigger: " << (collision.isTrigger ? "Yes" : "No");
		os << " Active: " << (collision.isActive ? "Yes" : "No");
		return os;
	}

	// -------------------------------------------------------------------------
	// COLLISION RESULT IMPLMENTATION
	// -------------------------------------------------------------------------
	CollisionResult::CollisionResult()
		: hasCollision(false)
		, penetrationVector{ 0.0f, 0.0f }
		, penetrationDepth(0.0f)
		, contactPoint{ 0.0f, 0.0f }
	{

	}

	// -------------------------------------------------------------------------
	// COLLISION DETECTION IMPLMENTATION
	// -------------------------------------------------------------------------
	CollisionResult CollisionDetection::CheckCollision(
		const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA,
		const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB)
	{
		// early exit: skip collision if either collider is inactive
		if (!colliderA.isActive || !colliderB.isActive)
		{
			return CollisionResult(); // return default "no collision result"
		}

		// calculate actual world positiions of the colliders
		// world position = transform position + collider offset
		Math::eVec2<float> posA = { transformA.pos.x + colliderA.offset.v1, transformA.pos.y + colliderA.offset.v2 };
		Math::eVec2<float> posB = { transformB.pos.x + colliderB.offset.v1, transformB.pos.y + colliderB.offset.v2 };

		// convey to the collision detection function accordingly
		// (BASED ON the types of the 2 colliders)

		// scenario 1: if both colliders are circles
		if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::CIRCLE)
		{	
			// scale the radii
			float radiusA = colliderA.data.circle.radius * transformA.scale.x;
			float radiusB = colliderB.data.circle.radius * transformB.scale.x;
			return CheckCircleCircle(posA, radiusA, posB, radiusB);
		}

		// scenario 2: if both colliders are rectangles
		else if (colliderA.type == ColliderType::RECTANGLE && colliderB.type == ColliderType::RECTANGLE)
		{
			// Scale rectangle dimensions
			CollisionComponent::ColliderData::RectangleData scaledRectA = colliderA.data.rectangle;
			scaledRectA.width *= transformA.scale.x;
			scaledRectA.height *= transformA.scale.y;

			CollisionComponent::ColliderData::RectangleData scaledRectB = colliderB.data.rectangle;
			scaledRectB.width *= transformB.scale.x;
			scaledRectB.height *= transformB.scale.y;

			return CheckRectangleRectangle(posA, scaledRectA, posB, scaledRectB);
		}

		// scenario 3: if collider A is a circle, and collider B is a rectangle
		else if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::RECTANGLE)
		{
			float radiusA = colliderA.data.circle.radius * transformA.scale.x;
			CollisionComponent::ColliderData::RectangleData scaledRectB = colliderB.data.rectangle;
			scaledRectB.width *= transformB.scale.x;
			scaledRectB.height *= transformB.scale.y;

			return CheckCircleRectangle(posA, radiusA, posB, scaledRectB);
		}

		// scenario 4: if collider A is a rectangle, and collider B is a circle
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

		// scenario 5: if collider A is a point
		else if (colliderA.type == ColliderType::POINT)
		{

			return CheckPointCollider(posA, colliderB, posB);
		}

		// scenario 6: if collider B is a point
		else if (colliderB.type == ColliderType::POINT)
		{
			return CheckPointCollider(posB, colliderA, posA);
		}

		// any unsupported collision type combination
		return CollisionResult{};
	}

	bool CollisionDetection::CheckCollision(
		const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA,
		const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB,
		const Math::eVec2<float>& endPos, Math::eVec2<float>& colPoint, Math::eVec2<float>& normalAtCol, float& colTi)
	{

		// early exit: skip collision if either collider is inactive
		if (!colliderA.isActive || !colliderB.isActive)
		{
			return false; // return "no collision result"
		}

		// calculate actual world positiions of the colliders
		// world position = transform position + collider offset
		Math::eVec2<float> posA = { transformA.pos.x + colliderA.offset.v1, transformA.pos.y + colliderA.offset.v2 };
		Math::eVec2<float> posB = { transformB.pos.x + colliderB.offset.v1, transformB.pos.y + colliderB.offset.v2 };

		// convey to the collision detection function accordingly
		// (BASED ON the types of the 2 colliders)
		// Dynamic circle and line collision
		if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::LINE)
		{
			//std::cout << "CIRCLE RADIUS ::: " << colliderA.data.circle.radius << std::endl;
			return CheckCircleLine(posA, colliderA.data.circle.radius, endPos, colliderB, colPoint, normalAtCol, colTi);
		}


	}



	CollisionResult CollisionDetection::CheckCircleCircle(const Math::eVec2<float>& posA, float radiusA, const Math::eVec2<float>& posB, float radiusB) {
		CollisionResult result;

		// calculate the vector from circle A center to circle B center
		Math::eVec2<float> distance = posB - posA;

		// use squared distance (avoiding the use of sqrt)
		float distanceSquared = Math::eVec2SquaredLength(distance);
		float combinedRadius = radiusA + radiusB;
		float combinedRadiusSquared = combinedRadius * combinedRadius;

		// check if circle are overlapping
		// they overlap if the distance between centers < sum of radii
		if (distanceSquared < combinedRadiusSquared)
		{
			result.hasCollision = true;

			// now need actual distance
			float actualDistance = Math::eVec2Length(distance);

			// avoid division by zero if circles are at exactly the same position
			if (actualDistance > 0.001f)
			{
				// calculate penetration depth (how much they overlap)
				result.penetrationDepth = combinedRadius - actualDistance;

				// calculate penetration vecotr (direction and magnitude to separate them)
				Math::eVec2<float> normalizedDistance = distance / actualDistance;
				result.penetrationVector = normalizedDistance * result.penetrationDepth;

				// contact point is on the edge of circle A in the direction of circle B
				result.contactPoint = posA + (normalizedDistance * radiusA);

			}
			else {
				// circles are at the exact same position - push them apart
				result.penetrationDepth = combinedRadius;
				result.penetrationVector = Math::eVec2<float>{ combinedRadius, 0.0f }; // push along x-axis
				result.contactPoint = posA;
			}
		}

		return result;

	} // definition for CollisionResult CollisionDetection::CheckCircleCircle

	CollisionResult CollisionDetection::CheckRectangleRectangle(
		const Math::eVec2<float>& posA, const CollisionComponent::ColliderData::RectangleData& rectA,
		const Math::eVec2<float>& posB, const CollisionComponent::ColliderData::RectangleData& rectB) 
	{
		CollisionResult result;

		// calculate half-extents (distance from center to edge) for AABB calculations later
		float halfWidthA = rectA.width * 0.5f;
		float halfHeightA = rectA.height * 0.5f;
		float halfWidthB = rectB.width * 0.5f;
		float halfHeightB = rectB.height * 0.5f;

		// calculate distance between between centers
		Math::eVec2<float> distance = posB - posA;

		// calcualte overlap on each axis
		// if rectangles overlap, the overlap = (sum of half-widths) - (distance between centers)
		float overlapX = (halfWidthA + halfWidthB) - std::abs(distance.v1);
		float overlapY = (halfHeightA + halfHeightB) - std::abs(distance.v2);

		// rectangles collide only if they overlap both axes!
		if (overlapX > 0 && overlapY > 0) {
			result.hasCollision = true;

			// separate along the axis with the smallest overlap (separating the axis theorem)
			if (overlapX < overlapY)
			{
				// separate along X-axis
				result.penetrationDepth = overlapX;
				result.penetrationVector.v1 = (distance.v1 < 0) ? -overlapX : overlapX;
				result.penetrationVector.v2 = 0.0f;

				// contact point is on the edge of rectangle A
				float contactX = (distance.v1 < 0) ? posA.v1 - halfWidthA : posA.v1 + halfWidthA;
				result.contactPoint = Math::eVec2<float>{ contactX , posA.v2 };
				
			}
			else {
				// separate along y-axis
				result.penetrationDepth = overlapY;
				result.penetrationVector.v1 = 0.0f;
				result.penetrationVector.v2 = (distance.v2 < 0) ? -overlapY : overlapY;

				// contact point is on the edge of rectangle A
				float contactY = (distance.v2 < 0) ? posA.v2 - halfHeightA : posA.v2 + halfHeightA;
				result.contactPoint = Math::eVec2<float>{ posA.v1, contactY };
			}
		}

		return result;

	} // definition for CheckRectangleRectangle

	CollisionResult CollisionDetection::CheckCircleRectangle(
		const Math::eVec2<float>& circlePos, float radius,
		const Math::eVec2<float>& rectPos, const CollisionComponent::ColliderData::RectangleData& rect)
	{
		CollisionResult result;

		// calculate rectangle bounds
		float halfWidth = rect.width * 0.5f;
		float halfHeight = rect.height * 0.5f;

		// find the closest point on the rectangle to the circle center
		// handles both cases:
		//	1. circle outside rectangle
		//	2. circle inside rectangle
		Math::eVec2<float> closestPoint;
		closestPoint.v1 = std::max(rectPos.v1 - halfWidth, std::min(circlePos.v1, rectPos.v1 + halfWidth));
		closestPoint.v2 = std::max(rectPos.v2 - halfHeight, std::min(circlePos.v2, rectPos.v2 + halfWidth));

		// calculate distance from circle center to closest point on rectangle
		Math::eVec2<float> distance = closestPoint - circlePos;
		float distanceSquared = Math::eVec2SquaredLength(distance);

		// check if circle overlaps with rectangle
		// circle overlaps if distance to closest point < radius
		if (distanceSquared < (radius * radius)) {
			result.hasCollision = true;
			result.contactPoint = closestPoint;

			float actualDistance = Math::eVec2Length(distance);

			if (actualDistance > 0.001f) {
				
				// normal case: circle is outside or touching the rectangle edge
				Math::eVec2<float> normalizedDistance = distance / actualDistance;
				result.penetrationDepth = radius - actualDistance;
				result.penetrationVector = -normalizedDistance * result.penetrationDepth; // negative to push circle awway

			}
			else {
				// special case: circle center is inside rectangle
				// need to find the shortest path to push the circle out
				float distToLeft = (circlePos.v1 - (rectPos.v1 - halfWidth));
				float distToRight = ((rectPos.v1 + halfWidth) - circlePos.v1);
				float distToTop = (circlePos.v2 - (rectPos.v2 - halfHeight));
				float distToBottom = ((rectPos.v2 + halfHeight) - circlePos.v2);

				// find the shortest distance to and edge
				float minDist = std::min({ distToLeft, distToRight, distToTop, distToBottom });

				// push circle out through the nearest edge
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

	} //definition for CheckCircleRectangle
	
	CollisionResult CollisionDetection::CheckPointCollider(
		const Math::eVec2<float>& pointPos, const CollisionComponent& collider, 
		const Math::eVec2<float>& colliderPos)
	{
		CollisionResult result;
		
		switch (collider.type) 
		{
			case ColliderType::CIRCLE: {
				// point vs circle: check if point is within circle radius
				Math::eVec2<float> distance = pointPos - colliderPos;
				float distanceSquared = Math::eVec2SquaredLength(distance);
				float radiusSquared = collider.data.circle.radius * collider.data.circle.radius;

				if (distanceSquared <= radiusSquared) {
					result.hasCollision = true;
					result.contactPoint = pointPos;

					// calculate penetration for point inside circle
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

				bool insideX = (pointPos.v1 >= colliderPos.v1 - halfWidth) && (pointPos.v1 <= colliderPos.v1 + halfWidth);
				bool insideY = (pointPos.v2 >= colliderPos.v2 - halfHeight) && (pointPos.v2 <= colliderPos.v2 + halfHeight);

				if (insideX && insideY) {
					result.hasCollision = true;
					result.contactPoint = pointPos;

					// for point collisions, penetration info is usually not needed
					result.penetrationDepth = 0.0f;
					result.penetrationVector = Math::eVec2<float>{ 0.0f, 0.0f };
				}

				break;

			} 
			
			case ColliderType::POINT:
			{
				// point vs point: check if they're in the same position
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

	} // definition for CheckPointCollider

	bool CollisionDetection::CheckCircleLine(const Math::eVec2<float>& circlePos, const float& radius, const Math::eVec2<float>& endPoint, const CollisionComponent& lineCol,
		Math::eVec2<float>& pointAtCol, Math::eVec2<float>& normalAtCol, float& colTi)
	{

		if ((Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) -
			 Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1)) <= -radius)
		{
			// We consider an imaginary line that is -R away, where we test for the oppposite direction of the line's outward normal -> inner plane
			//check whether the normalized velocity is within the end points of the line segment
			// velocity V = end pt - start pt
			Math::eVec2<float> vel{ endPoint - circlePos };

			// normalized vector M = normalized(V.y, -V.x)
			Math::eVec2<float> mVel;
			Math::eVec2<float> tmp{ vel.v2, -vel.v1 };
			Math::eVec2Normalize(mVel, tmp);
		
			//calculate end points of imaginary line as p1' and p2'
			//P1' = P1 – R*N and P2' = P2 – R*N 

			Math::eVec2<float> innerP1 = lineCol.data.line.p1 - (lineCol.data.line.pNormal * radius);
			Math::eVec2<float> innerP2 = lineCol.data.line.p2 - (lineCol.data.line.pNormal * radius);

			//check if the circle is moving towards the imaginary line: M.Bsp0' * M.Bsp1' < 0
			if ((Math::eVec2DotProduct(mVel, (innerP1 - circlePos)) * Math::eVec2DotProduct(mVel, (innerP2 - circlePos))) < 0)
			{
				//calculate the time of intersection
				//Ti = (N.P0 - N.Bs - R)/(N.V)
				colTi = ((Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1) - 
						  Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) - radius) / 
						  Math::eVec2DotProduct(lineCol.data.line.pNormal, vel));

				//if time of intersection occurs within the next frame
				if (0 <= colTi && colTi <= 1.f) {

					//intersection pt = Bs + V*Ti
					pointAtCol = circlePos + (vel * colTi);
					normalAtCol = -lineCol.data.line.pNormal;
					return true;

				}
			}
			else 
			{
				//return CheckMovingCircleToLineEdge(false, circle, ptEnd, lineSeg, interPt, normalAtCollision, interTime);
			}
		}

		else if ((Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) -
				  Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1)) >= radius)
		{
			// Here we consider an imaginary line R away on the outer plane -> outward normal
			// velocity V = end pt - start pt
			Math::eVec2<float> vel{ endPoint - circlePos };

			// normalized vector M = normalized(V.y, -V.x)
			Math::eVec2<float> mVel;
			Math::eVec2Normalize(mVel, { vel.v2, -(vel.v1) });

			//calculate end points of imaginary line as p1' and p2'
			//P1' = P1 + R*N and P2' = P2 + R*N 
			Math::eVec2<float> outerP1 = lineCol.data.line.p1 + (lineCol.data.line.pNormal * radius);
			Math::eVec2<float> outerP2 = lineCol.data.line.p2 + (lineCol.data.line.pNormal * radius);
			//check if the circle is moving towards the imaginary line: M.Bsp0' * M.Bsp1' < 0
			if ((Math::eVec2DotProduct(mVel, (outerP1 - circlePos)) * Math::eVec2DotProduct(mVel, (outerP2 - circlePos))) < 0)
			{
				//calculate the time of intersection
				//Ti = (N.P0 - N.Bs + R)/(N.V)
				colTi = ((Math::eVec2DotProduct(lineCol.data.line.pNormal, lineCol.data.line.p1) -
					Math::eVec2DotProduct(lineCol.data.line.pNormal, circlePos) + radius) /
					Math::eVec2DotProduct(lineCol.data.line.pNormal, vel));
				//if time of intersection occurs within the next frame
				if (0 <= colTi && colTi <= 1.f)
				{
					//intersection pt = Bs + V*Ti
					pointAtCol = circlePos + (vel * colTi);
					normalAtCol = lineCol.data.line.pNormal;
					return true;
				}
			}

			else 
			{
				//return CheckMovingCircleToLineEdge(false, circle, ptEnd, lineSeg, interPt, normalAtCollision, interTime);
			}
		}
		else 
		{

			//return CheckMovingCircleToLineEdge(true, circle, ptEnd, lineSeg, interPt, normalAtCollision, interTime);
		}

		return false;
	}
		
	 void CollisionDetection::CollisionReflection(const Math::eVec2<float>& interPt,
		const Math::eVec2<float>& normal,
		Math::eVec2<float>& nextPos,
		Math::eVec2<float>& reflected)
	{

		//Penetration = BiBe
		Math::eVec2<float> penetration = nextPos - interPt;
		//B'e = Bi + penetration - 2(penetration.normal) * normal
		nextPos = interPt + penetration - (2.0f * (Math::eVec2DotProduct(penetration, normal)) * normal);

		// New position = collision point + reflected penetration
		reflected = nextPos - interPt;

		// Return normalized reflection direction
		Math::eVec2Normalize(reflected, reflected);

	}


	// =============================================================================
	// COLLISION EVENTS IMPLEMENTATION
	// =============================================================================

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

	// =============================================================================
	// COLLISION SYSTEM IMPLEMENTATION
	// =============================================================================

	CollisionSystem::CollisionSystem(Event::EnoEventBus& event_bus) : eventBus_(event_bus)
	{
		// storing event bus reference
		// system is ready to use 
	}

	void CollisionSystem::Update(ECS::EntityManager& entityManager)
	{
		// step1: find all entities that can participate in collision detection
		// must have both transform and collision components
		auto& entityMap = entityManager.getMap();
		std::vector<std::pair<uint32_t, ECS::EntityLoc*>> collidableEntities;

		// iterate through all entities in the world
		for (auto& [entityID, entityLoc] : entityMap)
		{
			// check if this entity has the required components for collision
			if (HasRequiredComponents(entityLoc))
			{
				collidableEntities.emplace_back(entityID, &entityLoc);
			}
		}

		// step 2: check collision between all pairs of collidable entities
		for (size_t i = 0; i < collidableEntities.size(); ++i) {
			for (size_t j = i + 1; j < collidableEntities.size(); ++j) {
				// check collision between entity i and entity j
				CheckEntityCollision(collidableEntities[i].first,
					*collidableEntities[i].second,
					collidableEntities[j].first,
					*collidableEntities[j].second);
			}
		}
	}

	void CollisionSystem::AddCollisionCallback(std::function<void(const CollisionEvent&)> callback) 
	{
		collisionCallbacks_.push_back(callback);
	}

	void CollisionSystem::VisualizeColliders(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager)
	{
		auto& entityMap = entityManager.getMap(); 

		// iterate through all entities
		for (auto& [entityID, entityLoc] : entityMap)
		{
			// check if entity has required components
			if (!HasRequiredComponents(entityLoc))
				continue;

			// get transform and collision components
			auto& transform = GetTransformComponent(entityLoc);
			auto& collision = GetCollisionComponent(entityLoc);

			// skip if collider is inactive
			if (!collision.isActive)
				continue;

			// calculate world position
			Math::eVec2<float> worldPos = {
				transform.pos.x + collision.offset.v1,
				transform.pos.y + collision.offset.v2
			};

			// choose color based on trigger status
			Graphics::Color color = collision.isTrigger ? Graphics::Color::Red : Graphics::Color::Red;

			// draw based on collider type
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
					
					
					// calculate top-left corner (drawRect uses top-left pos), but worldPos is CENTER
					Math::eVec2<float> topLeft = {
						worldPos.v1 - (scaledWidth * 0.5f),
						worldPos.v2 - (scaledHeight * 0.5f)
					};


					graphics.drawRect(
						topLeft,
						Math::eVec2<float>{scaledWidth, scaledHeight},
						color,
						false // not filled, just outline
						//transform.rot.z
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

	void CollisionSystem::ClearCallbacks()
	{
		collisionCallbacks_.clear(); 
	}

	bool CollisionSystem::HasRequiredComponents(const ECS::EntityLoc& entityLoc) {
		bool hasTransform = false;
		bool hasCollision = false;

		// check each component type in the entity's archetype 
		for (const auto& type : entityLoc.archetype->types_)
		{
			if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
			{
				hasTransform = true;
			}
			else if (type == std::type_index(typeid(CollisionComponent)))
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
		// create and populate the collision event
		CollisionEvent event;
		event.entityA_ID = entityA_ID;
		event.entityB_ID = entityB_ID;
		event.result = result;
		event.isEntityA_Trigger = collisionA.isTrigger;
		event.isEntityB_Trigger = collisionB.isTrigger;

		// publish the event to the event bus for other systems to handle 
		eventBus_.publish(event);

		// call all registered callbacks
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
		// Find the Collision component's index in this entity's archetype
		size_t collisionIndex = entityLoc.archetype->findComponentIndex<CollisionComponent>();

		// Retrieve the component from the appropriate chunk
		return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<CollisionComponent>(
			collisionIndex, entityLoc.entityIndexInChunk);
	}

	// =============================================================================
	// UTILITY FUNCTIONS IMPLEMENTATION
	// =============================================================================

	void RunBasicCollisionTest()
	{
		std::cout << "=== Basic Collision System Test ===" << std::endl;

		// This is a simple test to verify collision detection works
		// In a real game, you'd create entities through the entity manager

		// Test 1: Circle vs Circle collision
		std::cout << "\nTest 1: Circle vs Circle" << std::endl;
		CollisionComponent circleA(1.0f);  // Radius 1.0 at origin
		CollisionComponent circleB(1.0f);  // Radius 1.0

		ECSComponents::TransformComponent transformA{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };
		ECSComponents::TransformComponent transformB{ {1.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };

		CollisionResult result1 = CollisionDetection::CheckCollision(circleA, transformA, circleB, transformB);
		std::cout << "Collision detected: " << (result1.hasCollision ? "Yes" : "No") << std::endl;
		if (result1.hasCollision) {
			std::cout << "Penetration depth: " << result1.penetrationDepth << std::endl;
		}

		// Test 2: Rectangle vs Rectangle collision
		std::cout << "\nTest 2: Rectangle vs Rectangle" << std::endl;
		CollisionComponent rectA(2.0f, 2.0f);  // 2x2 rectangle
		CollisionComponent rectB(2.0f, 2.0f);  // 2x2 rectangle

		ECSComponents::TransformComponent transformC{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };
		ECSComponents::TransformComponent transformD{ {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };

		CollisionResult result2 = CollisionDetection::CheckCollision(rectA, transformC, rectB, transformD);
		std::cout << "Collision detected: " << (result2.hasCollision ? "Yes" : "No") << std::endl;
		if (result2.hasCollision) {
			std::cout << "Penetration depth: " << result2.penetrationDepth << std::endl;
		}

		std::cout << "\n=== Test Complete ===" << std::endl;
	}



} // namespace Collision