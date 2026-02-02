/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         rune_component.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
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
    //enum class RuneType
    //{
    //    LEFT,   // Left rune relative to mouse
    //    RIGHT   // Right rune relative to mouse
    //};

    constexpr int RUNE_COUNT = 6;

    /*============================================================================================
    * \brief    Represents the 3 different rune colors in the spell system
    *//*========================================================================================*/
    constexpr int RUNE_COLOR_COUNT = 3;

    enum class RuneColor
    {
        RED,    // Fire-based attacks
        BLUE,   // Ice/Water-based attacks
        GREEN   // Nature/Poison-based attacks
    };

    /*============================================================================================
    * \brief    Helper struct for rune asset names and properties
    *//*========================================================================================*/
    struct RuneAssets
    {
        // Get texture name for a rune color (left rune)
        // TODO: Replace with actual texture names when Sabrina/Jeremy provides them
        static const char* getLeftRuneTexture(RuneColor color)
        {
            (void)color;    // Unused for now
            return "Left_Rune"; // Use exisiting texture for all colors
        }

        // Get texture name for a rune color (right rune)
        // TODO: Replace with actual texture names when Sabrina/Jeremy provides them
        static const char* getRightRuneTexture(RuneColor color)
        {
            (void)color;    // Unused for now
            return "Right_Rune";    // Use existing texture for all colors
        }

        // Get spell/projectle texture name for a rune color
        // TODO: Replace with actual texture names when available
        static const char* getSpellTexture(RuneColor color)
        {
            (void)color;    // Unused for now
            return "Fireball";      // Use existing texture for all colors
        }

        // Get display name for a rune color
        static const char* getColorName(RuneColor color)
        {
            switch (color)
            {
                case RuneColor::RED:        return "Fire";
                case RuneColor::BLUE:       return "Ice";
                case RuneColor::GREEN:      return "Poison";
                default:                    return "Unknown";
            }
        }

        // Get sound effect name for casting a spell
        // TODO: Replace with actual sound names when available
        static const char* getCastSound(RuneColor color)
        {
            (void)color;    // Unused for now
            return "Fireball-Cast"; // Use existing sound for all colors
        }
    };

    struct RuneComponent
    {
        // RuneType type;              // Which side rune this is
        int runeIndex;              // The color/element of this rune
        RuneColor color;            // The color/element of this rune
        bool isHovered;             // Is mouse currently over this rune
        bool isConnected;           // Has this rune been connected to its pair
        float radius;               // Collision radius for hover detection

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuneComponent()
            : runeIndex(0)
            , color(RuneColor::RED)
            , isHovered(false)
            , isConnected(false)
            , radius(20.0f)
        {
        }

        RuneComponent(int index, RuneColor runeColor, float runeRadius = 20.0f)
            : runeIndex(index)
            , color(runeColor)
            , isHovered(false)
            , isConnected(false)
            , radius(runeRadius)
        {
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            int colorInt = static_cast<int>(color);
            l("runeIndex", runeIndex, "int");
            l("color", colorInt, "int");
            l("isHovered", isHovered, "bool");
            l("isConnected", isConnected, "bool");
            l("radius", radius, "float");
            color = static_cast<RuneColor>(colorInt);
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            int colorInt = static_cast<int>(color);
            l("runeIndex", runeIndex, "int");
            l("color", colorInt, "int");
            l("isHovered", isHovered, "bool");
            l("isConnected", isConnected, "bool");
            l("radius", radius, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const RuneComponent& rc)
        {
            os << "RuneComponent: "
               << "index=" << rc.runeIndex
               << ", color=" << static_cast<int>(rc.color)
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
    * \brief    Component for spell projectiles - links spell color to damage type
    *//*========================================================================================*/
    struct SpellComponent
    {
        RuneColor spellColor;   // The color/element of this spell
        float baseDamage;       // Base damage amount
        float speed;            // Projectile speed
        float lifetime;         // Time before spell disappears
        float elapsedTime;      // Time since spawn

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> siganls_;

        SpellComponent()
            : spellColor(RuneColor::RED)
            , baseDamage(25.0f)
            , speed(400.0f)
            , lifetime(5.0f)
            , elapsedTime(0.0f)
        {
        }

        SpellComponent(RuneColor color, float damage = 25.0f, float spd = 400.0f, float life = 5.0f)
            : spellColor(color)
            , baseDamage(damage)
            , speed(spd)
            , lifetime(life)
            , elapsedTime(0.0f)
        {
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            int colorInt = static_cast<int>(spellColor);
            l("spellColor", colorInt, "int");
            l("baseDamage", baseDamage, "float");
            l("speed", speed, "float");
            l("lifetime", lifetime, "float");
            l("elapsedTime", elapsedTime, "float");
            spellColor = static_cast<RuneColor>(colorInt);
        }
        
        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            int colorInt = static_cast<int>(spellColor);
            l("spellColor", colorInt, "int");
            l("baseDamage", baseDamage, "float");
            l("speed", speed, "float");
            l("lifetime", lifetime, "float");
            l("elapsedTime", elapsedTime, "float");

        }

        friend std::ostream& operator<<(std::ostream& os, const SpellComponent& sc)
        {
            os << "SpellComponent: "
                << "color=" << static_cast<int>(sc.spellColor)
                << ", baseDamage=" << sc.baseDamage
                << ", speed=" << sc.speed
                << ", lifetime=" << sc.lifetime
                << ", elapsedTime=" << sc.elapsedTime;
            return os;
        }
    };

    /*============================================================================================
    * \brief    Component for floating text that rises and fades (e.g., "Invalid" message)
    *//*========================================================================================*/
    struct FloatingTextComponent
    {
        char text[64];              // The text to display
        float lifetime;             // Total time before disappearing
        float elapsedTime;          // Time since spawn
        float riseSpeed;            // How fast the text rises (pixels per second)
        float initialAlpha;         // Starting alpha value

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        FloatingTextComponent()
            : lifetime(1.5f)
            , elapsedTime(0.0f)
            , riseSpeed(50.0f)
            , initialAlpha(1.0f)
        {
            strcpy(text, "");
        }

        FloatingTextComponent(const char* msg, float life = 1.5f, float speed = 50.0f)
            : lifetime(life)
            , elapsedTime(0.0f)
            , riseSpeed(speed)
            , initialAlpha(1.0f)
        {
            strncpy(text, msg, 63);
            text[63] = '\0';
        }

        // Get current alpha based on elapsed time (fades out)
        float getCurrentAlpha() const
        {
            float progress = elapsedTime / lifetime;
            return initialAlpha * (1.0f - progress);
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("lifetime", lifetime, "float");
            l("elapsedTime", elapsedTime, "float");
            l("riseSpeed", riseSpeed, "float");
            l("initialAlpha", initialAlpha, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("lifetime", lifetime, "float");
            l("elapsedTime", elapsedTime, "float");
            l("riseSpeed", riseSpeed, "float");
            l("initialAlpha", initialAlpha, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const FloatingTextComponent& ft)
        {
            os << "FloatingTextComponent: "
               << "text=" << ft.text
               << ", lifetime=" << ft.lifetime
               << ", elapsedTime=" << ft.elapsedTime;
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

        // Track all 6 runes
        uint32_t runeEntityIDs[RUNE_COUNT]; // IDs of all spawned runes

        // Connection tracking
        int firstConnectedRuneIndex;        // Index of firt rune in connection (-1 if none)
        int secondConnectedRuneIndex;       // Index of second rune in connection (-1 if none)

        float runeCircleRadius;             // Radius of the rune circle pattern

        // uint32_t leftRuneEntityID;      // ID of spawned left rune
        // uint32_t rightRuneEntityID;     // ID of spawned right rune

        //float runeSpawnOffset;          // Distance from mouse to spawn runes

        //// Rune Selection System
        //RuneColor selectedRuneColor;    // Currently selected rune color
        //int selectedRuneIndex;          // Index 0-5 for the 6 runes

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuneSystemStateComponent()
            : isLeftClickHeld(false)
            , runesActive(false)
            , isDrawingLine(false)
            , fireballSpawned(false)
            , lineStart(0.0f, 0.0f)
            , lineEnd(0.0f, 0.0f)
            , firstConnectedRuneIndex(-1)
            , secondConnectedRuneIndex(-1)
            , runeCircleRadius(100.0f)
        {
            for (int i = 0; i < RUNE_COUNT; ++i) {
                runeEntityIDs[i] = UINT32_MAX;
            }
        }

        // Reset connection state
        void resetConnection()
        {
            firstConnectedRuneIndex = -1;
            secondConnectedRuneIndex = -1;
            isDrawingLine = false;
            fireballSpawned = false;
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
            l("runeCircleRadius", runeCircleRadius, "float");
            l("firstConnectedRuneIndex", firstConnectedRuneIndex, "int");
            l("secondConnectedRuneIndex", secondConnectedRuneIndex, "int");
            for (int i = 0; i < RUNE_COUNT; ++i) {
                std::string key = "runeEntityID_" + std::to_string(i);
                l(key.c_str(), runeEntityIDs[i], "uint32_t");
            }
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
            l("runeCircleRadius", runeCircleRadius, "float");
            l("firstConnectedRuneIndex", firstConnectedRuneIndex, "int");
            l("secondConnectedRuneIndex", secondConnectedRuneIndex, "int");
            for (int i = 0; i < RUNE_COUNT; ++i) {
                std::string key = "runeEntityID_" + std::to_string(i);
                l(key.c_str(), runeEntityIDs[i], "uint32_t");
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const RuneSystemStateComponent& rs)
        {
            os << "RuneSystemState: "
                << "leftClickHeld=" << rs.isLeftClickHeld
                << ", runesActive=" << rs.runesActive
                << ", isDrawingLine=" << rs.isDrawingLine
                << ", firstConnected=" << rs.firstConnectedRuneIndex
                << ", secondConnected=" << rs.secondConnectedRuneIndex;
            return os;
        }
    };
}
