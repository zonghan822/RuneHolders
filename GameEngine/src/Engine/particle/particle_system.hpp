/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         particle_system.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

#include "particle.hpp"
#include "particle_component.hpp"
#include "../ecs/entity.hpp"
#include "../event/event_bus.hpp"
#include <unordered_map>
#include <memory>

namespace Graphics {
    class EnoGraphicsManager;
}

namespace Particles {

class ParticleSystem {
public:
    explicit ParticleSystem(Event::EnoEventBus& bus);
    ~ParticleSystem() = default;

    // Main update loop - emission and physics
    void Update(ECS::EntityManager& entityManager, float deltaTime);

    // Render particles for a single entity
    void RenderEntity(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager,
                      uint32_t entityID);

    // Manual control methods
    void EmitBurst(ECS::EntityManager& entityManager, uint32_t entityID, int count);
    void StartEmission(ECS::EntityManager& entityManager, uint32_t entityID);
    void StopEmission(ECS::EntityManager& entityManager, uint32_t entityID);

private:
    Event::EnoEventBus& bus_;

    // Particle pools mapped by entity ID
    std::unordered_map<uint32_t, std::unique_ptr<ParticlePool>> pools_;

    // Component accessors
    ECSComponents::ParticleEmitterComponent& GetEmitter(const ECS::EntityLoc& entityLoc);
    ECSComponents::TransformComponent& GetTransform(const ECS::EntityLoc& entityLoc);

    // Get or create particle pool for entity
    ParticlePool* getOrCreatePool(uint32_t entityID, int maxParticles);

    // Remove pool for deleted entity
    void removePool(uint32_t entityID);

    // Emit a single particle from emitter
    void emitParticle(ECSComponents::ParticleEmitterComponent& emitter,
                     const Math::eVec2<float>& emitterPosition,
                     ParticlePool& pool);

    // Update emitter emission logic
    void updateEmitter(ECSComponents::ParticleEmitterComponent& emitter,
                      const Math::eVec2<float>& emitterPosition,
                      ParticlePool& pool,
                      float deltaTime);

    // Update particle colors based on lifetime
    void updateParticleColors(ECSComponents::ParticleEmitterComponent& emitter, ParticlePool& pool);

    // Random helper functions
    float randomRange(float min, float max);
    Math::eVec2<float> randomVelocity(const ECSComponents::ParticleEmitterComponent& emitter);
    Math::eVec2<float> randomPositionInShape(const ECSComponents::ParticleEmitterComponent& emitter);
};

} // namespace Particles
