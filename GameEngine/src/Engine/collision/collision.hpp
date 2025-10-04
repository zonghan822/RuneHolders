/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         collision.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-10-2025
* @brief        This file contains the collision detection system including collision
*               components (circle, rectangle, line, point), collision detection algorithms,
*               collision system for ECS integration, and event handling for collision response.
*//*========================================================================================*/
#pragma once

// Engine includes
#include "../math/math.hpp"
#include "../component/components/transform.hpp"
#include "../ecs/entity.hpp"
#include "../event/event_bus.hpp"

// Standard library includes
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>

namespace Collision
{
	/*============================================================================================
	 * \brief       Defines the type of collision shape for a collider
	*//*========================================================================================*/	
	enum class ColliderType
	{
		CIRCLE = 0,		// Circular collision areas (round objects)
		RECTANGLE = 1,	// Rectangular collision areas (AABB)
		LINE = 2,		// Line segment
		POINT = 3		// Point collision (projectiles, click detection)
	};

	/*============================================================================================
	 * \brief       Component that stores collision information for entities. Can be attached to
	 *              any entity to give it collision detection. Supports different collision shapes
	 *              and can be configured as either a solid collider (blocks movement) or trigger
	 *              (detects overlap only).
	*//*========================================================================================*/	
	struct CollisionComponent
	{
		ColliderType type;			// What shape is the collider?
		Math::eVec2<float> offset;	// Offset from entity's transform position
		bool isTrigger;				// Is this a trigger (no physics response) or solid collider?
		bool isActive;				// Can this collider currently participate in collision detection

		/*============================================================================================
		* \brief       Union to store different collision shape data efficiently. Only stores the
		*              data needed for the current collider type.
		*//*========================================================================================*/		
		union ColliderData
		{
			struct CircleData {
				float radius;		// Radius for circle collider
			} circle;

			struct RectangleData {
				float width;		// Width of rectangle collider
				float height;		// Height of rectangle collider
			} rectangle;

			struct LineData {
				Math::eVec2<float> p1;			// First point of line collider
				Math::eVec2<float> p2;			// Second point of line collider
				Math::eVec2<float> pNormal;		// Normalized outward normal of line collider
			} line;
			// Point collider doesn't need additional data beyond its position
		} data;

		// CONSTRUCTORS

		/*============================================================================================
		* \brief       Default constructor - creates a basic circle collider
		*//*========================================================================================*/		
		CollisionComponent();

		/*============================================================================================
		* \brief       Create a CIRCLE collider
		* \param[in]   float radius - Size of the circular collision area
		* \param[in]   Math::eVec2<float> colliderOffset - Offset from the entity's transform position
		* \param[in]   bool trigger - If TRUE, this collider won't block movement but will detect overlaps
		*//*========================================================================================*/		
		CollisionComponent(
			float radius,
			Math::eVec2<float> colliderOffset = { 0.0f, 0.0f },
			bool trigger = false
		);

		/*============================================================================================
		* \brief       Create a RECTANGLE collider
		* \param[in]   float width - Width of the rectangular collision area
		* \param[in]   float height - Height of the rectangular collision area
		* \param[in]   Math::eVec2<float> colliderOffset - Offset from the entity's transform position
		* \param[in]   bool trigger - If TRUE, this collider won't block movement but will detect overlaps
		*//*========================================================================================*/		
		CollisionComponent(
			float width, 
			float height, 
			Math::eVec2<float> colliderOffset = { 0.0f, 0.0f, }, 
			bool trigger = false);

		/*============================================================================================
		* \brief       Create a LINE collider
		* \param[in]   Math::eVec2<float> point1 - Start point of line
		* \param[in]   Math::eVec2<float> point2 - End point of line
		* \param[in]   bool trigger - If TRUE, this collider won't block movement but will detect overlaps
		*//*========================================================================================*/		
		CollisionComponent(
			Math::eVec2<float> point1,
			Math::eVec2<float> point2,
			bool trigger = false);

