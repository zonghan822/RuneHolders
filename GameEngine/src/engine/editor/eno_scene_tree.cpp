#include "eno_scene_tree.hpp"

namespace Editor
{
	Node* EnoSceneTree::create(Node* parent)
	{
		auto node = std::make_unique<Node>();
		uint32_t id = generateID();

		node->name_ = "Node" + std::to_string(id);
		node->sceneObjectID_ = id;
		node->nestedLayer_ = 0;
		node->parent_ = parent;
		node->enabled_ = true;
		node->script_ = nullptr;

		Node* unode = node.get();
		tree_[node->sceneObjectID_] = std::move(node);

		if (parent)
			parent->children_.push_back(unode);

		return unode;
	}

	// Nest a node
	void EnoSceneTree::nest(Node* node)
	{
		if (node == selectedNode_) return;

		selectedNode_->children_.push_back(node);
		node->parent_ = selectedNode_;
		node->nestedLayer_ = selectedNode_->nestedLayer_ + 1;
	}

	// Nest a node overload
	void EnoSceneTree::nest(Node* src, Node* dst)
	{
		if (!src || !dst) return;

		dst->children_.push_back(src);
		src->parent_ = dst;
		src->nestedLayer_ = dst->nestedLayer_ + 1;
	}

	// Select a node and display all information
	void EnoSceneTree::select(const uint32_t& sceneID)
	{
		auto it = tree_.find(sceneID);
		if (it == tree_.end())
		{
			throw std::runtime_error("Failed to find Node " + sceneID);
		}

		selectedNode_ = it->second.get();
	}

	// Detach a node from its parent
	void EnoSceneTree::detach(Node* node, Node* dst)
	{
		if (!node || !node->parent_) return;

		// Remove node from parent
		auto& siblings = node->parent_->children_;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());

		uint32_t prevLayer = node->nestedLayer_;
		uint32_t diff{};

		// Set node onto new parent
		if (dst)
		{
			node->parent_ = dst;
			dst->children_.push_back(node);
			node->nestedLayer_ = dst->nestedLayer_ + 1;

			diff = prevLayer - node->nestedLayer_;
		}
		// Set node onto base layer
		else
		{
			node->parent_ = nullptr;
			node->nestedLayer_ = 0;

			diff = prevLayer;
		}

		// Propogate changes to children recursively
		for (Node* child : node->children_)
		{
			updateNestedLayer(child, diff);
		}
	}

	// Delete the selected node and all of its children
	void EnoSceneTree::kill()
	{
		if (!selectedNode_) return;

		// Kill children recursively
		if (!selectedNode_->children_.empty())
		{
			for (Node* child : selectedNode_->children_)
			{
				kill(child);
			}
		}

		selectedNode_->children_.clear();

		// Kill parent / current node
		if (selectedNode_->parent_)
		{
			auto& siblings = selectedNode_->parent_->children_;

			siblings.erase(
				std::remove(siblings.begin(), siblings.end(), selectedNode_),
				siblings.end());
		}

		tree_.erase(selectedNode_->sceneObjectID_);
		selectedNode_ = nullptr;
	}

	// Delete a node and all of its children
	void EnoSceneTree::kill(Node* node)
	{
		if (!node) return;
		if (selectedNode_ && (selectedNode_ == node))
			selectedNode_ = nullptr;

		// Kill children recursively
		if (!node->children_.empty())
		{
			for (Node* child : node->children_)
			{
				kill(child);
			}
		}

		node->children_.clear();

		// Kill parent / current node
		if (node->parent_)
		{
			auto& siblings = node->parent_->children_;

			siblings.erase(
				std::remove(siblings.begin(), siblings.end(), node),
				siblings.end());
		}

		tree_.erase(node->sceneObjectID_);
	}

	// Toggle a node (to enable or disable)
	// With the selected node
	void EnoSceneTree::toggleEnable()
	{
		if (selectedNode_)
			selectedNode_->enabled_ ^= 1;
	}

	// With a pointer to the node
	void EnoSceneTree::toggleEnable(Node* node)
	{
		if (node)
			node->enabled_ ^= 1;
	}

	// With the scene id
	void EnoSceneTree::toggleEnable(const uint32_t& sceneID)
	{
		auto it = tree_.find(sceneID);
		if (it == tree_.end())
		{
			throw std::runtime_error("Failed to find Node " + sceneID);
		}

		it->second->enabled_ ^= 1;
	}

	// Attach a script to a node
	void EnoSceneTree::attachScript(Node* node, std::unique_ptr<Script::EnoScript>&& script)
	{
		if (node)
			node->script_ = std::move(script);
	}

	// Attach a script to the selected Node
	void EnoSceneTree::attachScript(std::unique_ptr<Script::EnoScript>&& script)
	{
		if (selectedNode_)
			selectedNode_->script_ = std::move(script);
	}

	// Detach a script and release it
	void detachScript(std::unique_ptr<Script::EnoScript>&& script)
	{
		script.release();
	}

	// View node information
	void EnoSceneTree::nodeView(const Node* node)
	{
		if (node)
			display(node);
	}

	// View selected node information
	void EnoSceneTree::selectedNodeView()
	{
		if (selectedNode_)
			display(selectedNode_);
	}

	// View tree structure
	void EnoSceneTree::treeView()
	{
		std::cout << "Node Count: " << tree_.size() << std::endl <<
			"________________" << std::endl;

		for (const auto& node : tree_)
		{
			display(node.second.get());
		}
	}

	// Generate an id for a node
	uint32_t EnoSceneTree::generateID()
	{
		if (salvagedID_.empty())
			return static_cast<uint32_t>(tree_.size());
		else
		{
			uint32_t id = salvagedID_.back();
			salvagedID_.pop_back();
			return id;
		}
	}

	// Helper for printing Node information
	void EnoSceneTree::display(const Node* node)
	{
		// ID | Name | Layer | Parent
		std::cout << '[' << node->sceneObjectID_ << "] " <<
			node->name_ << std::endl <<
			"Layer: " << node->nestedLayer_ <<
			" | Parent: " << (node->parent_ ? node->parent_->name_ : "NULL") << std::endl;

		// Children
		if (!node->children_.empty())
		{
			std::cout << "Children: ";
			for (size_t i = 0; i < node->children_.size(); i++)
			{
				if (i > 0) std::cout << " | ";
				std::cout << node->children_[i]->name_;
			}
		}

		std::cout << std::endl << std::endl;
	}

	// Helper for updating nested layer through children nodes
	void EnoSceneTree::updateNestedLayer(Node* node, const uint32_t& diff)
	{
		if (!node) return;

		node->nestedLayer_ -= diff;

		for (Node* child : node->children_)
		{
			updateNestedLayer(node, diff);
		}
	}
}