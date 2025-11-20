#pragma once

// Engine Component(s)
#include "../ecs/entity.hpp"

#include "../scripting/eno_script.hpp"
#include "../eno_core_host/eno_core_host.hpp"


// stds
#include <sstream>
#include <set>

namespace Editor
{
	struct Node
	{
		// Scene Data
		std::string name_;
		uint32_t sceneObjectID_		{ 0 };
		uint32_t nestedLayer_		{ 0 };

		// Entity Data
		ECS::Entity entity_;

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

		enum BraceType { Open, Close };

		EnoSceneTree(ECS::EntityManager&, Core::EnoCoreHost&);
		EnoSceneTree(const EnoSceneTree&) = delete;
		EnoSceneTree(EnoSceneTree&&) = delete;

		// Get all nodes
		std::vector<Node*> getAllNodes();

		// Create a node
		Node* create(Node* parent = nullptr);

		// Create a node from an existing entity (for ImGui integration)
		Node* createFromEntity(ECS::Entity entity, Node* parent = nullptr);

		// Get a node by entity ID
		Node* getNodeByEntityID(uint32_t entityID);

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

		// Serialize/Deserialize scene tree data
		void save(const std::string&, const std::string& sceneName);
		void load(const std::string&);

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

	private:
		inline static uint32_t id_ = 0;

		ECS::EntityManager& entMng_;
		Core::EnoCoreHost& coreHost_;

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

		// Helper for getting children node ids
		void getChildrenID(Node*, std::vector<uint32_t>&);

		// Helpers for data serialization
		void appendIndent(std::ofstream&, const uint32_t&);
		size_t findMatchingBrace(const std::string&, const size_t&);
		std::string trim(const std::string&);

		std::string extractFname(const std::string&);

		void setParent(const std::vector<std::pair<uint32_t, uint32_t>>&);
		void setChildren(const std::vector<std::pair<uint32_t, std::vector<uint32_t>>>&);

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
				catch(...)
				{
					return std::nullopt;
				}
			}

			std::cout << "No " << param << " found!" << std::endl;
			return std::nullopt;
		}

		// Find the specified parameter in a file (ID, Layer, Parent)
		// "ERROR" indicates either brace encasing the value cannot be found
		// std::nullopt means that param being looked for does not exist
		template<>
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

		// Find the specified parameter in a file (Children)
		std::optional<std::vector<uint32_t>> extractCont(
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
	};
}