		/*============================================================================================
		* \brief       Create a point collider (e.g., bullets, mouse clicks, etc.)
		* \param[in]   Math::eVec2<float> colliderOffset - Offset from the entity's transform position
		* \param[in]   bool trigger - If TRUE, this collider won't block movement but will detect overlaps
		* \return      CollisionComponent - Point collider instance
		*//*========================================================================================*/		
		static CollisionComponent CreatePointCollider(
			Math::eVec2<float> colliderOffset = { 0.0f, 0.0f }, 
			bool trigger = false);

		// UTILITY FUNCTIONS

		/*============================================================================================
		* \brief       Set the radius of a circle collider
		* \param[in]   float radius - New radius value
		* \return      void
		* \note        Only works if this is a circle collider
		*//*========================================================================================*/		
		void SetRadius(float radius);

		/*============================================================================================
		* \brief       Set the size of a rectangle collider
		* \param[in]   float width - New width value
		* \param[in]   float height - New height value
		* \return      void
		* \note        Only works if this is a rectangle collider
		*//*========================================================================================*/		
		void SetRectangleSize(float width, float height);

		/*============================================================================================
		* \brief       Change the offset of this collider from the entity's position
		* \param[in]   Math::eVec2<float> const& newOffset - New offset vector
		* \return      void
		*//*========================================================================================*/		
		void SetOffset(const Math::eVec2<float>& newOffset);

		/*============================================================================================
		* \brief       Enable or disable this collider
		* \param[in]   bool active - If FALSE, this collider will be ignored during collision detection
		* \return      void
		*//*========================================================================================*/		
		void SetActive(bool active);

		/*============================================================================================
		* \brief       Set whether this collider is a trigger or solid
		* \param[in]   bool trigger - If TRUE, collider becomes a trigger (no physics response)
		* \return      void
		*//*========================================================================================*/		
		void SetTrigger(bool trigger);

		/*============================================================================================
		* \brief       Get the area covered by this collider
		* \return      float - Area in square units
		*//*========================================================================================*/		
		float GetColliderArea() const;
	};
	
	/*============================================================================================
	* \brief       Stream operator for easy debugging of collision components
	* \param[in]   std::ostream& os - Output stream
	* \param[in]   CollisionComponent const& collision - CollisionComponent to print
	* \return      std::ostream& - Reference to the output stream
	*//*========================================================================================*/	
	std::ostream& operator<<(std::ostream& os, const CollisionComponent& collision);


	// COLLISION DETECTION

	/*============================================================================================
	* \brief       Result of a collision detection check. Contains all information about a
	*              collision that occurred, including where it happened and how to separate
	*              the objects.
	*//*========================================================================================*/	
	struct CollisionResult
	{
		bool hasCollision;						// Did a collision occur?
		Math::eVec2<float> penetrationVector;	// Vector to separate objects (points from A to B)
		float penetrationDepth;					// How deep is the penetration
		Math::eVec2<float> contactPoint;		// Where the collision occurred in the world space
		
		/*============================================================================================
		* \brief       Default constructor - no collision
		*//*========================================================================================*/		
		CollisionResult();
	};

	/*============================================================================================
	* \brief       Static class containing collision detection algorithms. This class handles
	*              the math for detecting collisions between different types of shapes.
	*//*========================================================================================*/
	class CollisionDetection
	{
	public:
		/*============================================================================================
		* \brief       Main collision detection function. Automatically determines the correct
		*              algorithm based on collider types and performs the collision check.
		* \param[in]   CollisionComponent const& colliderA - First collision component
		* \param[in]   ECSComponents::TransformComponent const& transformA - Transform of first entity (for position)
		* \param[in]   CollisionComponent const& colliderB - Second collision component
		* \param[in]   ECSComponents::TransformComponent const& transformB - Transform of second entity (for position)
		* \return      CollisionResult - Containing all collision information
		*//*========================================================================================*/		
		static CollisionResult CheckCollision(
			const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA,
			const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB);

