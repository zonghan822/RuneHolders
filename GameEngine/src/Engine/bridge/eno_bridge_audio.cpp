/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_audio.cpp
* @author       Koe Yun Ji (yunji.koe@digipen.edu)   57%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   29%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   14%
* @brief        Audio component bridge — play, stop, volume, pitch.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/audio.hpp"
#include "../settings/eno_config.hpp"
#include <cmath>

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

        // Returns 1 if the FMOD channel is still playing, 0 if it has finished or never started.
        ENGINE_API int __stdcall IsAudioPlaying(uint32_t sceneObjectID)
        {
            auto* a = getAudio(sceneObjectID);
            if (!a || !a->audioMod_.ch_) return 0;
            bool playing = false;
            a->audioMod_.ch_->isPlaying(&playing);
            return playing ? 1 : 0;
        }

        ENGINE_API float __stdcall GetAudioVolume(uint32_t sceneObjectID)
        {
            auto* a = getAudio(sceneObjectID);
            return a ? static_cast<float>(a->volume_) : 0.0f;
        }

        ENGINE_API void __stdcall SetAudioVolume(uint32_t sceneObjectID, float volume)
        {
            if (auto* a = getAudio(sceneObjectID))
            {
                a->volume_ = static_cast<double>(volume);
                // Also update the live FMOD channel so the change takes effect immediately
                if (a->audioMod_.ch_)
                    a->audioMod_.ch_->setVolume(static_cast<float>(volume / 100.0));
            }
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

        ENGINE_API void __stdcall SetAudioSpatial(uint32_t sceneObjectID, int32_t spatial)
        {
            if (auto* a = getAudio(sceneObjectID))
                a->spatialAudio_ = (spatial != 0);
        }

        ENGINE_API void __stdcall SetAudioMaxDistance(uint32_t sceneObjectID, float distance)
        {
            if (auto* a = getAudio(sceneObjectID))
                a->maxAudibleDistance_ = distance;
        }

        ENGINE_API void __stdcall SetAudioPaused(bool paused)
        {
            if (!gEngine) return;
            gEngine->getAudioManager().togglePauseAllGroups(paused);
        }

        ENGINE_API void __stdcall SetGroupPaused(const char* groupName, bool paused)
        {
            if (!gEngine) return;
            try { gEngine->getAudioManager().setPauseGroup(std::string(groupName), paused); }
            catch (...) {}
        }

        ENGINE_API void __stdcall SetAudioGroup(uint32_t sceneObjectID, const char* groupName)
        {
            auto* a = getAudio(sceneObjectID);
            if (!a) return;
            std::strncpy(a->audioMod_.chGroup_, groupName, sizeof(a->audioMod_.chGroup_) - 1);
            a->audioMod_.chGroup_[sizeof(a->audioMod_.chGroup_) - 1] = '\0';
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

        // Saves master/BGM/SFX volumes (0-100) to the project config file.
        ENGINE_API void __stdcall SaveAudioVolumes(float master, float bgm, float sfx)
        {
            if (!gEngine) return;
            auto& cfg = gEngine->getConfigManager();
            cfg.set("Audio Master", std::to_string(master));
            cfg.set("Audio BGM",    std::to_string(bgm));
            cfg.set("Audio SFX",    std::to_string(sfx));
            cfg.write();
        }

        // Play a looping sound by name. The channel is tracked internally so it
        // can be stopped later with AudioStopInstant. Calling again while already
        // playing restarts the sound from the beginning.
        ENGINE_API void __stdcall AudioPlayInstantLoop(const char* groupName, const char* soundName)
        {
            if (!gEngine) return;
            gEngine->getEventBus().publish(Audio::PlayInstantLoop{ std::string(groupName), std::string(soundName) });
        }

        // Stop a looping sound previously started with AudioPlayInstantLoop.
        ENGINE_API void __stdcall AudioStopInstant(const char* soundName)
        {
            if (!gEngine) return;
            gEngine->getEventBus().publish(Audio::StopInstant{ std::string(soundName) });
        }

        // Loads one audio volume value (0-100) from the config file by key.
        // Returns defaultVal if the key is absent or cannot be parsed.
        ENGINE_API float __stdcall LoadAudioVolume(const char* key, float defaultVal)
        {
            if (!gEngine) return defaultVal;
            std::string val = gEngine->getConfigManager().get(std::string(key));
            if (val.empty()) return defaultVal;
            try { return std::stof(val); }
            catch (...) { return defaultVal; }
        }

        // Apply or remove a simple low-pass filter on a channel group.
        // cutoff > 0: insert filter at that frequency (Hz); cutoff <= 0: remove the filter.
        ENGINE_API void __stdcall SetGroupLowPass(const char* groupName, float cutoff)
        {
            if (!gEngine) return;
            try
            {
                gEngine->getAudioManager().setGroupLowPassFilter(std::string(groupName), cutoff);
            }
            catch (...) {}
        }

        // Play a sound immediately by name without needing a scene entity.
        // groupName: "SFX" or "BGM", soundName: asset name e.g. "Typing_01"
        ENGINE_API void __stdcall AudioPlayInstant(uint32_t sceneObjectID, float x, float y)
        {
            if (!gEngine) return;

            // DEBUG: Verify AudioPlayInstant is being called and with what world position
            std::cout << "[DEBUG][Spatial] AudioPlayInstant called: entityID=" << sceneObjectID
                      << "  soundPos=(" << x << ", " << y << ")" << std::endl;

            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
            ECS::EntityManager& entMng = gEngine->getEntityManager();

            // Get Audio Component of source
            auto& ac = node->entity_.get<ECSComponents::AudioComponent>(entMng);

            // DEBUG: Show audio component state before play
            std::cout << "[DEBUG][Spatial]   AudioComponent: isPlaying=" << ac.isPlaying_
                      << "  spatialAudio=" << ac.spatialAudio_
                      << "  ch=" << (void*)ac.audioMod_.ch_
                      << "  maxDist=" << ac.maxAudibleDistance_ << std::endl;

            // Construct audio source vec2
            Math::eVec2 v2{ x, y };

            // Play relative to the camera
            std::vector<Editor::Node*> nodes = gEngine->getEnoEditor().getSceneTree().getAllNodes();
            bool foundCamera = false;
            for (const auto& camNode : nodes)
            {
                // Find the first entity with a camera and play relative to it
                if (camNode->entity_.has<ECSComponents::CameraComponent>(entMng))
                {
                    // Get camera pos
                    auto camPos = camNode->entity_.get<ECSComponents::CameraComponent>(entMng).position;

                    // DEBUG: Show camera position and calculated spatial values
                    float dx = x - camPos.x;
                    float dy = y - camPos.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    float maxDist = ac.maxAudibleDistance_ > 0.0f ? ac.maxAudibleDistance_ : 1.0f;
                    float expectedPan = std::max(-1.0f, std::min(dx / maxDist, 1.0f));
                    float expectedVol = (1.0f - std::min(dist / maxDist, 1.0f)) * static_cast<float>(ac.volume_ / 100.0);
                    std::cout << "[DEBUG][Spatial]   Camera at (" << camPos.x << ", " << camPos.y << ")"
                              << "  dx=" << dx << "  dy=" << dy << "  dist=" << dist << std::endl;
                    std::cout << "[DEBUG][Spatial]   Expected pan=" << expectedPan
                              << "  expectedVol=" << expectedVol
                              << "  (NOTE: updateSpatialAudio will skip if isPlaying_=false or ch_=null)" << std::endl;

                    foundCamera = true;
                    gEngine->getAudioManager().play(ac, v2, camPos);

                    // DEBUG: Show what pan/volume actually got applied after play()
                    float actualVol = -1.0f, actualPan = -999.0f;
                    if (ac.audioMod_.ch_)
                    {
                        ac.audioMod_.ch_->getVolume(&actualVol);
                        // FMOD has no getPan — read panScale_ instead
                    }
                    std::cout << "[DEBUG][Spatial]   After play(): isPlaying=" << ac.isPlaying_
                              << "  ch=" << (void*)ac.audioMod_.ch_
                              << "  actualVol=" << actualVol
                              << "  storedPanScale=" << ac.panScale_
                              << " (if pan=0.0 spatial pan was NOT applied)" << std::endl;
                }
            }
            if (!foundCamera)
                std::cout << "[DEBUG][Spatial]   WARNING: No camera entity found — spatial audio skipped entirely!" << std::endl;
        }

        // Assign a sound file to a sound component
        ENGINE_API void __stdcall SetSound(uint32_t id, const char* soundName)
        {
            if (!gEngine) return;

            FMOD::Sound* sound{};

            // Get requested sound
            auto& sounds = gEngine->getAudioManager().getSounds();
            auto it = sounds.find(soundName);
            if (it == sounds.end())
            {
                // Request sound from loader
                gEngine->getAudioManager().publishSoundRequest(soundName);

                // Verify that requested sound can be found
                it = sounds.find(soundName);
                if (it == sounds.end())
                {
                    std::cout << "Requested sound could not be retrieved" << std::endl;
                    return;
                }
            }

            sound = it->second;

            // Assign sound
            ECS::EntityManager& entMng = gEngine->getEntityManager();
            Editor::EnoSceneTree& tree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = tree.getNodeBySceneObjectID(id);

            if (node)
            {
                if (node->entity_.has<ECSComponents::AudioComponent>(entMng))
                    node->entity_.get<ECSComponents::AudioComponent>(entMng).audioMod_.sound_ = sound;
                else
                    std::cerr << "No audio component available to the node that is being set a sound with!" << std::endl;
            }
            else
            {
                std::cerr << "Invalid node ID passed for setting a sound!" << std::endl;
            }
        }
    }
}
