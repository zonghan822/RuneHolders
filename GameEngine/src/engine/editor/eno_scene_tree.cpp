#include "eno_scene_tree.hpp"

#include "../math/math.hpp"
#include "../ai/state/ai_events.hpp"

namespace Editor
{
	EnoSceneTree::EnoSceneTree(ECS::EntityManager& entMng, Core::EnoCoreHost& coreHost, Map::TileMapSystem& tileMap)
		:
		entMng_{ entMng },
		coreHost_{ coreHost },
		tileMap_{ tileMap }
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
		{
			node->script_ = std::move(script);

			// Script attached will receive node metadata
			// Retrieve each unique class stored for the script
			for (const auto& org : node->script_->origin())
			{
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
			for (const auto& org : selectedNode_->script_->origin())
			{
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
				coreHost_.setMetaData(org, node->sceneObjectID_, false);
			}
		}
	}

	// Detach a script from the selected node
	void EnoSceneTree::detachScript()
	{
		if (selectedNode_ && selectedNode_->script_)
		{
			for (const auto& org : selectedNode_->script_->origin())
			{
				coreHost_.setMetaData(org, selectedNode_->sceneObjectID_, false);
			}
		}
	}

	void EnoSceneTree::save(const std::string& name, const std::string& sceneName)
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

			// Listeners
			appendIndent(save, jsonLevel);
			size_t ltnCount{};

			if (!it->second->listeners_.empty())
			{
				save << "\"Listeners\": { ";
				for (const auto& listener : it->second->listeners_)
				{
					save << "[ " << listener << " ]";
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

			// Closing brace (for node properties)
			appendIndent(save, --jsonLevel);
			save << "}\n\n";

			// Adjusting for next iteration
			--jsonLevel;

			++it;
		}

		// Closing brace (for a node)
		save << "}\n\n";

		// Save tilemap data embedded in scene file
		save << "\"TileMap\":\n";
		save << "{\n";
		appendIndent(save, 1);
		save << "\"Width\": { " << tileMap_.width() << " }\n";
		appendIndent(save, 1);
		save << "\"Height\": { " << tileMap_.height() << " }\n";
		appendIndent(save, 1);
		save << "\"CellSize\": { " << tileMap_.cellSize() << " }\n";

		// Save layers
		appendIndent(save, 1);
		save << "\"Layers\":\n";
		appendIndent(save, 1);
		save << "[\n";

		const auto& layers = tileMap_.layers();
		for (size_t i = 0; i < layers.size(); ++i) {
			const auto& layer = layers[i];
			appendIndent(save, 2);
			save << "{\n";

			appendIndent(save, 3);
			save << "\"Name\": { " << layer.name() << " }\n";
			appendIndent(save, 3);
			save << "\"IsNav\": { " << (layer.isNavLayer() ? "true" : "false") << " }\n";

			// Save tiles for this layer
			appendIndent(save, 3);
			save << "\"Tiles\":\n";
			appendIndent(save, 3);
			save << "[\n";

			for (int y = 0; y < tileMap_.height(); ++y) {
				appendIndent(save, 4);
				save << "[ ";
				for (int x = 0; x < tileMap_.width(); ++x) {
					const auto& tile = layer.tiles()[y][x];
					save << "{ \"walk\": " << (tile.walkable ? "true" : "false")
						 << ", \"tex\": \"" << tile.textureName << "\""
						 << ", \"uv0\": [" << tile.uv0.x << ", " << tile.uv0.y << "]"
						 << ", \"uv1\": [" << tile.uv1.x << ", " << tile.uv1.y << "]"
						 << ", \"hasTex\": " << (tile.hasTexture ? "true" : "false") << " }";
					if (x < tileMap_.width() - 1) save << ", ";
				}
				save << " ]";
				if (y < tileMap_.height() - 1) save << ",";
				save << "\n";
			}

			appendIndent(save, 3);
			save << "]\n";

			appendIndent(save, 2);
			save << "}";
			if (i < layers.size() - 1) save << ",";
			save << "\n";
		}

		appendIndent(save, 1);
		save << "]\n";
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
			auto cids = extractChildren(data, "Children", nodeStart, nodeEnd);
			if (cids.has_value())
				children.push_back({ node->sceneObjectID_, cids.value() });
			else
				missingData = true;

			// Listeners
			auto ltnVec = extractListeners(data, "Listeners", nodeStart, nodeEnd);
			if (ltnVec.has_value())
				listeners.push_back({ node->sceneObjectID_, std::move(ltnVec.value()) });

			// Enabled
			any = extract<bool>(data, "Enabled", nodeStart, nodeEnd);
			if (!any.has_value()) {
				std::cerr << "[EnoSceneTree] Warning: Enabled field missing for node, defaulting to false" << std::endl;
			}
			node->enabled_ = any.value_or(false);

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

		// Load tilemap data embedded in scene file
		size_t tilemapPos = data.find("\"TileMap\"");
		if (tilemapPos != std::string::npos) {
			std::cout << "[EnoSceneTree] Loading tilemap data..." << std::endl;

			size_t tilemapStart = data.find('{', tilemapPos);
			if (tilemapStart == std::string::npos) {
				std::cerr << "[EnoSceneTree] Warning: TileMap section malformed, skipping" << std::endl;
			}
			else {
				size_t tilemapEnd = findMatchingBrace(data, tilemapStart);
				if (tilemapEnd == std::string::npos) {
					std::cerr << "[EnoSceneTree] Warning: TileMap section incomplete, skipping" << std::endl;
				}
				else {
					// Extract tilemap dimensions
					auto width = extract<uint32_t>(data, "Width", tilemapStart, tilemapEnd);
					auto height = extract<uint32_t>(data, "Height", tilemapStart, tilemapEnd);

					std::cout << "[EnoSceneTree] Width: " << (width.has_value() ? std::to_string(width.value()) : "missing")
							  << ", Height: " << (height.has_value() ? std::to_string(height.value()) : "missing") << std::endl;

					if (!width.has_value() || !height.has_value()) {
						std::cerr << "[EnoSceneTree] Warning: TileMap dimensions missing, using defaults" << std::endl;
					}
					else if (width.value() == UINT32_MAX || height.value() == UINT32_MAX) {
						std::cerr << "[EnoSceneTree] Warning: TileMap dimensions empty, using defaults" << std::endl;
					}
					else {
						std::cout << "[EnoSceneTree] Loading tilemap with dimensions " << width.value() << "x" << height.value() << std::endl;
						loadTilemapData(data, tilemapStart, tilemapEnd, width.value(), height.value());
						std::cout << "[EnoSceneTree] Tilemap loaded, layers count: " << tileMap_.layers().size() << std::endl;
					}
				}
			}
		}
		else {
			std::cout << "[EnoSceneTree] No tilemap section found, clearing tilemap" << std::endl;
			// Scene doesn't have tilemap data - reset to default empty tilemap
			// Clear all layers
			while (tileMap_.layers().size() > 0) {
				tileMap_.removeLayer(0);
			}

			// Add one default empty layer
			tileMap_.addLayer("Tile Layer 1", false);
		}

	}