		/*============================================================================================
		* \brief       Dynamic collision detection function. Automatically determines the correct
		*              algorithm based on collider types, performs the collision check and projects
		*              a reflected vector if any.
		* \param[in]   CollisionComponent const& colliderA - First collision component
		* \param[in]   ECSComponents::TransformComponent const& transformA - Transform of first entity (for position)
		* \param[in]   CollisionComponent const& colliderB - Second collision component
		* \param[in]   ECSComponents::TransformComponent const& transformB - Transform of second entity (for position)
		* \param[in]   Math::eVec2<float> const& endPos - Next position of object
		* \param[out]  Math::eVec2<float>& colPoint - Point of collision
		* \param[out]  Math::eVec2<float>& normalAtCol - Normalized outward normal at collision point
		* \param[out]  float& colTi - Time till collision
		* \return      bool - Returns true if collision occurs
		//*========================================================================================*/		
		static bool CheckDynamicCollision(
			const CollisionComponent& colliderA, const ECSComponents::TransformComponent& transformA,
			const CollisionComponent& colliderB, const ECSComponents::TransformComponent& transformB,
			const Math::eVec2<float>& endPos, Math::eVec2<float>& colPoint, Math::eVec2<float>& normalAtCol, float& colTi);

		/*============================================================================================
		* \brief       Calculate collision reflection for physics response
		* \param[in]   Math::eVec2<float> const& interPt - Intersection point
		* \param[in]   Math::eVec2<float> const& normal - Surface normal at collision
		* \param[out]  Math::eVec2<float>& nextPos - Modified to reflected position
		* \param[out]  Math::eVec2<float>& reflected - Output reflection direction (normalized)
		* \return      void
		*//*========================================================================================*/
		static void CollisionReflection(const Math::eVec2<float>& interPt,
			const Math::eVec2<float>& normal,
			Math::eVec2<float>& nextPos,
			Math::eVec2<float>& reflected);

	private:
		/*============================================================================================
		// Collision detection functions
		/*============================================================================================

		/*============================================================================================
		 * \brief       Check collision between two circles
		 * \param[in]   Math::eVec2<float> const& posA - Position of first circle
		 * \param[in]   float radiusA - Radius of first circle
		 * \param[in]   Math::eVec2<float> const& posB - Position of second circle
		 * \param[in]   float radiusB - Radius of second circle
		 * \return      CollisionResult - Contains collision details
		/*===========================================================================================*/
		static CollisionResult CheckCircleCircle(
			const Math::eVec2<float>& posA, float radiusA,
			const Math::eVec2<float>& posB, float radiusB);

		/*============================================================================================
		 * \brief       Check collision between two axis-aligned rectangles
		 * \param[in]   Math::eVec2<float> const& posA - Position of first rectangle center
		 * \param[in]   CollisionComponent::ColliderData::RectangleData const& rectA - Size data of first rectangle
		 * \param[in]   Math::eVec2<float> const& posB - Position of second rectangle center
		 * \param[in]   CollisionComponent::ColliderData::RectangleData const& rectB - Size data of second rectangle
		 * \return      CollisionResult - Contains collision details
		*//*========================================================================================*/		
		static CollisionResult CheckRectangleRectangle(
			const Math::eVec2<float>& posA, const CollisionComponent::ColliderData::RectangleData& rectA,
			const Math::eVec2<float>& posB, const CollisionComponent::ColliderData::RectangleData& rectB);

		/*============================================================================================
		 * \brief       Check collision between a circle and a rectangle
		 * \param[in]   Math::eVec2<float> const& circlePos - Position of circle center
		 * \param[in]   float radius - Radius of circle
		 * \param[in]   Math::eVec2<float> const& rectPos - Position of rectangle center
		 * \param[in]   CollisionComponent::ColliderData::RectangleData const& rect - Size data of rectangle
		 * \return      CollisionResult - Contains collision details
		*//*========================================================================================*/		
		static CollisionResult CheckCircleRectangle(
			const Math::eVec2<float>& circlePos, float radius,
			const Math::eVec2<float>& rectPos, const CollisionComponent::ColliderData::RectangleData& rect);

