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

    void update(float deltaTime, const Math::eVec2<float>& gravity, float drag);

    // Get interpolation factor (0.0 = just born, 1.0 = dead)
    float getLifetimeRatio() const {
        if (maxLifetime <= 0.0f) return 1.0f;
        return 1.0f - (lifetime / maxLifetime);
    }
};

class ParticlePool {
public:
    explicit ParticlePool(size_t capacity = 200);

    // Activate a new particle and return pointer to it (nullptr if pool is full)
    Particle* activateParticle();

    // Update all active particles
    void updateAll(float deltaTime, const Math::eVec2<float>& gravity, float drag);

    // Get active particle count
    size_t getActiveCount() const { return activeCount_; }

    // Access particles for rendering
    const std::vector<Particle>& getParticles() const { return particles_; }
    std::vector<Particle>& getParticles() { return particles_; }

    // Reset all particles to inactive
    void reset();

    // Resize pool capacity
    void resize(size_t newCapacity);

private:
    std::vector<Particle> particles_;
    size_t activeCount_ = 0;
};

} // namespace Particles
