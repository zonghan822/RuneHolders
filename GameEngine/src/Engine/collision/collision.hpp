/*************************************************
* @file collision.hpp
* @brief Collision detection system for the Eno engine
* @author Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio)
* 
* This file contains the collision detection system including:
*		- collision components (circle, rectangle, point)
*		- collision detection algorithms
*		- collision system for ECS integration
*		- event handling for collision response
* 
* USAGE:
*		1. Register Collision Component in component registry
*		2. Create entities with Transform and Collision componens
*		3. Create CollisionSystem and call Update() each frame
*		4. Subscribe to CollisionEvent for game-specific responses
**************************************************/

#pragma once

// Engine includes
#include "../math/math.hpp"
#include "../component/components/transform.hpp"
#include "../ecs/entity.hpp"
#include "../event/eventBus.hpp"

// standard library includes
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>

namespace Collision
{
	//*****************************************
	// Collision Component Definitions
	//*****************************************

	/* 
	@brief defines the type of collision shape 
	*/
	enum class ColliderType
	{
		CIRCLE = 0,		// circular collision areas (round objects)
		RECTANGLE = 1,	// rectangular collision areas (AABB)
		POINT = 2		// point collision (projectiles, click detection)
	};

	/* @brief component that stores collision information for entities 
		
		- can be attached to any entity to give it collision detection
		- supports different collision shapes and can be configured as either
			a solid collider (blocks movement) or trigger (detects overlap only)
	*/
	struct CollisionComponent
	{
		ColliderType type;			// what shape is the collider?
		Math::eVec2<float> offset;	// offset from entity's transform position
		bool isTrigger;				// is this a trigger (no physics response) or solid collider?
		bool isActive;				// can this collider currently participate in collision detection

		/*
		@brief 'Union' to store different collision shape data efficiently
				only stores the data needed for the current collider type
		*/
		union ColliderData
		{
			struct CircleData {
				float radius;		// radius for circle collider
			} circle;

			struct RectangleData {
				float width;		// width of rectangle collider
				float height;		// height of rectangle collider
			} rectangle;

			// point collider doesn't need additional data beyond its position
		} data;

		/****** CONSTRUCTORS ******/

		/*
		@brief Default constructor - creates a basic circle collider
		*/
		CollisionComponent();

		/*
		@brief create a CIRCLE collider
		@param radius - size of the circular collision area
		@param colliderOffset - offset from the entity's transform position
		@param trigger - if TRUE, this collider won't block movement but will detect overlaps
		*/
		CollisionComponent(
			float radius,
			Math::eVec2<float> colliderOffset = { 0.0f, 0.0f },
			bool trigger = false
		);

		/*
		@brief create a RECTANGLE collider
		@param width - width of the recetangular collision area
		@param height - height of the rectangular collision area
		@param colliderOffset - offset from the entity's transform position
		@param trigger - if TRUE, this collider, won't block movement but will detect overlaps
		*/
		CollisionComponent(
			float width, 
			float height, 
			Math::eVec2<float> colliderOffset = { 0.0f, 0.0f, }, 
			bool trigger = false);

		/*
		@brief create a point collider (e.g., bullets, mouse clicks, etc.)
		@param colliderOffset - offset from the entity's transform position
		@param trigger - if TRUE, this collider won't block movement but will detect overlaps
		*/
		static CollisionComponent CreatePointCollider(
			Math::eVec2<float> colliderOffset = { 0.0f, 0.0f }, 
			bool trigger = false);

		//*****************************************
		// UTILITY functions
		//*****************************************

		/*
		@brief set the radius of a circle collider
		@param radius new radius value
		@note only works if this is a circle collider
		*/
		void SetRadius(float radius);

		/*
		@brief set the size of a rectangle collider
		@param width - new width value
		@param height - new height value
		@note only works if this is a rectangle collider
		*/
		void SetRectangleSize(float width, float height);

		/*
		@brief change the offset of this collider from the entity's positon
		@param newOffset - new offset vector
		*/
		void SetOffset(const Math::eVec2<float>& newOffset);

		/*
		@brief enable or disable this collider
		@param active - IF FALSE, this collider will be ignored during collision detection
		*/
		void SetActive(bool active);

		/*
		@brief set whether this collider is a trigger or solid
		@param trigger - IF TRUE, collider becomes a trigger (no physics reponse)
		*/
		void SetTrigger(bool trigger);

