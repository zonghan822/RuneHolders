#include "eno_scene_tree.hpp"

#include "../math/math.hpp"

namespace Editor
{
	EnoSceneTree::EnoSceneTree(ECS::EntityManager& entMng, Core::EnoCoreHost& coreHost)
		:
		entMng_{ entMng },
		coreHost_{ coreHost }
	{}

	std::vector<Node*> EnoSceneTree::getAllNodes()
	{
		std::vector<Node*> nodes;

		for (const auto& node : tree_)
		{
			nodes.push_back(node.second.get());
		}

		return nodes;
	}

	Node* EnoSceneTree::create(Node* parent)
	{
		auto node = std::make_unique<Node>();
		uint32_t id = generateID();

		node->name_ = "Node" + std::to_string(id);
		node->sceneObjectID_ = id;
		node->nestedLayer_ = 0;
		node->entity_ = entMng_.create<
			ECSComponents::CollisionComponent, ECSComponents::RenderComponent,
			ECSComponents::PhysicsComponent, ECSComponents::PlayerComponent,
			ECSComponents::TransformComponent>(
				ECSComponents::CollisionComponent{ 10.f, 10.f, Math::eVec2{5.f, 5.f}, true },
				ECSComponents::RenderComponent{ 20.f, 20.f, "Trapezoid", 0},
				ECSComponents::PhysicsComponent{ 1.f },
				ECSComponents::PlayerComponent{  },
				ECSComponents::TransformComponent{ Math::eVec2{7.f, 7.f }, Math::eVec2{9.f, 9.f }, Math::eVec2{11.f, 11.f } }
				);

		node->parent_ = parent;
		node->enabled_ = true;
		node->script_ = nullptr;

		Node* unode = node.get();
		tree_[node->sceneObjectID_] = std::move(node);

		if (parent)
			parent->children_.push_back(unode);

		return unode;
	}

	// Create a node from an existing entity (for ImGui integration)
	Node* EnoSceneTree::createFromEntity(ECS::Entity entity, Node* parent)
	{
		auto node = std::make_unique<Node>();
		uint32_t id = generateID();

		node->name_ = "Node" + std::to_string(id);
		node->sceneObjectID_ = id;
		node->nestedLayer_ = parent ? parent->nestedLayer_ + 1 : 0;
		node->entity_ = entity;
		node->parent_ = parent;
		node->enabled_ = true;
		node->script_ = nullptr;

		Node* unode = node.get();
		tree_[node->sceneObjectID_] = std::move(node);

		if (parent)
			parent->children_.push_back(unode);

		return unode;
	}

