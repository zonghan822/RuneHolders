/*============================================================================================
* @file         collision_component.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio)
* @date         02-11-2025
* @brief        Collision component for ECS. Stores collision shape data including type,
*               dimensions, offset, and trigger status for collision detection system.
*===========================================================================================*/

#pragma once

#include "../../math/math.hpp"

namespace ECSComponents
{
    /*============================================================================================
     * \brief       Defines the type of collision shape for a collider
     *===========================================================================================*/
    enum class ColliderType
    {
        CIRCLE = 0,     // Circular collision areas (round objects)
        RECTANGLE = 1,  // Rectangular collision areas (AABB)
        LINE = 2,       // Line segment
        POINT = 3       // Point collision (projectiles, click detection)
    };

    /*============================================================================================
     * \brief       Component that stores collision information for entities. Supports different
     *              collision shapes and can be configured as either a solid collider (blocks
     *              movement) or trigger (detects overlap only)
     *===========================================================================================*/
    struct CollisionComponent
    {
        ColliderType type;              // What shape is the collider?
        Math::eVec2<float> offset;      // Offset from entity's transform position
        bool isTrigger;                 // Is this a trigger (no physics response) or solid collider?
        bool isActive;                  // Can this collider currently participate in collision detection
        float rotation;                 // Rotation angle in radians

        /*============================================================================================
         * \brief       Union to store different collision shape data efficiently. Only stores the
         *              data needed for the current collider type.
         *===========================================================================================*/
        union ColliderData
        {
            struct CircleData {
                float radius;           // Radius for circle collider
            } circle;

            struct RectangleData {
                float width;            // Width of rectangle collider
                float height;           // Height of rectangle collider
            } rectangle;

            struct LineData {
                Math::eVec2<float> p1;          // First point of line collider
                Math::eVec2<float> p2;          // Second point of line collider
                Math::eVec2<float> pNormal;     // Normalized outward normal of line collider
            } line;
            // Point collider doesn't need additional data beyond its position
        } data;

        /*============================================================================================
         * \brief       Default constructor - creates a basic circle collider
         *===========================================================================================*/
        CollisionComponent()
            : type(ColliderType::CIRCLE)
            , offset{ 0.0f, 0.0f }
            , isTrigger(false)
            , isActive(true)
            , rotation(0.0f)
        {
            data.circle.radius = 10.0f;
        }

        /*============================================================================================
         * \brief       Create a CIRCLE collider
         * \param[in]   float radius - Size of the circular collision area
         * \param[in]   Math::eVec2<float> const& colliderOffset - Offset from entity's transform
         * \param[in]   bool trigger - If TRUE, won't block movement but will detect overlaps
         *===========================================================================================*/
        CollisionComponent(
            float radius,
            const Math::eVec2<float>& colliderOffset = { 0.0f, 0.0f },
            bool trigger = false
        )
            : type(ColliderType::CIRCLE)
            , offset(colliderOffset)
            , isTrigger(trigger)
            , isActive(true)
            , rotation(0.0f)
        {
            data.circle.radius = radius;
        }

        /*============================================================================================
         * \brief       Create a RECTANGLE collider
         * \param[in]   float width - Width of the rectangular collision area
         * \param[in]   float height - Height of the rectangular collision area
         * \param[in]   Math::eVec2<float> const& colliderOffset - Offset from entity's transform
         * \param[in]   bool trigger - If TRUE, won't block movement but will detect overlaps
         *===========================================================================================*/
        CollisionComponent(
            float width,
            float height,
            const Math::eVec2<float>& colliderOffset = { 0.0f, 0.0f },
            bool trigger = false
        )
            : type(ColliderType::RECTANGLE)
            , offset(colliderOffset)
            , isTrigger(trigger)
            , isActive(true)
            , rotation(0.0f)
        {
            data.rectangle.width = width;
            data.rectangle.height = height;
        }

        /*============================================================================================
         * \brief       Create a LINE collider
         * \param[in]   Math::eVec2<float> const& point1 - Start point of line
         * \param[in]   Math::eVec2<float> const& point2 - End point of line
         * \param[in]   bool trigger - If TRUE, won't block movement but will detect overlaps
         *===========================================================================================*/
        CollisionComponent(
            const Math::eVec2<float>& point1,
            const Math::eVec2<float>& point2,
            bool trigger = false
        )
            : type(ColliderType::LINE)
            , offset{ 0.0f, 0.0f }
            , isTrigger(trigger)
            , isActive(true)
            , rotation(0.0f)
        {
            data.line.p1 = point1;
            data.line.p2 = point2;

            // Calculate normal
            Math::eVec2<float> lineVec = point2 - point1;
            data.line.pNormal = Math::eVec2<float>{ -lineVec.y, lineVec.x };
            float length = sqrt(data.line.pNormal.x * data.line.pNormal.x +
                data.line.pNormal.y * data.line.pNormal.y);
            if (length > 0.0f) {
                data.line.pNormal.x /= length;
                data.line.pNormal.y /= length;
            }
        }

