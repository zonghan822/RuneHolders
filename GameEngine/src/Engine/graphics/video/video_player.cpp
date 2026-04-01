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
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "fmod.hpp"

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

        // Find video and audio streams
        videoStreamIndex_ = -1;
        audioStreamIndex_ = -1;
        for (unsigned i = 0; i < formatCtx_->nb_streams; ++i)
        {
            auto type = formatCtx_->streams[i]->codecpar->codec_type;
            if (type == AVMEDIA_TYPE_VIDEO && videoStreamIndex_ < 0)
                videoStreamIndex_ = static_cast<int>(i);
            else if (type == AVMEDIA_TYPE_AUDIO && audioStreamIndex_ < 0)
                audioStreamIndex_ = static_cast<int>(i);
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

        // Open audio codec if an audio stream exists
        if (audioStreamIndex_ >= 0)
        {
            AVStream* aStream = formatCtx_->streams[audioStreamIndex_];
            const AVCodec* aCodec = avcodec_find_decoder(aStream->codecpar->codec_id);
            if (aCodec)
            {
                audioCodecCtx_ = avcodec_alloc_context3(aCodec);
                avcodec_parameters_to_context(audioCodecCtx_, aStream->codecpar);
                if (avcodec_open2(audioCodecCtx_, aCodec, nullptr) == 0)
                {
                    // Resample to interleaved float stereo 44100 Hz
                    audioSampleRate_ = 44100;
                    audioChannels_   = 2;

                    // Use av_channel_layout_default to avoid C99 designated-initialiser
                    // syntax that is not valid C++20 (nested .u.mask form).
                    AVChannelLayout stereoLayout{};
                    av_channel_layout_default(&stereoLayout, audioChannels_);

                    swr_alloc_set_opts2(&swrCtx_,
                        &stereoLayout,               AV_SAMPLE_FMT_FLT, audioSampleRate_,
                        &audioCodecCtx_->ch_layout,  audioCodecCtx_->sample_fmt,
                                                     audioCodecCtx_->sample_rate,
                        0, nullptr);
                    swr_init(swrCtx_);
                    audioFrame_ = av_frame_alloc();

                    decodeAllAudio();  // buffer entire audio track into audioPcm_
                }
                else
                {
                    avcodec_free_context(&audioCodecCtx_);
                }
            }
        }

        // Decode the first video frame so getTexture() is valid immediately after load().
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
        // Stop and release FMOD resources first
        if (fmodChannel_) { fmodChannel_->stop(); fmodChannel_ = nullptr; }
        if (fmodSound_)   { fmodSound_->release(); fmodSound_ = nullptr; }

        if (rgbaBuffer_)   { av_free(rgbaBuffer_);          rgbaBuffer_   = nullptr; }
        if (packet_)       { av_packet_free(&packet_); }
        if (frame_)        { av_frame_free(&frame_);        frame_        = nullptr; }
        if (rgbaFrame_)    { av_frame_free(&rgbaFrame_);    rgbaFrame_    = nullptr; }
        if (audioFrame_)   { av_frame_free(&audioFrame_);   audioFrame_   = nullptr; }
        if (swsCtx_)       { sws_freeContext(swsCtx_);      swsCtx_       = nullptr; }
        if (swrCtx_)       { swr_free(&swrCtx_);            swrCtx_       = nullptr; }
        if (audioCodecCtx_){ avcodec_free_context(&audioCodecCtx_); }
        if (codecCtx_)     { avcodec_free_context(&codecCtx_); }
        if (formatCtx_)    { avformat_close_input(&formatCtx_); }

        audioPcm_.clear();
        texture_.reset();
        loaded_            = false;
        state_             = VideoState::IDLE;
        currentTime_       = 0.0f;
        nextFramePts_      = 0.0f;
        videoStreamIndex_  = -1;
        audioStreamIndex_  = -1;
        width_ = height_   = 0;
    }

    // -------------------------------------------------------------------------
    void VideoPlayer::play(FMOD::System* fmodSystem)
    {
        if (!loaded_) return;

        // If resuming from pause, just unpause the FMOD channel
        if (state_ == VideoState::PAUSED && fmodChannel_)
        {
            fmodChannel_->setPaused(false);
            state_ = VideoState::PLAYING;
            return;
        }

        state_ = VideoState::PLAYING;

        // Create and play FMOD sound from the pre-decoded PCM buffer
        if (!fmodSystem)
        {
            std::cout << "[VideoPlayer] play() called without FMOD system — no audio\n";
        }
        else if (audioPcm_.empty())
        {
            std::cout << "[VideoPlayer] play() audioPcm_ is empty — audio decode failed or no audio stream\n";
        }
        else
        {
            // Release any previous sound
            if (fmodChannel_) { fmodChannel_->stop(); fmodChannel_ = nullptr; }
            if (fmodSound_)   { fmodSound_->release(); fmodSound_ = nullptr; }

            FMOD_CREATESOUNDEXINFO exinfo{};
            exinfo.cbsize            = sizeof(exinfo);
            exinfo.length            = static_cast<unsigned int>(audioPcm_.size() * sizeof(float));
            exinfo.numchannels       = audioChannels_;
            exinfo.defaultfrequency  = audioSampleRate_;
            exinfo.format            = FMOD_SOUND_FORMAT_PCMFLOAT;

            FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_OPENRAW | FMOD_CREATESAMPLE;
            if (looping_) mode |= FMOD_LOOP_NORMAL;

            std::cout << "[VideoPlayer] Creating FMOD sound: " << audioPcm_.size()
                      << " floats, " << audioChannels_ << " ch, " << audioSampleRate_ << " Hz\n";

            FMOD_RESULT res = fmodSystem->createSound(
                reinterpret_cast<const char*>(audioPcm_.data()),
                mode, &exinfo, &fmodSound_);

            if (res != FMOD_OK)
            {
                std::cerr << "[VideoPlayer] createSound failed: " << res << "\n";
            }
            else
            {
                res = fmodSystem->playSound(fmodSound_, nullptr, false, &fmodChannel_);
                if (res != FMOD_OK)
                    std::cerr << "[VideoPlayer] playSound failed: " << res << "\n";
                else
                    std::cout << "[VideoPlayer] Audio playback started\n";
            }
        }
    }

    void VideoPlayer::pause()
    {
        if (state_ == VideoState::PLAYING)
        {
            state_ = VideoState::PAUSED;
            if (fmodChannel_) fmodChannel_->setPaused(true);
        }
    }

    void VideoPlayer::stop()
    {
        if (!loaded_) return;
        if (fmodChannel_) { fmodChannel_->stop(); fmodChannel_ = nullptr; }
        if (fmodSound_)   { fmodSound_->release(); fmodSound_ = nullptr; }
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
    void VideoPlayer::decodeAllAudio()
    {
        if (!audioCodecCtx_ || !swrCtx_ || !audioFrame_ || !formatCtx_) return;

        // Seek to the beginning of the file so we can read all packets
        av_seek_frame(formatCtx_, -1, 0, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(audioCodecCtx_);

        audioPcm_.clear();

        AVPacket* pkt = av_packet_alloc();
        while (av_read_frame(formatCtx_, pkt) >= 0)
        {
            if (pkt->stream_index != audioStreamIndex_)
            {
                av_packet_unref(pkt);
                continue;
            }

            if (avcodec_send_packet(audioCodecCtx_, pkt) == 0)
            {
                while (avcodec_receive_frame(audioCodecCtx_, audioFrame_) == 0)
                {
                    // Resample to float stereo
                    int outSamples = static_cast<int>(
                        av_rescale_rnd(
                            swr_get_delay(swrCtx_, audioCodecCtx_->sample_rate) + audioFrame_->nb_samples,
                            audioSampleRate_, audioCodecCtx_->sample_rate, AV_ROUND_UP));

                    std::vector<float> tmp(static_cast<size_t>(outSamples) * audioChannels_);
                    uint8_t* outPtr = reinterpret_cast<uint8_t*>(tmp.data());
                    int converted = swr_convert(swrCtx_, &outPtr, outSamples,
                        const_cast<const uint8_t**>(audioFrame_->data), audioFrame_->nb_samples);

                    if (converted > 0)
                        audioPcm_.insert(audioPcm_.end(), tmp.begin(),
                                         tmp.begin() + converted * audioChannels_);

                    av_frame_unref(audioFrame_);
                }
            }
            av_packet_unref(pkt);
        }
        av_packet_free(&pkt);

        // Flush resampler
        int remaining = static_cast<int>(swr_get_delay(swrCtx_, audioCodecCtx_->sample_rate));
        if (remaining > 0)
        {
            std::vector<float> tmp(static_cast<size_t>(remaining) * audioChannels_);
            uint8_t* outPtr = reinterpret_cast<uint8_t*>(tmp.data());
            int converted = swr_convert(swrCtx_, &outPtr, remaining, nullptr, 0);
            if (converted > 0)
                audioPcm_.insert(audioPcm_.end(), tmp.begin(),
                                 tmp.begin() + converted * audioChannels_);
        }

        // Seek video stream back to start so decodeNextFrame() begins from frame 0
        av_seek_frame(formatCtx_, videoStreamIndex_, 0, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(codecCtx_);

        std::cout << "[VideoPlayer] Decoded " << audioPcm_.size() / audioChannels_
                  << " audio samples (" << audioSampleRate_ << " Hz, "
                  << audioChannels_ << " ch)\n";
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
