#pragma once


// Signals
#include "../../event/signals.hpp"

// GLM
#include <glm/glm.hpp>
#include <iostream>
#include "../../math/math.hpp"

// Forward declarations
namespace Editor { struct Node; }
namespace ECS    { class EntityManager; }

namespace ECSComponents
{
	struct TransformComponent
	{
		// Spatial properties only - position, rotation, scale
		Math::eVec2<float> pos{};
		Math::eVec2<float> rot{0.0f, 0.0f};
		Math::eVec2<float> scale{ 1.0f, 1.0f };  // Default to unit scale

		// Reset for play/stop state (editor functionality)
		Math::eVec2<float> resetPos{};

		// Default constructor
		TransformComponent() = default;

		// Constructor
		TransformComponent(Math::eVec2<float> _pos, Math::eVec2<float> _rot, Math::eVec2<float> _scale)
			: pos(_pos), rot(_rot), scale(_scale) {}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

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
			l("Position",		pos,		"eVec2(float)");
			l("Rotation",		rot,		"eVec2(float)");
			l("Scale",			scale,		"eVec2(float)");
			l("Reset Position", resetPos,	"eVec2(float)");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Position",		pos,		"eVec2(float)");
			l("Rotation",		rot,		"eVec2(float)");
			l("Scale",			scale,		"eVec2(float)");
			l("Reset Position", resetPos,	"eVec2(float)");
		}
	};

	// print overload
	inline std::ostream& operator<<(std::ostream& os, const TransformComponent& c)
	{
		std::cout << "Position: (" << c.pos.x	<< ', ' << c.pos.y	 << ')' <<
					 "Rotation: (" << c.rot.x	<< ', ' << c.rot.y	 << ')' <<
					 "Scale: ("    << c.scale.x << ', ' << c.scale.y << ')' <<
					  std::endl;

		return os;
	}

	// Moves a node to newPos and offsets all descendant positions by the same delta.
	void moveNodeWithChildren(Editor::Node* node, Math::eVec2<float> newPos, ECS::EntityManager& entityMng);

}