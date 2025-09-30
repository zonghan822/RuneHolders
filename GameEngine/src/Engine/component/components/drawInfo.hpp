#pragma once

#include <iostream>
#include <string>

//i would want to add the math and graphics library here
#include "../../graphics/graphics.hpp"

enum WindowID { Main, Edit, FPS };
static WindowID activeWindow = WindowID::Main;

enum Shapes {
	Circle = 0,
	Rectangle = 1
};
enum Color {
	Green = 0,
	Red = 1
};

//supposed to be in edit but i'll put here for now -yunqi
enum Filters{ 
	Name = 0, 
	Type = 1, 
	ID   = 2
};

namespace ECSComponents
{
	struct DrawInfoComponent
	{
		DrawInfoComponent() = default;

		DrawInfoComponent(float xPosition, float yPosition, float w, float h, const char* n, int c):
			xPos{ xPosition }, yPos{ yPosition }, width{w}, height{h}
		{
			strncpy_s(shape, n, sizeof(shape) - 1);
			shape[sizeof(shape) - 1] = '\0';

			if (c == Color::Green) {
				color = Graphics::Color::Green;
			}
			else if (c == Color::Red) {
				color = Graphics::Color::Red;
			}
			else {
				color = Graphics::Color::Blue;
			}

		}

		float xPos, yPos, width, height;
		char shape[32] = "Rectangle";
		Graphics::Color color = Graphics::Color::Blue;
		
	};

	inline std::ostream& operator<<(std::ostream& os, const DrawInfoComponent& draw)
	{
		
		auto& shape   = draw.shape;
		auto& xPos    = draw.xPos;
		auto& yPos    = draw.yPos;
		auto& width   = draw.width;
		auto& height  = draw.height;

		std::cout 
		<< "SHAPE: "	<< shape  << '\n'
		<< "x: "		<< xPos   << '\n'
		<< "y: "		<< yPos   << '\n'
		<< "width: "	<< width  << '\n'
		<< "height: "	<< height << '\n';

		return os;
	}
}