	void EnoSceneTree::loadTilemapData(const std::string& data, size_t tilemapStart, size_t tilemapEnd, uint32_t width, uint32_t height) {
		// Extract cell size - need a float version
		size_t cellSizePos = data.find("CellSize", tilemapStart);
		float cellSize = tileMap_.cellSize(); // default
		if (cellSizePos != std::string::npos && cellSizePos < tilemapEnd) {
			size_t cellSizeStart = data.find('{', cellSizePos);
			size_t cellSizeClose = data.find('}', cellSizeStart);
			if (cellSizeStart != std::string::npos && cellSizeClose != std::string::npos) {
				std::string cellSizeStr = trim(data.substr(cellSizeStart + 1, cellSizeClose - cellSizeStart - 1));
				if (!cellSizeStr.empty()) {
					try {
						cellSize = std::stof(cellSizeStr);
					}
					catch (...) {
						std::cerr << "[EnoSceneTree] Warning: Failed to parse cell size, using default" << std::endl;
					}
				}
			}
		}

		// DON'T resize or clear - we'll update existing tilemap structure
		// This preserves the tilemap if scene doesn't have tilemap data
		bool needsResize = (tileMap_.width() != static_cast<int>(width) ||
							tileMap_.height() != static_cast<int>(height) ||
							tileMap_.cellSize() != cellSize);

		if (needsResize) {
			tileMap_.resize(static_cast<int>(width), static_cast<int>(height), cellSize, false);
		}

		// Clear existing layers and reload from scene
		while (tileMap_.layers().size() > 0) {
			tileMap_.removeLayer(0);
		}

		// Find layers array
		size_t layersPos = data.find("\"Layers\"", tilemapStart);
		if (layersPos != std::string::npos && layersPos < tilemapEnd) {
			size_t layersArrayStart = data.find('[', layersPos);
			if (layersArrayStart == std::string::npos) {
				std::cerr << "[EnoSceneTree] Warning: Layers array not found" << std::endl;
				return;
			}

			// Find matching closing bracket for layers array
			size_t layersArrayEnd = layersArrayStart;
			int bracketDepth = 1;
			size_t searchPos = layersArrayStart + 1;
			while (bracketDepth > 0 && searchPos < tilemapEnd) {
				if (data[searchPos] == '[') bracketDepth++;
				else if (data[searchPos] == ']') bracketDepth--;

				if (bracketDepth == 0) {
					layersArrayEnd = searchPos;
					break;
				}
				searchPos++;
			}

			if (layersArrayEnd == layersArrayStart) {
				std::cerr << "[EnoSceneTree] Warning: Layers array incomplete (no matching ])" << std::endl;
				return;
			}

	
			// Parse each layer
			size_t layerPos = data.find('{', layersArrayStart);
			while (layerPos != std::string::npos && layerPos < layersArrayEnd) {
				size_t layerEnd = findMatchingBrace(data, layerPos);
				if (layerEnd == std::string::npos || layerEnd > layersArrayEnd) break;

				// Extract layer properties
				auto layerName = extract<std::string>(data, "Name", layerPos, layerEnd);
				auto isNav = extract<bool>(data, "IsNav", layerPos, layerEnd);

				if (layerName.has_value() && isNav.has_value()) {
					// Add new layer
					tileMap_.addLayer(layerName.value(), isNav.value());
					int layerIdx = static_cast<int>(tileMap_.layers().size()) - 1;

					// Parse tiles
					size_t tilesPos = data.find("\"Tiles\"", layerPos);
					if (tilesPos != std::string::npos && tilesPos < layerEnd) {
						size_t tilesArrayStart = data.find('[', tilesPos);

						// Parse each row
						for (int y = 0; y < static_cast<int>(height); ++y) {
							size_t rowStart = data.find('[', tilesArrayStart + 1);
							if (rowStart == std::string::npos) break;

							// Find matching closing bracket for row array (using depth matching)
							size_t rowEnd = rowStart;
							int bracketDepth = 1;
							size_t searchPos = rowStart + 1;
							while (bracketDepth > 0 && searchPos < data.length()) {
								if (data[searchPos] == '[') bracketDepth++;
								else if (data[searchPos] == ']') bracketDepth--;

								if (bracketDepth == 0) {
									rowEnd = searchPos;
									break;
								}
								searchPos++;
							}
							if (rowEnd == rowStart) break;

							// Move to next row for next iteration
							tilesArrayStart = rowEnd;


							// Parse each tile in the row
							for (int x = 0; x < static_cast<int>(width); ++x) {
								size_t tileStart = data.find('{', rowStart);
								if (tileStart == std::string::npos || tileStart > rowEnd) break;

								size_t tileEnd = data.find('}', tileStart);
								if (tileEnd == std::string::npos) break;

								std::string tileData = data.substr(tileStart, tileEnd - tileStart + 1);

								// Parse tile properties
								bool walkable = tileData.find("\"walk\": true") != std::string::npos;
								bool hasTex = tileData.find("\"hasTex\": true") != std::string::npos;


								tileMap_.setWalkable(layerIdx, x, y, walkable);

								if (hasTex) {
									// Extract texture name
									size_t texPos = tileData.find("\"tex\": \"");
									if (texPos != std::string::npos) {
										size_t texStart = texPos + 8;
										size_t texEnd = tileData.find('\"', texStart);
										std::string texName = tileData.substr(texStart, texEnd - texStart);


										// Extract UV coordinates
										size_t uv0Pos = tileData.find("\"uv0\": [");
										size_t uv1Pos = tileData.find("\"uv1\": [");

										float uv0x = 0, uv0y = 0, uv1x = 1, uv1y = 1;
										if (uv0Pos != std::string::npos) {
											try {
												size_t uv0Start = uv0Pos + 8;
												size_t uv0End = tileData.find(']', uv0Start);
												std::string uv0Str = tileData.substr(uv0Start, uv0End - uv0Start);
												size_t comma = uv0Str.find(',');
												uv0x = std::stof(uv0Str.substr(0, comma));
												uv0y = std::stof(uv0Str.substr(comma + 1));
											}
											catch (...) {}
										}
										if (uv1Pos != std::string::npos) {
											try {
												size_t uv1Start = uv1Pos + 8;
												size_t uv1End = tileData.find(']', uv1Start);
												std::string uv1Str = tileData.substr(uv1Start, uv1End - uv1Start);
												size_t comma = uv1Str.find(',');
												uv1x = std::stof(uv1Str.substr(0, comma));
												uv1y = std::stof(uv1Str.substr(comma + 1));
											}
											catch (...) {}
										}

										tileMap_.setTile(layerIdx, x, y, texName, {uv0x, uv0y}, {uv1x, uv1y});
									}
								}

								rowStart = tileEnd + 1;
							}
						}
					}
				}

				// Find next layer
				layerPos = data.find('{', layerEnd);
			}
		}

		// Ensure we always have at least one layer
		if (tileMap_.layers().empty()) {
			tileMap_.addLayer("Tile Layer 1", false);
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
			while (pos < listeners.size())
			{
				size_t ltnStrt = listeners.find_first_of('[', pos);
				if (ltnStrt == std::string::npos) break;

				size_t ltnEnd = listeners.find_first_of(']', ltnStrt);
				if (ltnEnd == std::string::npos) break;

				std::string str = trim(listeners.substr(ltnStrt + 1, ltnEnd - ltnStrt - 1));

				std::istringstream iss{ str };
				std::string idStr, compStr, sigStr;

				std::getline(iss, idStr, ',');
				std::getline(iss, compStr, ',');
				std::getline(iss, sigStr, ',');

				Listener l;
				l.listenerID_ = static_cast<uint32_t>(std::stoul(trim(idStr)));
				l.component_ = stripPrefix(trim(compStr));
				l.signal_ = trim(sigStr);

				ltnVec.push_back(l);

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
		std::string prefix = "struct ECSComponents::";

		if (str.rfind(prefix, 0) == 0)
			return str.substr(prefix.length());

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
		// Simply use the static counter and increment it
		// Don't reuse IDs
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

	// ====================== Signals ======================

	void EnoSceneTree::signalComponents(Node* node)
	{

	}
}