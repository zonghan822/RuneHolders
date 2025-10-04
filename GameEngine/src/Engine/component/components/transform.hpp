#pragma once


// GLM
#include <glm/glm.hpp>
#include <iostream>
#include "../../math/math.hpp"


namespace ECSComponents
{
	struct TransformComponent
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
		glm::vec2 vel;
	};

	inline std::ostream& operator<<(std::ostream& os, const TransformComponent& c)
	{
		std::cout << "Position: (" << c.pos.x << ', ' << c.pos.y << ', ' << c.pos.z << ')' <<
					 "Rotation: (" << c.rot.x << ', ' << c.rot.y << ', ' << c.rot.z << ')' <<
					 "Scale: ("    << c.scale.x << ', ' << c.scale.y << ', ' << c.scale.z << ')'
					 << std::endl;

		return os;
	}

	//im creating a transform componenent for 2d objects
	struct eVec2TransformComponent
	{
		Math::eVec2<float> pos;
		float rot;
		Math::eVec2<float> scale;
		Math::eVec2<float> vel;

		Math::eMtx3x3<float> transform;

		eVec2TransformComponent(float x, float y, float w, float h, float v1, float v2, float r = 0.f) : pos{ x,y }, scale{ w,h }, vel{v1, v2}, rot { r }, transform{} {};

	};



}