		/*
		@brief Get the area covered by this collider (for debugging??)
		@return Area in square units
		*/
		float GetColliderArea() const;
	};
	
	/*
	@brief Stream operator for easy debugging of collision components
	@param os - output stream
	@param collision - CollisionComponent to print
	@return Reference to the output stream
	*/
	std::ostream& operator<<(std::ostream& os, const CollisionComponent& collision);


	// =============================================================================
	// COLLISION DETECTION
	// =============================================================================

	/*
	@brief result of a collision detection check

	contains all information about a collision that occured, including where it happeened,
	and how to separate the objects.
	*/
	struct CollisionResult
	{
		bool hasCollision;						// did a collision occur?
		Math::eVec2<float> penetrationVector;	// vector to separate objects (points from A to B)
		float penetrationDepth;					// how deep is the penetration
		Math::eVec2<float> contactPoint;		// where the collision occurred in the world space
		
		/*
		@brief default constructor - no collision
		*/
		CollisionResult();
	};

	/*
	@brief static class containing collision detection algorithms
			
			this class handles the math for detecting collisions between different types of shapes!
	*/
	class CollisionDetection
	{
	public:
		/*
		@brief main collision detection function

		automatically determines the correct algorithm based on collider types,
		and performs the collision check

		@param colliderA - first collision component
		@param transformA - transform first entity (for position)
		@param colliderB - second collision component
		@param transformB transform of second entity (for position)
		@return CollisionResult - containing all collision information
		*/
		static CollisionResult CheckCollision(
			const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA,
			const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB);

	private:
		//*****************************************
		// collision detection functions
		//*****************************************	

		/*
		@brief check collision between 2 circles
		@param posA - position of first circle
		@param radiusA - radius of first circle
		@param posB - position of second circle
		@param radiusB - radius of second circle
		@return CollisionResult - with collision details
		*/
		static CollisionResult CheckCircleCircle(
			const Math::eVec2<float>& posA, float radiusA,
			const Math::eVec2<float>& posB, float radiusB);

		/*
		@brief check collision between 2 axis-aligned rectnagles
		@param posA - position of first rectangle center
		@param rectA - size data of first rectangle
		@param posB - position of second rectangle center
		@param rectB - size data of second rectangle
		@return CollisionResult - with collision details
		*/
		static CollisionResult CheckRectangleRectangle(
			const Math::eVec2<float>& posA, const CollisionComponent::ColliderData::RectangleData& rectA,
			const Math::eVec2<float>& posB, const CollisionComponent::ColliderData::RectangleData& rectB);

		/*
		@brief check collision between a circle and a rectangle
		@param circlePos - position of circle center
		@param radius - radius of circle
		@param rectPos - position of rectangle center
		@param rect - Size data of rectangle
		@return CollisionResult - with collision details
		*/
		static CollisionResult CheckCircleRectangle(
			const Math::eVec2<float>& circlePos, float radius,
			const Math::eVec2<float>& rectPos, const CollisionComponent::ColliderData::RectangleData& rect);

		/*
		@brief Check collision between a point and any other collider
		@param pointPos - position of the point
		@param collider - the other collider to check against
		@param colliderPos - position of the other collider
		@return CollisionResult - with collision details
		*/
		static CollisionResult CheckPointCollider(
			const Math::eVec2<float>& pointPos, const CollisionComponent& colldier, const Math::eVec2<float>& colliderPos);
	};

	//======================================================================================
	// COLLISION EVENTS
	//======================================================================================

	/*
	* @brief event sent when a collision occurs
	* 
	* this event is published through the event bus whenver 2 entities collide.
	* 
	* game systems can subscribe to this event to handle collisions
	*/
	struct CollisionEvent
	{
		uint32_t entityA_ID;	// ID of first entity in collision
		uint32_t entityB_ID;	// ID of second entity in collision
		CollisionResult result; // detailed collision information
		bool isEntityA_Trigger;	// is entity A's collider a trigger?
		bool isEntityB_Trigger; // is entity B's collider a trigger?

		/*
		* @brief Default constructor
		*/
		CollisionEvent();
	};

	/*
	* @brief Event sent when an Entity enters a trigger area
	*/
	struct TriggerEnterEvent 
	{
		uint32_t triggerEntityID;			// entity with the trigger collider
		uint32_t otherEntityID;				// entity that entered the trigger
		Math::eVec2<float> contactPoint;	// where the trigger was activated

