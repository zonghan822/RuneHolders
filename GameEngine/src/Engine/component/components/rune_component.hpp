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

    enum class RuneVisualState
    {
        UNAVAILABLE,        // Grey Small
        AVAILABLE_START,    // White Large - valid starting rune
        SELECTED,           // Green Large
        NEXT_VALID,         // White Large - next valid in sequence
        INVALID             // Red Small - invalid selection feedback
    };

    // Line color states for connection lines between runes
    enum class LineColorState
    {
        SELECTING,          // Yellow - while drawing
        SUCCESS,            // Green - valid spell complete
        INVALID             // Red - invalid combo
    };

    /*============================================================================================
    * \brief    Legacy color enum for combat system compatibility
    *//*========================================================================================*/
    enum class RuneColor
    {
        RED,
        BLUE,
        GREEN
    };

    /*============================================================================================
    * \brief    Helper struct for rune asset names and properties
    *//*========================================================================================*/
    struct RuneAssets
    {
        // Rune spritesheet: Runes_White_Sheet_Large.png
        static constexpr int RUNE_FRAME_WIDTH = 56;
        static constexpr int RUNE_FRAME_HEIGHT = 56;
        static constexpr int RUNE_SHEET_COLUMNS = 6;

        // Maps position index (0-5 clockwise from top-left) to tune number (1-6)
        static constexpr int positionToRuneNumber[6] = { 6, 1, 2, 3, 4, 5 };

        static void getRuneUV(int runeNumber, Math::eVec2<float>& uvStart, Math::eVec2<float>& uvEnd)
        {
            int column = runeNumber - 1;  // Rune 1 = column 0, etc.
            float uMin = column / 6.0f;
            float uMax = (column + 1) / 6.0f;
            uvStart = { uMin, 0.0f };
            uvEnd = { uMax, 1.0f };
        }
    };

    struct RuneComponent
    {
        int runeNumber;              // Rune number 1-6 (matches spritesheet column + 1)
        int positionIndex;           // Position in circle (0-5, clockwise from top-left)
        bool isHovered;              // Is mouse currently over this rune
        bool isConnected;            // Has this rune been connected to its pair
        bool isBanned;               // Is this rune banned (by crystal fly promixity)
        float radius;                // Collision radius for hover detection
        RuneVisualState visualState; // Current visual state for guidance

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuneComponent()
            : runeNumber(1)
            , positionIndex(0)
            , isHovered(false)
            , isConnected(false)
            , isBanned(false)
            , radius(20.0f)
            , visualState(RuneVisualState::UNAVAILABLE)
        {
        }

        RuneComponent(int posIdx, int runeNum, float runeRadius = 20.0f)
            : runeNumber(runeNum)
            , positionIndex(posIdx)
            , isHovered(false)
            , isConnected(false)
            , isBanned(false)
            , radius(runeRadius)
            , visualState(RuneVisualState::UNAVAILABLE)
        {
        }        
        
        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("runeNumber", runeNumber, "int");
            l("positionIndex", positionIndex, "int");
            l("isHovered", isHovered, "bool");
            l("isConnected", isConnected, "bool");
            l("isBanned", isBanned, "bool");
            l("radius", radius, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("runeNumber", runeNumber, "int");
            l("positionIndex", positionIndex, "int");
            l("isHovered", isHovered, "bool");
            l("isConnected", isConnected, "bool");
            l("isBanned", isBanned, "bool");
            l("radius", radius, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const RuneComponent& rc)
        {
            os << "RuneComponent: runeNumber= " << rc.runeNumber
               << ", positionIndex=" << rc.positionIndex
               << ", isHovered=" << rc.isHovered
               << ", isConnected=" << rc.isConnected
               << ", isBanned" << rc.isBanned;
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
    * \brief    Spell types that can be cast
    *//*========================================================================================*/
    enum class SpellType
    {
        NONE,
        ICE,    // Blue spell: 3-2-5-4, damages snail
        FIRE,   // Red spell: 6-1-5-2, damages crystal fly
        LIGHT   // Yellow spell: 4-1-3-6, damages bat
    };

    /*============================================================================================
    * \brief    Animation states for RuptureMark spells
    *//*========================================================================================*/
    enum class RuptureMarkState
    {
        SPAWNING,       // Initial spawn animation
        IDLE,           // Waiting for enemy to trigger
        WINDUP,         // Enemy triggered, charging up
        EXPLODING,      // Dealing damage
        FINISHED        // Ready to be destroyed
    };

    /*============================================================================================
    * \brief    Component for RuptureMark trap-style spells
    *//*========================================================================================*/
    struct RuptureMarkComponent
    {
        SpellType spellType;    
        RuptureMarkState state;     // Current animation state
        float stateTimer;           // Time in current state
        float detectionRadius;      // Radius to detect enemies
        float damage;               // Damage dealt on explosion
        uint32_t triggeredByEntityID;   // ID of enemy that triggered this mark

        // Animation timing
        float spawnDuration;
        float windupDuration;
        float explosionDuration;

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuptureMarkComponent()
            : spellType(SpellType::ICE)
            , state(RuptureMarkState::SPAWNING)
            , stateTimer(0.0f)
            , detectionRadius(50.0f)
            , damage(50.0f)
            , triggeredByEntityID(UINT32_MAX)
            , spawnDuration(0.8f)
            , windupDuration(0.5f)
            , explosionDuration(0.4f)
        {
        }

        RuptureMarkComponent(SpellType type, float dmg = 50.0f, float radius = 50.0f)
            : spellType(type)
            , state(RuptureMarkState::SPAWNING)
            , stateTimer(0.0f)
            , detectionRadius(radius)
            , damage(dmg)
            , triggeredByEntityID(UINT32_MAX)
            , spawnDuration(0.8f)
            , windupDuration(0.5f)
            , explosionDuration(0.4f)
        {
        }

        // Get the enemy type name this spell can damage
        // Returns name to match in entity name OR texture path (e.g., "Slug" matches "Enemy-Slug-Idle-Sheet")
        const char* getTargetEnemyName() const
        {
            switch (spellType)
            {
            case SpellType::ICE:   return "Trailblight";
            case SpellType::FIRE:  return "NullFly";
            case SpellType::LIGHT: return "Bloodwing";
            default:               return "";
            }
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            int typeInt = static_cast<int>(spellType);
            int stateInt = static_cast<int>(state);
            l("spellType", typeInt, "int");
            l("state", stateInt, "int");
            l("stateTimer", stateTimer, "float");
            l("detectionRadius", detectionRadius, "float");
            l("damage", damage, "float");
            spellType = static_cast<SpellType>(typeInt);
            state = static_cast<RuptureMarkState>(stateInt);
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            int typeInt = static_cast<int>(spellType);
            int stateInt = static_cast<int>(state);
            l("spellType", typeInt, "int");
            l("state", stateInt, "int");
            l("stateTimer", stateTimer, "float");
            l("detectionRadius", detectionRadius, "float");
            l("damage", damage, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const RuptureMarkComponent& rm)
        {
            os << "RuptureMarkComponent: "
                << "spellType=" << static_cast<int>(rm.spellType)
                << ", state=" << static_cast<int>(rm.state)
                << ", damage=" << rm.damage;
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
        static constexpr int SPELL_SEQUENCE_LENGTH = 4;
        static constexpr float INVALID_STATE_DURATION = 1.0f;   // Duration of red flash
        static constexpr float SUCCESS_STATE_DURATION = 0.5f;   // Duration of green flash

        bool isLeftClickHeld;           // Is left mouse button held down
        bool runesActive;               // Are runes currently spawned
        bool isDrawingLine;             // Is player drawing a connection line
        bool spellCasted;               // Has a spell been cast this cycle

        // Invalid state tracking
        bool isInvalidState;            // Are we in the invalid (red flash) state?
        float invalidStateTimer;        // Timer for invalid state duration

        // Success state tracking
        bool isSuccessState;            // Are we in the success (green flash) state?
        float successStateTimer;        // Timer for success state duration

        LineColorState lineColorState;  // Current color state for connection lines

        // Track all 6 runes
        uint32_t runeEntityIDs[RUNE_COUNT]; // IDs of all spawned runes

        // Connection sequence tracking (stores rune NUMBERS 1-6, not indices)
        int connectedRuneSequence[SPELL_SEQUENCE_LENGTH];
        int connectionCount;            // How many runes connected so far (0-4)

        // Line drawing - store positions of connected runes for multi-segment line
        Math::eVec2<float> connectedPositions[SPELL_SEQUENCE_LENGTH];
        Math::eVec2<float> lineEnd;     // Current mouse position (end of line)

        // Rune visual states
        float runeSizeLarge;            // Size for available/selected runes (default 56)
        float runeSizeSmall;            // Size for unavailable/invalid runes (default 40)
        float lineWidth;                // Connection line thickness (default 8)

        float runeCircleRadius;         // Radius of the rune circle pattern
        Math::eVec2<float> circleCenter; // Center of the rune circle (for spell spawn)

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        RuneSystemStateComponent()
            : isLeftClickHeld(false)
            , runesActive(false)
            , isDrawingLine(false)
            , spellCasted(false)
            , isInvalidState(false)
            , invalidStateTimer(0.0f)
            , isSuccessState(false)
            , successStateTimer(0.0f)
            , lineColorState(LineColorState::SELECTING)
            , connectionCount(0)
            , lineEnd(0.0f, 0.0f)
            , runeCircleRadius(100.0f)
            , runeSizeLarge(56.0f)
            , runeSizeSmall(40.0f)
            , lineWidth(8.0f)
            , circleCenter(0.0f, 0.0f)
        {
            for (int i = 0; i < RUNE_COUNT; ++i) {
                runeEntityIDs[i] = UINT32_MAX;
            }
            for (int i = 0; i < SPELL_SEQUENCE_LENGTH; ++i) {
                connectedRuneSequence[i] = -1;
                connectedPositions[i] = {0.0f, 0.0f};
            }
        }

        // Reset connection state
        void resetConnection()
        {
            connectionCount = 0;
            isDrawingLine = false;
            spellCasted = false;
            isInvalidState = false;
            invalidStateTimer = 0.0f;
            isSuccessState = false;
            successStateTimer = 0.0f;
            lineColorState = LineColorState::SELECTING;
            for (int i = 0; i < SPELL_SEQUENCE_LENGTH; ++i) {
                connectedRuneSequence[i] = -1;
                connectedPositions[i] = {0.0f, 0.0f};
            }
        }

        // Check if a rune number is a valid next selection
        bool isValidNextRune(int runeNumber) const
        {
            int validMask = getValidNextRunes();
            return (validMask & (1 << runeNumber)) != 0;
        }

        // Add a rune to the connection sequence
        bool addRuneToSequence(int runeNumber, const Math::eVec2<float>& position)
        {
            if (connectionCount >= SPELL_SEQUENCE_LENGTH) return false;

            // Check if this rune is already in the sequence
            for (int i = 0; i < connectionCount; ++i) {
                if (connectedRuneSequence[i] == runeNumber) return false;
            }

            connectedRuneSequence[connectionCount] = runeNumber;
            connectedPositions[connectionCount] = position;
            connectionCount++;
            return true;
        }

        // Check if current sequence matches a spell pattern
        SpellType checkSpellPattern() const
        {
            if (connectionCount != SPELL_SEQUENCE_LENGTH) return SpellType::NONE;

            // Spell patterns (rune numbers 1-6)
            constexpr int ICE_PATTERN[4] = {3, 2, 5, 4};
            constexpr int FIRE_PATTERN[4] = {6, 1, 5, 2};
            constexpr int LIGHT_PATTERN[4] = {4, 1, 3, 6};

            bool matchesIce = true;
            bool matchesFire = true;
            bool matchesLight = true;

            for (int i = 0; i < SPELL_SEQUENCE_LENGTH; ++i) {
                if (connectedRuneSequence[i] != ICE_PATTERN[i]) matchesIce = false;
                if (connectedRuneSequence[i] != FIRE_PATTERN[i]) matchesFire = false;
                if (connectedRuneSequence[i] != LIGHT_PATTERN[i]) matchesLight = false;
            }

            if (matchesIce) return SpellType::ICE;
            if (matchesFire) return SpellType::FIRE;
            if (matchesLight) return SpellType::LIGHT;
            return SpellType::NONE;
        }

        // Get valid next runes based on current sequence
        // Returns bitmask where bit N = rune N is valid
        int getValidNextRunes() const
        {
            // Spell patterns
            constexpr int ICE_PATTERN[4] = { 3, 2, 5, 4 };
            constexpr int FIRE_PATTERN[4] = { 6, 1, 5, 2 };
            constexpr int LIGHT_PATTERN[4] = { 4, 1, 3, 6 };

            int validMask = 0;

            // If no runes selected, return starting runes (3, 4 ,6)
            if (connectionCount == 0)
            {
                validMask |= (1 << 3);  // Rune 3 (Ice start)
                validMask |= (1 << 4);  // Rune 4 (Light start)
                validMask |= (1 << 6);  // Rune 6 (Fire start)
                return validMask;
            }

            // Check which patterns still match
            bool iceValid = true, fireValid = true, lightValid = true;
            for (int i = 0; i < connectionCount; ++i)
            {
                if (connectedRuneSequence[i] != ICE_PATTERN[i]) iceValid = false;
                if (connectedRuneSequence[i] != FIRE_PATTERN[i]) fireValid = false;
                if (connectedRuneSequence[i] != LIGHT_PATTERN[i]) lightValid = false;
            }

            // Add next rune from each still-valid pattern
            if (connectionCount < 4)
            {
                if (iceValid) validMask |= (1 << ICE_PATTERN[connectionCount]);
                if (fireValid) validMask |= (1 << FIRE_PATTERN[connectionCount]);
                if (lightValid) validMask |= (1 << LIGHT_PATTERN[connectionCount]);
            }

            return validMask;
        }

        // Serialization support
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("isLeftClickHeld", isLeftClickHeld, "bool");
            l("runesActive", runesActive, "bool");
            l("isDrawingLine", isDrawingLine, "bool");
            l("spellCasted", spellCasted, "bool");
            l("runeCircleRadius", runeCircleRadius, "float");
            l("runeSizeLarge", runeSizeLarge, "float");
            l("runeSizeSmall", runeSizeSmall, "float");
            l("lineWidth", lineWidth, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("isLeftClickHeld", isLeftClickHeld, "bool");
            l("runesActive", runesActive, "bool");
            l("isDrawingLine", isDrawingLine, "bool");
            l("spellCasted", spellCasted, "bool");
            l("runeCircleRadius", runeCircleRadius, "float");
            l("runeSizeLarge", runeSizeLarge, "float");
            l("runeSizeSmall", runeSizeSmall, "float");
            l("lineWidth", lineWidth, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const RuneSystemStateComponent& rs)
        {
            os << "RuneSystemState: "
                << "leftClickHeld=" << rs.isLeftClickHeld
                << ", runesActive=" << rs.runesActive
                << ", isDrawingLine=" << rs.isDrawingLine
                << ", connectionCount=" << rs.connectionCount;
            return os;
        }
    };
}
