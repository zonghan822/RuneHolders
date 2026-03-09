/*============================================================================================
* @file         eno_bridge_combat.cpp
* @brief        Combat bridge — health, max health, heal, death check.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/combat_components.hpp"

namespace Bridge
{
    static ECSComponents::HealthComponent* getHealth(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::HealthComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::HealthComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        ENGINE_API float __stdcall GetHealth(uint32_t sceneObjectID)
        {
            auto* h = getHealth(sceneObjectID);
            return h ? h->currentHealth : 0.0f;
        }

        ENGINE_API void __stdcall SetHealth(uint32_t sceneObjectID, float health)
        {
            if (auto* h = getHealth(sceneObjectID))
            {
                h->currentHealth = health;
                if (h->currentHealth <= 0.0f)
                {
                    h->currentHealth = 0.0f;
                    h->isDead = true;
                }
                else
                {
                    h->isDead = false;
                }
            }
        }

        ENGINE_API float __stdcall GetMaxHealth(uint32_t sceneObjectID)
        {
            auto* h = getHealth(sceneObjectID);
            return h ? h->maxHealth : 0.0f;
        }

        ENGINE_API void __stdcall SetMaxHealth(uint32_t sceneObjectID, float maxHealth)
        {
            if (auto* h = getHealth(sceneObjectID))
                h->maxHealth = maxHealth;
        }

        ENGINE_API bool __stdcall IsDead(uint32_t sceneObjectID)
        {
            auto* h = getHealth(sceneObjectID);
            return h ? h->isDead : false;
        }

        ENGINE_API void __stdcall HealEntity(uint32_t sceneObjectID, float amount)
        {
            if (auto* h = getHealth(sceneObjectID))
                h->heal(amount);
        }
    }
}