		/*============================================================================================
		* \brief       Check collision between a point and any other collider
		* \param[in]   Math::eVec2<float> const& pointPos - Position of the point
		* \param[in]   CollisionComponent const& collider - The other collider to check against
		* \param[in]   Math::eVec2<float> const& colliderPos - Position of the other collider
		* \return      CollisionResult - Contains collision details
		*//*========================================================================================*/		
		static CollisionResult CheckPointCollider(
			const Math::eVec2<float>& pointPos, const CollisionComponent& colldier, const Math::eVec2<float>& colliderPos);

		/*============================================================================================
		* \brief       Check collision between a moving circle and a line segment
		* \param[in]   Math::eVec2<float> const& circlePos - Current position of circle center
		* \param[in]   float const& radius - Radius of circle
		* \param[in]   Math::eVec2<float> const& endPoint - Target end position of circle
		* \param[in]   CollisionComponent const& lineCol - Line collider to check against
		* \param[out]  Math::eVec2<float>& pointAtCol - Point of collision in world space
		* \param[out]  Math::eVec2<float>& normalAtCol - Normalized outward normal at collision point
		* \param[out]  float& colTi - Time of intersection (0.0 to 1.0)
		* \return      bool - Returns true if collision occurs
		*//*========================================================================================*/		
		static bool CheckCircleLine(
			const Math::eVec2<float>& circlePos, const float& radius, const Math::eVec2<float>& endPoint, const CollisionComponent& lineCol,
			Math::eVec2<float>& pointAtCol, Math::eVec2<float>& normalAtCol, float& colTi);

		/*============================================================================================
		* \brief       Check collision between two moving circles
		* \param[in]   Math::eVec2<float> const& posA - Position of first circle
		* \param[in]   Math::eVec2<float> const& velA - Velocity of first circle
		* \param[in]   float radiusA - Radius of first circle
		* \param[in]   Math::eVec2<float> const& posB - Position of second circle
		* \param[in]   Math::eVec2<float> const& velB - Velocity of second circle
		* \param[in]   float radiusB - Radius of second circle
		* \param[out]  Math::eVec2<float>& colPoint - Collision point in world space
		* \param[out]  Math::eVec2<float>& normalAtCol - Normal at collision point
		* \param[out]  float& colTi - Time of intersection (0.0 to 1.0)
		* \return      bool - Returns true if collision occurs
		*//*========================================================================================*/		
		static bool CheckDynamicCircleCircle(
			const Math::eVec2<float>& posA, const Math::eVec2<float>& velA, float radiusA,
			const Math::eVec2<float>& posB, const Math::eVec2<float>& velB, float radiusB,
			Math::eVec2<float>& colPoint, Math::eVec2<float>& normalAtCol, float& colTi);
		

	};

	// COLLISION EVENTS

	/*============================================================================================
	* \brief       Event sent when a collision occurs between two entities
	*//*========================================================================================*/
	struct CollisionEvent
	{
		uint32_t entityA_ID;	// ID of first entity in collision
		uint32_t entityB_ID;	// ID of second entity in collision
		CollisionResult result; // Detailed collision information
		bool isEntityA_Trigger;	// Is entity A's collider a trigger?
		bool isEntityB_Trigger; // Is entity B's collider a trigger?

		// Default constructor
		CollisionEvent();
	};

	
	// Event sent when an Entity enters a trigger area
	struct TriggerEnterEvent 
	{
		uint32_t triggerEntityID;			// Entity with the trigger collider
		uint32_t otherEntityID;				// Entity that entered the trigger
		Math::eVec2<float> contactPoint;	// Where the trigger was activated

		TriggerEnterEvent();
	};

	
	// Event sent when an entity exits a trigger area

	struct TriggerExitEvent {
		uint32_t triggerEntityID;			// Entity with the trigger colliser
		uint32_t otherEntityID;				// Entity that exited the trigger

		TriggerExitEvent();
	};

	// COLLISION SYSTEMS
	
