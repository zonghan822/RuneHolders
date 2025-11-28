#pragma once

// Engine Component(s)
#include "../ecs/entity.hpp"

#include "../scripting/eno_script.hpp"
#include "../eno_core_host/eno_core_host.hpp"
#include "../tilemap/tilemap.hpp"


// stds
#include <sstream>
#include <set>

namespace Editor
{
	struct Listener
	{
		uint32_t listenerID_;
		std::string component_;
		std::string signal_;
	};

	inline std::ostream& operator<<(std::ostream& os, const Listener& l)
	{
		return os << l.listenerID_ << ", " << l.component_ << ", " << l.signal_;
	}

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

		std::vector<Listener> listeners_;

		// Enabled
		bool enabled_{ false };
	};

	class EnoSceneTree
	{
	public:

		enum BraceType { Open, Close };

		EnoSceneTree(ECS::EntityManager&, Core::EnoCoreHost&, Map::TileMapSystem&);
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
		void detachScript(Node* node);
		void detachScript();

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

		// ====================== Signals ======================

		// Expose components and their respective signals
		void signalComponents(Node*);

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
				emitter->listeners_.push_back(
					Listener{
						listener->sceneObjectID_,
						std::string{ typeid(Component).name() },
						signalName });

				// Connect the listener to the emitter
				component.signals[signalName].connect([this, signalName, funcClassPair](const std::vector<std::any>& params)
					{
						coreHost_.signalEmitted(signalName, funcClassPair.first, funcClassPair.second);
					});
			}
		}

	private:
		inline static uint32_t id_ = 0;

		ECS::EntityManager& entMng_;
		Core::EnoCoreHost& coreHost_;
		Map::TileMapSystem& tileMap_;

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
		std::string stripPrefix(const std::string&);
		std::string extractFname(const std::string&);

		// Helper for tilemap loading
		void loadTilemapData(const std::string& data, size_t tilemapStart, size_t tilemapEnd, uint32_t width, uint32_t height);

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
		std::optional<std::vector<Listener>> extractListeners(
			const std::string& data,
			const std::string& param,
			const size_t& start,
			const size_t& end)
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
	};
}