	// Get a node by entity ID
	Node* EnoSceneTree::getNodeByEntityID(uint32_t entityID)
	{
		for (const auto& [id, node] : tree_)
		{
			if (node->entity_.id() == entityID)
				return node.get();
		}
		return nullptr;
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
	void EnoSceneTree::detachScript(std::unique_ptr<Script::EnoScript>&& script)
	{
		script.release();
	}

	void EnoSceneTree::save(const std::string& name)
	{
		std::string filename{ name };
		// Append preferred extension for save file format
		if (filename.find(".txt") == std::string::npos)
		{
			filename += ".txt";
		}

		std::ofstream save{ filename };

		if (!save)
		{
			throw std::runtime_error("Failed to create a save file for the scene tree!");
		}

		// Format
		// ------
		// {
		//		[Node Name]
		//		{
		//			ID: -
		//			Layer: -
		//			Entity Data:
		//						{
		//							-
		//						}
		//			Attached Script Path: -
		//			Relationships (Parent/Children IDs): -
		//			Enabled: -
		//		}
		// }

		// Write to file (json)
		// Save by node

		// json indent level 
		// used to track number of indents needed, 
		// start at 0
		uint32_t jsonLevel{};

		// Opening brace (for a node)
		save << "{\n\n";

		auto it = tree_.cbegin();
		while (it != tree_.cend())
		{
			// Name
			appendIndent(save, ++jsonLevel);
			save << "[" <<it->second->name_ << "]\n";

			// Opening brace (for node properties)
			appendIndent(save, jsonLevel);
			save << "{\n\t";

			// Scene Object ID
			appendIndent(save, jsonLevel++);
			save << "\"ID\": { " << it->second->sceneObjectID_ << " }\n";

			// Scene Layer
			appendIndent(save, jsonLevel);
			save << "\"Layer\": { " << it->second->nestedLayer_ << " }\n";

			// Entity 
			entMng_.saveJson(save, it->second->entity_, jsonLevel, [&](std::ofstream& file, const uint32_t& level)
				{
					appendIndent(file, level);
				});

			// Attached Script
			appendIndent(save, jsonLevel);
			if (it->second->script_)
				save << "\"Script\": { " << it->second->script_->path() << " }\n";
			else
				save << "\"Script\": { }\n";

			// Relationships
			// Parent
			appendIndent(save, jsonLevel);
			std::string pid{};
			if (it->second->parent_)
				save << "\"Parent\": { " << std::to_string(it->second->parent_->sceneObjectID_) << " }\n";
			else
				save << "\"Parent\": { }\n";

			// Children
			std::vector<uint32_t> cids{};
			getChildrenID(it->second.get(), cids);
			size_t count{};

			appendIndent(save, jsonLevel);
			if (!cids.empty())
			{
				save << "\"Children\": { ";
				std::for_each(cids.begin(), cids.end(), [&save, &count, &cids](const uint32_t& id)
					{
						save << id;
						if (count < cids.size() - 1)
							save << ", ";
						++count;
					});
				save << " }\n";
			}
			else
			{
				save << "\"Children\": { }\n";
			}

			// Enabled
			appendIndent(save, jsonLevel);
			save << "\"Enabled\": { " << (it->second->enabled_ ? "true" : "false") << " }\n";

			// Closing brace (for node properties)
			appendIndent(save, --jsonLevel);
			save << "}\n\n";

			// Adjusting for next iteration
			--jsonLevel;

			++it;
		}

		// Closing brace (for a node)
		save << "}\n";
	}

	// Deserialize data
	void EnoSceneTree::load(const std::string& name)
	{
		std::string filename{ name };
		// Append preferred extension for save file format
		if (filename.find(".txt") == std::string::npos)
		{
			filename += ".txt";
		}

		std::ifstream save{ filename };
		std::stringstream text;
		text << save.rdbuf();
		std::string data = text.str();

		// Setup
		// Relationships are set after all nodes are instantiated
		
		// parents => { node : parent of node, ... }
		std::vector<std::pair<uint32_t, uint32_t>> parents;

		// children => { node : children of node, ... }
		std::vector<std::pair<uint32_t, std::vector<uint32_t>>> children;

		// Find context node first
		size_t start = data.find('[');
		while (start != std::string::npos)
		{
			auto node = std::make_unique<Node>();
			
			// Extract name
			size_t end = data.find(']', start);
			node->name_ = data.substr(start + 1, end - start - 1);
			std::cout << node->name_ << "<< Node name" << std::endl;
			// Find start and end braces of the indentation of context node
			size_t nodeStart = data.find('{', end);
			size_t nodeEnd = findMatchingBrace(data, nodeStart);

			// Track if there are missing fields
			bool missingData{ false };

			// Within these two braces, extract all node params
			// ID
			auto any = extract<uint32_t>(data, "ID", nodeStart, nodeEnd);
			(any.has_value()) ? node->sceneObjectID_ = any.value() : missingData = true;
			std::cout << "ID : " << any.value() << std::endl;
			// Layer
			any = extract<uint32_t>(data, "Layer", nodeStart, nodeEnd);
			(any.has_value()) ? node->nestedLayer_ = any.value() : missingData = true;

			// Data
			entMng_.loadJson(entMng_, data, node->entity_, nodeStart, nodeEnd, [&](const std::string& data, const size_t& openBrace)
				{
					return findMatchingBrace(data, openBrace);
				});

			// Script
			std::optional<std::string> scriptPath = extract<std::string>(data, "Script", nodeStart, nodeEnd);
			if (scriptPath.has_value() && !scriptPath.value().empty())
			{
				// Extract script name, convert to wide and assign
				std::string fname{ extractFname(scriptPath.value()) };
				std::wstring wfname = { fname.begin(), fname.end() };
				std::wstring wpath = { scriptPath.value().begin(), scriptPath.value().end() };

				node->script_ = std::make_unique<Script::EnoScript>(coreHost_, wfname, wpath);
			}

			// Parent
			uint32_t parentID{};
			any = extract<uint32_t>(data, "Parent", nodeStart, nodeEnd);
			if (!any.has_value())
			{
				missingData = true;
			}
			else if (any.value() == UINT32_MAX)
			{ /* empty by design */ }
			else
			{
				parentID = any.value();
				parents.push_back({ node->sceneObjectID_, parentID });
			}

			// Children
			auto cids = extractCont(data, "Children", nodeStart, nodeEnd);
			if (cids.has_value())
				children.push_back({ node->sceneObjectID_, cids.value() });
			else
				missingData = true;

			// Enabled
			any = extract<bool>(data, "Enabled", nodeStart, nodeEnd);
			if (!any.has_value()) missingData = true;
			node->enabled_ = any.value_or(false);

			if (missingData)
				throw std::runtime_error("Failed to retrieve value for one of the save data fields!");

			// Map new node
			tree_[node->sceneObjectID_] = std::move(node);

			// Find the next context node
			start = data.find('[', nodeEnd);
		}

		setParent(parents);
		setChildren(children);
	}
	
	// Helper for assigning parents when deserializing save file
	void EnoSceneTree::setParent(const std::vector<std::pair<uint32_t, uint32_t>>& parents)
	{
		for (const auto& [node, parent] : parents)
		{
			auto it = tree_.find(node);
			auto it2 = tree_.find(parent);

			if (it != tree_.end() || it2 != tree_.end())
			{
				it->second->parent_ = it2->second.get();
			}
		}
	}

	// Helper for assigning children when deserializing save file
	void EnoSceneTree::setChildren(const std::vector<std::pair<uint32_t, std::vector<uint32_t>>>& ncpairs)
	{
		for (const auto& [node, children] : ncpairs)
		{
			auto it = tree_.find(node);
			if (it != tree_.end())
			{
				std::for_each(children.begin(), children.end(), [&](const uint32_t& cid)
					{
						auto it2 = tree_.find(cid);
						if (it2 != tree_.end())
							it->second->children_.push_back(it2->second.get());
						else
							std::cerr << "Child in deserialization transit does not exist!" << std::endl;
					});
			}
		}
	}

	// Appends indentation(s) to save file for json format
	void EnoSceneTree::appendIndent(std::ofstream& save, const uint32_t& level)
	{
		for (size_t i = 0; i < level; i++)
		{
			save << "\t";
		}
	}

	// Look for the matching closing brace and return its pos
	size_t EnoSceneTree::findMatchingBrace(const std::string& data, const size_t& openBrace)
	{
		if (data[openBrace] != '{') return std::string::npos;

		size_t pos = openBrace + 1;
		int depth = 1;

		// Look through data and when }
		while (pos < data.size() && depth > 0)
		{
			if (data[pos] == '{') 
				++depth;

			// found closing
			else if (data[pos] == '}') 
				--depth;

			++pos;
		}

		if (depth == 0)
			return pos - 1;

		return std::string::npos;
	}

	// Filter all special characters from the str passed
	std::string EnoSceneTree::trim(const std::string& str)
	{
		const auto start = str.find_first_not_of(" \t\n\r");
		if (start == std::string::npos)
			return "";

		const auto end = str.find_last_not_of(" \t\n\r");
		return str.substr(start, end - start + 1);
	}

	// Extract file name from the path specified
	std::string EnoSceneTree::extractFname(const std::string& path)
	{
		size_t pos = path.find_last_of("\\/");
		if (pos == std::string::npos)
			return path;

		return path.substr(pos + 1);
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

	// Generate an id for a node (id starts from 1)
	// 0 will be a sentinel to indicate that there is no id
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
		std::cout << 
			'[' << node->sceneObjectID_ << ']' <<
			'[' << (node->enabled_ ? "active" : "inactive") << "] " <<
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
			std::cout << std::endl;
		}

		std::cout << std::endl;
	}

	// Helper for updating nested layer through children nodes
	void EnoSceneTree::updateNestedLayer(Node* node, const uint32_t& diff)
	{
		if (!node) return;

		node->nestedLayer_ -= diff;

		std::for_each(node->children_.begin(), node->children_.end(), [&](Node* child)
			{
				updateNestedLayer(node, diff);
			});
	}

	// Helper for getting children node ids
	void EnoSceneTree::getChildrenID(Node* node, std::vector<uint32_t>& cids)
	{
		std::for_each(node->children_.begin(), node->children_.end(), [&](Node* child)
			{
				getChildrenID(child, cids);
				cids.push_back(child->sceneObjectID_);
			});
	}
}