/*============================================================================================
* @file         transform.cpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date			06/03/2026
* @brief		Transform utility functions (node movement with child propagation)
*//*========================================================================================*/

#include "transform.hpp"
#include "../../editor/eno_scene_tree.hpp"
#include "../../ecs/entity.hpp"

// ---------------------------------------------------------------------------
// File-local recursive helper — not exposed in the header
// ---------------------------------------------------------------------------
static void moveChildrenRecursive(Editor::Node* node,
	Math::eVec2<float> delta,
	ECS::EntityManager& entityMng)
{
	if (!node) return;

	if (node->entity_.has<ECSComponents::TransformComponent>(entityMng))
	{
		auto& t = node->entity_.get<ECSComponents::TransformComponent>(entityMng);
		t.pos.x += delta.x;
		t.pos.y += delta.y;
	}

	for (Editor::Node* child : node->children_)
		moveChildrenRecursive(child, delta, entityMng);
}

// ---------------------------------------------------------------------------

namespace ECSComponents
{
	void moveNodeWithChildren(Editor::Node* node,
		Math::eVec2<float> newPos,
		ECS::EntityManager& entityMng)
	{
		if (!node || !node->entity_.has<TransformComponent>(entityMng))
			return;

		auto& t = node->entity_.get<TransformComponent>(entityMng);

		Math::eVec2<float> delta = { newPos.x - t.pos.x, newPos.y - t.pos.y };
		t.pos = newPos;

		for (Editor::Node* child : node->children_)
			moveChildrenRecursive(child, delta, entityMng);
	}
}
