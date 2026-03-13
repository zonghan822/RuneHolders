/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         video_player.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         07-03-2026
* @brief        FFmpeg video decode pipeline implementation.
*//*========================================================================================*/



// Include graphics.hpp BEFORE video_player.hpp so EnoTexture is fully defined.
#include "../graphics.hpp"
#include "video_player.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <iostream>

namespace Graphics
{
    VideoPlayer::VideoPlayer() = default;

    VideoPlayer::~VideoPlayer()
    {
        unload();
    }

    // -------------------------------------------------------------------------
    bool VideoPlayer::load(const std::string& path)
    {
        unload();
        filePath_ = path;

        // Open container
        if (avformat_open_input(&formatCtx_, path.c_str(), nullptr, nullptr) < 0)
        {
            std::cerr << "[VideoPlayer] Cannot open file: " << path << "\n";
            return false;
        }

        if (avformat_find_stream_info(formatCtx_, nullptr) < 0)
        {
            std::cerr << "[VideoPlayer] Cannot find stream info\n";
            avformat_close_input(&formatCtx_);
            return false;
        }

        // Find the first video stream
        videoStreamIndex_ = -1;
        for (unsigned i = 0; i < formatCtx_->nb_streams; ++i)
        {
            if (formatCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex_ = static_cast<int>(i);
                break;
            }
        }

        if (videoStreamIndex_ < 0)
        {
            std::cerr << "[VideoPlayer] No video stream in: " << path << "\n";
            avformat_close_input(&formatCtx_);
            return false;
        }

        AVStream* stream = formatCtx_->streams[videoStreamIndex_];

        // Find and open codec
        const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec)
        {
            std::cerr << "[VideoPlayer] Codec not found\n";
            avformat_close_input(&formatCtx_);
            return false;
        }

        codecCtx_ = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codecCtx_, stream->codecpar);

        if (avcodec_open2(codecCtx_, codec, nullptr) < 0)
        {
            std::cerr << "[VideoPlayer] Failed to open codec\n";
            avcodec_free_context(&codecCtx_);
            avformat_close_input(&formatCtx_);
            return false;
        }

        width_    = codecCtx_->width;
        height_   = codecCtx_->height;
        timeBase_ = static_cast<float>(av_q2d(stream->time_base));

        if (formatCtx_->duration != AV_NOPTS_VALUE)
            duration_ = static_cast<float>(formatCtx_->duration) / AV_TIME_BASE;

        // Pixel-format conversion: native → RGBA
        swsCtx_ = sws_getContext(
            width_, height_, codecCtx_->pix_fmt,
            width_, height_, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!swsCtx_)
        {
            std::cerr << "[VideoPlayer] sws_getContext failed\n";
            avcodec_free_context(&codecCtx_);
            avformat_close_input(&formatCtx_);
            return false;
        }

        // Allocate frame buffers
        frame_     = av_frame_alloc();
        rgbaFrame_ = av_frame_alloc();
        packet_    = av_packet_alloc();

        int bufSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width_, height_, 1);
        rgbaBuffer_ = static_cast<uint8_t*>(av_malloc(bufSize));
        av_image_fill_arrays(
            rgbaFrame_->data, rgbaFrame_->linesize,
            rgbaBuffer_, AV_PIX_FMT_RGBA, width_, height_, 1);

        // Decode the first frame so getTexture() is valid immediately after load().
        decodeNextFrame();

        loaded_      = true;
        state_       = VideoState::IDLE;
        currentTime_ = 0.0f;

        std::cout << "[VideoPlayer] Loaded: " << path
                  << " (" << width_ << "x" << height_
                  << ", " << duration_ << "s)\n";
        return true;
    }

    // -------------------------------------------------------------------------
    void VideoPlayer::unload()
    {
        if (rgbaBuffer_) { av_free(rgbaBuffer_);     rgbaBuffer_ = nullptr; }
        if (packet_)     { av_packet_free(&packet_); }
        if (frame_)      { av_frame_free(&frame_);   }
        if (rgbaFrame_)  { av_frame_free(&rgbaFrame_); }
        if (swsCtx_)     { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
        if (codecCtx_)   { avcodec_free_context(&codecCtx_); }
        if (formatCtx_)  { avformat_close_input(&formatCtx_); }

        texture_.reset();
        loaded_           = false;
        state_            = VideoState::IDLE;
        currentTime_      = 0.0f;
        nextFramePts_     = 0.0f;
        videoStreamIndex_ = -1;
        width_ = height_  = 0;
    }

    // -------------------------------------------------------------------------
    void VideoPlayer::play()
    {
        if (loaded_) state_ = VideoState::PLAYING;
    }

    void VideoPlayer::pause()
    {
        if (state_ == VideoState::PLAYING) state_ = VideoState::PAUSED;
    }

    void VideoPlayer::stop()
    {
        if (!loaded_) return;
        rewind();
        state_       = VideoState::IDLE;
        currentTime_ = 0.0f;
    }

    // -------------------------------------------------------------------------
    void VideoPlayer::update(float dt)
    {
        if (state_ != VideoState::PLAYING || !loaded_) return;

        currentTime_ += dt;

        // Decode frames until we are at or past currentTime_.
        while (nextFramePts_ <= currentTime_)
        {
            if (!decodeNextFrame())
            {
                if (looping_)
                {
                    rewind();
                    currentTime_ = 0.0f;
                }
                else
                {
                    state_ = VideoState::ENDED;
                }
                break;
            }
        }
    }

    // -------------------------------------------------------------------------
    bool VideoPlayer::decodeNextFrame()
    {
        while (av_read_frame(formatCtx_, packet_) >= 0)
        {
            if (packet_->stream_index != videoStreamIndex_)
            {
                av_packet_unref(packet_);
                continue;
            }

            int ret = avcodec_send_packet(codecCtx_, packet_);
            av_packet_unref(packet_);
            if (ret < 0) continue;

            ret = avcodec_receive_frame(codecCtx_, frame_);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) continue;
            if (ret < 0) return false;

            // Convert pixel format to RGBA
            sws_scale(swsCtx_,
                frame_->data, frame_->linesize, 0, height_,
                rgbaFrame_->data, rgbaFrame_->linesize);

            // Record the PTS of this frame in seconds
            int64_t pts = (frame_->best_effort_timestamp != AV_NOPTS_VALUE)
                        ? frame_->best_effort_timestamp
                        : frame_->pts;
            nextFramePts_ = static_cast<float>(pts) * timeBase_;

            // Upload to EnoTexture — create on first frame, stream-update after
            if (!texture_ || !texture_->isValid())
            {
                texture_ = std::make_shared<EnoTexture>();
                texture_->loadFromMemory(width_, height_, 4, rgbaBuffer_);
            }
            else
            {
                texture_->updatePixels(width_, height_, 4, rgbaBuffer_);
            }

            return true;
        }
        return false; // stream exhausted
    }

    // -------------------------------------------------------------------------
    void VideoPlayer::rewind()
    {
        av_seek_frame(formatCtx_, videoStreamIndex_, 0, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(codecCtx_);
        nextFramePts_ = 0.0f;
    }

} // namespace Graphics
