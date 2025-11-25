/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         drawInfo.hpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @date         01-10-2025
* @brief        This file contains the component constructor for Drawing Information
*				This Component allows the drawing system to act directly upon it so long as it
*				exists.
*//*========================================================================================*/

#pragma once

#include <iostream>
#include <string>

//i would want to add the math and graphics library here
#include "../../graphics/graphics.hpp"
#include "../../graphics/animation/animation.hpp"

#define FILE_PATH_LEN 240


enum WindowID { Main, Edit, FPS, Prefabs, Assets };
static WindowID activeWindow = WindowID::Main;

enum Shapes {
	Rect = 0
};
enum Color {
	Green = 0,
	Red = 1
};

//inline std::ostream& operator<<(std::ostream& os, const Color& c)
//{
//	switch (c)
//	{
//	case Green: return os << "Green";
//	case Red:	return os << "Red";
//	default:    return os << static_cast<int>(c);
//	}
//}

//supposed to be in edit but i'll put here for now -yunqi
enum Filters {
	Architype = 0,
	Last = 1,
	Select = 2,
	All = 3
};

namespace ECSComponents
{
	// RenderComponent: Stores ONLY rendering properties (size, color, texture)
	// Position/rotation/scale are in TransformComponent
	struct RenderComponent
	{
		// Rendering properties - DECLARED FIRST to match initialization order
		float width = 10.0f;
		float height = 10.0f;
		char shape[32] = "Rectangle";
		Graphics::Color color = Graphics::Color::Green;

		// Texture support
		std::shared_ptr<Graphics::EnoTexture> texture = nullptr;
		char texturePath[FILE_PATH_LEN];
		int texID = 0;  // For sprite sheet indexing

		// Default constructor
		RenderComponent() = default;

		// Constructor for basic rendering (no position - use TransformComponent!)
		RenderComponent(float w, float h, const char* n, int c)
			: width(w), height(h), texture(nullptr), texID(0)
		{
			strncpy_s(shape, n, sizeof(shape) - 1);
			shape[sizeof(shape) - 1] = '\0';

			memset(texturePath, 0, sizeof(texturePath));

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

		// Explicit copy constructor
		RenderComponent(const RenderComponent& other)
			: width(other.width), height(other.height), color(other.color),
			texture(other.texture), texID(other.texID)
		{
			strncpy_s(shape, other.shape, sizeof(shape) - 1);
			shape[sizeof(shape) - 1] = '\0';

			strncpy_s(texturePath, other.texturePath, sizeof(texturePath) - 1);
			texturePath[sizeof(texturePath) - 1] = '\0';
		}

		// Copy assignment operator
		RenderComponent& operator=(const RenderComponent& other)
		{
			if (this != &other) {
				width = other.width;
				height = other.height;
				strncpy_s(shape, other.shape, sizeof(shape) - 1);
				shape[sizeof(shape) - 1] = '\0';
				color = other.color;
				texture = other.texture;
				texID = other.texID;
			}
			return *this;
		}

		// For serialization purposes
		// Generalized way of using reflection to access and iterate
		// over fields!
		// 
		// The idea is to (in a function call) create a lambda that will be a "visitor",
		// and extract all information as parameters of that lambda. Each call over a
		// field of the component is used to extract metadata on 1 field [name, value, type]
		// 
		// Source: https://stackoverflow.com/questions/41453/how-can-i-add-reflection-to-a-c-application
		// Additional learning: https://refactoring.guru/design-patterns/visitor
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Width", width, "float");
			l("Height", height, "float");
			l("Shape", shape, "char32");
			l("Color", color, "Color(RGBA)");

			l("Texture Path", texturePath, "texchar240");
			l("TexID", texID, "int");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Width", width, "float");
			l("height", height, "float");
			l("Shape", shape, "char32");
			l("Color", color, "Color(RGBA)");

			l("Texture Path", texturePath, "texchar240");
			l("TexID", texID, "int");
		}
	};

	// Keep DrawInfoComponent as alias for backwards compatibility
	using DrawInfoComponent = RenderComponent;

	inline std::ostream& operator<<(std::ostream& os, const RenderComponent& render)
	{
		auto& shape = render.shape;
		auto& width = render.width;
		auto& height = render.height;

		std::cout
			<< "SHAPE: " << shape << '\n'
			<< "width: " << width << '\n'
			<< "height: " << height << '\n';

		return os;
	}

	//(Yun Qi) Animation Controller so that any entity that wants to have an animation has their own controller.
	struct AnimationControllerComponent {
		uint32_t controllerID;

		AnimationControllerComponent(): controllerID(0) {}

		AnimationControllerComponent(uint32_t id): controllerID(id) {}

		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("ControllerID", controllerID, "uint32_t");

		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("ControllerID", controllerID, "uint32_t");
		}

	};

}