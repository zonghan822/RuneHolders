/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         visibility.cpp
* @author       Caven Koh (caven.koh@digipen.edu)   75%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   25%
* @date         29/01/2026
* @brief        Contains functions to sort entities into layers for rendering
*//*========================================================================================*/


#include "visibility.hpp"
#include "canvas.hpp"
#include "../../engine/eno_engine.hpp"
#include "../../ui/canvas_system.hpp"
#include "transform.hpp"

namespace ECSComponents {

	void collectChildrenRecursive(Editor::Node* node, ECS::EntityManager& entityMng, std::map<int, std::vector<ECS::Entity*>>& layers)
	{
		if (!node) return;

		// Only collect entities that have VisibilityComponent
		if (node->entity_.has<VisibilityComponent>(entityMng)) {
			auto& vis = node->entity_.get<VisibilityComponent>(entityMng);

			// Skip invisible entities
			if (!vis.visible) return;

			// Get the layer of the current object
			int zIndex = static_cast<int>(vis.z_index);

			// Skip if this layer is globally disabled
			if (!isLayerVisible(zIndex)) return;

			layers[zIndex].push_back(&node->entity_);
		}

		// Recursively process children (each uses its own z_index)
		for (auto* child : node->children_) {
			collectChildrenRecursive(child, entityMng, layers);
		}
	}

	// Walk up the scene tree from the given entity; return false if any ancestor is invisible.
	bool isAncestorVisible(uint32_t entityID, Editor::EnoSceneTree& tree, ECS::EntityManager& entityMng)
	{
		Editor::Node* node = tree.getNodeByEntityID(entityID);
		if (!node) return true;

		Editor::Node* current = node->parent_;
		while (current) {
			if (current->entity_.has<VisibilityComponent>(entityMng)) {
				if (!current->entity_.get<VisibilityComponent>(entityMng).visible) {
					return false;
				}
			}
			current = current->parent_;
		}
		return true;
	}

	void collectObjectsByLayer(Engine::EnoEngine& eng, std::vector<ECS::Entity>& cachedEntities)
	{
		// Reset the map
		sortedObjectLayers.clear();

		auto& entityMng  = eng.getEntityManager();
		auto& sceneTree  = eng.getEnoEditor().getSceneTree();

		// Sort cached entities by their z_index
		for (auto& entity : cachedEntities) {
			// Skip canvas entities themselves
			if (entity.has<CanvasComponent>(entityMng)) {
				continue;
			}

			// Skip entities belonging to canvas
			if (UI::CanvasSystem::hasCanvasAncestor(entity.id(), sceneTree, entityMng)) {
				continue;
			}

			// Check if entity has VisibilityComponent
			if (entity.has<VisibilityComponent>(entityMng)) {
				auto& vis = entity.get<VisibilityComponent>(entityMng);

				// Skip invisible entities
				if (!vis.visible) continue;

				// Skip if any ancestor in the scene tree is invisible
				if (!isAncestorVisible(entity.id(), sceneTree, entityMng)) continue;

				// Get the layer of the current object
				int zIndex = static_cast<int>(vis.z_index);

				// Skip if this layer is globally disabled
				if (!isLayerVisible(zIndex)) continue;

				sortedObjectLayers[zIndex].push_back(&entity);
			}
			else {
				// Entities without VisibilityComponent go to default layer 3
				// Only add if default layer is visible
				if (isLayerVisible(3)) {
					sortedObjectLayers[3].push_back(&entity);
				}
			}
		}

		// Y-sort each layer: entities with lower Y (top of screen) draw first (behind)
		for (auto& [z, bucket] : sortedObjectLayers)
		{
			std::sort(bucket.begin(), bucket.end(),
				[&entityMng](ECS::Entity* a, ECS::Entity* b)
				{
					float ay = a->has<TransformComponent>(entityMng)
						? a->get<TransformComponent>(entityMng).pos.y : 0.f;
					float by = b->has<TransformComponent>(entityMng)
						? b->get<TransformComponent>(entityMng).pos.y : 0.f;
					return ay < by;
				});
		}
	}

}