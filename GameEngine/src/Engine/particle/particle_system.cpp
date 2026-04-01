/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         particle_system.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#include "particle_system.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/drawInfo.hpp"
#include "../graphics/graphics.hpp"
#include <random>
#include <cmath>
#include <algorithm>
#include <typeindex>

namespace Particles {

namespace {
    // Random number generator
    thread_local std::mt19937 rng(std::random_device{}());
}

ParticleSystem::ParticleSystem(Event::EnoEventBus& bus) : bus_(bus) {
}

ECSComponents::ParticleEmitterComponent& ParticleSystem::GetEmitter(const ECS::EntityLoc& entityLoc) {
    size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::ParticleEmitterComponent>();
    return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::ParticleEmitterComponent>(
        index, entityLoc.entityIndexInChunk);
}

ECSComponents::TransformComponent& ParticleSystem::GetTransform(const ECS::EntityLoc& entityLoc) {
    size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();
    return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
        index, entityLoc.entityIndexInChunk);
}

void ParticleSystem::Update(ECS::EntityManager& entityManager, float deltaTime) {
    // Get all entities in the world
    auto& entityMap = entityManager.getMap();

    // Clean up pools for deleted entities
    std::vector<uint32_t> poolsToRemove;
    for (auto& [poolEntityID, pool] : pools_) {
        if (entityMap.find(poolEntityID) == entityMap.end()) {
            poolsToRemove.push_back(poolEntityID);
        }
    }
    for (uint32_t id : poolsToRemove) {
        pools_.erase(id);
    }

    // Iterate through all entities with ParticleEmitterComponent
    for (auto& [entityID, entityLoc] : entityMap) {
        // Check if entity has required components
        bool hasEmitter = false;
        bool hasTransform = false;

        for (const auto& type : entityLoc.archetype->types_) {
            if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent)))
                hasEmitter = true;
            else if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
                hasTransform = true;
        }

        if (!hasEmitter || !hasTransform)
            continue;

        // Get components
        auto& emitter = GetEmitter(entityLoc);
        auto& transform = GetTransform(entityLoc);

        // Get or create pool
        ParticlePool* pool = getOrCreatePool(entityID, emitter.maxParticles);
        if (!pool) continue;

        // Update emitter state
        if (emitter.duration >= 0.0f) {
            emitter.elapsedTime += deltaTime;
            if (emitter.elapsedTime >= emitter.duration) {
                emitter.isEmitting = false;
            }
        }

        // Update emission
        updateEmitter(emitter, transform.pos, *pool, deltaTime);

        // Update particle physics
        pool->updateAll(deltaTime, emitter.gravity, emitter.drag);

        // Update particle colors
        updateParticleColors(emitter, *pool);

        // Update active count
        emitter.activeParticleCount = static_cast<int>(pool->getActiveCount());
    }
}

