/*============================================================================================
* @file         eno_scene_tree.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for a scene tree
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
		uint32_t sceneObjectID_;
		uint32_t nestedLayer_;

		// Entity Data
		//ECS::Entity entity_;
	};

	class EnoSceneTree
	{
	public:

		// Create a node
		/*============================================================================================
			* \brief			Create a node on the scene tree
		*//*========================================================================================*/
		void create()
		{
			Node node;
			node.sceneObjectID_ = static_cast<uint32_t>(nodes_.size());
			node.nestedLayer_ = 0;

			nodes_.push_back(node);
		}

		//// Nest a node
		//void nest(Node& node)
		//{
		//}

		//// Select a node and display all information
		//Node select(const uint32_t& sceneID)
		//{
		//}

		//void insert()
		//{

		//}

		//void print()
		//{

		//}

	private:
		std::vector<Node> nodes_;

		Node selectedNode_;
	};
}