        /*============================================================================================
         * \brief       Create a point collider (e.g., bullets, mouse clicks, etc.)
         * \param[in]   Math::eVec2<float> const& colliderOffset - Offset from entity's transform
         * \param[in]   bool trigger - If TRUE, won't block movement but will detect overlaps
         * \return      CollisionComponent - Point collider instance
         *===========================================================================================*/
        static CollisionComponent CreatePointCollider(
            const Math::eVec2<float>& colliderOffset = { 0.0f, 0.0f },
            bool trigger = false
        )
        {
            CollisionComponent comp;
            comp.type = ColliderType::POINT;
            comp.offset = colliderOffset;
            comp.isTrigger = trigger;
            comp.isActive = true;
            comp.rotation = 0.0f;
            return comp;
        }


        // For serialization purposes
        // Generalized way of using reflection to access and iterate
        // over fields!
        // 
        // The idea is to (in a function call) create a lambda that will be a "visitor",
        // and extract all information as parameters of that lambda. Each call over a
        // field of the component is used to extract metadata on 1 field [name, value, type]
        // 
        // Source: https://stackoverflow.com/questions/41453/how-can-i-add-reflection-to-a-c-application
        // Additional learning: https://refactoring.guru/design-patterns/visitor
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            switch (type)
            {
            case ColliderType::CIRCLE:
                l("Type", type, "ColliderType");
                break;

            case ColliderType::LINE:
                l("Type", type, "ColliderType");
                break;

            case ColliderType::POINT:
                l("Type", type, "ColliderType");
                break;

            case ColliderType::RECTANGLE:
                l("Type", type, "ColliderType");
                break;

            default:
                throw std::runtime_error("Unreflected type specified!");
            }

            l("Offset",             offset,                 "eVec2(float)");
            l("Is Trigger",         isTrigger,              "bool");
            l("Is Active",          isActive,               "bool");
            l("Rotation",           rotation,               "float");

            switch (type)
            {
            case ColliderType::CIRCLE:
                l("Radius",         data.circle.radius,     "float");
                break;

            case ColliderType::RECTANGLE:
                l("Width",          data.rectangle.width,   "float");
                l("Height",         data.rectangle.height,  "float");
                break;

            case ColliderType::LINE:
                l("p1",             data.line.p1,           "eVec2(float)");
                l("p2",             data.line.p2,           "eVec2(float)");
                l("pNormal",        data.line.pNormal,      "eVec2(float)");
                break;

            case ColliderType::POINT:
                break;

            default:
                throw std::runtime_error("Unreflected collider type specified!");
            }


        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            switch (type)
            {
            case ColliderType::CIRCLE:
                l("Type", type, "ColliderType::Circle");
                break;

            case ColliderType::LINE:
                l("Type", type, "ColliderType::Line");
                break;

            case ColliderType::POINT:
                l("Type", type, "ColliderType::Point");
                break;

            case ColliderType::RECTANGLE:
                l("Type", type, "ColliderType::Rectangle");
                break;

            default:
                throw std::runtime_error("Unreflected collider type specified!");
            }

