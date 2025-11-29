/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         rune_component.hpp
* @author       [Your Name]
* @date         2025-11-29
* @brief        Component for rune entities in the rune-based spell system
*//*========================================================================================*/

#pragma once

#include "../../math/math.hpp"
#include "../../event/signals.hpp"
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <string>

// Forward declarations
namespace ECS {
    class Entity;
}

namespace ECSComponents
{
    /*============================================================================================
    * \brief    Represents a rune entity that can be spawned and connected
    *//*========================================================================================*/
    enum class RuneType
    {
        LEFT,   // Left rune relative to mouse
        RIGHT   // Right rune relative to mouse
    };

    struct RuneComponent
    {
        RuneType type;              // Which side rune this is
        bool isHovered;             // Is mouse currently over this rune
        bool isConnected;           // Has this rune been connected to its pair
        float radius;               // Collision radius for hover detection

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuneComponent()
            : type(RuneType::LEFT)
            , isHovered(false)
            , isConnected(false)
            , radius(20.0f)
        {
        }

        RuneComponent(RuneType runeType, float runeRadius = 20.0f)
            : type(runeType)
            , isHovered(false)
            , isConnected(false)
            , radius(runeRadius)
        {
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            int typeInt = static_cast<int>(type);
            l("type", typeInt, "int");
            l("isHovered", isHovered, "bool");
            l("isConnected", isConnected, "bool");
            l("radius", radius, "float");
            type = static_cast<RuneType>(typeInt);
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            int typeInt = static_cast<int>(type);
            l("type", typeInt, "int");
            l("isHovered", isHovered, "bool");
            l("isConnected", isConnected, "bool");
            l("radius", radius, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const RuneComponent& rc)
        {
            os << "RuneComponent: "
               << "type=" << static_cast<int>(rc.type)
               << ", isHovered=" << rc.isHovered
               << ", isConnected=" << rc.isConnected
               << ", radius=" << rc.radius;
            return os;
        }
    };

    /*============================================================================================
    * \brief    Component for fireball projectiles with lifetime tracking
    *//*========================================================================================*/
    struct FireballComponent
    {
        float lifetime;          // Time in seconds before fireball disappears
        float elapsedTime;       // Time since spawn

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        FireballComponent()
            : lifetime(5.0f)
            , elapsedTime(0.0f)
        {
        }

        FireballComponent(float lifeSeconds)
            : lifetime(lifeSeconds)
            , elapsedTime(0.0f)
        {
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("lifetime", lifetime, "float");
            l("elapsedTime", elapsedTime, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("lifetime", lifetime, "float");
            l("elapsedTime", elapsedTime, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const FireballComponent& fc)
        {
            os << "FireballComponent: "
               << "lifetime=" << fc.lifetime
               << ", elapsedTime=" << fc.elapsedTime;
            return os;
        }
    };

    /*============================================================================================
    * \brief    Component to track the state of the rune spawning system
    *           Attach this to a dedicated manager entity or the player
    *//*========================================================================================*/
    struct RuneSystemStateComponent
    {
        bool isLeftClickHeld;           // Is left mouse button held down
        bool runesActive;               // Are runes currently spawned
        bool isDrawingLine;             // Is player drawing a connection line
        bool fireballSpawned;           // Has fireball been spawned this cycle

        Math::eVec2<float> lineStart;   // Starting position of line (when hovering left rune)
        Math::eVec2<float> lineEnd;     // End position of line (follows mouse)

        uint32_t leftRuneEntityID;      // ID of spawned left rune
        uint32_t rightRuneEntityID;     // ID of spawned right rune

        float runeSpawnOffset;          // Distance from mouse to spawn runes

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuneSystemStateComponent()
            : isLeftClickHeld(false)
            , runesActive(false)
            , isDrawingLine(false)
            , fireballSpawned(false)
            , lineStart(0.0f, 0.0f)
            , lineEnd(0.0f, 0.0f)
            , leftRuneEntityID(UINT32_MAX)
            , rightRuneEntityID(UINT32_MAX)
            , runeSpawnOffset(100.0f)  // Increased from 50.0f for wider separation
        {
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("isLeftClickHeld", isLeftClickHeld, "bool");
            l("runesActive", runesActive, "bool");
            l("isDrawingLine", isDrawingLine, "bool");
            l("fireballSpawned", fireballSpawned, "bool");
            l("lineStartX", lineStart.x, "float");
            l("lineStartY", lineStart.y, "float");
            l("lineEndX", lineEnd.x, "float");
            l("lineEndY", lineEnd.y, "float");
            l("leftRuneEntityID", leftRuneEntityID, "uint32_t");
            l("rightRuneEntityID", rightRuneEntityID, "uint32_t");
            l("runeSpawnOffset", runeSpawnOffset, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("isLeftClickHeld", isLeftClickHeld, "bool");
            l("runesActive", runesActive, "bool");
            l("isDrawingLine", isDrawingLine, "bool");
            l("fireballSpawned", fireballSpawned, "bool");
            l("lineStartX", lineStart.x, "float");
            l("lineStartY", lineStart.y, "float");
            l("lineEndX", lineEnd.x, "float");
            l("lineEndY", lineEnd.y, "float");
            l("leftRuneEntityID", leftRuneEntityID, "uint32_t");
            l("rightRuneEntityID", rightRuneEntityID, "uint32_t");
            l("runeSpawnOffset", runeSpawnOffset, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const RuneSystemStateComponent& rs)
        {
            os << "RuneSystemState: "
               << "leftClickHeld=" << rs.isLeftClickHeld
               << ", runesActive=" << rs.runesActive
               << ", isDrawingLine=" << rs.isDrawingLine;
            return os;
        }
    };
}
