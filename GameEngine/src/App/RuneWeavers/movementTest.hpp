/*============================================================================================
* @file         movementTest.hpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         03-10-2025
* @brief        This file contains functions to instatiante objects for dynamic collision
*               testing
* 
* * All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once
#include "math/math.hpp"
#include "component/components/drawInfo.hpp"
#include "component/components/transform.hpp"
#include "component/components/name.hpp"
#include "ecs/entity.hpp"
#include "engine/eno_engine.hpp"
#include "collision/collision.hpp"



struct AABB
{

    Math::eVec2<float>min; // Minimum 2d pos
    Math::eVec2<float>max; // Maximum 2d pos

    /*============================================================================================
     * \brief       This constructor takes in a transform component as well as a collision component
     *              to determine the min and max positions of the object using its coords and dimensions
    *//*========================================================================================*/

    AABB(ECSComponents::TransformComponent transform, ECSComponents::CollisionComponent coll);

};

/*============================================================================================
 * \brief Performs swept AABB collision detection between two dynamic rectangular objects
 *
 * Uses continuous collision detection to determine if and when two axis-aligned bounding
 * boxes will collide during the current frame. Calculates the time of impact and collision
 * normal for physics response.
 *
 * \param[in] ECSComponents::CollisionComponent const& colA - Collision component of first object
 * \param[in] ECSComponents::TransformComponent const& transformA - Transform of first object
 * \param[in] ECSComponents::CollisionComponent const& colB - Collision component of second object
 * \param[in] ECSComponents::TransformComponent const& transformB - Transform of second object
 * \param[out] float& colTi - Time of collision in range [0,1] where 0 is frame start
 * \param[out] Math::eVec2<float>& normal - Normalized collision normal vector
 * \param[in] const double& dt - Reference to dt
 * \return bool - True if collision detected during frame, false otherwise
 *//*========================================================================================*/
bool CheckDynamicSquare(
    const ECSComponents::CollisionComponent& colA,
    const ECSComponents::TransformComponent& transformA,
    const ECSComponents::CollisionComponent& colB,
    const ECSComponents::TransformComponent& transformB,
    float& colTi,                 // time of collision (output)
    Math::eVec2<float>& normal,
    const double& dt
);

/*============================================================================================
 * \brief Creates test entities for collision testing
 *
 * Instantiates rectangular test objects with collision components, transform data, and
 * initial velocities for physics simulation testing.
 *
 * \param[in] ECS::EntityManager& ent - Entity manager to create entities in
 * \return void
 *//*========================================================================================*/
void createTestObject(ECS::EntityManager& ent);

/*============================================================================================
 * \brief Updates test objects with physics simulation and collision response
 *
 * Performs per-frame physics updates including position integration, collision detection,
 * and impulse-based collision response for all test entities. Uses swept AABB for
 * continuous collision detection.
 *
 * \param[in] ECS::EntityManager& ent - Entity manager containing entities to update
 * \param[in] Engine::EnoEngine const& eng - Engine instance providing delta time
 * \return void
 *//*========================================================================================*/
void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng);

