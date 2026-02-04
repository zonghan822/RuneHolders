#include "particle.hpp"
#include <algorithm>

namespace Particles {

void Particle::update(float deltaTime, const Math::eVec2<float>& gravity, float drag) {
    if (!active) return;

    // Apply gravity
    velocity += gravity * deltaTime;

    // Apply drag
    velocity *= drag;

    // Integrate position
    position += velocity * deltaTime;

    // Update rotation
    rotation += rotationSpeed * deltaTime;

    // Decrease lifetime
    lifetime -= deltaTime;

    // Deactivate when dead
    if (lifetime <= 0.0f) {
        active = false;
    }
}

ParticlePool::ParticlePool(size_t capacity) {
    particles_.resize(capacity);
}

Particle* ParticlePool::activateParticle() {
    // Try to find an inactive particle to reuse
    for (auto& particle : particles_) {
        if (!particle.active) {
            particle.active = true;
            activeCount_++;
            return &particle;
        }
    }

    // Pool is full
    return nullptr;
}

void ParticlePool::updateAll(float deltaTime, const Math::eVec2<float>& gravity, float drag) {
    size_t newActiveCount = 0;

    for (auto& particle : particles_) {
        if (particle.active) {
            particle.update(deltaTime, gravity, drag);
            if (particle.active) {
                newActiveCount++;
            }
        }
    }

    activeCount_ = newActiveCount;
}

void ParticlePool::reset() {
    for (auto& particle : particles_) {
        particle.active = false;
    }
    activeCount_ = 0;
}

void ParticlePool::resize(size_t newCapacity) {
    particles_.resize(newCapacity);
    // Recount active particles in case we shrunk
    activeCount_ = 0;
    for (const auto& particle : particles_) {
        if (particle.active) {
            activeCount_++;
        }
    }
}

} // namespace Particles
