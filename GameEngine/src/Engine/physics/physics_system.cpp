/*============================================================================================
* All content ? 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         physics_system.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         05-11-2025
* @brief        Implementation of centralized physics system. Handles force integration,
*               velocity updates, AABB collision detection, and momentum transfer to enable
*               realistic pushing interactions between physics-enabled entities.
*===========================================================================================*/

#include "physics_system.hpp"
#include <cmath>
#include <typeindex>

namespace Physics
{
    /*============================================================================================
     * \brief       Main physics update loop - processes all entities with PhysicsComponent
     * \param[in]   ECS::EntityManager& entityManager - Entity manager reference
     * \param[in]   float deltaTime - Time step in seconds
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::Update(ECS::EntityManager& entityManager, float deltaTime)
    {
        // Get all entities in the world
        auto& entityMap = entityManager.getMap();

        // PHASE 1: Apply forces and integrate velocities for all physics entities
        for (auto& [entityID, entityLoc] : entityMap)
        {
            // Check if entity has PhysicsComponent
            bool hasPhysics = false;
            bool hasTransform = false;

            for (const auto& type : entityLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(ECSComponents::PhysicsComponent)))
                    hasPhysics = true;
                else if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
                    hasTransform = true;
            }

            // Skip entities without physics
            if (!hasPhysics || !hasTransform)
                continue;

            // Get physics component
            auto& physics = GetPhysics(entityLoc);

            // Skip kinematic objects (manually controlled, like trigger zones)
            if (physics.isKinematic)
                continue;

            // Apply gravity if enabled
            if (physics.useGravity)
            {
                ApplyGravity(physics);
            }

            // Apply drag/friction forces
            ApplyDrag(physics);

            // Integrate forces into velocity
            IntegrateForces(physics, deltaTime);

            // Clamp velocity to constraints
            physics.ClampVelocity();
        }

        // PHASE 2: Integrate velocities into positions with collision detection
        for (auto& [entityID, entityLoc] : entityMap)
        {
            // Check if entity has all required components for physics movement
            bool hasPhysics = false;
            bool hasTransform = false;
            bool hasCollision = false;

            for (const auto& type : entityLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(ECSComponents::PhysicsComponent)))
                    hasPhysics = true;
                else if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
                    hasTransform = true;
                else if (type == std::type_index(typeid(ECSComponents::CollisionComponent)))
                    hasCollision = true;
            }

            // Skip entities without required components
            if (!hasPhysics || !hasTransform || !hasCollision)
                continue;

            // Get components
            auto& transform = GetTransform(entityLoc);
            auto& physics = GetPhysics(entityLoc);
            auto& collision = GetCollision(entityLoc);

            // Skip kinematic objects
            if (physics.isKinematic)
                continue;

            // Skip inactive colliders
            if (!collision.isActive)
                continue;

            // Integrate velocity into position (with collision response)
            IntegrateVelocity(entityID, entityLoc, transform, physics, collision, entityManager, deltaTime);
        }
    }

    /*============================================================================================
     * \brief       Apply gravity force to physics object
     * \param[in]   ECSComponents::PhysicsComponent& physics - Physics component to modify
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::ApplyGravity(ECSComponents::PhysicsComponent& physics)
    {
        // F = m * g (gravitational force)
        Math::eVec2<float> gravityForce = {
            0.0f,
            GRAVITY * physics.mass  // Positive Y is down in many graphics systems
        };

        physics.AddForce(gravityForce);
    }

    /*============================================================================================
     * \brief       Apply drag forces that oppose velocity (simulates air resistance/friction)
     * \param[in]   ECSComponents::PhysicsComponent& physics - Physics component to modify
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::ApplyDrag(ECSComponents::PhysicsComponent& physics)
    {
        // Drag force opposes velocity: F_drag = -drag * velocity * mass
        // Higher drag value = faster deceleration
        Math::eVec2<float> dragForce = {
            -physics.velocity.x * physics.drag * physics.mass,
            -physics.velocity.y * physics.drag * physics.mass
        };

        physics.AddForce(dragForce);
    }

    /*============================================================================================
     * \brief       Integrate accumulated forces into velocity (Semi-implicit Euler)
     * \param[in]   ECSComponents::PhysicsComponent& physics - Physics to update
     * \param[in]   float deltaTime - Time step
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::IntegrateForces(ECSComponents::PhysicsComponent& physics, float deltaTime)
    {
        // Calculate acceleration from forces: a = F / m (Newton's Second Law)
        physics.acceleration.x = physics.force.x / physics.mass;
        physics.acceleration.y = physics.force.y / physics.mass;

        // Integrate acceleration into velocity: v = v + a * dt
        physics.velocity.x += physics.acceleration.x * deltaTime;
        physics.velocity.y += physics.acceleration.y * deltaTime;

        // Clear forces for next frame (important!)
        physics.ClearForces();
    }

    /*============================================================================================
     * \brief       Integrate velocity into position with collision detection and response
     * \param[in]   uint32_t entityID - ID of the current entity (for self-collision check)
     * \param[in]   ECS::EntityLoc const& entityLoc - Entity being moved
     * \param[in]   ECSComponents::TransformComponent& transform - Entity's transform
     * \param[in]   ECSComponents::PhysicsComponent& physics - Entity's physics
     * \param[in]   ECSComponents::CollisionComponent& collision - Entity's collision
     * \param[in]   ECS::EntityManager& entityManager - Entity manager for collision checks
     * \param[in]   float deltaTime - Time step
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::IntegrateVelocity(
        uint32_t entityID,
        const ECS::EntityLoc& entityLoc,
        ECSComponents::TransformComponent& transform,
        ECSComponents::PhysicsComponent& physics,
        ECSComponents::CollisionComponent& collision,
        ECS::EntityManager& entityManager,
        float deltaTime)
    {
        // Handle both rectangle and circle colliders
        // (Note: for circles, we treat them as square AABBS for simplified physics)

        // Calculate displacement from velocity: displacement = velocity * dt
        Math::eVec2<float> displacement = {
            physics.velocity.x * deltaTime,
            physics.velocity.y * deltaTime
        };

        // Target position if no collision
        Math::eVec2<float> targetPos = {
            transform.pos.x + displacement.x,
            transform.pos.y + displacement.y
        };

        // Check for collisions with all other entities
        auto& entityMap = entityManager.getMap();
        bool hadCollision = false;

        for (auto& [otherID, otherLoc] : entityMap)
        {
            // Skip self (compare map keys)
            if (entityID == otherID)
                continue;

            // Check if other entity has collision component
            bool otherHasCollision = false;
            bool otherHasTransform = false;
            bool otherHasPhysics = false;

            for (const auto& type : otherLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(ECSComponents::CollisionComponent)))
                    otherHasCollision = true;
                else if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
                    otherHasTransform = true;
                else if (type == std::type_index(typeid(ECSComponents::PhysicsComponent)))
                    otherHasPhysics = true;
            }

            if (!otherHasCollision || !otherHasTransform)
                continue;

            // Get other entity's components
            auto& otherTransform = GetTransform(otherLoc);
            auto& otherCollision = GetCollision(otherLoc);

            // Skip inactive colliders or triggers
            if (!otherCollision.isActive || otherCollision.isTrigger)
                continue;

            // Check collision based on collider types
            bool overlaps = false;

            // Handle different collision type combinations
            if (collision.type == ECSComponents::ColliderType::RECTANGLE &&
                otherCollision.type == ECSComponents::ColliderType::RECTANGLE)
            {
                // Rectangle vs Rectangle - use AABB
                float scaledWidthA, scaledHeightA;
                float scaledWidthB, scaledHeightB;
                GetScaledColliderDimensions(transform, collision, scaledWidthA, scaledHeightA);
                GetScaledColliderDimensions(otherTransform, otherCollision, scaledWidthB, scaledHeightB);

                overlaps = CheckAABBOverlap(
                    targetPos,
                    scaledWidthA,
                    scaledHeightA,
                    otherTransform.pos,
                    scaledWidthB,
                    scaledHeightB
                );
            }
            else if (collision.type == ECSComponents::ColliderType::CIRCLE &&
                     otherCollision.type == ECSComponents::ColliderType::CIRCLE)
            {
                // Circle vs Circle
                float radiusA = collision.data.circle.radius * transform.scale.x;
                float radiusB = otherCollision.data.circle.radius * otherTransform.scale.x;
                overlaps = CheckCircleCircleOverlap(targetPos, radiusA, otherTransform.pos, radiusB);
            }
            else if (collision.type == ECSComponents::ColliderType::CIRCLE &&
                     otherCollision.type == ECSComponents::ColliderType::RECTANGLE)
            {
                // Circle vs Rectangle
                float radius = collision.data.circle.radius * transform.scale.x;
                float scaledWidthB, scaledHeightB;
                GetScaledColliderDimensions(otherTransform, otherCollision, scaledWidthB, scaledHeightB);
                overlaps = CheckCircleRectOverlap(targetPos, radius, otherTransform.pos, scaledWidthB, scaledHeightB);
            }
            else if (collision.type == ECSComponents::ColliderType::RECTANGLE &&
                     otherCollision.type == ECSComponents::ColliderType::CIRCLE)
            {
                // Rectangle vs Circle (same as Circle vs Rectangle, just swapped)
                float radius = otherCollision.data.circle.radius * otherTransform.scale.x;
                float scaledWidthA, scaledHeightA;
                GetScaledColliderDimensions(transform, collision, scaledWidthA, scaledHeightA);
                overlaps = CheckCircleRectOverlap(otherTransform.pos, radius, targetPos, scaledWidthA, scaledHeightA);
            }

            if (overlaps)
            {
                hadCollision = true;

                // If other entity has physics, resolve collision with momentum transfer
                if (otherHasPhysics)
                {
                    auto& otherPhysics = GetPhysics(otherLoc);

                    // Don't push kinematic objects (like walls)
                    if (!otherPhysics.isKinematic)
                    {
                        // Resolve collision with momentum transfer (this enables pushing!)
                        ResolveCollision(
                            transform, physics, collision,
                            otherTransform, otherPhysics, otherCollision
                        );
                    }
                    else
                    {
                        // Hit a static wall - just stop movement
                        physics.velocity = { 0.0f, 0.0f };
                    }
                }
                else
                {
                    // Other entity has no physics (static obstacle) - stop movement
                    physics.velocity = { 0.0f, 0.0f };
                }

                break;  // Handle one collision per frame for stability
            }
        }

        // If no collision, update position normally
        if (!hadCollision)
        {
            transform.pos.x = targetPos.x;
            transform.pos.y = targetPos.y;
        }
    }

    /*============================================================================================
     * \brief       Check if two AABB rectangles overlap
     * \return      bool - True if rectangles intersect
     *===========================================================================================*/
    bool PhysicsSystem::CheckAABBOverlap(
        const Math::eVec2<float>& posA, float widthA, float heightA,
        const Math::eVec2<float>& posB, float widthB, float heightB) const
    {
        // Calculate AABB bounds (assuming position is center)
        float halfWidthA = widthA * 0.5f;
        float halfHeightA = heightA * 0.5f;
        float halfWidthB = widthB * 0.5f;
        float halfHeightB = heightB * 0.5f;

        float minXA = posA.x - halfWidthA;
        float maxXA = posA.x + halfWidthA;
        float minYA = posA.y - halfHeightA;
        float maxYA = posA.y + halfHeightA;

        float minXB = posB.x - halfWidthB;
        float maxXB = posB.x + halfWidthB;
        float minYB = posB.y - halfHeightB;
        float maxYB = posB.y + halfHeightB;

        // AABB overlap test
        return !(maxXA < minXB || minXA > maxXB || maxYA < minYB || minYA > maxYB);
    }

