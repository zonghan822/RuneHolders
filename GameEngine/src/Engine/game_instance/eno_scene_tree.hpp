/*============================================================================================
* @file         eno_scene_tree.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for a scene tree
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

#include "../ecs/entity.hpp"
#include <set>

namespace GameInstance
{
	/*============================================================================================
		* \brief			Node definition
	*//*========================================================================================*/
	struct Node
	{
		// Scene Data
		//uint32_t sceneObjectID_;
		//uint32_t nestedLayer_;

		// Entity Data
		//ECS::Entity entity_;

		//YunQi's implementation
		uint32_t entityID = 0;       // ECS entity reference
		uint32_t parentID = 0;	     // Parent Node(if any)
		std::vector<uint32_t> children; // List of children


	};

	class EnoSceneTree
	{
	public:

		/*============================================================================================
			* \brief			Create a node on the scene tree
		*//*========================================================================================*/
		//void create()
		//{
		//	Node node;
		//	node.sceneObjectID_ = static_cast<uint32_t>(nodes_.size());
		//	node.nestedLayer_ = 0;

		//	nodes_.push_back(node);
		//}

		Node& addEntityNode(ECS::Entity entity)
		{
			uint32_t id = entity.id();

			auto& node = nodes_[id];
			node.entityID = id;
			node.parentID = 0; // root by default
			return node;
		}


		void setParent(uint32_t childID, uint32_t parentID) {
			// Remove from old parent, if any
			if (nodes_.count(childID)) {
				uint32_t oldParent = nodes_[childID].parentID;
				if (oldParent != 0 && nodes_.count(oldParent)) {
					auto& siblings = nodes_[oldParent].children;
					siblings.erase(std::remove(siblings.begin(), siblings.end(), childID), siblings.end());
				}
			}

			// Assign new parent
			nodes_[childID].parentID = parentID;

			// Add child to new parent's list
			if (parentID != 0 && nodes_.count(parentID)) {
				nodes_[parentID].children.push_back(childID);
			}
		}

		// Returns entityNode iterator
		Node* getNode(uint32_t entityID) {
			auto it = nodes_.find(entityID);
			return (it != nodes_.end()) ? &it->second : nullptr;
		}

		// List of nodes that have no parent to display for topmost layer of imgui
		std::vector<uint32_t> getRootNodes() const {
			std::vector<uint32_t> roots;
			for (const auto& [id, node] : nodes_) {
				if (node.parentID == 0) {
					roots.push_back(id);
				}
			}
			return roots;
		}

		void removeNodeAndDestroyEntity(uint32_t entityID, ECS::EntityManager& em)
		{
			// Check if node exists in scene tree
			auto nodeIt = nodes_.find(entityID);
			if (nodeIt == nodes_.end()) {
				return;
			}

			// Collect all nodes to delete (this node and all descendants)
			std::vector<uint32_t> nodesToDelete;
			std::set<uint32_t> processedNodes; // Track what we've collected to avoid duplicates
			collectNodeAndDescendantsWithCheck(entityID, nodesToDelete, processedNodes);

			for (uint32_t id : nodesToDelete) {
				std::cout << id << " ";
			}
			std::cout << "\n";

			// Remove this node from its parent's children list
			uint32_t parentID = nodeIt->second.parentID;
			if (parentID != 0) {
				auto parentIt = nodes_.find(parentID);
				if (parentIt != nodes_.end()) {
					auto& parentChildren = parentIt->second.children;
					auto removeIt = std::remove(parentChildren.begin(), parentChildren.end(), entityID);
					if (removeIt != parentChildren.end()) {
						parentChildren.erase(removeIt, parentChildren.end());
					}
				}
			}

			// Now process deletion in reverse order (children first, then parent) so that children are destroyed before their parents
			for (auto it = nodesToDelete.rbegin(); it != nodesToDelete.rend(); ++it) {
				uint32_t idToDelete = *it;

				// Check if entity exists in EntityManager before trying to destroy
				try {
					// Check if entity exists in the entity manager's map
					auto& entityMap = em.getMap();
					if (entityMap.find(idToDelete) != entityMap.end()) {
						em.destroyEntity(idToDelete);
					}
				}
				catch (const std::exception& e) {
					std::cout << " Exception while destroying entity " << idToDelete << ": " << e.what() << "\n";
				}

				// Remove from scene tree
				auto eraseIt = nodes_.find(idToDelete);
				if (eraseIt != nodes_.end()) {
					nodes_.erase(eraseIt);
					std::cout << "Removed node " << idToDelete << " from scene tree\n";
				}
			}
		}


		void removeNode(uint32_t id)
		{
			// --- Find the node first ---
			auto it = nodes_.find(id);
			if (it == nodes_.end())
				return; // nothing to remove

			Node& node = it->second;

			// --- Recursively remove all children ---
			for (uint32_t childID : node.children)
			{
				removeNode(childID);
			}

			// --- Remove this node from its parent's children list ---
			if (node.parentID != 0 && nodes_.count(node.parentID))
			{
				auto& parentChildren = nodes_[node.parentID].children;
				parentChildren.erase(
					std::remove(parentChildren.begin(), parentChildren.end(), id),
					parentChildren.end());
			}

			// --- Finally remove this node from the map ---
			nodes_.erase(it);
		}

		const std::unordered_map<uint32_t, Node>& getAllNodes() const {
			return nodes_;
		}


	private:
		//std::vector<Node> nodes_;

		void collectNodeAndDescendantsWithCheck(uint32_t nodeID, std::vector<uint32_t>& collection, std::set<uint32_t>& processed)
		{
			// Check if we've already processed this node (avoid duplicates/cycles)
			if (processed.count(nodeID) > 0) {
				return;
			}

			auto it = nodes_.find(nodeID);
			if (it == nodes_.end()) {
				return;
			}

			// Mark as processed and add to collection
			processed.insert(nodeID);
			collection.push_back(nodeID);

			// Make a copy of children to avoid iterator invalidation
			std::vector<uint32_t> childrenCopy = it->second.children;

			// Recursively collect all children
			for (uint32_t childID : childrenCopy) {
				collectNodeAndDescendantsWithCheck(childID, collection, processed);
			}
		}

		//YunQi
		std::unordered_map<uint32_t, Node> nodes_;
	};
}