/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_video.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         09-03-2026
* @brief        Video component bridge — play, pause, stop, query state/time.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/video_component.hpp"
#include "../graphics/video/video_player.hpp"

namespace Bridge
{
    // Returns the VideoComponent for the given scene object, or nullptr.
    static ECSComponents::VideoComponent* getVideoComp(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::VideoComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::VideoComponent>(gEngine->getEntityManager());
    }

    // Returns the VideoPlayer associated with the entity's VideoComponent, or nullptr.
    static Graphics::VideoPlayer* getVideoPlayer(uint32_t sceneObjectID)
    {
        auto* comp = getVideoComp(sceneObjectID);
        if (!comp || strlen(comp->videoName) == 0) return nullptr;
        auto player = gEngine->getGraphicsManager().getVideo(std::string(comp->videoName));
        return player ? player.get() : nullptr;
    }

    extern "C"
    {
        // Start (or resume) playback of the entity's video.
        ENGINE_API void __stdcall VideoPlay(uint32_t sceneObjectID)
        {
            auto* comp = getVideoComp(sceneObjectID);
            if (!comp || strlen(comp->videoName) == 0) return;
            auto player = gEngine->getGraphicsManager().getVideo(std::string(comp->videoName));
            if (player && player->isLoaded())
            {
                player->setLooping(comp->loop);
                player->play(gEngine->getAudioManager().getSystem());
                comp->pausedByEngine_ = false;
            }
        }

        // Pause playback without rewinding.
        ENGINE_API void __stdcall VideoPause(uint32_t sceneObjectID)
        {
            if (auto* p = getVideoPlayer(sceneObjectID))
                p->pause();
        }

        // Stop playback and rewind to the beginning (initial serialised state).
        ENGINE_API void __stdcall VideoStop(uint32_t sceneObjectID)
        {
            if (auto* comp = getVideoComp(sceneObjectID))
                comp->pausedByEngine_ = false;
            if (auto* p = getVideoPlayer(sceneObjectID))
                p->stop();
        }

        // Returns true while the video is actively playing.
        ENGINE_API bool __stdcall IsVideoPlaying(uint32_t sceneObjectID)
        {
            auto* p = getVideoPlayer(sceneObjectID);
            return p && p->getState() == Graphics::VideoState::PLAYING;
        }

        // Current playback position in seconds.
        ENGINE_API float __stdcall GetVideoTime(uint32_t sceneObjectID)
        {
            auto* p = getVideoPlayer(sceneObjectID);
            return p ? p->getCurrentTime() : 0.0f;
        }

        // Total duration of the video in seconds.
        ENGINE_API float __stdcall GetVideoDuration(uint32_t sceneObjectID)
        {
            auto* p = getVideoPlayer(sceneObjectID);
            return p ? p->getDuration() : 0.0f;
        }

        // Enable or disable looping for this entity's video.
        ENGINE_API void __stdcall SetVideoLoop(uint32_t sceneObjectID, bool loop)
        {
            auto* comp = getVideoComp(sceneObjectID);
            if (!comp) return;
            comp->loop = loop;
            auto player = gEngine->getGraphicsManager().getVideo(std::string(comp->videoName));
            if (player) player->setLooping(loop);
        }

        // Returns true if looping is enabled.
        ENGINE_API bool __stdcall GetVideoLoop(uint32_t sceneObjectID)
        {
            auto* comp = getVideoComp(sceneObjectID);
            return comp ? comp->loop : false;
        }
    }
}
