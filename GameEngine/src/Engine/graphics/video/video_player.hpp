/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         video_player.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         07-03-2026
* @brief        FFmpeg-backed video player that decodes frames and streams them to an
*//*========================================================================================*/



#pragma once

#include <string>
#include <memory>
#include <cstdint>

// Forward-declare FFmpeg structs so consumers don't need to include FFmpeg headers.
struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
struct AVFrame;
struct AVPacket;

// Forward-declare EnoTexture to avoid circular include with graphics.hpp.
namespace Graphics { class EnoTexture; }

namespace Graphics
{
    enum class VideoState { IDLE, PLAYING, PAUSED, ENDED };

    /*============================================================================================
    * \class   VideoPlayer
    * \brief   Decodes a video file frame-by-frame using FFmpeg and uploads each frame as an
    *          RGBA texture (EnoTexture) that can be drawn or previewed via ImGui.
    *
    *          Typical usage:
    *            auto vp = std::make_shared<VideoPlayer>();
    *            vp->load("assets/video/intro.mp4");
    *            vp->play();
    *            // each frame:
    *            vp->update(deltaTime);
    *            auto tex = vp->getTexture(); // draw with drawTexturedQuad or ImGui::Image
    *//*========================================================================================*/
    class VideoPlayer
    {
    public:
        VideoPlayer();
        ~VideoPlayer();

        // Non-copyable (owns FFmpeg contexts).
        VideoPlayer(const VideoPlayer&) = delete;
        VideoPlayer& operator=(const VideoPlayer&) = delete;

        // Opens and prepares the video file. Returns false on failure.
        bool load(const std::string& path);

        // Releases all FFmpeg and OpenGL resources.
        void unload();

        // Playback controls.
        void play();
        void pause();
        void stop(); // rewinds and returns to IDLE

        // Advance playback by dt seconds; uploads the current frame to the texture when needed.
        // Must be called every frame while the player is in PLAYING state.
        void update(float dt);

        // --- Getters ---
        VideoState getState()       const { return state_; }
        float      getDuration()    const { return duration_; }
        float      getCurrentTime() const { return currentTime_; }
        int        getWidth()       const { return width_; }
        int        getHeight()      const { return height_; }
        bool       isLoaded()       const { return loaded_; }
        const std::string& getFilePath() const { return filePath_; }

        // The current decoded frame as a shareable OpenGL texture.
        // May be nullptr before the first frame has been decoded.
        std::shared_ptr<EnoTexture> getTexture() const { return texture_; }

        void setLooping(bool loop) { looping_ = loop; }
        bool isLooping() const     { return looping_; }

    private:
        // Reads packets until a video frame is decoded and converted to RGBA.
        // Returns false when the stream is exhausted.
        bool decodeNextFrame();

        // Seek back to the start of the stream.
        void rewind();

        // FFmpeg pipeline
        AVFormatContext* formatCtx_  = nullptr;
        AVCodecContext*  codecCtx_   = nullptr;
        SwsContext*      swsCtx_     = nullptr;
        AVFrame*         frame_      = nullptr;
        AVFrame*         rgbaFrame_  = nullptr;
        AVPacket*        packet_     = nullptr;
        uint8_t*         rgbaBuffer_ = nullptr;

        int   videoStreamIndex_ = -1;
        int   width_            = 0;
        int   height_           = 0;
        float duration_         = 0.0f;  // total duration in seconds
        float currentTime_      = 0.0f;  // playback cursor in seconds
        float timeBase_         = 0.0f;  // stream time-base in seconds/PTS-unit
        float nextFramePts_     = 0.0f;  // PTS of the last decoded frame in seconds

        bool loaded_  = false;
        bool looping_ = false;
        VideoState state_ = VideoState::IDLE;

        std::shared_ptr<EnoTexture> texture_;
        std::string filePath_;
    };

} // namespace Graphics
