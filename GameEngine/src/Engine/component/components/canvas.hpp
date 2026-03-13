/*============================================================================================
* @file         canvas.hpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         27/02/2026
* @brief        This file contains the declaration of the canvas component class for UI elements
*//*========================================================================================*/
#pragma once

// Signals
#include "../../event/signals.hpp"

#include "../../math/math.hpp"
#include <iostream>
#include <vector>
#include <map>

// Forward declarations
namespace Engine
{
	class EnoEngine;
}

namespace ECS
{
	class Entity;
	class EntityManager;
}

namespace Editor
{
	struct Node;
	class EnoSceneTree;
}

namespace ECSComponents
{
	struct CanvasComponent
	{
		// Default constructor
		CanvasComponent() = default;

		// Constructor with viewport size
		CanvasComponent(float width, float height) : viewportWidth(width), viewportHeight(height) {}
		// Canvas viewport dimensions (1920 x 1080 by default)
		float viewportWidth{ 1920.0f };
		float viewportHeight{ 1080.0f };

		// Canvas-specific layer (1-5, separate from global z_index 0-19)
		uint32_t canvasLayer{ 3 };

		// Whether canvas follows camera for HUD behavior
		bool followCamera{ true };

		// Scene node ID of the camera to follow (-1 means no specific camera selected)
		uint32_t followCameraEntityID{ static_cast<uint32_t>(-1) };

		// Offset when following camera (for positioning like top-left, top-right, etc.)
		Math::eVec2<float> cameraOffset{ 0.0f, 0.0f };

		// Whether this canvas is active
		bool isActive{ true };

		// Runtime-only: position saved before the first camera-follow move so it
		// can be restored when the canvas is deactivated.
		Math::eVec2<float> originalPos{ 0.0f, 0.0f };
		bool hasOriginalPos{ false };

		// Accessors
		const float& getViewPortWidth() const
		{
			return viewportWidth;
		}

		const float& getViewPortHeight() const
		{
			return viewportHeight;
		}

		const bool& getActive() const
		{
			return isActive;
		}

		const bool& getFollowCamera() const
		{

			return followCamera;
		}

		// Mutators
		void setViewPortWidth(float w)
		{
			viewportWidth = w;
		}

		void setViewPortHeight(float h)
		{
			viewportHeight = h;
		}

		void setActive(bool flag)
		{
			isActive = flag;
		}

		void setFollowCamera(bool flag)
		{
			followCamera = flag;
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		// For serialization
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Viewport Width",			viewportWidth,			"float");
			l("Viewport Height",		viewportHeight,			"float");
			l("Canvas Layer",			canvasLayer,			"uint32_t");
			l("Follow Camera",			followCamera,			"bool");
			l("Follow Camera Entity",	followCameraEntityID,	"uint32_t");
			l("Is Active",				isActive,				"bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Viewport Width",			viewportWidth,			"float");
			l("Viewport Height",		viewportHeight,			"float");
			l("Canvas Layer",			canvasLayer,			"uint32_t");
			l("Follow Camera",			followCamera,			"bool");
			l("Follow Camera Entity",	followCameraEntityID,	"uint32_t");
			l("Is Active",				isActive,				"bool");
		}
	};

	// Print overload
	inline std::ostream& operator<<(std::ostream& os, const CanvasComponent& c)
	{
		os << "Viewport: " << c.viewportWidth << "x" << c.viewportHeight
		   << " | Layer: " << c.canvasLayer
		   << " | Follow Camera: " << (c.followCamera ? "Yes" : "No")
		   << " | Follow Camera Entity: " << c.followCameraEntityID
		   << " | Active: " << (c.isActive ? "Yes" : "No")
		   << std::endl;
		return os;
	}

}