void ParticleSystem::RenderEntity(Graphics::EnoGraphicsManager& graphics, ECS::EntityManager& entityManager,
                                  uint32_t entityID) {
    auto mapIt = entityManager.getMap().find(entityID);
    if (mapIt == entityManager.getMap().end()) return;

    auto& entityLoc = mapIt->second;

    bool hasEmitter = false;
    for (const auto& type : entityLoc.archetype->types_) {
        if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent))) {
            hasEmitter = true;
            break;
        }
    }
    if (!hasEmitter) return;

    auto& emitter = GetEmitter(entityLoc);

    auto poolIt = pools_.find(entityID);
    if (poolIt == pools_.end()) return;

    ParticlePool& pool = *poolIt->second;
    const auto& particles = pool.getParticles();

    // Read texture and shape from RenderComponent if present
    std::shared_ptr<Graphics::EnoTexture> tex = nullptr;
    bool useCircle = false;
    {
        // Construct a temporary Entity to use the standard get<> accessor
        ECS::Entity tmpEntity(entityID);
        if (tmpEntity.has<ECSComponents::RenderComponent>(entityManager)) {
            auto& renderComp = tmpEntity.get<ECSComponents::RenderComponent>(entityManager);
            if (renderComp.texture && renderComp.texture->isValid())
                tex = renderComp.texture;
            if (strcmp(renderComp.shape, "Circle") == 0)
                useCircle = true;
        }
    }

    if (emitter.blendMode == Particles::BlendMode::Additive)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& particle : particles) {
        if (!particle.active) continue;

        Graphics::Color particleColor(particle.r, particle.g, particle.b, particle.a);
        Math::eVec2<float> size{particle.size, particle.size};

        if (tex)
        {
            graphics.drawTexturedQuad(
                particle.position,
                size,
                tex,
                particleColor,
                particle.rotation
            );
        }
        else if (useCircle)
        {
            graphics.drawCircle(
                { particle.position.x + particle.size * 0.5f, particle.position.y + particle.size * 0.5f },
                particle.size * 0.5f,
                particleColor,
                true
            );
        }
        else
        {
            graphics.drawRect(
                particle.position,
                size,
                particleColor,
                true,
                particle.rotation
            );
        }
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::EmitBurst(ECS::EntityManager& entityManager, uint32_t entityID, int count) {
    auto it = entityManager.getMap().find(entityID);
    if (it == entityManager.getMap().end()) return;

    auto& entityLoc = it->second;

    // Check for components
    bool hasEmitter = false;
    bool hasTransform = false;
    for (const auto& type : entityLoc.archetype->types_) {
        if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent)))
            hasEmitter = true;
        else if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
            hasTransform = true;
    }

    if (!hasEmitter || !hasTransform) return;

    auto& emitter = GetEmitter(entityLoc);
    auto& transform = GetTransform(entityLoc);

    ParticlePool* pool = getOrCreatePool(entityID, emitter.maxParticles);
    if (!pool) return;

    for (int i = 0; i < count; ++i) {
        emitParticle(emitter, transform.pos, *pool);
    }
}

void ParticleSystem::StartEmission(ECS::EntityManager& entityManager, uint32_t entityID) {
    auto it = entityManager.getMap().find(entityID);
    if (it == entityManager.getMap().end()) return;

    auto& entityLoc = it->second;

    bool hasEmitter = false;
    for (const auto& type : entityLoc.archetype->types_) {
        if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent))) {
            hasEmitter = true;
            break;
        }
    }

    if (!hasEmitter) return;

    auto& emitter = GetEmitter(entityLoc);
    emitter.isEmitting = true;
    emitter.elapsedTime = 0.0f;
    emitter.emissionTimer = 0.0f;
}

void ParticleSystem::StopEmission(ECS::EntityManager& entityManager, uint32_t entityID) {
    auto it = entityManager.getMap().find(entityID);
    if (it == entityManager.getMap().end()) return;

    auto& entityLoc = it->second;

    bool hasEmitter = false;
    for (const auto& type : entityLoc.archetype->types_) {
        if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent))) {
            hasEmitter = true;
            break;
        }
    }

    if (!hasEmitter) return;

    auto& emitter = GetEmitter(entityLoc);
    emitter.isEmitting = false;
}

ParticlePool* ParticleSystem::getOrCreatePool(uint32_t entityID, int maxParticles) {
    auto it = pools_.find(entityID);
    if (it != pools_.end()) {
        // Resize if needed
        if (it->second->getParticles().size() != static_cast<size_t>(maxParticles)) {
            it->second->resize(maxParticles);
        }
        return it->second.get();
    }

    // Create new pool
    auto pool = std::make_unique<ParticlePool>(maxParticles);
    auto* poolPtr = pool.get();
    pools_[entityID] = std::move(pool);
    return poolPtr;
}

void ParticleSystem::removePool(uint32_t entityID) {
    pools_.erase(entityID);
}

