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
        // Only handle rectangle colliders for now (can extend to circles later)
        if (collision.type != ECSComponents::ColliderType::RECTANGLE)
            return;

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

            // Only handle rectangle-rectangle collisions for now
            if (otherCollision.type != ECSComponents::ColliderType::RECTANGLE)
                continue;

            // Check AABB overlap at target position
            bool overlaps = CheckAABBOverlap(
                targetPos,
                collision.data.rectangle.width,
                collision.data.rectangle.height,
                otherTransform.pos,
                otherCollision.data.rectangle.width,
                otherCollision.data.rectangle.height
            );

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
        // Calculate penetration depth and collision normal
        Math::eVec2<float> normal;
        float penetration;

        CalculateAABBPenetration(
            transformA.pos,
            collisionA.data.rectangle.width,
            collisionA.data.rectangle.height,
            transformB.pos,
            collisionB.data.rectangle.width,
            collisionB.data.rectangle.height,
            normal,
            penetration
        );

        // Separate the objects (remove overlap)
        // Move both objects by half the penetration distance
        float separationA = penetration * 0.5f;
        float separationB = penetration * 0.5f;

        transformA.pos.x -= normal.x * separationA;
        transformA.pos.y -= normal.y * separationA;
        transformB.pos.x += normal.x * separationB;
        transformB.pos.y += normal.y * separationB;

        // Calculate relative velocity along collision normal
        Math::eVec2<float> relativeVel = {
            physicsA.velocity.x - physicsB.velocity.x,
            physicsA.velocity.y - physicsB.velocity.y
        };

        float velAlongNormal = relativeVel.x * normal.x + relativeVel.y * normal.y;

        // Don't resolve if velocities are separating
        if (velAlongNormal > 0)
            return;

        // Calculate restitution (bounciness) - use minimum of both objects
        float restitution = (physicsA.restitution < physicsB.restitution) ?
            physicsA.restitution : physicsB.restitution;

        // Calculate impulse scalar (momentum transfer magnitude)
        // Formula: j = -(1 + e) * velAlongNormal / (1/massA + 1/massB)
        float impulseMagnitude = -(1.0f + restitution) * velAlongNormal;
        impulseMagnitude /= (1.0f / physicsA.mass) + (1.0f / physicsB.mass);

        // Calculate impulse vector
        Math::eVec2<float> impulse = {
            normal.x * impulseMagnitude,
            normal.y * impulseMagnitude
        };

        // Apply impulse to both objects (momentum transfer - THIS IS THE PUSHING!)
        // Object A (pusher) loses momentum
        physicsA.velocity.x -= impulse.x / physicsA.mass;
        physicsA.velocity.y -= impulse.y / physicsA.mass;

        // Object B (being pushed) gains momentum
        physicsB.velocity.x += impulse.x / physicsB.mass;
        physicsB.velocity.y += impulse.y / physicsB.mass;
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
     * \brief       Helper functions to get components from entity location
     *===========================================================================================*/
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