/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         drawInfo.hpp
* @author       Koe Yun Ji (yunji.koe@digipen.edu)   62%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   21%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   15%
* @date         01-10-2025
* @brief        This file contains the component constructor for Drawing Information
*//*========================================================================================*/



#pragma once

#include <iostream>
#include <string>

// Signals
#include "../../event/signals.hpp"

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

// Used for deletion of Entities
enum Filters {
	Last = 0,
	Select = 1,
	All = 2
};

namespace ECSComponents
{
	// RenderComponent: Stores ONLY rendering properties (size, color, texture)
	// Position/rotation/scale are in TransformComponent
	struct RenderComponent
	{
		// Rendering properties - DECLARED FIRST to match initialization order
		float width = 64.0f;
		float height = 64.0f;
		char shape[32] = "Rectangle";
		Graphics::Color color = Graphics::Color::White;

		// Texture support
		std::shared_ptr<Graphics::EnoTexture> texture = nullptr;
		char texturePath[FILE_PATH_LEN] = "";
		int texID = 0;  // For sprite sheet indexing

		// UV region — normalized 0..1 coordinates into the texture.
		// Default is the full texture; set these to render a sub-region (e.g. one cell of a spritesheet).
		Math::eVec2<float> uvStart{ 0.0f, 0.0f };
		Math::eVec2<float> uvEnd  { 1.0f, 1.0f };

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
			texture(other.texture), texID(other.texID),
			uvStart(other.uvStart), uvEnd(other.uvEnd)
		{
			strncpy_s(shape, other.shape, sizeof(shape) - 1);
			shape[sizeof(shape) - 1] = '\0';

			strncpy_s(texturePath, other.texturePath, sizeof(texturePath) - 1);
			texturePath[sizeof(texturePath) - 1] = '\0';
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

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
				strncpy_s(texturePath, other.texturePath, sizeof(texturePath) - 1);
				texturePath[sizeof(texturePath) - 1] = '\0';
				texID = other.texID;
				uvStart = other.uvStart;
				uvEnd = other.uvEnd;
			}
			return *this;
		}

