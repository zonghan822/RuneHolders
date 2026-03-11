/*============================================================================================
* @file         eno_scene_tree.hpp
* @author       Zhe An (zhean.l@digipen.edu)
* @date         30-11-2025
* @brief        Scene tree to keep track of nodes and heirarchiel entity relationships
*//*========================================================================================*/

#pragma once

// Engine Component(s)
#include "../ecs/entity.hpp"

#include "../scripting/eno_script.hpp"
#include "../eno_core_host/eno_core_host.hpp"
#include "../tilemap/tilemap.hpp"
#include "../component/components/buttons.hpp"
#include "../component/components/visibility.hpp"

// stds
#include <sstream>
#include <set>
#include <type_traits>

namespace Editor
{
	struct Listener
	{
		uint32_t listenerID_		{ 0 };
		std::string component_;
		std::string signal_;
	};

#define PRESET(name, ...) name,
	enum class NodeType : uint32_t
	{
		EMPTY,
		CUSTOM,
		PRESET_LIST
	};
#undef PRESET

	struct Node
	{
		// Scene Data
		std::string name_;
		uint32_t sceneObjectID_		{ 0 };
		uint32_t nestedLayer_		{ 0 };

		NodeType type_;

		// Non-empty when this node was created from a user prefab.
		// Matches a name in EnoPrefabManager::userPrefabData_.
		std::string prefabName_;

		// Entity Data
		ECS::Entity entity_;

		// Attached script
		std::unique_ptr<Script::EnoScript> script_;

		// Relationships
		Node* parent_{ nullptr };
		std::vector<Node*> children_;

		std::vector<Listener> listeners_;

		// Enabled
		bool enabled_{ false };
	};

	class EnoSceneTree
	{
	public:

		enum BraceType { Open, Close };

		EnoSceneTree(ECS::EntityManager&, Core::EnoCoreHost&, Map::TileMapSystem&, ECSComponents::EnoPrefabManager&);
		EnoSceneTree(const EnoSceneTree&) = delete;
		EnoSceneTree(EnoSceneTree&&) = delete;

		// Get all nodes
		std::vector<Node*> getAllNodes();

		// Create a node
		Node* create();
		Node* create(Node* parent);
		Node* create(NodeType);

		// Create a node from an existing entity (for ImGui integration)
		Node* createFromEntity(ECS::Entity entity, Node* parent = nullptr);

		// Find node by name
		Node* getNodeByName(const std::string&);

		// Get a node by entity ID
		Node* getNodeByEntityID(uint32_t entityID);

		// Get a node by scene object ID
		Node* getNodeBySceneObjectID(uint32_t sceneObjID);

		// Get child node by index
		Node* getChild(Node*, uint32_t idx);

		// Modify entity data by preset
		void modNodeType(Node*, NodeType);

		// Nest a node
		void nest(Node*);
		void nest(Node*, Node*);
		void nest(uint32_t, uint32_t);
		
		// Detach a node from its parent
		void unparent(Node* node);
		void unparent(uint32_t);

		// Attach a node to a new parent
		void reparent(Node*, Node* dst = nullptr);
		void reparent(uint32_t, uint32_t dst = 0);

		// Select a node and display all information
		void select(const uint32_t&);

		// Detach a node from its parent
		void detach(Node*, Node* dst = nullptr);

		// Delete a node and all of its children
		void kill();
		void kill(Node*);
		void kill(uint32_t);

		// Toggle the use of a node
		void toggleEnable();
		void toggleEnable(Node*);
		void toggleEnable(const uint32_t&);

		// Attach script
		void attachScript(Node*, std::unique_ptr<Script::EnoScript>&&);
		void attachScript(std::unique_ptr<Script::EnoScript>&&);

		// Detach script
		void detachScript(Node* node);
		void detachScript();

		// Re-register all scripts (call after scripting system is initialized)
		void registerAllScripts();

		// Serialize/Deserialize scene tree data
		void save(const std::string&);
		void load(const std::string&);

		bool hasTilemap() const { return hasTilemap_; }
		void setHasTilemap(bool v) { hasTilemap_ = v; }

		template<typename T>
		std::optional<T> extract(
			const std::string&, const std::string&, 
			const size_t&, const size_t&);

		// View node information
		void nodeView(const Node* node);

		// View selected node information
		void selectedNodeView();

		// View tree structure
		void treeView();

		// ====================== Signals ======================

		// Conect a signal from a node to another
		template <typename Component>
		void connect(Node* emitter, Node* listener, std::string signalName)
		{
			// Verify that the emitter has the specified component && a script attached
			if (emitter->entity_.has<Component>(entMng_) && listener->script_)
			{
				auto& component = emitter->entity_.get<Component>(entMng_);
				std::string emitName = emitter->name_;

				// Write the signal (receiving) function into the listener's script
				std::pair<std::string, std::string> funcClassPair = listener->script_->signalWrite(emitName, signalName);

				// Record listeners on the emitter for serialization process
				// Note: listenerID_ stores sceneObjectID_ for serialization purposes
				emitter->listeners_.push_back(
					Listener{
						listener->sceneObjectID_,
						std::string{ typeid(Component).name() },
						signalName });

				// Connect the listener to the emitter using the generated method name
				component.signals_[signalName].connect(
					[this, funcClassPair, listenerID = listener->sceneObjectID_](const std::vector<std::any>& params)
					{
						coreHost_.signalEmitted(listenerID, funcClassPair.first, funcClassPair.second);
					});
			}
		}

