#pragma once
 
namespace ECS {

	class Entity;

	struct ComponentAdded {
		Entity entity;
		std::type_index type;
	};

	struct ComponentRemoved {
		Entity entity;
		std::type_index type;
	};

	struct EntityDestroyed {
		Entity entity;
	};

}