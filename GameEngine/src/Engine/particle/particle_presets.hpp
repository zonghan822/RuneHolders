/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         particle_presets.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

#include "particle_component.hpp"

namespace Particles::Presets {

// Spell effect presets
ECSComponents::ParticleEmitterComponent CreateFireball();
ECSComponents::ParticleEmitterComponent CreateIceShard();
ECSComponents::ParticleEmitterComponent CreateLightning();
ECSComponents::ParticleEmitterComponent CreateMagicAura();
ECSComponents::ParticleEmitterComponent CreateExplosion();
ECSComponents::ParticleEmitterComponent CreateHealingSparkles();
ECSComponents::ParticleEmitterComponent CreateSmokeTrail();

} // namespace Particles::Presets
