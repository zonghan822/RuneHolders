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

    Math::eVec2<float>min;
    Math::eVec2<float>max;

    AABB(ECSComponents::TransformComponent transform, Collision::CollisionComponent coll)
    {
        float minx = transform.pos.x - coll.data.rectangle.width / 2.0f;
        float miny = transform.pos.y - coll.data.rectangle.height / 2.0f;

        float maxx = transform.pos.x + coll.data.rectangle.width / 2.0f;
        float maxy = transform.pos.y + coll.data.rectangle.height / 2.0f;

        min = { minx, miny };
        max = { maxx, maxy };
    }

   

};

/*============================================================================================
 * \brief Returns the maximum of two float values
 * \param[in] float a - First value
 * \param[in] float b - Second value
 * \return float - The larger of the two values
 *//*========================================================================================*/
float max(float a, float b);

/*============================================================================================
 * \brief Returns the minimum of two float values
 * \param[in] float a - First value
 * \param[in] float b - Second value
 * \return float - The smaller of the two values
 *//*========================================================================================*/
float min(float a, float b);

/*============================================================================================
 * \brief Performs swept AABB collision detection between two dynamic rectangular objects
 *
 * Uses continuous collision detection to determine if and when two axis-aligned bounding
 * boxes will collide during the current frame. Calculates the time of impact and collision
 * normal for physics response.
 *
 * \param[in] Collision::CollisionComponent const& colA - Collision component of first object
 * \param[in] ECSComponents::TransformComponent const& transformA - Transform of first object
 * \param[in] Collision::CollisionComponent const& colB - Collision component of second object
 * \param[in] ECSComponents::TransformComponent const& transformB - Transform of second object
 * \param[out] float& colTi - Time of collision in range [0,1] where 0 is frame start
 * \param[out] Math::eVec2<float>& normal - Normalized collision normal vector
 * \return bool - True if collision detected during frame, false otherwise
 *//*========================================================================================*/
bool CheckDynamicSquare(
    const Collision::CollisionComponent& colA,
    const ECSComponents::TransformComponent& transformA,
    const Collision::CollisionComponent& colB,
    const ECSComponents::TransformComponent& transformB,
    float& colTi,                 // time of collision (output)
    Math::eVec2<float>& normAtCol,
    const float& dt
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

