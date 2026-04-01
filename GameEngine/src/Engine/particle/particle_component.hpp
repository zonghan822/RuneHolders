/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         particle_component.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

#include "../math/math.hpp"
#include "../event/signals.hpp"
#include <string>
#include <unordered_map>

namespace Particles {
    enum class EmissionShape {
        Point,
        Circle,
        Cone,
        Box
    };

    enum class BlendMode {
        Alpha,
        Additive
    };

    enum class ParticleShape {
        Square,
        Circle
    };
}

namespace ECSComponents {

struct ParticleEmitterComponent {
    // Emission properties
    float emissionRate = 50.0f;           // Particles per second
    int maxParticles = 200;               // Maximum number of active particles
    float duration = -1.0f;               // -1 for infinite, otherwise duration in seconds
    Particles::EmissionShape shape = Particles::EmissionShape::Point;
    float shapeRadius = 0.0f;             // For Circle
    float shapeAngle = 45.0f;             // For Cone (degrees)
    Math::eVec2<float> shapeSize = {10.0f, 10.0f}; // For Box

    // Particle properties
    Math::eVec2<float> velocityMin = {-50.0f, -50.0f};
    Math::eVec2<float> velocityMax = {50.0f, 50.0f};
    float lifetimeMin = 1.0f;             // Seconds
    float lifetimeMax = 2.0f;             // Seconds
    float sizeMin = 4.0f;                 // Pixels
    float sizeMax = 8.0f;                 // Pixels
    float rotationMin = 0.0f;             // Degrees
    float rotationMax = 360.0f;           // Degrees
    float rotationSpeed = 0.0f;           // Degrees per second

    // Physics
    Math::eVec2<float> gravity = {0.0f, 0.0f};
    float drag = 1.0f;                    // Velocity multiplier per frame (0.99 = slight drag)

    // Visual
    float colorStartR = 1.0f, colorStartG = 1.0f, colorStartB = 1.0f, colorStartA = 1.0f;
    float colorEndR = 1.0f, colorEndG = 1.0f, colorEndB = 1.0f, colorEndA = 0.0f;
    std::string texture = "";             // Empty string = solid color
    Particles::BlendMode blendMode = Particles::BlendMode::Alpha;
    Particles::ParticleShape particleShape = Particles::ParticleShape::Square;

    // Serialization proxies for enums (kept in sync by reflect)
    int shapeInt = 0;           // EmissionShape
    int blendModeInt = 0;       // BlendMode
    int particleShapeInt = 0;   // ParticleShape

    // State (internal)
    float emissionTimer = 0.0f;
    int activeParticleCount = 0;
    bool isEmitting = true;
    float elapsedTime = 0.0f;

    // Serialization support (if needed)
    ParticleEmitterComponent() = default;

    // Signals for event handling
    std::unordered_map<std::string, Events::EnoSignal> signals_;

    // Serialization
    template <typename Visitor>
    void reflect(Visitor&& v) {
        v("emissionRate", emissionRate, "float");
        v("maxParticles", maxParticles, "int");
        v("duration", duration, "float");
        v("emissionShape", shapeInt, "int");
        v("shapeRadius", shapeRadius, "float");
        v("shapeAngle", shapeAngle, "float");
        v("shapeSize", shapeSize, "eVec2(float)");
        v("velocityMin", velocityMin, "eVec2(float)");
        v("velocityMax", velocityMax, "eVec2(float)");
        v("lifetimeMin", lifetimeMin, "float");
        v("lifetimeMax", lifetimeMax, "float");
        v("sizeMin", sizeMin, "float");
        v("sizeMax", sizeMax, "float");
        v("rotationMin", rotationMin, "float");
        v("rotationMax", rotationMax, "float");
        v("rotationSpeed", rotationSpeed, "float");
        v("gravity", gravity, "eVec2(float)");
        v("drag", drag, "float");
        v("colorStartR", colorStartR, "float");
        v("colorStartG", colorStartG, "float");
        v("colorStartB", colorStartB, "float");
        v("colorStartA", colorStartA, "float");
        v("colorEndR", colorEndR, "float");
        v("colorEndG", colorEndG, "float");
        v("colorEndB", colorEndB, "float");
        v("colorEndA", colorEndA, "float");
        v("blendMode", blendModeInt, "int");
        v("particleShape", particleShapeInt, "int");
        v("isEmitting", isEmitting, "bool");
        // Sync enums from int proxies after deserialization
        shape         = static_cast<Particles::EmissionShape>(shapeInt);
        blendMode     = static_cast<Particles::BlendMode>(blendModeInt);
        particleShape = static_cast<Particles::ParticleShape>(particleShapeInt);
    }

    template <typename Visitor>
    void reflect(Visitor&& v) const {
        // Sync int proxies from enums before serialization
        const_cast<int&>(shapeInt)         = static_cast<int>(shape);
        const_cast<int&>(blendModeInt)     = static_cast<int>(blendMode);
        const_cast<int&>(particleShapeInt) = static_cast<int>(particleShape);
        v("emissionRate", emissionRate, "float");
        v("maxParticles", maxParticles, "int");
        v("duration", duration, "float");
        v("emissionShape", shapeInt, "int");
        v("shapeRadius", shapeRadius, "float");
        v("shapeAngle", shapeAngle, "float");
        v("shapeSize", shapeSize, "eVec2(float)");
        v("velocityMin", velocityMin, "eVec2(float)");
        v("velocityMax", velocityMax, "eVec2(float)");
        v("lifetimeMin", lifetimeMin, "float");
        v("lifetimeMax", lifetimeMax, "float");
        v("sizeMin", sizeMin, "float");
        v("sizeMax", sizeMax, "float");
        v("rotationMin", rotationMin, "float");
        v("rotationMax", rotationMax, "float");
        v("rotationSpeed", rotationSpeed, "float");
        v("gravity", gravity, "eVec2(float)");
        v("drag", drag, "float");
        v("colorStartR", colorStartR, "float");
        v("colorStartG", colorStartG, "float");
        v("colorStartB", colorStartB, "float");
        v("colorStartA", colorStartA, "float");
        v("colorEndR", colorEndR, "float");
        v("colorEndG", colorEndG, "float");
        v("colorEndB", colorEndB, "float");
        v("colorEndA", colorEndA, "float");
        v("blendMode", blendModeInt, "int");
        v("particleShape", particleShapeInt, "int");
        v("isEmitting", isEmitting, "bool");
    }
};

} // namespace ECSComponents
