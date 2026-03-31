/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_particle.cpp
* @brief        Particle system bridge — emit burst, start/stop emission, configure emitter.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../particle/particle_component.hpp"

namespace Bridge
{
    static ECSComponents::ParticleEmitterComponent* getEmitter(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::ParticleEmitterComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::ParticleEmitterComponent>(gEngine->getEntityManager());
    }

    static uint32_t getEntityID(uint32_t sceneObjectID)
    {
        if (!gEngine) return UINT32_MAX;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return UINT32_MAX;
        return node->entity_.id();
    }

    extern "C"
    {
        ENGINE_API void __stdcall ParticleEmitBurst(uint32_t sceneObjectID, int count)
        {
            auto* ps = gEngine ? gEngine->getParticleSystem() : nullptr;
            if (!ps) return;
            uint32_t eid = getEntityID(sceneObjectID);
            if (eid == UINT32_MAX) return;
            ps->EmitBurst(gEngine->getEntityManager(), eid, count);
        }

        ENGINE_API void __stdcall ParticleStartEmission(uint32_t sceneObjectID)
        {
            auto* ps = gEngine ? gEngine->getParticleSystem() : nullptr;
            if (!ps) return;
            uint32_t eid = getEntityID(sceneObjectID);
            if (eid == UINT32_MAX) return;
            ps->StartEmission(gEngine->getEntityManager(), eid);
        }

        ENGINE_API void __stdcall ParticleStopEmission(uint32_t sceneObjectID)
        {
            auto* ps = gEngine ? gEngine->getParticleSystem() : nullptr;
            if (!ps) return;
            uint32_t eid = getEntityID(sceneObjectID);
            if (eid == UINT32_MAX) return;
            ps->StopEmission(gEngine->getEntityManager(), eid);
        }

        ENGINE_API void __stdcall SetParticleEmissionRate(uint32_t sceneObjectID, float rate)
        {
            if (auto* e = getEmitter(sceneObjectID)) e->emissionRate = rate;
        }

        ENGINE_API void __stdcall SetParticleMaxParticles(uint32_t sceneObjectID, int max)
        {
            if (auto* e = getEmitter(sceneObjectID)) e->maxParticles = max;
        }

        ENGINE_API void __stdcall SetParticleLifetime(uint32_t sceneObjectID, float minLife, float maxLife)
        {
            if (auto* e = getEmitter(sceneObjectID))
            {
                e->lifetimeMin = minLife;
                e->lifetimeMax = maxLife;
            }
        }

        ENGINE_API void __stdcall SetParticleSize(uint32_t sceneObjectID, float minSize, float maxSize)
        {
            if (auto* e = getEmitter(sceneObjectID))
            {
                e->sizeMin = minSize;
                e->sizeMax = maxSize;
            }
        }

        ENGINE_API void __stdcall SetParticleVelocity(uint32_t sceneObjectID, float minX, float minY, float maxX, float maxY)
        {
            if (auto* e = getEmitter(sceneObjectID))
            {
                e->velocityMin = { minX, minY };
                e->velocityMax = { maxX, maxY };
            }
        }

        ENGINE_API void __stdcall SetParticleColorStart(uint32_t sceneObjectID, float r, float g, float b, float a)
        {
            if (auto* e = getEmitter(sceneObjectID))
            {
                e->colorStartR = r; e->colorStartG = g;
                e->colorStartB = b; e->colorStartA = a;
            }
        }

        ENGINE_API void __stdcall SetParticleColorEnd(uint32_t sceneObjectID, float r, float g, float b, float a)
        {
            if (auto* e = getEmitter(sceneObjectID))
            {
                e->colorEndR = r; e->colorEndG = g;
                e->colorEndB = b; e->colorEndA = a;
            }
        }

        ENGINE_API void __stdcall SetParticleGravity(uint32_t sceneObjectID, float gx, float gy)
        {
            if (auto* e = getEmitter(sceneObjectID)) e->gravity = { gx, gy };
        }

        ENGINE_API void __stdcall SetParticleDrag(uint32_t sceneObjectID, float drag)
        {
            if (auto* e = getEmitter(sceneObjectID)) e->drag = drag;
        }

        ENGINE_API bool __stdcall GetParticleIsEmitting(uint32_t sceneObjectID)
        {
            auto* e = getEmitter(sceneObjectID);
            return e ? e->isEmitting : false;
        }
    }
}
