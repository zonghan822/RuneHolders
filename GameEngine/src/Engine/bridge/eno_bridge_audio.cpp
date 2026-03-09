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
            auto* a = getAudio(sceneObjectID);
            if (!a) return;
            // Reset flag so audioMgr.play() doesn't skip via its already-playing guard.
            // This mirrors the old PlayEntityAudio behaviour.
            a->isPlaying_ = false;
            gEngine->getAudioManager().play(*a);
        }

        ENGINE_API void __stdcall AudioStop(uint32_t sceneObjectID)
        {
            auto* a = getAudio(sceneObjectID);
            if (!a) return;
            if (a->audioMod_.ch_)
            {
                a->audioMod_.ch_->stop();
                a->audioMod_.ch_ = nullptr;
            }
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

        ENGINE_API void __stdcall SetGroupVolume(const char* groupName, float volume)
        {
            if (!gEngine) return;
            try
            {
                FMOD::ChannelGroup* group = gEngine->getAudioManager().getChannelGroup(std::string(groupName));
                group->setVolume(volume / 100.0f);
            }
            catch (...) {}
        }

        ENGINE_API float __stdcall GetGroupVolume(const char* groupName)
        {
            if (!gEngine) return 0.0f;
            try
            {
                FMOD::ChannelGroup* group = gEngine->getAudioManager().getChannelGroup(std::string(groupName));
                float vol = 0.0f;
                group->getVolume(&vol);
                return vol * 100.0f;
            }
            catch (...) {}
            return 0.0f;
        }

        ENGINE_API void __stdcall SetMasterVolume(float volume)
        {
            if (!gEngine) return;
            FMOD::ChannelGroup* master = nullptr;
            gEngine->getAudioManager().getSystem()->getMasterChannelGroup(&master);
            if (master) master->setVolume(volume / 100.0f);
        }

        ENGINE_API float __stdcall GetMasterVolume()
        {
            if (!gEngine) return 0.0f;
            FMOD::ChannelGroup* master = nullptr;
            gEngine->getAudioManager().getSystem()->getMasterChannelGroup(&master);
            if (!master) return 0.0f;
            float vol = 0.0f;
            master->getVolume(&vol);
            return vol * 100.0f;
        }
    }
}
