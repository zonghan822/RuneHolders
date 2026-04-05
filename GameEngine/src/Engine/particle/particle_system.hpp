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

    /** @brief Tick all emitters: spawn new particles and simulate existing ones. */
    void Update(ECS::EntityManager& entityManager, float deltaTime);

    /** @brief Draw all active particles belonging to a single entity. */
    void RenderEntity(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager,
                      uint32_t entityID);

    /** @brief Immediately emit a fixed number of particles from an entity's emitter. */
    void EmitBurst(ECS::EntityManager& entityManager, uint32_t entityID, int count);
    /** @brief Enable continuous emission for the given entity's particle emitter. */
    void StartEmission(ECS::EntityManager& entityManager, uint32_t entityID);
    /** @brief Disable continuous emission for the given entity's particle emitter. */
    void StopEmission(ECS::EntityManager& entityManager, uint32_t entityID);

private:
    Event::EnoEventBus& bus_;

    // Particle pools mapped by entity ID
    std::unordered_map<uint32_t, std::unique_ptr<ParticlePool>> pools_;

    // Component accessors
    ECSComponents::ParticleEmitterComponent& GetEmitter(const ECS::EntityLoc& entityLoc);
    ECSComponents::TransformComponent& GetTransform(const ECS::EntityLoc& entityLoc);

    /** @brief Return the pool for an entity, creating one if it does not exist yet. */
    ParticlePool* getOrCreatePool(uint32_t entityID, int maxParticles);

    /** @brief Destroy the particle pool associated with a deleted entity. */
    void removePool(uint32_t entityID);

    /** @brief Spawn a single particle using the emitter's current settings. */
    void emitParticle(ECSComponents::ParticleEmitterComponent& emitter,
                     const Math::eVec2<float>& emitterPosition,
                     ParticlePool& pool);

    /** @brief Handle rate-based emission timing and spawn particles as needed. */
    void updateEmitter(ECSComponents::ParticleEmitterComponent& emitter,
                      const Math::eVec2<float>& emitterPosition,
                      ParticlePool& pool,
                      float deltaTime);

    /** @brief Interpolate each particle's color between start and end based on lifetime ratio. */
    void updateParticleColors(ECSComponents::ParticleEmitterComponent& emitter, ParticlePool& pool);

    /** @brief Return a uniformly distributed random float in [min, max]. */
    float randomRange(float min, float max);
    /** @brief Generate a random initial velocity vector based on the emitter's shape and range. */
    Math::eVec2<float> randomVelocity(const ECSComponents::ParticleEmitterComponent& emitter);
    /** @brief Generate a random spawn position within the emitter's emission shape. */
    Math::eVec2<float> randomPositionInShape(const ECSComponents::ParticleEmitterComponent& emitter);
};

} // namespace Particles
