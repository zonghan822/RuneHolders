/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         component_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   50%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include <typeindex>
 
namespace ECS {

	struct ComponentAdded {
		uint32_t entityID;
		std::type_index type;
	};

	struct ComponentRemoved {
		uint32_t entityID;
		std::type_index type;
	};

	struct EntityDestroyed {
		uint32_t entityID;
	};

}