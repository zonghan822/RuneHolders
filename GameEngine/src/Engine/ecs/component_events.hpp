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