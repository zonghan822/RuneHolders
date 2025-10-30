#pragma once

#include "../ecs/entity.hpp"
#include "../scripting/eno_script.hpp"

namespace Editor
{
	struct Node
	{
		// Scene Data
		std::string name_;
		uint32_t sceneObjectID_{ 0 };
		uint32_t nestedLayer_{ 0 };

		// Entity Data
		//ECS::Entity entity_;

		// Attached script
		std::unique_ptr<Script::EnoScript> script_;

		// Relationships
		Node* parent_{ nullptr };
		std::vector<Node*> children_;

		// Enabled
		bool enabled_{ false };
	};

	class EnoSceneTree
	{
	public:

		EnoSceneTree() = default;
		EnoSceneTree(const EnoSceneTree&) = delete;
		EnoSceneTree(EnoSceneTree&&) = delete;

		// Create a node
		Node* create(Node* parent = nullptr);

		// Nest a node
		void nest(Node*);
		void nest(Node*, Node*);

		// Select a node and display all information
		void select(const uint32_t&);

		// Detach a node from its parent
		void detach(Node*, Node* dst = nullptr);

		// Delete a node and all of its children
		void kill();
		void kill(Node*);

		// Toggle the use of a node
		void toggleEnable();
		void toggleEnable(Node*);
		void toggleEnable(const uint32_t&);

		// Attach script
		void attachScript(Node*, std::unique_ptr<Script::EnoScript>&&);
		void attachScript(std::unique_ptr<Script::EnoScript>&&);

		// Detach script
		void detachScript(std::unique_ptr<Script::EnoScript>&&);

		// View node information
		void nodeView(const Node* node);

		// View selected node information
		void selectedNodeView();

		// View tree structure
		void treeView();

	private:
		inline static uint32_t id_ = 0;

		std::unordered_map<uint32_t, std::unique_ptr<Node>> tree_;
		Node* selectedNode_{ nullptr };

		std::vector<uint32_t> salvagedID_;

		// Button input operation states
		bool creating_{ false };


		// Generate an id for a node
		uint32_t generateID();

		// Helper for printing Node information
		void display(const Node*);

		// Helper for updating nested layer
		void updateNestedLayer(Node*, const uint32_t&);
	};
}