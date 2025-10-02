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

float max(float a, float b);
float min(float a, float b);

void createTestObject(ECS::EntityManager& ent);
void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng);

bool CheckDynamicSquare(
    const Collision::CollisionComponent& colA,
    const ECSComponents::TransformComponent& transformA,
    const Collision::CollisionComponent& colB,
    const ECSComponents::TransformComponent& transformB,
    float& colTi,                 // time of collision (output)
    Math::eVec2<float>& normAtCol,
    const float& dt
);