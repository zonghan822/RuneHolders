/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         physics_system.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)   50%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   38%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   12%
* @date         05-11-2025
* @brief        Central physics system that manages force-based physics simulation for all
*//*========================================================================================*/



#pragma once

#include "../math/math.hpp"
#include "../ecs/entity.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/physics_component.hpp"
#include "../component/components/collision_component.hpp"

namespace Physics
{
    /*============================================================================================
     * \brief       Central physics system that simulates Newtonian physics for all entities.
     *              Applies forces, integrates velocities, handles collisions, and enables
     *              realistic interactions like pushing boxes.
     *
     * Physics Pipeline (called each frame):
     * 1. Apply accumulated forces (from player input, gravity, etc.)
     * 2. Calculate acceleration: a = F/m (Newton's Second Law)
     * 3. Integrate velocity: v = v + a*dt
     * 4. Apply velocity damping (friction/drag)
     * 5. Integrate position: p = p + v*dt (with collision response)
     * 6. Resolve collisions and transfer momentum between objects
     * 7. Clear forces for next frame
     *===========================================================================================*/
    class PhysicsSystem
    {
    public:
        /*============================================================================================
         * \brief       Default constructor
         *===========================================================================================*/
        PhysicsSystem(Event::EnoEventBus& bus) : bus_{bus} {}

        /*============================================================================================
         * \brief       Main physics update - call once per frame BEFORE rendering
         * \param[in]   ECS::EntityManager& entityManager - Entity manager containing all entities
         * \param[in]   float deltaTime - Time elapsed since last frame (seconds)
         * \return      void
         *
         * This function processes physics for ALL entities with PhysicsComponent in the world.
         * It's separate from PlayerSystem to allow boxes, enemies, projectiles, etc. to also
         * use physics-based movement.
         *===========================================================================================*/
        void Update(ECS::EntityManager& entityManager, float deltaTime);

    private:
        /*============================================================================================
         * \brief       Apply gravity force to an entity if useGravity is enabled
         * \param[in]   ECSComponents::PhysicsComponent& physics - Physics component to modify
         * \return      void
         *===========================================================================================*/
        void ApplyGravity(ECSComponents::PhysicsComponent& physics);

        /*============================================================================================
         * \brief       Apply drag/friction forces that oppose velocity
         * \param[in]   ECSComponents::PhysicsComponent& physics - Physics component to modify
         * \return      void
         *===========================================================================================*/
        void ApplyDrag(ECSComponents::PhysicsComponent& physics);

        /*============================================================================================
         * \brief       Integrate forces into acceleration and velocity (Euler integration)
         * \param[in]   ECSComponents::PhysicsComponent& physics - Physics component to update
         * \param[in]   float deltaTime - Time step for integration
         * \return      void
         *
         * Physics integration:
         * acceleration = force / mass
         * velocity += acceleration * deltaTime
         * force = 0 (clear for next frame)
         *===========================================================================================*/
        void IntegrateForces(ECSComponents::PhysicsComponent& physics, float deltaTime);

        /*============================================================================================
         * \brief       Integrate velocity into position with AABB collision detection and response
         * \param[in]   uint32_t entityID - ID of current entity (for self-collision check)
         * \param[in]   ECS::EntityLoc const& entityLoc - Entity to move
         * \param[in]   ECSComponents::TransformComponent& transform - Entity's transform
         * \param[in]   ECSComponents::PhysicsComponent& physics - Entity's physics
         * \param[in]   ECSComponents::CollisionComponent& collision - Entity's collision
         * \param[in]   ECS::EntityManager& entityManager - Entity manager for checking collisions
         * \param[in]   float deltaTime - Time step for integration
         * \return      void
         *
         * This function:
         * 1. Calculates target position: targetPos = currentPos + velocity * dt
         * 2. Checks for collisions at target position
         * 3. If collision detected: resolves position, transfers momentum
         * 4. If no collision: updates position normally
         *===========================================================================================*/
        void IntegrateVelocity(
            uint32_t entityID,
            ECSComponents::TransformComponent& transform,
            ECSComponents::PhysicsComponent& physics,
            ECSComponents::CollisionComponent& collision,
            ECS::EntityManager& entityManager,
            float deltaTime);

