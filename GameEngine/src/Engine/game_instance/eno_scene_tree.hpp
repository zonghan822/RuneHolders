#pragma once

#include "../ecs/entity.hpp"

namespace GameInstance
{
	struct Node
	{
		// Scene Data
		uint32_t sceneObjectID_;
		uint32_t nestedLayer_;

		// Entity Data
		//ECS::Entity entity_;
	};

	class EnoSceneTree
	{
	public:

		// Create a node
		void create()
		{
			Node node;
			node.sceneObjectID_ = static_cast<uint32_t>(nodes_.size());
			node.nestedLayer_ = 0;

			nodes_.push_back(node);
		}

		// Nest a node
		void nest(Node& node)
		{
		}

		// Select a node and display all information
		Node select(const uint32_t& sceneID)
		{
		}

		void insert()
		{

		}

		void print()
		{

		}

	private:
		std::vector<Node> nodes_;

		
	};
}