		TriggerEnterEvent();
	};

	/*
	* @brief Event sent when an entity exits a trigger area
	*/
	struct TriggerExitEvent {
		uint32_t triggerEntityID;			// entity with the trigger colliser
		uint32_t otherEntityID;				// entity that exited the trigger

		TriggerExitEvent();
	};

	//======================================================================================
	// COLLISION SYSTEM
	//======================================================================================
	
	/*
	* @brief Main collision system that integrates with the ECS
	* 
	* This system handles:
	*	- finding all entities with collision components
	*	- checking collisions between all pairs
	*	- publishing collision events
	*	- managing collision callbacks
	* 
	* usage in game loop:
	*	1. create the system: CollisionSystem collisionSystem(eventBus);
	*	2. each frame call: collisionSystem.Update(entityManager);
	*	3. subscribe to CollisionEvent to handle specific collision responses
	*/
	class CollisionSystem
	{
	public:
		/*
		* @brief Constrcutor
		* @param eventBus - reference to the engine's event bus for publishing collision events
		*/
		CollisionSystem(Event::EnoEventBus& eventBus);

		/*
		* @brief Main update function - call this every frame
		* 
		* This function:
		*	1. finds all entities with collision components
		*	2. checks collisions between all pairs
		*	3. publishes events and calls callbacks for any collisions found
		* 
		* @param entityManager - reference to the entity mananger
		*/
		void Update(ECS::EntityManager& entityManager);

		/*
		* @brief Add a callback function for collision events
		* 
		* these callbacks are called immediately when collisions are detected,
		* in addition to the events published to the event bus.
		* 
		* @param callback Function to call when collisions occur
		*/
		void AddCollisionCallback(std::function<void(const CollisionEvent&)> callback);

		/*
		* @brief Remove all collision callbacks
		*/
		void ClearCallbacks();

	private:
		// -------------------------------------------------------------------------
		// PRIVATE MEMBER VARIABLES
		// -------------------------------------------------------------------------

		Event::EnoEventBus& eventBus_;
		// event bus for publishing collision events
		std::vector<std::function<void(const CollisionEvent&)>> collisionCallbacks_; // direct callback functions

		// -------------------------------------------------------------------------
		// PRIVATE HELPER FUNCTIONS
		// -------------------------------------------------------------------------

		/*
		* @brief check if an entity has the required components for collisions
		* @param entityLoc - entity location information
		* @return True if entity has both transform and collision components
		*/
		bool HasRequiredComponents(const ECS::EntityLoc& entityLoc);

		/*
		* @brief Check collision between 2 specific entities
		* @param entityA_ID - ID of first entity
		* @param entityA_Loc - location info of first entity
		* @param entityB_ID - ID of second entity
		* @param entityB_Loc - location info of second entity
		*/
		void CheckEntityCollision(
			uint32_t entityA_ID, const ECS::EntityLoc& entityA_Loc,
			uint32_t entityB_ID, const ECS::EntityLoc& entityB_Loc);

		/*
		* @brief handle a collision that has been deteced
		* @param entityA_ID - ID of first entity
		* @param entityB_ID - ID of second entity
		* @param collisionA - collision component of first entity
		* @param collisionB - collision component of second entity
		* @param result - collision detection result
		*/
		void HandleCollision(
			uint32_t entityA_ID, uint32_t entityB_ID,
			const CollisionComponent& collisionA, const CollisionComponent& collisionB,
			const CollisionResult& result);

		/*
		* @brief get transform component from entity location
		* @param entityLoc - entity location information
		* @return reference to entity's transform component
		*/
		ECSComponents::TransformComponent& GetTransformComponent(const ECS::EntityLoc& entityLoc);

		/*
		* @brief get collision component from entity location
		* @param entityLoc - entity location information
		* @return reference to the entity's collision component
		*/
		CollisionComponent& GetCollisionComponent(const ECS::EntityLoc& entityLoc);
	};

	// -------------------------------------------------------------------------
	// UTILITY FUNCTION
	// -------------------------------------------------------------------------

	/*
	* @brief run a collision test (for debugging)
	* 
	* creates test entities and shows collision detections
	* call this function to verify if the collision system is working
	*/
	void RunBasicCollisionTest();

} // namespace Collision