/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         video_component.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         09-03-2026
* @brief        Component for attaching a loaded video to an entity.
*//*========================================================================================*/



#pragma once

#include <cstring>
#include <unordered_map>
#include <string>
#include "../../event/signals.hpp"

namespace ECSComponents
{
    struct VideoComponent
    {
        char videoName[240] = {};
        bool autoPlay = false;
        bool loop     = false;

        // Runtime flag — true while the engine's game-pause is suppressing this video.
        // Not serialized; reset to false on stop/reload.
        bool pausedByEngine_ = false;

        std::unordered_map<std::string, Events::EnoSignal> signals_;

        VideoComponent() { memset(videoName, 0, sizeof(videoName)); }

        VideoComponent(const VideoComponent& other)
            : autoPlay(other.autoPlay), loop(other.loop), pausedByEngine_(false)
        {
            strncpy_s(videoName, other.videoName, sizeof(videoName) - 1);
            videoName[sizeof(videoName) - 1] = '\0';
        }

        VideoComponent& operator=(const VideoComponent& other)
        {
            if (this != &other)
            {
                strncpy_s(videoName, other.videoName, sizeof(videoName) - 1);
                videoName[sizeof(videoName) - 1] = '\0';
                autoPlay        = other.autoPlay;
                loop            = other.loop;
                pausedByEngine_ = false; // always reset runtime state on copy
            }
            return *this;
        }

        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("Video Name", videoName, "texchar240");
            l("Auto Play",  autoPlay,  "bool");
            l("Loop",       loop,      "bool");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("Video Name", videoName, "texchar240");
            l("Auto Play",  autoPlay,  "bool");
            l("Loop",       loop,      "bool");
        }
    };
}
