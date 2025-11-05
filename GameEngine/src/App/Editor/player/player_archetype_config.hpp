/*============================================================================================
* @file         player_archetype_config.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio)
* @date         02-11-2025
* @brief        Player entity archetype configuration. Defines helper functions to create
*               complete player entities with all required components.
*===========================================================================================*/

#pragma once

#include "../Engine/component/components/transform.hpp"
#include "../Engine/component/components/name.hpp"
#include "../Engine/component/components/player_component.hpp"
#include "../Engine/component/components/collision_component.hpp"
#include "../Engine/component/components/physics_component.hpp"
#include "../Engine/component/components/drawInfo.hpp"
#include "../Engine/ecs/entity.hpp"

// Forward declare PlayerConfig to avoid circular dependency
namespace Player { struct PlayerConfig; }

namespace Player
{
    /*============================================================================================
     * \brief       Helper function to create a complete player entity with all required
     *              components from raw values (no config file needed)
     * \param[in]   ECS::EntityManager& entityManager - Reference to entity manager
     * \param[in]   Math::eVec2<float> const& position - Initial spawn position
     * \param[in]   float maxSpeed - Maximum movement speed (default: 200.0f)
     * \param[in]   float acceleration - Acceleration rate (default: 800.0f)
     * \param[in]   float friction - Friction coefficient (default: 8.0f)
     * \param[in]   float colliderWidth - Collision box width (default: 48.0f)
     * \param[in]   float colliderHeight - Collision box height (default: 48.0f)
     * \return      ECS::Entity - The created player entity with all components
     *===========================================================================================*/
    inline ECS::Entity CreatePlayerEntity(
        ECS::EntityManager& entityManager,
        const Math::eVec2<float>& position,
        float maxSpeed = 200.0f,
        float acceleration = 800.0f,
        float friction = 8.0f,
        float colliderWidth = 48.0f,
        float colliderHeight = 48.0f)
    {
        // Create NameComponent with "Player" name
        ECSComponents::NameComponent nameComp("Player");

        // Create TransformComponent at spawn position
        ECSComponents::TransformComponent transformComp{
            position,                       // pos
            {0.0f, 0.0f},                  // rot
            {1.0f, 1.0f}                   // scale
        };

        // Create PlayerComponent with provided values
        ECSComponents::PlayerComponent playerComp(maxSpeed);
        playerComp.acceleration = acceleration;
        playerComp.friction = friction;
        playerComp.maxSpeed = maxSpeed;
        playerComp.canMove = true;
        playerComp.spriteWidth = colliderWidth;
        playerComp.spriteHeight = colliderHeight;
        playerComp.colliderWidth = colliderWidth;
        playerComp.colliderHeight = colliderHeight;

        // Create CollisionComponent (rectangle collider for player)
        ECSComponents::CollisionComponent collisionComp(
            colliderWidth,
            colliderHeight,
            { 0.0f, 0.0f },                  // offset
            false                           // not a trigger
        );

        // Create PhysicsComponent (optional - for future physics integration)
        ECSComponents::PhysicsComponent physicsComp(1.0f);  // mass = 1.0
        physicsComp.useGravity = false;                     // No gravity for top-down
        physicsComp.isKinematic = false;                      // Manually controlled

        // Create entity with all components
        // Note: We create with Transform and Name first, then add others
        ECS::Entity player = entityManager.create<
            ECSComponents::TransformComponent,
            ECSComponents::NameComponent
        >(transformComp, nameComp);

        // Add remaining components
        player.add<ECSComponents::PlayerComponent>(entityManager, playerComp);
        player.add<ECSComponents::CollisionComponent>(entityManager, collisionComp);
        player.add<ECSComponents::PhysicsComponent>(entityManager, physicsComp);

        // Create DrawInfoComponent with proper initialization for dragging
        // Calculate top-left position from center position
        float topLeftX = position.x - (colliderWidth * 0.5f);
        float topLeftY = position.y - (colliderHeight * 0.5f);
        ECSComponents::RenderComponent drawComp(
            colliderWidth, colliderHeight,
            "Rectangle",
            Color::Green
        );
        player.add<ECSComponents::DrawInfoComponent>(entityManager, drawComp);

        return player;
    }

    /*============================================================================================
     * \brief       Helper function to create a complete player entity from PlayerConfig
     * \param[in]   ECS::EntityManager& entityManager - Reference to entity manager
     * \param[in]   Math::eVec2<float> const& position - Initial spawn position
     * \param[in]   PlayerConfig const& config - Player configuration loaded from file
     * \return      ECS::Entity - The created player entity with all components
     *===========================================================================================*/
    inline ECS::Entity CreatePlayerEntityFromConfig(
        ECS::EntityManager& entityManager,
        const Math::eVec2<float>& position,
        const PlayerConfig& config)
    {
        return CreatePlayerEntity(
            entityManager,
            position,
            config.maxSpeed,
            config.acceleration,
            config.friction,
            config.colliderWidth,
            config.colliderHeight
        );
    }

} // namespace Player