        /*============================================================================================
         * \brief       Check if two AABB rectangles overlap (Axis-Aligned Bounding Box test)
         * \param[in]   Math::eVec2<float> const& posA - Position of first entity
         * \param[in]   float widthA - Width of first collider
         * \param[in]   float heightA - Height of first collider
         * \param[in]   Math::eVec2<float> const& posB - Position of second entity
         * \param[in]   float widthB - Width of second collider
         * \param[in]   float heightB - Height of second collider
         * \return      bool - True if rectangles overlap
         *===========================================================================================*/
        bool CheckAABBOverlap(
            const Math::eVec2<float>& posA, float widthA, float heightA,
            const Math::eVec2<float>& posB, float widthB, float heightB) const;

        /*============================================================================================
         * \brief       Check if two circles overlap
         * \param[in]   Math::eVec2<float> const& posA - Position of first circle
         * \param[in]   float radiusA - Radius of first circle
         * \param[in]   Math::eVec2<float> const& posB - Position of second circle
         * \param[in]   float radiusB - Radius of second circle
         * \return      bool - True if circles overlap
         *===========================================================================================*/
        bool CheckCircleCircleOverlap(
            const Math::eVec2<float>& posA, float radiusA,
            const Math::eVec2<float>& posB, float radiusB) const;

        /*============================================================================================
         * \brief       Check if a circle overlaps with a rectangle
         * \param[in]   Math::eVec2<float> const& circlePos - Position of circle
         * \param[in]   float radius - Radius of circle
         * \param[in]   Math::eVec2<float> const& rectPos - Position of rectangle
         * \param[in]   float rectWidth - Width of rectangle
         * \param[in]   float rectHeight - Height of rectangle
         * \return      bool - True if circle and rectangle overlap
         *===========================================================================================*/
        bool CheckCircleRectOverlap(
            const Math::eVec2<float>& circlePos, float radius,
            const Math::eVec2<float>& rectPos, float rectWidth, float rectHeight) const;

        /*============================================================================================
         * \brief       Resolve collision between two entities with momentum transfer
         * \param[in]   ECSComponents::TransformComponent& transformA - First entity's transform
         * \param[in]   ECSComponents::PhysicsComponent& physicsA - First entity's physics
         * \param[in]   ECSComponents::CollisionComponent& collisionA - First entity's collision
         * \param[in]   ECSComponents::TransformComponent& transformB - Second entity's transform
         * \param[in]   ECSComponents::PhysicsComponent& physicsB - Second entity's physics
         * \param[in]   ECSComponents::CollisionComponent& collisionB - Second entity's collision
         * \return      void
         *
         * This function implements proper collision response:
         * 1. Separates the two objects (removes overlap)
         * 2. Calculates collision normal (direction of push)
         * 3. Transfers momentum from moving object to stationary/slower object
         * 4. Applies restitution (bounciness)
         *
         * This enables the player to push boxes!
         *===========================================================================================*/
        void ResolveCollision(
            ECSComponents::TransformComponent& transformA,
            ECSComponents::PhysicsComponent& physicsA,
            ECSComponents::CollisionComponent& collisionA,
            ECSComponents::TransformComponent& transformB,
            ECSComponents::PhysicsComponent& physicsB,
            ECSComponents::CollisionComponent& collisionB);

        /*============================================================================================
         * \brief       Calculate penetration depth and collision normal between two AABBs
         * \param[in]   Math::eVec2<float> const& posA - Position of first entity
         * \param[in]   float widthA - Width of first collider
         * \param[in]   float heightA - Height of first collider
         * \param[in]   Math::eVec2<float> const& posB - Position of second entity
         * \param[in]   float widthB - Width of second collider
         * \param[in]   float heightB - Height of second collider
         * \param[out]  Math::eVec2<float>& normal - Collision normal (direction to separate)
         * \param[out]  float& penetration - How much the objects overlap
         * \return      void
         *===========================================================================================*/
        void CalculateAABBPenetration(
            const Math::eVec2<float>& posA, float widthA, float heightA,
            const Math::eVec2<float>& posB, float widthB, float heightB,
            Math::eVec2<float>& normal, float& penetration) const;

