#include "particle_presets.hpp"

namespace Particles::Presets {

ECSComponents::ParticleEmitterComponent CreateFireball() {
    ECSComponents::ParticleEmitterComponent emitter;

    // Emission
    emitter.emissionRate = 100.0f;
    emitter.maxParticles = 200;
    emitter.shape = Particles::EmissionShape::Circle;
    emitter.shapeRadius = 5.0f;

    // Particle properties
    emitter.velocityMin = {-20.0f, -60.0f};
    emitter.velocityMax = {20.0f, -20.0f};
    emitter.lifetimeMin = 0.3f;
    emitter.lifetimeMax = 0.6f;
    emitter.sizeMin = 6.0f;
    emitter.sizeMax = 12.0f;

    // Physics
    emitter.gravity = {0.0f, 50.0f};
    emitter.drag = 0.98f;

    // Visual - orange to red to transparent
    emitter.colorStartR = 1.0f; emitter.colorStartG = 0.6f; emitter.colorStartB = 0.1f; emitter.colorStartA = 1.0f;
    emitter.colorEndR = 1.0f; emitter.colorEndG = 0.1f; emitter.colorEndB = 0.0f; emitter.colorEndA = 0.0f;
    emitter.blendMode = Particles::BlendMode::Additive;

    return emitter;
}

//ECSComponents::ParticleEmitterComponent CreateIceShard() {
//    ParticleEmitterComponent emitter;
//
//    // Emission
//    emitter.emissionRate = 60.0f;
//    emitter.maxParticles = 150;
//    emitter.shape = Particles::EmissionShape::Circle;
//    emitter.shapeRadius = 8.0f;
//
//    // Particle properties
//    emitter.velocityMin = {-30.0f, 10.0f};
//    emitter.velocityMax = {30.0f, 50.0f};
//    emitter.lifetimeMin = 0.5f;
//    emitter.lifetimeMax = 1.0f;
//    emitter.sizeMin = 4.0f;
//    emitter.sizeMax = 8.0f;
//
//    // Physics - falling ice
//    emitter.gravity = {0.0f, 100.0f};
//    emitter.drag = 0.99f;
//
//    // Visual - cyan to blue
//    emitter.colorStartR = 0.5f; emitter.colorStartG = 0.8f; emitter.colorStartB = 1.0f; emitter.colorStartA = 1.0f;
//    emitter.colorEndR = 0.1f; emitter.colorEndG = 0.3f; emitter.colorEndB = 0.8f; emitter.colorEndA = 0.0f;
//    emitter.blendMode = Particles::BlendMode::Alpha;
//
//    return emitter;
//}
//
//ECSComponents::ParticleEmitterComponent CreateLightning() {
//    ParticleEmitterComponent emitter;
//
//    // Emission
//    emitter.emissionRate = 150.0f;
//    emitter.maxParticles = 100;
//    emitter.shape = Particles::EmissionShape::Cone;
//    emitter.shapeRadius = 15.0f;
//    emitter.shapeAngle = 30.0f;
//
//    // Particle properties - fast, directional
//    emitter.velocityMin = {0.0f, -200.0f};
//    emitter.velocityMax = {0.0f, -100.0f};
//    emitter.lifetimeMin = 0.1f;
//    emitter.lifetimeMax = 0.3f;
//    emitter.sizeMin = 3.0f;
//    emitter.sizeMax = 6.0f;
//
//    // Physics - no gravity
//    emitter.gravity = {0.0f, 0.0f};
//    emitter.drag = 0.95f;
//
//    // Visual - white/yellow electric
//    emitter.colorStartR = 1.0f; emitter.colorStartG = 1.0f; emitter.colorStartB = 0.8f; emitter.colorStartA = 1.0f;
//    emitter.colorEndR = 0.8f; emitter.colorEndG = 0.8f; emitter.colorEndB = 1.0f; emitter.colorEndA = 0.0f;
//    emitter.blendMode = Particles::BlendMode::Additive;
//
//    return emitter;
//}
//
//ECSComponents::ParticleEmitterComponent CreateMagicAura() {
//    ParticleEmitterComponent emitter;
//
//    // Emission
//    emitter.emissionRate = 40.0f;
//    emitter.maxParticles = 150;
//    emitter.shape = Particles::EmissionShape::Circle;
//    emitter.shapeRadius = 20.0f;
//
//    // Particle properties - slow floating
//    emitter.velocityMin = {-15.0f, -30.0f};
//    emitter.velocityMax = {15.0f, -10.0f};
//    emitter.lifetimeMin = 1.0f;
//    emitter.lifetimeMax = 2.0f;
//    emitter.sizeMin = 4.0f;
//    emitter.sizeMax = 10.0f;
//    emitter.rotationSpeed = 45.0f;
//
//    // Physics - float upward
//    emitter.gravity = {0.0f, -20.0f};
//    emitter.drag = 0.97f;
//
//    // Visual - purple/pink magic
//    emitter.colorStartR = 0.8f; emitter.colorStartG = 0.3f; emitter.colorStartB = 1.0f; emitter.colorStartA = 0.8f;
//    emitter.colorEndR = 1.0f; emitter.colorEndG = 0.5f; emitter.colorEndB = 0.8f; emitter.colorEndA = 0.0f;
//    emitter.blendMode = Particles::BlendMode::Additive;
//
//    return emitter;
//}
//
//ECSComponents::ParticleEmitterComponent CreateExplosion() {
//    ParticleEmitterComponent emitter;
//
//    // Emission - burst only
//    emitter.emissionRate = 500.0f;
//    emitter.maxParticles = 200;
//    emitter.duration = 0.1f; // Very short emission
//    emitter.shape = Particles::EmissionShape::Circle;
//    emitter.shapeRadius = 10.0f;
//
//    // Particle properties - radial explosion
//    emitter.velocityMin = {-150.0f, -150.0f};
//    emitter.velocityMax = {150.0f, 150.0f};
//    emitter.lifetimeMin = 0.4f;
//    emitter.lifetimeMax = 0.8f;
//    emitter.sizeMin = 8.0f;
//    emitter.sizeMax = 16.0f;
//
//    // Physics
//    emitter.gravity = {0.0f, 80.0f};
//    emitter.drag = 0.92f; // High drag for quick slowdown
//
//    // Visual - orange explosion
//    emitter.colorStartR = 1.0f; emitter.colorStartG = 0.8f; emitter.colorStartB = 0.2f; emitter.colorStartA = 1.0f;
//    emitter.colorEndR = 0.5f; emitter.colorEndG = 0.1f; emitter.colorEndB = 0.0f; emitter.colorEndA = 0.0f;
//    emitter.blendMode = Particles::BlendMode::Additive;
//
//    return emitter;
//}
//
//ECSComponents::ParticleEmitterComponent CreateHealingSparkles() {
//    ParticleEmitterComponent emitter;
//
//    // Emission
//    emitter.emissionRate = 50.0f;
//    emitter.maxParticles = 100;
//    emitter.shape = Particles::EmissionShape::Circle;
//    emitter.shapeRadius = 15.0f;
//
//    // Particle properties - gentle upward float
//    emitter.velocityMin = {-10.0f, -50.0f};
//    emitter.velocityMax = {10.0f, -20.0f};
//    emitter.lifetimeMin = 0.8f;
//    emitter.lifetimeMax = 1.5f;
//    emitter.sizeMin = 3.0f;
//    emitter.sizeMax = 6.0f;
//    emitter.rotationSpeed = 180.0f;
//
//    // Physics - float upward
//    emitter.gravity = {0.0f, -30.0f};
//    emitter.drag = 0.98f;
//
//    // Visual - green/white sparkles
//    emitter.colorStartR = 0.6f; emitter.colorStartG = 1.0f; emitter.colorStartB = 0.6f; emitter.colorStartA = 1.0f;
//    emitter.colorEndR = 1.0f; emitter.colorEndG = 1.0f; emitter.colorEndB = 1.0f; emitter.colorEndA = 0.0f;
//    emitter.blendMode = Particles::BlendMode::Additive;
//
//    return emitter;
//}
//
//ECSComponents::ParticleEmitterComponent CreateSmokeTrail() {
//    ParticleEmitterComponent emitter;
//
//    // Emission
//    emitter.emissionRate = 30.0f;
//    emitter.maxParticles = 100;
//    emitter.shape = Particles::EmissionShape::Point;
//
//    // Particle properties - slow drift
//    emitter.velocityMin = {-15.0f, -20.0f};
//    emitter.velocityMax = {15.0f, -5.0f};
//    emitter.lifetimeMin = 1.0f;
//    emitter.lifetimeMax = 2.0f;
//    emitter.sizeMin = 8.0f;
//    emitter.sizeMax = 16.0f;
//    emitter.rotationSpeed = 20.0f;
//
//    // Physics - gentle float
//    emitter.gravity = {0.0f, -10.0f};
//    emitter.drag = 0.98f;
//
//    // Visual - gray smoke
//    emitter.colorStartR = 0.3f; emitter.colorStartG = 0.3f; emitter.colorStartB = 0.3f; emitter.colorStartA = 0.6f;
//    emitter.colorEndR = 0.5f; emitter.colorEndG = 0.5f; emitter.colorEndB = 0.5f; emitter.colorEndA = 0.0f;
//    emitter.blendMode = Particles::BlendMode::Alpha;
//
//    return emitter;
//}

} // namespace Particles::Presets
