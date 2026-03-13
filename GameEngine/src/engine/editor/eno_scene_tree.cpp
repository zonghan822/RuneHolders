/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_scene_tree.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   42%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   28%
* @co-author    Caven Koh (caven.koh@digipen.edu)   22%
* @brief        See file name.
*//*========================================================================================*/

#include "eno_scene_tree.hpp"

#include "../math/math.hpp"
#include "../component/components/visibility.hpp"
#include "../file/file_system.hpp"
#include "../component/component_registry.hpp"
#include "../graphics/animation/animation.hpp"

namespace Editor
{
	EnoSceneTree::EnoSceneTree(ECS::EntityManager& entMng, Core::EnoCoreHost& coreHost, Map::TileMapSystem& tileMap, ECSComponents::EnoPrefabManager& prefabMng)
		:
		entMng_{ entMng },
		coreHost_{ coreHost },
		tileMap_{ tileMap },
		prefabMng_{ prefabMng }
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

	Node* EnoSceneTree::create()
	{
		return create(NodeType::DEFAULT);
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
			ECSComponents::PhysicsComponent, ECSComponents::TransformComponent, 
			ECSComponents::VisibilityComponent>(
				ECSComponents::CollisionComponent{ 10.f, 10.f, Math::eVec2{5.f, 5.f}, true },
				ECSComponents::RenderComponent{ 20.f, 20.f, "Trapezoid", 0},
				ECSComponents::PhysicsComponent{ 1.f },
				ECSComponents::TransformComponent{ Math::eVec2{7.f, 7.f }, Math::eVec2{9.f, 9.f }, Math::eVec2{11.f, 11.f } },
				ECSComponents::VisibilityComponent{}
				);
		node->type_ = NodeType::EMPTY;
		node->parent_ = parent;
		node->enabled_ = true;
		node->script_ = nullptr;

		Node* unode = node.get();
		tree_[node->sceneObjectID_] = std::move(node);

		if (parent)
			parent->children_.push_back(unode);

		return unode;
	}

	Node* EnoSceneTree::create(NodeType type)
	{
		std::cout << static_cast<int>(type) << std::endl;
		auto node = std::make_unique<Node>();
		uint32_t id = generateID();

		node->name_ = "Node" + std::to_string(id);
		node->sceneObjectID_ = id;
		node->nestedLayer_ = 0;
		modNodeType(node.get(), type);
		node->type_ = type;
		node->parent_ = nullptr;
		node->enabled_ = true;
		node->script_ = nullptr;

		Node* unode = node.get();
		tree_[node->sceneObjectID_] = std::move(node);

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

		// Add VisibilityComponent by default if entity doesn't have one
		if (!entity.has<ECSComponents::VisibilityComponent>(entMng_))
		{
			entity.add<ECSComponents::VisibilityComponent>(entMng_, ECSComponents::VisibilityComponent{});
		}

		Node* unode = node.get();
		tree_[node->sceneObjectID_] = std::move(node);

		if (parent)
			parent->children_.push_back(unode);

		return unode;
	}
	
