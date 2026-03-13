#pragma once

// Signals
#include "../../event/signals.hpp"

#include "../../math/math.hpp"
#include <iostream>

namespace ECSComponents
{
	struct CameraComponent
	{
		// Camera viewport dimensions (what area of the world is visible)
		float viewportWidth{ 1920.0f };
		float viewportHeight{ 1080.0f };

		// Camera position (center of the viewport in world space)
		Math::eVec2<float> position{ 0.0f, 0.0f };

		// Whether this camera is active
		bool isActive{ true };

		// When true, camera position is driven by the entity's TransformComponent every frame
		bool followTransform{ false };

		// Default constructor
		CameraComponent() = default;

		// Constructor with viewport size
		CameraComponent(float width, float height)
			: viewportWidth(width), viewportHeight(height) {}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		// For serialization
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Viewport Width",		viewportWidth,		"float");
			l("Viewport Height",	viewportHeight,		"float");
			l("Position",			position,			"eVec2(float)");
			l("Is Active",			isActive,			"bool");
			l("Follow Transform",	followTransform,	"bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Viewport Width",		viewportWidth,		"float");
			l("Viewport Height",	viewportHeight,		"float");
			l("Position",			position,			"eVec2(float)");
			l("Is Active",			isActive,			"bool");
			l("Follow Transform",	followTransform,	"bool");
		}
	};

	// Print overload
	inline std::ostream& operator<<(std::ostream& os, const CameraComponent& c)
	{
		os << "Viewport: " << c.viewportWidth << "x" << c.viewportHeight
		   << " | Position: (" << c.position.x << ", " << c.position.y << ")"
		   << " | Active: " << (c.isActive ? "Yes" : "No")
		   << std::endl;
		return os;
	}
}
