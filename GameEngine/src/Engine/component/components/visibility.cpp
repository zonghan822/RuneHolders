/*============================================================================================
* @file         visibility.cpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date
* @brief
*//*========================================================================================*/
#include "visibility.hpp"
#include "../../engine/eno_engine.hpp"

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
	}
}