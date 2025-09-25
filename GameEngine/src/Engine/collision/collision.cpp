
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

	CollisionComponent::CollisionComponent(float radius, Math::eVec2<float> colliderOffset, bool trigger)
		: type(ColliderType::CIRCLE)
		, offset(colliderOffset)
		, isTrigger(trigger)
		, isActive(true)
	{
		data.circle.radius = radius;
	}

	CollisionComponent::CollisionComponent(float width, float height, Math::eVec2<float> colliderOffset, bool trigger)
		: type(ColliderType::RECTANGLE)
		, offset(colliderOffset)
		, isTrigger(trigger)
		, isActive(true)
	{
		data.rectangle.width = width;
		data.rectangle.height = height;
	}

	CollisionComponent CollisionComponent::CreatePointCollider(Math::eVec2<float> colliderOffset, bool trigger) {
		CollisionComponent comp;
		comp.type = ColliderType::POINT;
		comp.offset = colliderOffset;
		comp.isTrigger = trigger;
		comp.isActive = true;
		return comp;
	}

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
			return CheckCircleCircle(posA, colliderA.data.circle.radius, posB, colliderB.data.circle.radius);
		}

		// scenario 2: if both colliders are rectangles
		else if (colliderA.type == ColliderType::RECTANGLE && colliderB.type == ColliderType::RECTANGLE)
		{
			return CheckRectangleRectangle(posA, colliderA.data.rectangle, posB, colliderB.data.rectangle);
		}

		// scenario 3: if collider A is a circle, and collider B is a rectangle
		else if (colliderA.type == ColliderType::CIRCLE && colliderB.type == ColliderType::RECTANGLE)
		{
			return CheckCircleRectangle(posA, colliderA.data.circle.radius, posB, colliderB.data.rectangle);
		}

		// scenario 4: if collider A is a rectangle, and collider B is a circle
		else if (colliderA.type == ColliderType::RECTANGLE && colliderB.type == ColliderType::CIRCLE)
		{
			// swap the order and negate the pentetration vector (since we're checking B vs A instead of A vs B)
			CollisionResult result = CheckCircleRectangle(posB, colliderB.data.circle.radius, posA, colliderA.data.rectangle);
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
		float halfWidthB = rectB.width * 0.0f;
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

	CollisionSystem::CollisionSystem(Event::EnoEventBus& eventBus) : eventBus_(eventBus)
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

	void CollisionSystem::CheckEntityCollision(uint32_t entityA_ID, const ECS::EntityLoc& entityA_Loc,
		uint32_t entityB_ID, const ECS::EntityLoc& entityB_Loc)
	{
		try
		{
			// get the required components from both entities
			auto& transformA = GetTransformComponent(entityA_Loc);
			auto& transformB = GetTransformComponent(entityB_Loc);
			auto& collisionA = GetTransformComponent(entityA_Loc);
			auto& collisionB = GetTransformComponent(entityB_Loc);
		}
	}
} // namespace Collision