    /*============================================================================================
     * \brief       Check if two circles overlap
     * \return      bool - True if circles intersect
     *===========================================================================================*/
    bool PhysicsSystem::CheckCircleCircleOverlap(
        const Math::eVec2<float>& posA, float radiusA,
        const Math::eVec2<float>& posB, float radiusB) const
    {
        // Calculate distance between circle centers
        float dx = posB.x - posA.x;
        float dy = posB.y - posA.y;
        float distanceSquared = dx * dx + dy * dy;

        // Check if distance is less than sum of radii
        float radiusSum = radiusA + radiusB;
        return distanceSquared < (radiusSum * radiusSum);
    }

    /*============================================================================================
     * \brief       Check if a circle overlaps with a rectangle
     * \return      bool - True if circle and rectangle intersect
     *===========================================================================================*/
    bool PhysicsSystem::CheckCircleRectOverlap(
        const Math::eVec2<float>& circlePos, float radius,
        const Math::eVec2<float>& rectPos, float rectWidth, float rectHeight) const
    {
        // Find the closest point on the rectangle to the circle
        float halfWidth = rectWidth * 0.5f;
        float halfHeight = rectHeight * 0.5f;

        // Clamp circle center to rectangle bounds
        float closestX = std::max(rectPos.x - halfWidth, std::min(circlePos.x, rectPos.x + halfWidth));
        float closestY = std::max(rectPos.y - halfHeight, std::min(circlePos.y, rectPos.y + halfHeight));

        // Calculate distance from circle center to closest point
        float dx = circlePos.x - closestX;
        float dy = circlePos.y - closestY;
        float distanceSquared = dx * dx + dy * dy;

        // Check if distance is less than radius
        return distanceSquared < (radius * radius);
    }

