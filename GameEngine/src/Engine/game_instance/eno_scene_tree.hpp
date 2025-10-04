/*============================================================================================
* @file         eno_scene_tree.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for a scene tree
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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

	private:
		std::vector<Node> nodes_;
	};
}