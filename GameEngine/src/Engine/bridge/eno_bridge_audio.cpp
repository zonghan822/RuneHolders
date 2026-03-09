/*============================================================================================
* @file         eno_bridge_audio.cpp
* @brief        Audio component bridge — play, stop, volume, pitch.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/audio.hpp"

namespace Bridge
{
    static ECSComponents::AudioComponent* getAudio(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::AudioComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::AudioComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        ENGINE_API void __stdcall AudioPlay(uint32_t sceneObjectID)
        {
            if (auto* a = getAudio(sceneObjectID))
                a->isPlaying_ = true;
        }

        ENGINE_API void __stdcall AudioStop(uint32_t sceneObjectID)
        {
            if (auto* a = getAudio(sceneObjectID))
                a->isPlaying_ = false;
        }

        ENGINE_API float __stdcall GetAudioVolume(uint32_t sceneObjectID)
        {
            auto* a = getAudio(sceneObjectID);
            return a ? static_cast<float>(a->volume_) : 0.0f;
        }

        ENGINE_API void __stdcall SetAudioVolume(uint32_t sceneObjectID, float volume)
        {
            if (auto* a = getAudio(sceneObjectID))
                a->volume_ = static_cast<double>(volume);
        }

        ENGINE_API float __stdcall GetAudioPitch(uint32_t sceneObjectID)
        {
            auto* a = getAudio(sceneObjectID);
            return a ? static_cast<float>(a->pitchScale_) : 1.0f;
        }

        ENGINE_API void __stdcall SetAudioPitch(uint32_t sceneObjectID, float pitch)
        {
            if (auto* a = getAudio(sceneObjectID))
                a->pitchScale_ = static_cast<double>(pitch);
        }
    }
}
