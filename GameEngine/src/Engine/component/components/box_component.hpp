/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         box_component.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   80%
* @co-author    Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)   20%
* @date         05-11-2025
* @brief        Component that marks an entity as a pushable box. Boxes use PhysicsComponent
*//*========================================================================================*/



#pragma once

// Signals
#include "../../event/signals.hpp"

namespace ECSComponents
{
    /*============================================================================================
     * \brief       Marks an entity as a pushable box. Entities with this component should also
     *              have: TransformComponent, PhysicsComponent, and CollisionComponent.
     *
     * Usage Example:
     * ```cpp
     * ECS::Entity box = entityManager.createEntity();
     * box.add<TransformComponent>(ent, {400.0f, 300.0f});
     * box.add<PhysicsComponent>(ent, 2.0f);  // Mass of 2.0 (heavier = harder to push)
     * box.add<CollisionComponent>(ent, 64.0f, 64.0f);  // 64x64 collision box
     * box.add<BoxComponent>(ent);  // Mark as pushable box
     * ```
     *===========================================================================================*/
    struct BoxComponent
    {
        bool isPushable;      // Can this box be pushed? (useful for locked boxes)
        float pushResistance; // How hard is it to push? (0-1, higher = harder)

        /*============================================================================================
         * \brief       Default constructor - creates a standard pushable box
         *===========================================================================================*/
        BoxComponent()
            : isPushable(true)
            , pushResistance(0.2f)  // Default: moderate resistance
        {
        }

        /*============================================================================================
         * \brief       Constructor with custom pushability
         * \param[in]   bool canPush - Can this box be pushed?
         * \param[in]   float resistance - Push resistance (0-1, higher = harder)
         *===========================================================================================*/
        BoxComponent(bool canPush, float resistance = 0.2f)
            : isPushable(canPush)
            , pushResistance(resistance)
        {
        }

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        // Serialization support (for saving/loading)
        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("isPushable", isPushable, "bool");
            l("pushResistance", pushResistance, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("isPushable", isPushable, "bool");
            l("pushResistance", pushResistance, "float");
        }
    };

} // namespace ECSComponents