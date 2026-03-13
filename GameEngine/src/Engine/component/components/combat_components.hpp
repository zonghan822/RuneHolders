/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         combat_components.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)   25%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   12%
* @co-author    Caven Koh (caven.koh@digipen.edu)   12%
* @date         2025-11-29
* @brief        Components for health and damage systems
*//*========================================================================================*/



#pragma once

#include "../../event/signals.hpp"
#include <iostream>
#include <unordered_map>
#include <string>

namespace ECSComponents
{
    // Legacy color enum for combat system (spell color matching)
    enum class RuneColor
    {
        RED,
        BLUE,
        GREEN
    };

    /*============================================================================================
    * \brief    Health component for entities that can take damage
    *           Use for: Players, Enemies, Destructible Objects
    *//*========================================================================================*/
    struct HealthComponent
    {
        float currentHealth;
        float maxHealth;
        bool isDead;
        bool invulnerable;      // Temporary invulnerability (e.g., i-frames)
        float invulnerabilityTimer;

        std::unordered_map<std::string, Events::EnoSignal> signals_;

        HealthComponent()
            : currentHealth(100.0f)
            , maxHealth(100.0f)
            , isDead(false)
            , invulnerable(false)
            , invulnerabilityTimer(0.0f)
        {
        }

        HealthComponent(float maxHp)
            : currentHealth(maxHp)
            , maxHealth(maxHp)
            , isDead(false)
            , invulnerable(false)
            , invulnerabilityTimer(0.0f)
        {
        }

        // Take damage and return true if entity died
        bool takeDamage(float amount)
        {
            if (invulnerable || isDead) {
                return false;
            }

            currentHealth -= amount;
            if (currentHealth <= 0.0f) {
                currentHealth = 0.0f;
                isDead = true;
                return true;
            }
            return false;
        }

        void heal(float amount)
        {
            if (isDead) return;

            currentHealth += amount;
            if (currentHealth > maxHealth) {
                currentHealth = maxHealth;
            }
        }

        // Get health percentage (0.0 to 1.0)
        float getHealthPercentage() const
        {
            return maxHealth > 0.0f ? currentHealth / maxHealth : 0.0f;
        }

        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("currentHealth", currentHealth, "float");
            l("maxHealth", maxHealth, "float");
            l("isDead", isDead, "bool");
            l("invulnerable", invulnerable, "bool");
            l("invulnerabilityTimer", invulnerabilityTimer, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("currentHealth", currentHealth, "float");
            l("maxHealth", maxHealth, "float");
            l("isDead", isDead, "bool");
            l("invulnerable", invulnerable, "bool");
            l("invulnerabilityTimer", invulnerabilityTimer, "float");
        }

        friend std::ostream& operator<<(std::ostream& os, const HealthComponent& hc)
        {
            os << "HealthComponent: "
               << "HP=" << hc.currentHealth << "/" << hc.maxHealth
               << ", isDead=" << hc.isDead;
            return os;
        }
    };

    /*============================================================================================
    * \brief    Damage component for entities that can deal damage
    *           Use for: Enemies, Projectiles, Traps, Hazards
    *//*========================================================================================*/
    struct DamageComponent
    {
        float damageAmount;
        float damageMultiplier;     // For buffs/debuffs
        bool destroyOnHit;          // Destroy entity when it deals damage?
        char damageType[32];        // e.g., "physical", "fire", "magic"

        // Spell color for color-based damage system
        RuneColor spellColor;       // Color of the spell (for matching vulnerability)
        bool useColorMatching;      // Whether to use the color-matching system

        std::unordered_map<std::string, Events::EnoSignal> signals_;

        DamageComponent()
            : damageAmount(10.0f)
            , damageMultiplier(1.0f)
            , destroyOnHit(false)
            , spellColor(RuneColor::RED)
            , useColorMatching(false)
        {
            strcpy_s(damageType, "physical");
        }

        DamageComponent(float damage, bool destroyOnHit = false)
            : damageAmount(damage)
            , damageMultiplier(1.0f)
            , destroyOnHit(destroyOnHit)
            , spellColor(RuneColor::RED)
            , useColorMatching(false)
        {
            strcpy_s(damageType, "physical");
        }

        DamageComponent(float damage, RuneColor color, bool destroyOnHit = false)
            : damageAmount(damage)
            , damageMultiplier(1.0f)
            , destroyOnHit(destroyOnHit)
            , spellColor(color)
            , useColorMatching(true)
        {
            strcpy_s(damageType, "spell");
        }

        // Get total damage after multipliers
        float getTotalDamage() const
        {
            return damageAmount * damageMultiplier;
        }

        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            int colorInt = static_cast<int>(spellColor);
            l("damageAmount", damageAmount, "float");
            l("damageMultiplier", damageMultiplier, "float");
            l("destroyOnHit", destroyOnHit, "bool");
            l("damageType", damageType, "char32");
            l("spellColor", colorInt, "int");
            l("useColorMatching", useColorMatching, "bool");
            spellColor = static_cast<RuneColor>(colorInt);
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            int colorInt = static_cast<int>(spellColor);
            l("damageAmount", damageAmount, "float");
            l("damageMultiplier", damageMultiplier, "float");
            l("destroyOnHit", destroyOnHit, "bool");
            l("damageType", damageType, "char32");
            l("spellColor", colorInt, "int");
            l("useColorMatching", useColorMatching, "bool");
        }

        friend std::ostream& operator<<(std::ostream& os, const DamageComponent& dc)
        {
            os << "DamageComponent: "
                << "damage=" << dc.getTotalDamage()
                << ", type=" << dc.damageType
                << ", spellColor=" << static_cast<int>(dc.spellColor);
            return os;
        }
    };
}