    /*============================================================================================
     * \brief       Resolve collision between two entities with momentum transfer (PUSHING!)
     * \return      void
     *
     * This function implements the core pushing mechanic:
     * 1. Separate overlapping objects
     * 2. Calculate collision normal
     * 3. Transfer momentum from moving object to target object
     * 4. Apply restitution (bounciness)
     *===========================================================================================*/
    void PhysicsSystem::ResolveCollision(
        ECSComponents::TransformComponent& transformA,
        ECSComponents::PhysicsComponent& physicsA,
        ECSComponents::CollisionComponent& collisionA,
        ECSComponents::TransformComponent& transformB,
        ECSComponents::PhysicsComponent& physicsB,
        ECSComponents::CollisionComponent& collisionB)
    {
        // Calculate penetration depth and collision normal based on collider types
        Math::eVec2<float> normal;
        float penetration;

        // Handle different collision type combinations
        if (collisionA.type == ECSComponents::ColliderType::RECTANGLE &&
            collisionB.type == ECSComponents::ColliderType::RECTANGLE)
        {
            // Rectangle vs Rectangle
            float scaledWidthA, scaledHeightA;
            float scaledWidthB, scaledHeightB;
            GetScaledColliderDimensions(transformA, collisionA, scaledWidthA, scaledHeightA);
            GetScaledColliderDimensions(transformB, collisionB, scaledWidthB, scaledHeightB);

            CalculateAABBPenetration(
                transformA.pos,
                scaledWidthA,
                scaledHeightA,
                transformB.pos,
                scaledWidthB,
                scaledHeightB,
                normal,
                penetration
            );
        }
        else if (collisionA.type == ECSComponents::ColliderType::CIRCLE &&
                 collisionB.type == ECSComponents::ColliderType::CIRCLE)
        {
            // Circle vs Circle
            float radiusA = collisionA.data.circle.radius * transformA.scale.x;
            float radiusB = collisionB.data.circle.radius * transformB.scale.x;

            CalculateCirclePenetration(
                transformA.pos, radiusA,
                transformB.pos, radiusB,
                normal, penetration
            );
        }
        else if (collisionA.type == ECSComponents::ColliderType::CIRCLE &&
                 collisionB.type == ECSComponents::ColliderType::RECTANGLE)
        {
            // Circle vs Rectangle
            float radius = collisionA.data.circle.radius * transformA.scale.x;
            float scaledWidthB, scaledHeightB;
            GetScaledColliderDimensions(transformB, collisionB, scaledWidthB, scaledHeightB);

            CalculateCircleRectPenetration(
                transformA.pos, radius,
                transformB.pos, scaledWidthB, scaledHeightB,
                normal, penetration
            );
        }
        else if (collisionA.type == ECSComponents::ColliderType::RECTANGLE &&
                 collisionB.type == ECSComponents::ColliderType::CIRCLE)
        {
            // Rectangle vs Circle
            float scaledWidthA, scaledHeightA;
            GetScaledColliderDimensions(transformA, collisionA, scaledWidthA, scaledHeightA);
            float radiusB = collisionB.data.circle.radius * transformB.scale.x;

            CalculateCircleRectPenetration(
                transformB.pos, radiusB,
                transformA.pos, scaledWidthA, scaledHeightA,
                normal, penetration
            );
            // Flip the normal since we calculated from B to A
            normal.x = -normal.x;
            normal.y = -normal.y;
        }
        else
        {
            // Unsupported collision type, return early
            return;
        }

        // Separate the objects (remove overlap)
        // Move both objects by half the penetration distance
        float separationA = penetration * 0.5f;
        float separationB = penetration * 0.5f;

        transformA.pos.x -= normal.x * separationA;
        transformA.pos.y -= normal.y * separationA;
        transformB.pos.x += normal.x * separationB;
        transformB.pos.y += normal.y * separationB;

        // Velocity-based pushing -> calculate how fast each object is 
        // moving along the collision normal
        float velAAlongNormal = physicsA.velocity.x * normal.x + physicsA.velocity.y * normal.y;
        float velBAlongNormal = physicsB.velocity.x * normal.x + physicsB.velocity.y * normal.y;

        // Calculate relative velocity (how fast they're approaching)
        float relativeVel = velAAlongNormal - velBAlongNormal;

        // Only resolve if objects are moving toward each other
        if (relativeVel > 0) {
            const float PUSH_STRENGTH = 0.5f; // How much velocity transfers (0-1)
            const float DAMPING = 0.7f;       // How much to reduce velocities (friction)

            // Calculate velocity transfer amount
            float velocityTransfer = relativeVel * PUSH_STRENGTH;

            // Transfer velocity from A to B
            physicsB.velocity.x += normal.x * velocityTransfer;
            physicsB.velocity.y += normal.y * velocityTransfer;

            // Reduce A's velocity (friction/resistance)
            physicsA.velocity.x -= normal.x * velocityTransfer;
            physicsA.velocity.y -= normal.y * velocityTransfer;

            physicsA.velocity.x *= DAMPING;
            physicsB.velocity.x *= DAMPING;
            physicsA.velocity.y *= DAMPING;
            physicsB.velocity.y *= DAMPING;


        }

    }