	private:
		inline static uint32_t id_ = 0;

		ECS::EntityManager& entMng_;
		Core::EnoCoreHost& coreHost_;
		Map::TileMapSystem& tileMap_;
		ECSComponents::EnoPrefabManager& prefabMng_;

		std::unordered_map<uint32_t, std::unique_ptr<Node>> tree_;
		Node* selectedNode_{ nullptr };
		bool hasTilemap_{ false };

		std::vector<uint32_t> salvagedID_;

		// Button input operation states
		bool creating_{ false };

		// Generate an id for a node
		uint32_t generateID();

		// Helper for printing Node information
		void display(const Node*);

		// Helper for updating nested layer
		void updateNestedLayer(Node*, const uint32_t&);

		// Helper for getting children node ids
		void getChildrenID(Node*, std::vector<uint32_t>&);

		// Helpers for data serialization
		void appendIndent(std::ofstream&, const uint32_t&);
		size_t findMatchingBrace(const std::string&, const size_t&);
		std::string trim(const std::string&);
		std::string stripPrefix(const std::string&);
		std::string extractFname(const std::string&);

		void setParent(const std::vector<std::pair<uint32_t, uint32_t>>&);
		void setChildren(const std::vector<std::pair<uint32_t, std::vector<uint32_t>>>&);
		void setListeners(std::vector<std::pair<uint32_t, std::vector<Listener>>>&);


		// ================== Extract Specializations ==================

		// Find the specified parameter in a file (ID, Layer, Parent)
		// UINT32_MAX indicates that either brace encasing the value cannot be found or 
		// a value cannot be found within both braces
		// std::nullopt indicates that param being looked for does not exist
		template<>
		std::optional<uint32_t> extract<uint32_t>(
			const std::string& data, const std::string& param,
			const size_t& start, const size_t& end)
		{
			size_t paramPos = data.find(param, start);
			if (paramPos != std::string::npos && paramPos < end)
			{
				size_t paramStrt = data.find_first_of('{', paramPos);
				size_t paramEnd = data.find_first_of('}', paramStrt);
				if (paramStrt == std::string::npos || paramEnd == std::string::npos)
					return std::nullopt;

				std::string str{ trim(data.substr(paramStrt + 1, paramEnd - paramStrt - 1)) };
				if (str.empty())
					return UINT32_MAX;

				try
				{
					return static_cast<uint32_t>(std::stoul(str));
				}
				catch (...)
				{
					return std::nullopt;
				}
			}

			std::cout << "No " << param << " found!" << std::endl;
			return std::nullopt;
		}

		// Find the specified parameter in a file (Enabled)
		template<>
		std::optional<bool> extract<bool>(
			const std::string& data, const std::string& param,
			const size_t& start, const size_t& end)
		{
			size_t paramPos = data.find(param, start);
			if (paramPos != std::string::npos && paramPos < end)
			{
				size_t paramStrt = data.find_first_of('{', paramPos);
				size_t paramEnd = data.find_first_of('}', paramStrt);
				if (paramStrt == std::string::npos || paramEnd == std::string::npos)
					return std::nullopt;

				// Ensure that bool value in file will always be consistent 
				// by converting to lowercase
				std::string str = trim(data.substr(paramStrt + 1, paramEnd - paramStrt - 1));
				if (str.empty())
					return std::nullopt;

				std::transform(std::begin(str), std::end(str), std::begin(str), ::tolower);

				return (str == "true") ? true : false;
			}

			std::cout << "No " << param << " found!" << std::endl;
			return std::nullopt;
		}

		// Find the specified parameter in a file (ID, Layer, Parent)
		// "ERROR" indicates either brace encasing the value cannot be found
		// std::nullopt means that param being looked for does not exist
		template <>
		std::optional<std::string> extract<std::string>(
			const std::string& data, const std::string& param,
			const size_t& start, const size_t& end)
		{
			size_t paramPos = data.find(param, start);
			if (paramPos != std::string::npos && paramPos < end)
			{
				size_t paramStrt = data.find_first_of('{', paramPos);
				size_t paramEnd = data.find_first_of('}', paramStrt);
				if (paramStrt == std::string::npos || paramEnd == std::string::npos)
					return "ERROR";

				return trim(data.substr(paramStrt + 1, paramEnd - paramStrt - 1));
			}

			std::cout << "No " << param << " found!" << std::endl;
			return std::nullopt;
		}

		// Finding the specified parameter in a file (Children)
		std::optional<std::vector<uint32_t>> extractChildren(
			const std::string&, const std::string&,
			const size_t&, const size_t&);

		// Specialization for finding the specified parameter in a file (Listener)
		std::optional<std::vector<Listener>> extractListeners(
			const std::string&, const std::string&,
			const size_t&, const size_t&);
	};

	inline std::ostream& operator<<(std::ostream& os, const Listener& l)
	{
		return os << l.listenerID_ << ", " << l.component_ << ", " << l.signal_;
	}
}