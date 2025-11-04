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

		Node& addNode(uint32_t entityID) {
			auto& node = nodes_[entityID];
			node.entityID = entityID;
			node.parentID = 0; // root of nodes
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


		void removeNode(uint32_t entityID)
		{
			// If the node doesn't exist, return and exit
			auto it = nodes_.find(entityID);
			if (it == nodes_.end())
				return;

			std::vector<uint32_t> toDelete;
			toDelete.push_back(entityID);

			for (size_t i = 0; i < toDelete.size(); ++i)
			{
				uint32_t current = toDelete[i];
				auto nodeIt = nodes_.find(current);
				if (nodeIt != nodes_.end())
				{
					// virtually delete all childrent that parent holds
					for (uint32_t childID : nodeIt->second.children)
						toDelete.push_back(childID);
				}
			}

			uint32_t parentID = it->second.parentID;
			if (parentID != 0 && nodes_.count(parentID))
			{
				auto& siblings = nodes_[parentID].children;
				siblings.erase(
					std::remove(siblings.begin(), siblings.end(), entityID),
					siblings.end());
			}

			for (uint32_t id : toDelete)
				nodes_.erase(id);
		}


		const std::unordered_map<uint32_t, Node>& getAllNodes() const {
			return nodes_; 
		}


	private:
		//std::vector<Node> nodes_;
		
		//YunQi
		std::unordered_map<uint32_t, Node> nodes_;
	};
}