        /*============================================================================================
         * \brief       Calculate penetration depth and collision normal between two circles
         * \param[in]   Math::eVec2<float> const& posA - Position of first circle
         * \param[in]   float radiusA - Radius of first circle
         * \param[in]   Math::eVec2<float> const& posB - Position of second circle
         * \param[in]   float radiusB - Radius of second circle
         * \param[out]  Math::eVec2<float>& normal - Collision normal (direction to separate)
         * \param[out]  float& penetration - How much the circles overlap
         * \return      void
         *===========================================================================================*/
        void CalculateCirclePenetration(
            const Math::eVec2<float>& posA, float radiusA,
            const Math::eVec2<float>& posB, float radiusB,
            Math::eVec2<float>& normal, float& penetration) const;

        /*============================================================================================
         * \brief       Calculate penetration depth and collision normal between circle and rectangle
         * \param[in]   Math::eVec2<float> const& circlePos - Position of circle
         * \param[in]   float radius - Radius of circle
         * \param[in]   Math::eVec2<float> const& rectPos - Position of rectangle
         * \param[in]   float rectWidth - Width of rectangle
         * \param[in]   float rectHeight - Height of rectangle
         * \param[out]  Math::eVec2<float>& normal - Collision normal (direction to separate)
         * \param[out]  float& penetration - How much they overlap
         * \return      void
         *===========================================================================================*/
        void CalculateCircleRectPenetration(
            const Math::eVec2<float>& circlePos, float radius,
            const Math::eVec2<float>& rectPos, float rectWidth, float rectHeight,
            Math::eVec2<float>& normal, float& penetration) const;

        /*============================================================================================
        * \brief       Calculate scaled collision dimensions from transform and collision component
        * \param[in]   ECSComponents::TransformComponent const& transform - Entity's transform
        * \param[in]   ECSComponents::CollisionComponent const& collision - Entity's collision
        * \param[out]  float& scaledWidth - output: width * transform.scale.x
        * \param[out]  float& scaledHeight - output: height * transform.scale.y 
        * \return      void
        * 
        * Function ensures consistent scaling across all physics calculations.
        * It applies the transform's scale to the collision component's dimensions.
        *===========================================================================================*/
        void GetScaledColliderDimensions(
            const ECSComponents::TransformComponent& transform,
            const ECSComponents::CollisionComponent& collision,
            float& scaledWidth,
            float& scaledHeight) const;

        /*============================================================================================
         * \brief       Helper to get transform component from entity location
         * \param[in]   ECS::EntityLoc const& entityLoc - Entity location data
         * \return      ECSComponents::TransformComponent& - Reference to transform
         *===========================================================================================*/
        ECSComponents::TransformComponent& GetTransform(const ECS::EntityLoc& entityLoc);

        /*============================================================================================
         * \brief       Helper to get physics component from entity location
         * \param[in]   ECS::EntityLoc const& entityLoc - Entity location data
         * \return      ECSComponents::PhysicsComponent& - Reference to physics component
         *===========================================================================================*/
        ECSComponents::PhysicsComponent& GetPhysics(const ECS::EntityLoc& entityLoc);

        /*============================================================================================
         * \brief       Helper to get collision component from entity location
         * \param[in]   ECS::EntityLoc const& entityLoc - Entity location data
         * \return      ECSComponents::CollisionComponent& - Reference to collision component
         *===========================================================================================*/
        ECSComponents::CollisionComponent& GetCollision(const ECS::EntityLoc& entityLoc);

        // Physics constants
        static constexpr float GRAVITY = 980.0f;  // Pixels per second squared (gravity acceleration)

        Event::EnoEventBus& bus_;
    };

} // namespace Physics