    /*============================================================================================
     * \brief       Calculate penetration depth and collision normal between AABBs
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::CalculateAABBPenetration(
        const Math::eVec2<float>& posA, float widthA, float heightA,
        const Math::eVec2<float>& posB, float widthB, float heightB,
        Math::eVec2<float>& normal, float& penetration) const
    {
        // Calculate half-extents
        float halfWidthA = widthA * 0.5f;
        float halfHeightA = heightA * 0.5f;
        float halfWidthB = widthB * 0.5f;
        float halfHeightB = heightB * 0.5f;

        // Calculate distance between centers
        Math::eVec2<float> distance = {
            posB.x - posA.x,
            posB.y - posA.y
        };

        // Calculate overlap on each axis
        float overlapX = (halfWidthA + halfWidthB) - std::abs(distance.x);
        float overlapY = (halfHeightA + halfHeightB) - std::abs(distance.y);

        // Find axis of minimum penetration (collision normal direction)
        if (overlapX < overlapY)
        {
            // Collision on X axis
            penetration = overlapX;
            normal.x = (distance.x > 0) ? 1.0f : -1.0f;
            normal.y = 0.0f;
        }
        else
        {
            // Collision on Y axis
            penetration = overlapY;
            normal.x = 0.0f;
            normal.y = (distance.y > 0) ? 1.0f : -1.0f;
        }
    }

    /*============================================================================================
     * \brief       Calculate penetration depth and collision normal between two circles
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::CalculateCirclePenetration(
        const Math::eVec2<float>& posA, float radiusA,
        const Math::eVec2<float>& posB, float radiusB,
        Math::eVec2<float>& normal, float& penetration) const
    {
        // Calculate vector from A to B
        Math::eVec2<float> distance = {
            posB.x - posA.x,
            posB.y - posA.y
        };

        // Calculate actual distance
        float distLength = std::sqrt(distance.x * distance.x + distance.y * distance.y);

        // Calculate penetration (how much circles overlap)
        penetration = (radiusA + radiusB) - distLength;

        // Calculate normal (direction from A to B)
        if (distLength > 0.0001f) {
            normal.x = distance.x / distLength;
            normal.y = distance.y / distLength;
        } else {
            // Circles are exactly on top of each other, use arbitrary direction
            normal.x = 1.0f;
            normal.y = 0.0f;
        }
    }

    /*============================================================================================
     * \brief       Calculate penetration depth and collision normal between circle and rectangle
     * \return      void
     *===========================================================================================*/
    void PhysicsSystem::CalculateCircleRectPenetration(
        const Math::eVec2<float>& circlePos, float radius,
        const Math::eVec2<float>& rectPos, float rectWidth, float rectHeight,
        Math::eVec2<float>& normal, float& penetration) const
    {
        // Find the closest point on the rectangle to the circle
        float halfWidth = rectWidth * 0.5f;
        float halfHeight = rectHeight * 0.5f;

        // Clamp circle center to rectangle bounds
        float closestX = std::max(rectPos.x - halfWidth, std::min(circlePos.x, rectPos.x + halfWidth));
        float closestY = std::max(rectPos.y - halfHeight, std::min(circlePos.y, rectPos.y + halfHeight));

        // Calculate vector from closest point to circle center
        Math::eVec2<float> distance = {
            circlePos.x - closestX,
            circlePos.y - closestY
        };

        // Calculate actual distance
        float distLength = std::sqrt(distance.x * distance.x + distance.y * distance.y);

        // Calculate normal (direction from closest point to circle center)
        if (distLength > 0.0001f) {
            // Circle center is outside or on edge of rectangle
            // Normal points from closest point to circle center 

            normal.x = distance.x / distLength;
            normal.y = distance.y / distLength;

            // Calculate penetration
            penetration = radius - distLength;

        } else {
            // Circle center is inside rectangle
            // Find the closest edge and push the circle out through that edge
            //float dx = circlePos.x - rectPos.x;
            //float dy = circlePos.y - rectPos.y;

            // Calculate distance to each edge
            float distToRight = (rectPos.x + halfWidth) - circlePos.x;
            float distToLeft = circlePos.x - (rectPos.x - halfWidth);
            float distToBottom = (rectPos.y + halfHeight) - circlePos.y;
            float distToTop = circlePos.y - (rectPos.y - halfHeight);

            // Find minimum distance to any edge
            float minDist = distToRight;
            normal.x = 1.0f;
            normal.y = 0.0f;

            if (distToLeft < minDist) {
                minDist = distToLeft;
                normal.x = -1.0f;
                normal.y = 0.0f;
            }
            if (distToBottom < minDist) {
                minDist = distToBottom;
                normal.x = 0.0f;
                normal.y = 1.0f;
            }
            if (distToTop < minDist) {
                minDist = distToTop;
                normal.x = 0.0f;
                normal.y = -1.0f;
            }

            penetration = radius + minDist;
        }
    }

    /*============================================================================================
    * \brief       Helper functions to get components from entity location
    *===========================================================================================*/
    void PhysicsSystem::GetScaledColliderDimensions(
        const ECSComponents::TransformComponent& transform,
        const ECSComponents::CollisionComponent& collision,
        float& scaledWidth,
        float& scaledHeight) const
    {
        // Apply transform scale to collision dimensions
        // This ensures the collision box matches the visual sprite size
        scaledWidth = collision.data.rectangle.width * transform.scale.x;
        scaledHeight = collision.data.rectangle.height * transform.scale.y;
    }
 
    ECSComponents::TransformComponent& PhysicsSystem::GetTransform(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
            index, entityLoc.entityIndexInChunk);
    }

    ECSComponents::PhysicsComponent& PhysicsSystem::GetPhysics(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::PhysicsComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::PhysicsComponent>(
            index, entityLoc.entityIndexInChunk);
    }

    ECSComponents::CollisionComponent& PhysicsSystem::GetCollision(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::CollisionComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::CollisionComponent>(
            index, entityLoc.entityIndexInChunk);
    }

} // namespace Physics