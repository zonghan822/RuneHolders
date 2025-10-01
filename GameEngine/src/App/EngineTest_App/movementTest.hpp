#pragma once
#include "math/math.hpp"
#include "component/components/drawInfo.hpp"
#include "component/components/transform.hpp"
#include "component/components/name.hpp"
#include "ecs/entity.hpp"
#include "engine/eno_engine.hpp"
#include "collision/collision.hpp"



void createTestObject(ECS::EntityManager& ent);
void updateTestObject(ECS::EntityManager& ent, const Engine::EnoEngine& eng);