void ParticleSystem::emitParticle(ECSComponents::ParticleEmitterComponent& emitter,
                                 const Math::eVec2<float>& emitterPosition,
                                 ParticlePool& pool) {
    Particle* particle = pool.activateParticle();
    if (!particle) return; // Pool is full

    // Set position
    particle->position = emitterPosition + randomPositionInShape(emitter);

    // Set velocity
    particle->velocity = randomVelocity(emitter);

    // Set lifetime
    particle->lifetime = randomRange(emitter.lifetimeMin, emitter.lifetimeMax);
    particle->maxLifetime = particle->lifetime;

    // Set size
    particle->size = randomRange(emitter.sizeMin, emitter.sizeMax);

    // Set rotation
    particle->rotation = randomRange(emitter.rotationMin, emitter.rotationMax);
    particle->rotationSpeed = emitter.rotationSpeed;

    // Set initial color
    particle->r = emitter.colorStartR;
    particle->g = emitter.colorStartG;
    particle->b = emitter.colorStartB;
    particle->a = emitter.colorStartA;
}

void ParticleSystem::updateEmitter(ECSComponents::ParticleEmitterComponent& emitter,
                                  const Math::eVec2<float>& emitterPosition,
                                  ParticlePool& pool,
                                  float deltaTime) {
    if (!emitter.isEmitting) return;

    emitter.emissionTimer += deltaTime;
    float timePerParticle = 1.0f / emitter.emissionRate;

    while (emitter.emissionTimer >= timePerParticle) {
        emitParticle(emitter, emitterPosition, pool);
        emitter.emissionTimer -= timePerParticle;
    }
}

void ParticleSystem::updateParticleColors(ECSComponents::ParticleEmitterComponent& emitter, ParticlePool& pool) {
    auto& particles = pool.getParticles();

    for (auto& particle : particles) {
        if (!particle.active) continue;

        // Interpolate color based on lifetime
        float t = particle.getLifetimeRatio();
        particle.r = emitter.colorStartR + (emitter.colorEndR - emitter.colorStartR) * t;
        particle.g = emitter.colorStartG + (emitter.colorEndG - emitter.colorStartG) * t;
        particle.b = emitter.colorStartB + (emitter.colorEndB - emitter.colorStartB) * t;
        particle.a = emitter.colorStartA + (emitter.colorEndA - emitter.colorStartA) * t;
    }
}

float ParticleSystem::randomRange(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

Math::eVec2<float> ParticleSystem::randomVelocity(const ECSComponents::ParticleEmitterComponent& emitter) {
    return Math::eVec2<float>{
        randomRange(emitter.velocityMin.x, emitter.velocityMax.x),
        randomRange(emitter.velocityMin.y, emitter.velocityMax.y)
    };
}

Math::eVec2<float> ParticleSystem::randomPositionInShape(const ECSComponents::ParticleEmitterComponent& emitter) {
    switch (emitter.shape) {
        case Particles::EmissionShape::Point:
            return {0.0f, 0.0f};

        case Particles::EmissionShape::Circle: {
            float angle = randomRange(0.0f, 2.0f * 3.14159265f);
            float radius = randomRange(0.0f, emitter.shapeRadius);
            return {
                std::cos(angle) * radius,
                std::sin(angle) * radius
            };
        }

        case Particles::EmissionShape::Cone: {
            float halfAngle = emitter.shapeAngle * 0.5f * 3.14159265f / 180.0f;
            float angle = randomRange(-halfAngle, halfAngle);
            float distance = randomRange(0.0f, emitter.shapeRadius);
            return {
                std::cos(angle) * distance,
                std::sin(angle) * distance
            };
        }

        case Particles::EmissionShape::Box:
            return {
                randomRange(-emitter.shapeSize.x * 0.5f, emitter.shapeSize.x * 0.5f),
                randomRange(-emitter.shapeSize.y * 0.5f, emitter.shapeSize.y * 0.5f)
            };

        default:
            return {0.0f, 0.0f};
    }
}

} // namespace Particles