	/*============================================================================================
	 * \brief       Main collision system that integrates with the ECS architecture. Handles
	 *              finding entities with collision components, checking collisions between all
	 *              pairs, publishing collision events through the event bus, and managing
	 *              collision callbacks for game-specific responses.
	*//*========================================================================================*/	
	class CollisionSystem
	{
	public:
		/*============================================================================================
		* \brief       Constructor for CollisionSystem
		* \param[in]   Event::EnoEventBus& event_bus - Reference to the engine's event bus for publishing collision events
		*//*========================================================================================*/		
		CollisionSystem(Event::EnoEventBus& event_bus);

		/*============================================================================================
		* \brief       Main update function for collision system - call every frame
		* \param[in]   ECS::EntityManager& entityManager - Reference to the entity manager
		* \return      void
		*//*========================================================================================*/
		void Update(ECS::EntityManager& entityManager);

		/*============================================================================================
		* \brief       Add a callback function for collision events
		* \param[in]   std::function<void(const CollisionEvent&)> callback - Function to call when collisions occur
		* \return      void
		*//*========================================================================================*/		
		void AddCollisionCallback(std::function<void(const CollisionEvent&)> callback);

		/*============================================================================================
		* \brief       Visualize all active colliders in the scene for debugging
		* \param[in]   Graphics::EnoGraphicsManager& graphics - Reference to graphics manager for drawing
		* \param[in]   ECS::EntityManager& entityManager - Reference to entity manager
		* \return      void
		*//*========================================================================================*/		
		void VisualizeColliders(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager);

		/*============================================================================================
		* \brief       Remove all collision callbacks
		* \return      void
		*//*========================================================================================*/
		void ClearCallbacks();

	private:
		// Event bus for publishing collision events
		Event::EnoEventBus& eventBus_;
		
		// Event bus for publishing collision events
		std::vector<std::function<void(const CollisionEvent&)>> collisionCallbacks_; 

		/*============================================================================================
		* \brief       Check if an entity has the required components for collision detection
		* \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
		* \return      bool - True if entity has both transform and collision components
		*//*========================================================================================*/		
		bool HasRequiredComponents(const ECS::EntityLoc& entityLoc);

		/*============================================================================================
		* \brief       Check collision between two specific entities
		* \param[in]   uint32_t entityA_ID - ID of first entity
		* \param[in]   ECS::EntityLoc const& entityA_Loc - Location info of first entity
		* \param[in]   uint32_t entityB_ID - ID of second entity
		* \param[in]   ECS::EntityLoc const& entityB_Loc - Location info of second entity
		* \return      void
		*//*========================================================================================*/
		void CheckEntityCollision(
			uint32_t entityA_ID, const ECS::EntityLoc& entityA_Loc,
			uint32_t entityB_ID, const ECS::EntityLoc& entityB_Loc);

		/*============================================================================================
		* \brief       Handle a collision that has been detected
		* \param[in]   uint32_t entityA_ID - ID of first entity
		* \param[in]   uint32_t entityB_ID - ID of second entity
		* \param[in]   CollisionComponent const& collisionA - Collision component of first entity
		* \param[in]   CollisionComponent const& collisionB - Collision component of second entity
		* \param[in]   CollisionResult const& result - Collision detection result
		* \return      void
		*//*========================================================================================*/		
		void HandleCollision(
			uint32_t entityA_ID, uint32_t entityB_ID,
			const CollisionComponent& collisionA, const CollisionComponent& collisionB,
			const CollisionResult& result);

		/*============================================================================================
		* \brief       Get transform component from entity location
		* \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
		* \return      ECSComponents::TransformComponent& - Reference to the entity's transform component
		*//*========================================================================================*/		
		ECSComponents::TransformComponent& GetTransformComponent(const ECS::EntityLoc& entityLoc);

		/*============================================================================================
		* \brief       Get collision component from entity location
		* \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
		* \return      CollisionComponent& - Reference to the entity's collision component
		*//*========================================================================================*/		
		CollisionComponent& GetCollisionComponent(const ECS::EntityLoc& entityLoc);
	};

	// UTILITY FUNCTIONS

	/*============================================================================================
	* \brief       Run a collision test for debugging. Creates test entities and shows collision
	*              detections. Call this function to verify if the collision system is working.
	* \return      void
	*//*========================================================================================*/	
	void RunBasicCollisionTest();

} // namespace Collision