#pragma once
#include "math.hpp"
#include "../component/components/drawInfo.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/name.hpp"
#include "../ecs/entity.hpp"



void createTestObject(ECS::EntityManager& ent){

	//test rect initial config
	Math::eVec2<float> pos{ 300.f, 100.f };
	const char* objName = {"testRect"};
	const char* shapeName = { "Rectangle" };
	float width = 100.f;
	float height = 100.f;
	int color = 0; // green

	//now i want to create an entity with 3 different components, transform, drawinfo and name
	ECSComponents::DrawInfoComponent DIC { pos.v1, pos.v2, width, height, shapeName, color };
	ECSComponents::NameComponent NC { objName };
	ECSComponents::eVec2TransformComponent TC { pos.v1, pos.v2, width, height };

	ECS::Entity rect = ent.create<ECSComponents::DrawInfoComponent, ECSComponents::NameComponent, ECSComponents::eVec2TransformComponent>(DIC, NC, TC);
	std::cout << rect.get<ECSComponents::DrawInfoComponent>();

}
	



void updateTestObject();
void testMovement();