            l("Offset",             offset,            "eVec2(float)");
            l("Is Trigger",         isTrigger,         "bool");
            l("Is Active",          isActive,          "bool");
            l("Rotation",           rotation,          "float");
        }

    };

    inline std::ostream& operator<<(std::ostream& os, const ColliderType& c)
    {
        switch (c)
        {
        case ColliderType::CIRCLE: return os << "Circle";
        case ColliderType::LINE: return os << "Line";
        case ColliderType::POINT: return os << "Point";
        case ColliderType::RECTANGLE: return os << "Rectangle";

        default:    return os << "Invalid collider type passed";
        }
    }
    
    /*============================================================================================
    * \brief       Auto-calculate AABB collision bounds from RenderComponent dimensions
    * \param[in]   RenderComponent const& render - Rendering component with width/height
    * \param[in]   Math::eVec2<float> const& offset - Optional offset from entity center
    * \param[in]   bool isTrigger - Whether this should be a trigger collider
    * \return      CollisionComponent - Rectangle collider matching render dimensions
    *===========================================================================================*/
    inline CollisionComponent CreateAABBFromRender(
        const struct RenderComponent& render,
        const Math::eVec2<float>& offset = { 0.0f, 0.0f },
        bool isTrigger = false
    );

    /*============================================================================================
    * \brief       Auto-calculate scaled AABB collision bounds from RenderComponent + Transform
    * \param[in]   RenderComponent const& render - Rendering component with width/height
    * \param[in]   TransformComponent const& transform - Transform with scale values
    * \param[in]   Math::eVec2<float> const& offset - Optional offset from entity center
    * \param[in]   bool isTrigger - Whether this should be a trigger collider
    * \return      CollisionComponent - Rectangle collider matching scaled render dimensions
    *===========================================================================================*/
    inline CollisionComponent CreateScaledAABBFromRender(
        const struct RenderComponent& render,
        const struct TransformComponent& transform,
        const Math::eVec2<float>& offset = { 0.0f, 0.0f},
        bool isTrigger = false
    );

    /*============================================================================================
    * \brief       Auto-calculate circle collision bounds from RenderComponent dimensions
    * \param[in]   RenderComponent const& render - Rendering component with width/height
    * \param[in]   Math::eVec2<float> const& offset - Optional offset from entity center
    * \param[in]   bool isTrigger - Whether this should be a trigger collider
    * \return      CollisionComponent - Circle collider with radius = width/2
    * \note        Uses width to determine radius (radius =width * 0.5f)
    *===========================================================================================*/
     inline CollisionComponent CreateCircleFromRender(
     const struct RenderComponent& render,
     const Math::eVec2<float>&offset = { 0.0f, 0.0f },
     bool isTrigger = false
     );

     /*============================================================================================
     * \brief       Auto-calculate scaled circle collision bounds from RenderComponent + Transform
     * \param[in]   RenderComponent const& render - Renderingcomponent with width/height
     * \param[in]   TransformComponent const& transform - Transform with scale values
     * \param[in]   Math::eVec2<float> const& offset - Optional offset from entity center
     * \param[in]   bool isTrigger - Whether this should be a trigger collider
     * \return      CollisionComponent - Circle collider with scaled radius
     * \note        Uses width * scale.x to determine radius (radius = (width * scale.x) * 0.5f)
     *===========================================================================================*/
     inline CollisionComponent CreateScaledCircleFromRender(
        const struct RenderComponent& render,
        const struct TransformComponent& transform,
        const Math::eVec2<float>&offset = { 0.0f, 0.0f },
        bool isTrigger = false
        );
    
} // namespace ECSComponents (close namespace temporarily)

#include "drawInfo.hpp"
#include "transform.hpp"

namespace ECSComponents
{
    // Implementations

    inline CollisionComponent CreateAABBFromRender(
        const RenderComponent& render,
        const Math::eVec2<float>& offset,
        bool isTrigger
    )
    {
        return CollisionComponent(
            render.width,
            render.height,
            offset,
            isTrigger
        );
    }

    inline CollisionComponent CreateScaledAABBFromRender(
        const RenderComponent& render,
        const TransformComponent& transform,
        const Math::eVec2<float>& offset,
        bool isTrigger
    )
    {
        float scaledWidth = render.width * transform.scale.x;
        float scaledHeight = render.height * transform.scale.y;

        return CollisionComponent(
            scaledWidth,
            scaledHeight,
            offset,
            isTrigger
        );
    }

    inline CollisionComponent CreateCircleFromRender(
        const RenderComponent& render,
        const Math::eVec2<float>& offset,
        bool isTrigger
    ) {
        // Calculate radius from width (radius = width/2)
        float radius = render.width * 0.5f;

        return CollisionComponent(
            radius, // Circle constructor takes raidus as first param
            offset,
            isTrigger
        );
    }

    inline CollisionComponent CreateScaledCircleFromRender(
        const RenderComponent& render,
        const TransformComponent& transform,
        const Math::eVec2<float>& offset,
        bool isTrigger
    ) {
        // Calculate scaled radius from width (radius = (width * scale)/2)
        float scaledWidth = render.width * transform.scale.x;
        float radius = scaledWidth * 0.5f;

        return CollisionComponent(
            radius, // Circle constructor takes radius as first param
            offset,
            isTrigger
        );
    }

} // namespace ECSComponents