		/** @brief Assign a texture and record its file path for serialization. */
		void setTexture(std::shared_ptr<Graphics::EnoTexture> tex, const std::string& path)
		{
			texture = tex;
			strncpy_s(texturePath, path.c_str(), sizeof(texturePath) - 1);
			texturePath[sizeof(texturePath) - 1] = '\0';
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
			l("UV Start", uvStart, "eVec2(float)");
			l("UV End", uvEnd, "eVec2(float)");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Width", width, "float");
			l("Height", height, "float");
			l("Shape", shape, "char32");
			l("Color", color, "Color(RGBA)");

			l("Texture Path", texturePath, "texchar240");
			l("TexID", texID, "int");
			l("UV Start", uvStart, "eVec2(float)");
			l("UV End", uvEnd, "eVec2(float)");
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

	// Structure to hold data for a single animation
	struct AnimationData {
		char name[64] = "";
		char texturePath[FILE_PATH_LEN] = "";
		int frameWidth = 0;
		int frameHeight = 0;
		int rows = 0;
		int cols = 0;
		float frameDuration = 0.1f;
		bool looping = true;
	};

	//(Yun Qi) Animation Controller so that any entity that wants to have an animation has their own controller.
	struct AnimationControllerComponent {
		uint32_t controllerID;

		// Store up to 10 animations (idle, walk, attack, etc.) - for runtime use
		static constexpr int MAX_ANIMATIONS = 10;
		AnimationData animations[MAX_ANIMATIONS];
		int animationCount = 0;
		char currentAnimationName[64] = "";

		// Serialization fields - using old format for persistence
		int frameWidth = 0;
		int frameHeight = 0;
		int rows = 0;
		int cols = 0;
		float frameDuration = 0.1f;
		bool looping = true;

		AnimationControllerComponent(): controllerID(0) {}

		AnimationControllerComponent(uint32_t id): controllerID(id) {}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		/** @brief Add or update an animation entry by name with the given spritesheet parameters. */
		void setAnimationData(const char* name, const char* texPath, int fw, int fh, int r, int c, float dur, bool loop) {
			// Check if animation already exists
			int index = -1;
			for (int i = 0; i < animationCount; ++i) {
				if (strcmp(animations[i].name, name) == 0) {
					index = i;
					break;
				}
			}

			// If not found and we have space, add new
			if (index == -1) {
				if (animationCount < MAX_ANIMATIONS) {
					index = animationCount++;
				} else {
					return; // No space
				}
			}

			// Set the data
			strncpy_s(animations[index].name, name, sizeof(animations[index].name) - 1);
			animations[index].name[sizeof(animations[index].name) - 1] = '\0';

			strncpy_s(animations[index].texturePath, texPath, sizeof(animations[index].texturePath) - 1);
			animations[index].texturePath[sizeof(animations[index].texturePath) - 1] = '\0';

			animations[index].frameWidth = fw;
			animations[index].frameHeight = fh;
			animations[index].rows = r;
			animations[index].cols = c;
			animations[index].frameDuration = dur;
			animations[index].looping = loop;

			// Set current animation name if not already set
			if (strlen(currentAnimationName) == 0) {
				strncpy_s(currentAnimationName, name, sizeof(currentAnimationName) - 1);
				currentAnimationName[sizeof(currentAnimationName) - 1] = '\0';
			}

			// Update serialization fields from the current animation
			updateSerializationFields();
		}

		/** @brief Sync legacy serialization fields from the currently active animation data. */
		void updateSerializationFields() {
			// Find the current animation and sync its data to the old format fields
			for (int i = 0; i < animationCount; ++i) {
				if (strcmp(animations[i].name, currentAnimationName) == 0) {
					frameWidth = animations[i].frameWidth;
					frameHeight = animations[i].frameHeight;
					rows = animations[i].rows;
					cols = animations[i].cols;
					frameDuration = animations[i].frameDuration;
					looping = animations[i].looping;
					return;
				}
			}
			// If no match found, use first animation if available
			if (animationCount > 0) {
				frameWidth = animations[0].frameWidth;
				frameHeight = animations[0].frameHeight;
				rows = animations[0].rows;
				cols = animations[0].cols;
				frameDuration = animations[0].frameDuration;
				looping = animations[0].looping;
			}
		}

		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			// Always save old format fields FIRST - these are required for backward compatibility and draw calls
			l("Frame Width", frameWidth, "int");
			l("Frame Height", frameHeight, "int");
			l("Rows", rows, "int");
			l("Cols", cols, "int");
			l("Frame Duration", frameDuration, "float");
			l("Looping", looping, "bool");

			// Don't serialize controllerID - it will be reassigned when component is added
			l("Animation Count", animationCount, "int");
			l("Current Animation", currentAnimationName, "char64");

			// Serialize all animations using new array format (ADDED, not replacing old format)
			for (int i = 0; i < animationCount && i < MAX_ANIMATIONS; ++i) {
				char indexStr[32];
				sprintf_s(indexStr, "%d", i);

				std::string nameField = std::string("Anim") + indexStr + " Name";
				std::string texField = std::string("Anim") + indexStr + " Texture";
				std::string fwField = std::string("Anim") + indexStr + " FrameW";
				std::string fhField = std::string("Anim") + indexStr + " FrameH";
				std::string rowsField = std::string("Anim") + indexStr + " Rows";
				std::string colsField = std::string("Anim") + indexStr + " Cols";
				std::string durField = std::string("Anim") + indexStr + " Duration";
				std::string loopField = std::string("Anim") + indexStr + " Looping";

				l(nameField.c_str(), animations[i].name, "char64");
				l(texField.c_str(), animations[i].texturePath, "texchar240");
				l(fwField.c_str(), animations[i].frameWidth, "int");
				l(fhField.c_str(), animations[i].frameHeight, "int");
				l(rowsField.c_str(), animations[i].rows, "int");
				l(colsField.c_str(), animations[i].cols, "int");
				l(durField.c_str(), animations[i].frameDuration, "float");
				l(loopField.c_str(), animations[i].looping, "bool");
			}
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Animation Count", animationCount, "int");
			l("Current Animation", currentAnimationName, "char64");

			// Serialize all animations using old format (Frame Width, Height, Rows, Cols)
			for (int i = 0; i < animationCount && i < MAX_ANIMATIONS; ++i) {
				char indexStr[32];
				sprintf_s(indexStr, "%d", i);

				std::string nameField = std::string("Anim") + indexStr + " Name";
				std::string texField = std::string("Anim") + indexStr + " Texture";
				std::string fwField = std::string("Anim") + indexStr + " FrameW";
				std::string fhField = std::string("Anim") + indexStr + " FrameH";
				std::string rowsField = std::string("Anim") + indexStr + " Rows";
				std::string colsField = std::string("Anim") + indexStr + " Cols";
				std::string durField = std::string("Anim") + indexStr + " Duration";
				std::string loopField = std::string("Anim") + indexStr + " Looping";

				l(nameField.c_str(), animations[i].name, "char64");
				l(texField.c_str(), animations[i].texturePath, "texchar240");
				l(fwField.c_str(), animations[i].frameWidth, "int");
				l(fhField.c_str(), animations[i].frameHeight, "int");
				l(rowsField.c_str(), animations[i].rows, "int");
				l(colsField.c_str(), animations[i].cols, "int");
				l(durField.c_str(), animations[i].frameDuration, "float");
				l(loopField.c_str(), animations[i].looping, "bool");
			}

			// Don't save old format fields when new format is used
			// This is only for loading old saves
		}

	};

}