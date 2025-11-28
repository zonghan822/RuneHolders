/*============================================================================================
* @file         physics_component.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio)
* @date         02-11-2025
* @brief        Physics component for ECS. Stores physics properties including mass, velocity,
*               acceleration, forces, and simulation flags for physics-based entities.
*===========================================================================================*/

#pragma once

// Signals
#include "../../event/signals.hpp"

#include "../../math/math.hpp"

namespace ECSComponents
{
    /*============================================================================================
     * \brief       Component that stores physics simulation data for entities. Used by the
     *              physics system to apply forces, calculate velocities, and update positions
     *              based on physics simulation
     *===========================================================================================*/
    struct PhysicsComponent
    {
        // Core physics properties
        Math::eVec2<float> velocity;        // Current velocity (pixels/sec)
        Math::eVec2<float> acceleration;    // Current acceleration (pixels/sec^2)
        Math::eVec2<float> force;           // Accumulated forces to apply this frame

        float mass;                         // Mass of the object (affects force calculations)
        float drag;                         // Air resistance/drag coefficient (0-1)
        float restitution;                  // Bounciness (0 = no bounce, 1 = perfect bounce)

        // Simulation flags
        bool useGravity;                    // Should gravity affect this object?
        bool isKinematic;                   // If true, physics won't move it (manually controlled)
        bool freezeRotation;                // Prevent rotation from physics

        // Constraints
        Math::eVec2<float> velocityMin;     // Minimum velocity clamp
        Math::eVec2<float> velocityMax;     // Maximum velocity clamp

        /*============================================================================================
         * \brief       Default constructor with common physics defaults
         * \param[in]   float objectMass - Mass of the object (default: 1.0f)
         *===========================================================================================*/
        PhysicsComponent(float objectMass = 1.0f)
            : velocity{ 0.0f, 0.0f }
            , acceleration{ 0.0f, 0.0f }
            , force{ 0.0f, 0.0f }
            , mass(objectMass)
            , drag(0.1f)                    // Light drag by default
            , restitution(0.5f)             // Medium bounciness
            , useGravity(false)             // No gravity by default
            , isKinematic(false)            // Dynamic by default
            , freezeRotation(true)          // Prevent rotation by default
            , velocityMin{ -1000.0f, -1000.0f }
            , velocityMax{ 1000.0f, 1000.0f }
        {}

        /*============================================================================================
         * \brief       Add a force to be applied this frame
         * \param[in]   Math::eVec2<float> const& forceToAdd - Force vector to add
         * \return      void
         *===========================================================================================*/
        void AddForce(const Math::eVec2<float>& forceToAdd)
        {
            force.x += forceToAdd.x;
            force.y += forceToAdd.y;
        }

        /*============================================================================================
         * \brief       Clear all accumulated forces (typically called after physics update)
         * \return      void
         *===========================================================================================*/
        void ClearForces()
        {
            force.x = 0.0f;
            force.y = 0.0f;
        }

        /*============================================================================================
         * \brief       Apply velocity clamping to stay within min/max bounds
         * \return      void
         *===========================================================================================*/
        void ClampVelocity()
        {
            // Clamp X velocity
            if (velocity.x < velocityMin.x) velocity.x = velocityMin.x;
            if (velocity.x > velocityMax.x) velocity.x = velocityMax.x;

            // Clamp Y velocity
            if (velocity.y < velocityMin.y) velocity.y = velocityMin.y;
            if (velocity.y > velocityMax.y) velocity.y = velocityMax.y;
        }

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

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
            l("Velocity",           velocity,           "eVec2(float)");
            l("Acceleration",       acceleration,       "eVec2(float)");
            l("Force",              force,              "eVec2(float)");

            l("Mass",               mass,               "float");
            l("Drag",               drag,               "float");
            l("Restitution",        restitution,        "float");

            l("UseGravity",         useGravity,         "bool");
            l("IsKinematic",        isKinematic,        "bool");
            l("FreezeRotation",     freezeRotation,     "bool");

            l("VelocityMin",        velocityMin,        "eVec2(float)");
            l("VelocityMax",        velocityMax,        "eVec2(float)");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("Velocity",           velocity,           "eVec2(float)");
            l("Acceleration",       acceleration,       "eVec2(float)");
            l("Force",              force,              "eVec2(float)");

            l("Mass",               mass,               "float");
            l("Drag",               drag,               "float");
            l("Restitution",        restitution,        "float");

            l("UseGravity",         useGravity,         "bool");
            l("IsKinematic",        isKinematic,        "bool");
            l("FreezeRotation",     freezeRotation,     "bool");

            l("VelocityMin",        velocityMin,        "eVec2(float)");
            l("VelocityMax",        velocityMax,        "eVec2(float)");
        }
    };

} // namespace ECSComponents