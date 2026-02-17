/*============================================================================================
* @file         player.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-10-2025
* @brief        Player component and system for character control. Contains player component
*               that marks entities as the player and player system that handles movement
*               with collision response and dash mechanics.
*//*========================================================================================*/
#pragma once

#include "math/math.hpp"
#include "component/components/transform.hpp"
#include "collision/collision.hpp"
#include "ecs/entity.hpp"
#include "input/eno_input.hpp"
#include "engine/eno_engine.hpp"
#include "player_config.hpp"
#include "component/components/player_component.hpp"


// Use the PlayerComponent from ECSComponents namespace
using PlayerComponent = ECSComponents::PlayerComponent;

namespace Player
{

    /*============================================================================================
    * \brief       Component that marks an entity as the player character. Attach this to an
    *              entity to designate it as the player. The entity should also have Transform
    *              and Collision components.
    *//*========================================================================================*/

    /*============================================================================================
    * \brief       System that handles player movement with collision response. This system reads
    *              WASD input for movement, calculates desired position, checks for collisions at
    *              new position, and only moves if no collision (collision response). Also handles
    *              dash mechanics with spacebar input.
    *//*========================================================================================*/
    class PlayerSystem
    {
    public:
        /*============================================================================================
        * \brief       Default constructor for PlayerSystem
        *//*========================================================================================*/
        PlayerSystem() = default;

        /*============================================================================================
        * \brief       Main update function - call every frame. Handles player input and movement
        *              with collision checking.
        * \param[in]   ECS::EntityManager& entityManager - Reference to entity manager
        * \param[in]   Input::EnoInputManager& inputManager - Reference to input manager for reading controls
        * \param[in]   float deltaTime - Time since last frame in seconds
        * \return      void
        *//*========================================================================================*/
        void Update(ECS::EntityManager& entityManager, Input::EnoInputManager& inputManager, float deltaTime, const Collision::CollisionSystem& collisionSystem);

        /*============================================================================================
        * \brief       Helper to get transform component from entity (made public for game_logic access)
        * \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
        * \return      ECSComponents::TransformComponent& - Reference to entity's transform component
        *//*========================================================================================*/
        ECSComponents::TransformComponent& GetTransform(const ECS::EntityLoc& entityLoc);

        /*============================================================================================
       * \brief       Helper to get collision component from entity location
       * \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
       * \return      ECSComponents::CollisionComponent& - Reference to entity's collision component
       *//*========================================================================================*/
        Collision::CollisionComponent& GetCollision(const ECS::EntityLoc& entityLoc);

    private:
        /*============================================================================================
        * \brief       Get movement input from WASD keys using EnoInputManager
        * \param[in]   Input::EnoInputManager& inputManager - Reference to the input manager
        * \return      Math::eVec2<float> - Normalized movement vector
        *//*========================================================================================*/
        Math::eVec2<float> GetMovementInput(Input::EnoInputManager& inputManager);

        /*============================================================================================
        * \brief       Check if player can move to a new position without colliding. Tests if the
        *              player's collider would overlap with any solid colliders at the target position.
        * \param[in]   ECS::EntityLoc const& playerEntity - The player entity location
        * \param[in]   ECSComponents::CollisionComponent const& playerCollider - Player's collision component
        * \param[in]   Math::eVec2<float> const& targetPos - Desired new position
        * \param[in]   ECS::EntityManager& entityManager - Entity manager reference
        * \return      bool - True if position is valid (no collisions)
        *//*========================================================================================*/
        bool CanMoveTo(
            uint32_t playerEntityID,
            const Collision::CollisionComponent& playerCollider,
            const ECSComponents::TransformComponent& playerTransform,
            const Math::eVec2<float>& targetPos,
            const ECSComponents::PlayerComponent& playerComp,
            ECS::EntityManager& entityManager,
            const Collision::SpatialHash& spatialHash);
    };

} // namespace Player