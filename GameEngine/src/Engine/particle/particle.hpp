/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         particle.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

#include "../math/math.hpp"
#include <vector>
#include <cstddef>

namespace Particles {

struct Particle {
    Math::eVec2<float> position = {0.0f, 0.0f};
    Math::eVec2<float> velocity = {0.0f, 0.0f};
    float lifetime = 0.0f;           // Remaining lifetime in seconds
    float maxLifetime = 1.0f;        // Original lifetime for interpolation
    float size = 1.0f;
    float rotation = 0.0f;           // Degrees
    float rotationSpeed = 0.0f;      // Degrees per second
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;  // Color components
    bool active = false;

    /** @brief Advance the particle by one time step, applying gravity, drag, and rotation. */
    void update(float deltaTime, const Math::eVec2<float>& gravity, float drag);

    // Get interpolation factor (0.0 = just born, 1.0 = dead)
    float getLifetimeRatio() const {
        if (maxLifetime <= 0.0f) return 1.0f;
        return 1.0f - (lifetime / maxLifetime);
    }
};

/** @brief Fixed-capacity pool that recycles inactive particles to avoid allocations. */
class ParticlePool {
public:
    explicit ParticlePool(size_t capacity = 200);

    /** @brief Activate the next available particle and return a pointer to it, or nullptr if full. */
    Particle* activateParticle();

    /** @brief Update all active particles by one time step and deactivate expired ones. */
    void updateAll(float deltaTime, const Math::eVec2<float>& gravity, float drag);

    // Get active particle count
    size_t getActiveCount() const { return activeCount_; }

    // Access particles for rendering
    const std::vector<Particle>& getParticles() const { return particles_; }
    std::vector<Particle>& getParticles() { return particles_; }

    /** @brief Deactivate all particles and reset the active count to zero. */
    void reset();

    /** @brief Resize the pool to a new capacity, deactivating all existing particles. */
    void resize(size_t newCapacity);

private:
    std::vector<Particle> particles_;
    size_t activeCount_ = 0;
};

} // namespace Particles