	// Find and get a node by name
	Node* EnoSceneTree::getNodeByName(const std::string& name)
	{
		for (const auto& node : tree_)
		{
			if (node.second->name_ == name)
			{
				// Found node
				return node.second.get();
			}
		}
		return nullptr;
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

	// Get a node by scene object ID
	Node* EnoSceneTree::getNodeBySceneObjectID(uint32_t sceneObjID)
	{
		auto it = tree_.find(sceneObjID);
		return it != tree_.end() ? it->second.get() : nullptr;
	}

	// Get a child by index
	Node* EnoSceneTree::getChild(Node* node, uint32_t idx)
	{
		if (node->children_.empty())
			return nullptr;

		return node->children_[idx];
	}

	void EnoSceneTree::modNodeType(Node* node, NodeType type)
	{
		node->type_ = type;
		switch (type)
		{
#define PRESET(name, ...) \
		case NodeType::name: \
			node->entity_ = entMng_.presetCreate(prefabMng_.retrieve<ECSComponents::Preset::name>()); \
			break;
			PRESET_LIST
#undef PRESET
		}
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

		// Detach from current parent first (if any)
		if (src->parent_)
		{
			auto& siblings = src->parent_->children_;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), src), siblings.end());
		}

		// Establish new parent-child relationship
		dst->children_.push_back(src);
		src->parent_ = dst;
		src->nestedLayer_ = dst->nestedLayer_ + 1;

		// Recursively update all children's nested layers
		std::function<void(Node*, uint32_t)> updateChildLayers = [&](Node* node, uint32_t parentLayer)
		{
			if (!node) return;
			node->nestedLayer_ = parentLayer + 1;
			for (Node* child : node->children_)
			{
				updateChildLayers(child, node->nestedLayer_);
			}
		};

		// Update all children of src to have correct nested layers
		for (Node* child : src->children_)
		{
			updateChildLayers(child, src->nestedLayer_);
		}
	}

	void EnoSceneTree::nest(uint32_t src, uint32_t dst)
	{
		auto it1 = tree_.find(src);
		auto it2 = tree_.find(dst);

		if (it1 == tree_.end() || it2 == tree_.end())
			return;

		nest(it1->second.get(), it2->second.get());
	}

	// Detach a node from its parent
	void EnoSceneTree::unparent(Node* node)
	{
		if (!node->parent_)
			return;

		auto& children = node->parent_->children_;

		auto it = std::find_if(children.begin(), children.end(),
			[node](Node* c) { return c->sceneObjectID_ == node->sceneObjectID_; });

		if (it != children.end())
			children.erase(it);

		node->parent_ = nullptr;

		int oldDepth = node->nestedLayer_;
		int newDepth = 0;
		int diff = oldDepth - newDepth;

		updateNestedLayer(node, diff);
	}

	// (Overload) Detach a node from its parent
	void EnoSceneTree::unparent(uint32_t id)
	{
		auto itID = tree_.find(id);

		if (itID == tree_.end())
			return;

		Node* node = itID->second.get();

		unparent(node);
	}

	void EnoSceneTree::reparent(Node* node, Node* newParent)
	{
		if (!node) return;

		// Prevent cycles
		Node* p = newParent;
		while (p)
		{
			if (p == node) return;
			p = p->parent_;
		}

		// Remove from old parent
		if (node->parent_)
		{
			auto& siblings = node->parent_->children_;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());
		}

		int32_t prevDepth = node->nestedLayer_;

		node->parent_ = newParent;

		if (newParent)
		{
			newParent->children_.push_back(node);
			node->nestedLayer_ = newParent->nestedLayer_ + 1;
		}
		else
		{
			// Root
			node->nestedLayer_ = 0;
		}

		int32_t diff = prevDepth - node->nestedLayer_;

		for (Node* child : node->children_)
			updateNestedLayer(child, diff);
	}

	// (Overload) Attach a node to a new parent
	void EnoSceneTree::reparent(uint32_t node, uint32_t newParent)
	{
		auto it1 = tree_.find(node);
		auto it2 = tree_.find(newParent);

		if (it1 == tree_.end() || it2 == tree_.end()) return;

		Node* cnode = it1->second.get();
		Node* pnode = it2->second.get();

		reparent(cnode, pnode);
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

		// Remove from tree
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

		// Save the ID for reuse before erasing
		salvagedID_.push_back(node->sceneObjectID_);
		uint32_t entityID = node->entity_.id();
		tree_.erase(node->sceneObjectID_);

		// Destroy the underlying ECS entity so render/physics systems stop processing it
		if (entityID != UINT32_MAX)
			entMng_.destroyEntity(entityID);
	}

	void EnoSceneTree::kill(uint32_t id)
	{
		// Defer the actual deletion so that scripts calling FreeNode() mid-frame
		// do not invalidate the getAllNodes() vector being iterated in beginFrame().
		pendingKill_.push_back(id);
	}

	void EnoSceneTree::flushKill()
	{
		for (uint32_t id : pendingKill_)
		{
			auto it = tree_.find(id);
			if (it != tree_.end())
				kill(it->second.get());
		}
		pendingKill_.clear();
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
		{
			node->script_ = std::move(script);

			// Script attached will receive node metadata
			// Retrieve each unique class stored for the script
			// FIX: Use entity_.id() instead of sceneObjectID_ so bridge functions work correctly
			for (const auto& org : node->script_->origin())
			{
				//coreHost_.setMetaData(org, node->entity_.id(), true);
				coreHost_.setMetaData(org, node->sceneObjectID_, true);
			}
		}
	}

	// Attach a script to the selected Node
	void EnoSceneTree::attachScript(std::unique_ptr<Script::EnoScript>&& script)
	{
		if (selectedNode_)
		{
			selectedNode_->script_ = std::move(script);

			// Script attached will receive node metadata
			// Retrieve each unique class stored for the script
			// FIX: Use entity_.id() instead of sceneObjectID_ so bridge functions work correctly
			for (const auto& org : selectedNode_->script_->origin())
			{
				//coreHost_.setMetaData(org, selectedNode_->entity_.id(), true);
				coreHost_.setMetaData(org, selectedNode_->sceneObjectID_, true);
			}
		}
	}

	// Detach a script from a node
	void EnoSceneTree::detachScript(Node* node)
	{
		if (node && node->script_)
		{
			for (const auto& org : node->script_->origin())
			{
				//coreHost_.setMetaData(org, node->entity_.id(), false);
				coreHost_.setMetaData(org, node->sceneObjectID_, false);
			}

			node->script_.reset();
		}
	}

	// Detach a script from the selected node
	void EnoSceneTree::detachScript()
	{
		if (selectedNode_ && selectedNode_->script_)
		{
			for (const auto& org : selectedNode_->script_->origin())
			{
				//coreHost_.setMetaData(org, selectedNode_->entity_.id(), false);
				coreHost_.setMetaData(org, selectedNode_->sceneObjectID_, false);
			}

			selectedNode_->script_.reset();
		}
	}

	// Attach a script to a node by filename (e.g. "PlayerController.cs") — used when spawning prefabs
	void EnoSceneTree::attachScriptByFilename(Node* node, const std::string& scriptFilename)
	{
		if (!node || scriptFilename.empty()) return;

		std::filesystem::path fullScriptPath = FileSystem::GetScriptsDir() / scriptFilename;
		std::string fname = extractFname(scriptFilename);
		std::wstring wfname(fname.begin(), fname.end());
		std::wstring wpath = fullScriptPath.wstring();

		auto script = std::make_unique<Script::EnoScript>(coreHost_, wfname, wpath);
		attachScript(node, std::move(script));
	}

	Node* EnoSceneTree::spawnFromPrefab(const std::string& prefabName, Node* parent, Animation::AnimationControllerRegistry& animReg)
	{
		const auto* compMap = prefabMng_.getUserPrefabComponents(prefabName);
		if (!compMap) return nullptr;

		ECS::Entity newEntity = entMng_.create<>();

		for (const auto& [compTypeIdx, compAny] : *compMap)
		{
			ECSComponents::EnoComponentRegistry::DeduceCompType(compTypeIdx, [&]<typename T>()
			{
				if (!newEntity.template has<T>(entMng_))
					newEntity.template add<T>(entMng_, std::any_cast<T>(compAny));
			});
		}

		// Each spawned instance needs its own animation controller
		if (newEntity.has<ECSComponents::AnimationControllerComponent>(entMng_))
		{
			auto& animCtrl = newEntity.get<ECSComponents::AnimationControllerComponent>(entMng_);
			animCtrl.controllerID = animReg.createController();
		}

		Node* node = createFromEntity(newEntity, parent);
		node->name_       = prefabName + "_" + std::to_string(node->sceneObjectID_);
		node->type_       = static_cast<NodeType>(prefabMng_.getUserPrefabNodeType(prefabName));
		node->prefabName_ = prefabName;

		std::string prefabScript = prefabMng_.getUserPrefabScript(prefabName);
		if (!prefabScript.empty())
			attachScriptByFilename(node, prefabScript);

		return node;
	}

	// Re-register all scripts with the scripting system
	// This is needed because scenes may load before the scripting system is initialized
	void EnoSceneTree::registerAllScripts()
	{
		for (auto& [id, node] : tree_)
		{
			if (node->script_)
			{
				for (const auto& org : node->script_->origin())
				{
					coreHost_.setMetaData(org, node->sceneObjectID_, true);
				}
			}
		}
	}

	void EnoSceneTree::save(const std::string& name)
	{
		std::string filename{ name };
		// Append preferred extension for save file format
		if (filename.find(".txt") == std::string::npos)
		{
			filename += ".txt";
		}

		std::string tilemapPath = filename.substr(0, filename.size() - 4) + "_tilemap.json";

		// Ensure the parent directory exists
		std::filesystem::create_directories(std::filesystem::path(filename).parent_path());

		std::ofstream save{ filename };

		if (!save)
		{
			std::cerr << "[EnoSceneTree] Failed to create save file: " << filename << std::endl;
			return;
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
			// Check if entity is valid before saving
			uint32_t entityID = it->second->entity_.id();
			auto& entityMap = entMng_.getMap();
			if (entityID == UINT32_MAX || entityMap.find(entityID) == entityMap.end()) {
				std::cerr << "[EnoSceneTree] Warning: Skipping node '" << it->second->name_
						  << "' with invalid/unmapped entity (ID: " << entityID << ")" << std::endl;
				++it;
				continue;
			}

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
			{
				// Extract just the filename from the full path for portability
				std::string scriptPath = it->second->script_->path();
				std::string scriptFilename = extractFname(scriptPath);
				save << "\"Script\": { " << scriptFilename << " }\n";
			}
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

			// Listeners
			appendIndent(save, jsonLevel);
			size_t ltnCount{};

			if (!it->second->listeners_.empty())
			{
				save << "\"Listeners\": { ";
				for (const auto& listener : it->second->listeners_)
				{
					save << "- " << listener << " `";
					if (ltnCount < it->second->listeners_.size() - 1)
						save << ", ";
					++ltnCount;
				}
				save << " }\n";
			}
			else
				save << "\"Listeners\": { }\n";

			// Enabled
			appendIndent(save, jsonLevel);
			save << "\"Enabled\": { " << (it->second->enabled_ ? "true" : "false") << " }\n";

			// Node Type
			appendIndent(save, jsonLevel);
			save << "\"NodeType\": { " << static_cast<uint32_t>(it->second->type_) << " }\n";

			// Prefab Name (empty if not a user prefab instance)
			appendIndent(save, jsonLevel);
			save << "\"PrefabName\": { " << it->second->prefabName_ << " }\n";

			// Closing brace (for node properties)
			appendIndent(save, --jsonLevel);
			save << "}\n\n";

			// Adjusting for next iteration
			--jsonLevel;

			++it;
		}

		// Closing brace (for a node)
		save << "}\n\n";

		// Save tilemap to separate JSON file (only if this scene has a tilemap)
		if (hasTilemap_)
			tileMap_.saveToFile(tilemapPath);

		// Save layer metadata
		save << "\"LayerMetadata\":\n";
		save << "{\n";
		appendIndent(save, 1);
		save << "\"AvailableLayers\": { " << ECSComponents::availableLayers << " }\n";
		save << "}\n";

	save.close();
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

		std::string tilemapPath = filename.substr(0, filename.size() - 4) + "_tilemap.json";

		std::ifstream save{ filename };
		if (!save.is_open())
		{
			std::cerr << "[EnoSceneTree] Failed to open save file: " << filename << std::endl;
			return;
		}

		std::stringstream text;
		text << save.rdbuf();
		std::string data = text.str();

		// Clear existing scene tree and destroy all entities before loading
		std::vector<uint32_t> entitiesToDestroy;
		for (auto& [id, node] : tree_)
		{
			uint32_t entityID = node->entity_.id();
			// Only add valid entity IDs (not UINT32_MAX which indicates invalid/uninitialized entity)
			if (entityID != UINT32_MAX)
			{
				entitiesToDestroy.push_back(entityID);
			}
		}

		// Clear the tree
		tree_.clear();
		selectedNode_ = nullptr;

		// Destroy all entities in the entity manager
		for (uint32_t entityID : entitiesToDestroy)
		{
			entMng_.destroyEntity(entityID);
		}

		if (entitiesToDestroy.size() > 0) {
			std::cout << "[EnoSceneTree] Cleared " << entitiesToDestroy.size() << " entities when loading scene" << std::endl;
		}

		// Setup
		// Relationships are set after all nodes are instantiated

		// parents => { node : parent of node, ... }
		std::vector<std::pair<uint32_t, uint32_t>> parents;

		// children => { node : children of node, ... }
		std::vector<std::pair<uint32_t, std::vector<uint32_t>>> children;

		// listeners => { node : listener of node, ... }
		std::vector<std::pair<uint32_t, std::vector<Listener>>> listeners;

		// Find the end of the nodes section (before TileMap section)
		size_t nodesEndPos = data.find("\"TileMap\"");
		if (nodesEndPos == std::string::npos) {
			nodesEndPos = data.length();
		}

		// Find context node first - only search within nodes section
		size_t start = data.find('[');
		while (start != std::string::npos && start < nodesEndPos)
		{
			auto node = std::make_unique<Node>();

			// Extract name
			size_t end = data.find(']', start);
			if (end >= nodesEndPos) break;  // Stop if we're past the nodes section
			node->name_ = data.substr(start + 1, end - start - 1);

			// Find start and end braces of the indentation of context node
			size_t nodeStart = data.find('{', end);
			size_t nodeEnd = findMatchingBrace(data, nodeStart);

			// Track if there are missing fields
			bool missingData{ false };

			// Within these two braces, extract all node params
			// ID
			auto any = extract<uint32_t>(data, "ID", nodeStart, nodeEnd);
			(any.has_value()) ? node->sceneObjectID_ = any.value() : missingData = true;

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

				// Reconstruct full path from scripts directory for portability
				// Check if it's just a filename (no path separators) or an absolute path
				std::filesystem::path fullScriptPath;
				if (scriptPath.value().find('/') == std::string::npos &&
					scriptPath.value().find('\\') == std::string::npos)
				{
					// It's just a filename, construct full path
					fullScriptPath = FileSystem::GetScriptsDir() / scriptPath.value();
				}
				else
				{
					// It's an absolute path (old format), use as-is for backward compatibility
					fullScriptPath = scriptPath.value();
				}

				std::wstring wpath = fullScriptPath.wstring();
				auto script = std::make_unique<Script::EnoScript>(coreHost_, wfname, wpath);
				attachScript(node.get(), std::move(script));
				//node->script_ = std::make_unique<Script::EnoScript>(coreHost_, wfname, wpath);
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
			auto cids = extractChildren(data, "Children", nodeStart, nodeEnd);
			if (cids.has_value())
				children.push_back({ node->sceneObjectID_, cids.value() });
			else
				missingData = true;

			// Listeners
			auto ltnVec = extractListeners(data, "Listeners", nodeStart, nodeEnd);
			if (ltnVec.has_value())
			{			
				listeners.push_back({ node->sceneObjectID_, std::move(ltnVec.value()) });
			}
			else
			{
				std::cout << "[EnoSceneTree] No listeners found for node " << node->sceneObjectID_ << std::endl;
			}

			// Enabled
			any = extract<bool>(data, "Enabled", nodeStart, nodeEnd);
			if (!any.has_value()) {
				std::cerr << "[EnoSceneTree] Warning: Enabled field missing for node, defaulting to false" << std::endl;
			}
			node->enabled_ = any.value_or(false);

			// Node Type (absent in old scenes, defaults to EMPTY)
			any = extract<uint32_t>(data, "NodeType", nodeStart, nodeEnd);
			node->type_ = any.has_value()
				? static_cast<NodeType>(any.value())
				: NodeType::EMPTY;

			// Prefab Name (absent in old scenes — optional field)
			{
				auto prefabOpt = extract<std::string>(data, "PrefabName", nodeStart, nodeEnd);
				if (prefabOpt.has_value())
					node->prefabName_ = prefabOpt.value();
			}

			if (missingData) {
				std::cerr << "[EnoSceneTree] Warning: Some fields missing for node " << node->name_ << ", using defaults" << std::endl;
			}

			// Map new node
			tree_[node->sceneObjectID_] = std::move(node);

			// Find the next context node - only within nodes section
			start = data.find('[', nodeEnd);
			if (start >= nodesEndPos) break;  // Stop if we found a bracket in tilemap section
		}

		setParent(parents);
		setChildren(children);
		setListeners(listeners);

		// Update the static id_ counter to be higher than any loaded sceneObjectID
		// This prevents newly created nodes from reusing IDs and overwriting existing nodes
		uint32_t maxLoadedID = 0;
		for (const auto& [nodeID, node] : tree_) {
			if (nodeID > maxLoadedID) {
				maxLoadedID = nodeID;
			}
		}
		// Set id_ to one more than the highest loaded ID
		if (maxLoadedID >= id_) {
			id_ = maxLoadedID + 1;
		}

		// Load tilemap from separate JSON file
		hasTilemap_ = tileMap_.loadFromFile(tilemapPath);
		if (!hasTilemap_) {
			while (tileMap_.layers().size() > 0)
				tileMap_.removeLayer(0);
			tileMap_.addLayer("Tile Layer 1", false);
		}

		// Load layer metadata (availableLayers)
		size_t layerMetadataPos = data.find("\"LayerMetadata\"");
		if (layerMetadataPos != std::string::npos) {

			size_t metadataStart = data.find('{', layerMetadataPos);
			if (metadataStart != std::string::npos) {
				size_t metadataEnd = findMatchingBrace(data, metadataStart);
				if (metadataEnd != std::string::npos) {
					// Extract availableLayers
					auto availLayers = extract<uint32_t>(data, "AvailableLayers", metadataStart, metadataEnd);
					if (availLayers.has_value() && availLayers.value() > 0 && availLayers.value() <= static_cast<uint32_t>(ECSComponents::MAX_LAYERS)) {
						ECSComponents::availableLayers = static_cast<int>(availLayers.value());
					}
					else {
						std::cerr << "[EnoSceneTree] Warning: Invalid AvailableLayers value, using default" << std::endl;
					}
				}
			}
		}
		else {
			std::cout << "[EnoSceneTree] No layer metadata found, using default availableLayers (4)" << std::endl;
			// Reset to default if not found in scene file (for backward compatibility)
			ECSComponents::availableLayers = 4;
		}

	// Re-load all user prefabs so cross-scene updates are applied
	prefabMng_.loadAllUserPrefabs();

	// Apply latest prefab data to any node that was created from a user prefab,
	// preserving each instance's own position.
	for (auto& [nodeID, node] : tree_)
	{
		if (node->prefabName_.empty()) continue;
		const auto* compMap = prefabMng_.getUserPrefabComponents(node->prefabName_);
		if (!compMap) continue;
		for (const auto& [compTypeIdx, compAny] : *compMap)
		{
			ECSComponents::EnoComponentRegistry::DeduceCompType(compTypeIdx, [&]<typename T>()
			{
				if (!node->entity_.has<T>(entMng_)) return;
				if constexpr (std::is_same_v<T, ECSComponents::TransformComponent>)
				{
					auto& existing = node->entity_.get<T>(entMng_);
					auto updated = std::any_cast<T>(compAny);
					updated.pos = existing.pos;
					existing = updated;
				}
				else
					node->entity_.get<T>(entMng_) = std::any_cast<T>(compAny);
			});
		}
	}
	}


	// Finding the specified parameter in a file (Children)
	std::optional<std::vector<uint32_t>> EnoSceneTree::extractChildren(
		const std::string& data, const std::string& param,
		const size_t& start, const size_t& end)
	{
		size_t paramPos = data.find(param, start);
		if (paramPos != std::string::npos && paramPos < end)
		{
			size_t paramStrt = data.find_first_of('{', paramPos);
			size_t paramEnd = data.find_first_of('}', paramStrt);

			// Collect and return all ids
			std::string str = trim(data.substr(paramStrt + 1, paramEnd - paramStrt - 1));
			std::istringstream iss{ str };

			std::vector<uint32_t> ids;
			uint32_t num;
			while (iss >> num)
			{
				ids.push_back(num);

				// If next immediate character is not uint32_t, consume
				if (!std::isalnum(iss.peek())) iss.get();
			}

			return ids;
		}

		return std::nullopt;
	}

	// Specialization for finding the specified parameter in a file (Listener)
	std::optional<std::vector<Listener>> EnoSceneTree::extractListeners(
		const std::string& data, const std::string& param,
		const size_t& start, const size_t& end)
	{
		std::vector<Listener> ltnVec{};

		size_t paramPos = data.find(param, start);

		if (paramPos != std::string::npos && paramPos < end)
		{

			size_t paramStrt = data.find_first_of('{', paramPos);
			size_t paramEnd = data.find_first_of('}', paramStrt);

			std::string listeners = data.substr(paramStrt + 1, paramEnd - paramStrt - 1);

			size_t pos{};
			int listenerCount = 0;
			while (pos < listeners.size())
			{

				size_t ltnStrt = listeners.find_first_of('-', pos);
				if (ltnStrt == std::string::npos)
				{
					break;
				}

				size_t ltnEnd = listeners.find_first_of('`', ltnStrt);
				if (ltnEnd == std::string::npos)
				{
					break;
				}


				std::string str = trim(listeners.substr(ltnStrt + 1, ltnEnd - ltnStrt - 1));

				std::istringstream iss{ str };
				std::string idStr, compStr, sigStr;

				std::getline(iss, idStr, ',');
				std::getline(iss, compStr, ',');
				std::getline(iss, sigStr, ',');


				Listener l;
				l.listenerID_ = static_cast<uint32_t>(std::stoul(trim(idStr)));

				std::string trimmedComp = trim(compStr);

				l.component_ = stripPrefix(trimmedComp);

				l.signal_ = trim(sigStr);


				ltnVec.push_back(l);
				listenerCount++;

				pos = ltnEnd + 1;
			}

			return ltnVec;
		}

		return std::nullopt;
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

	// Helper for assigning listeners when deserializing save file
	void EnoSceneTree::setListeners(std::vector<std::pair<uint32_t, std::vector<Listener>>>& nodeLtnPair)
	{

		for (const auto& [node, listener] : nodeLtnPair)
		{

			auto it = tree_.find(node);
			if (it != tree_.end())
			{

				std::for_each(listener.begin(), listener.end(), [&](const Listener& lid)
					{

						auto it2 = tree_.find(lid.listenerID_);
						if (it2 != tree_.end())
						{

							ECSComponents::EnoComponentRegistry::DeduceCompType(lid.component_, [&]<typename Component>()
							{
								connect<Component>(it->second.get(), it2->second.get(), lid.signal_);
							});
						}
						else
							std::cerr << "Listener in deserialization transit does not exist!" << std::endl;
					});
			}
			else
			{
				std::cerr << "[EnoSceneTree] Node " << node << " not found in tree!" << std::endl;
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

	std::string EnoSceneTree::stripPrefix(const std::string& str)
	{
		// MSVC returns "struct" or "class" depending on type
		std::string structPrefix = "struct ECSComponents::";
		std::string classPrefix = "class ECSComponents::";

		if (str.rfind(structPrefix, 0) == 0)
			return str.substr(structPrefix.length());

		if (str.rfind(classPrefix, 0) == 0)
			return str.substr(classPrefix.length());

		std::cerr << "Prefix to strip cannot be found!" << std::endl;
		return str;
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
		// Skip any IDs already occupied in the tree (guards against id_ being
		// out of sync with loaded scene node IDs, which would silently overwrite
		// the existing node when tree_[id] = std::move(node) is called).
		while (id_ == 0 || tree_.count(id_))
			++id_;
		return id_++;
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
				updateNestedLayer(child, diff);
			});
	}

	// Helper for getting children node ids
	void EnoSceneTree::getChildrenID(Node* node, std::vector<uint32_t>& cids)
	{
		for (Node* child : node->children_)
			cids.push_back(child->sceneObjectID_);
	}
}