#pragma once

#include "../../math/math.hpp"
#include <vector>
#include <string>

namespace Animation
{
    // Represents a single frame in an animation
    struct AnimationFrame
    {
        Math::eVec2<float> uvStart;   // Top-left UV coordinate (0.0 to 1.0)
        Math::eVec2<float> uvEnd;     // Bottom-right UV coordinate (0.0 to 1.0)
        float duration;                // Duration in seconds

        AnimationFrame(const Math::eVec2<float>& start = { 0.0f, 0.0f },
            const Math::eVec2<float>& end = { 1.0f, 1.0f },
            float dur = 0.1f)
            : uvStart(start), uvEnd(end), duration(dur) {
        }
    };

    // Animation class that manages a sequence of frames
    class EnoAnimation
    {
    public:
        EnoAnimation() : currentFrameIndex_(0), currentTime_(0.0f), looping_(true), playing_(true) {}

        // Add a frame to the animation
        void addFrame(const AnimationFrame& frame)
        {
            frames_.push_back(frame);
        }

        // Update animation with delta time and get current frame
        const AnimationFrame& update(float deltaTime)
        {
            if (!playing_ || frames_.empty())
            {
                static AnimationFrame defaultFrame;
                return frames_.empty() ? defaultFrame : frames_[currentFrameIndex_];
            }

            currentTime_ += deltaTime;

            // Check if we need to advance to next frame
            if (currentTime_ >= frames_[currentFrameIndex_].duration)
            {
                currentTime_ = 0.0f;
                currentFrameIndex_++;

                // Check if animation finished
                if (currentFrameIndex_ >= frames_.size())
                {
                    if (looping_)
                    {
                        currentFrameIndex_ = 0;
                    }
                    else
                    {
                        currentFrameIndex_ = frames_.size() - 1;
                        playing_ = false;
                    }
                }
            }

            return frames_[currentFrameIndex_];
        }

        // Get current frame without updating
        const AnimationFrame& getCurrentFrame() const
        {
            static AnimationFrame defaultFrame;
            return frames_.empty() ? defaultFrame : frames_[currentFrameIndex_];
        }

        // Control methods
        void reset()
        {
            currentFrameIndex_ = 0;
            currentTime_ = 0.0f;
            playing_ = true;
        }

        void play() { playing_ = true; }
        void pause() { playing_ = false; }
        void stop() { reset(); playing_ = false; }

        void setLooping(bool loop) { looping_ = loop; }
        bool isLooping() const { return looping_; }
        bool isPlaying() const { return playing_; }

        size_t getFrameCount() const { return frames_.size(); }
        size_t getCurrentFrameIndex() const { return currentFrameIndex_; }

    private:
        std::vector<AnimationFrame> frames_;
        size_t currentFrameIndex_;
        float currentTime_;
        bool looping_;
        bool playing_;
    };

    // Helper class to manage multiple animations for a sprite
    class AnimationController
    {
    public:
        AnimationController() : currentAnimation_(nullptr) {}

        // Add an animation with a name
        void addAnimation(const std::string& name, const EnoAnimation& animation)
        {
            animations_[name] = animation;
        }

        // Set the current animation
        bool setAnimation(const std::string& name)
        {
            auto it = animations_.find(name);
            if (it != animations_.end())
            {
                currentAnimationName_ = name;
                currentAnimation_ = &it->second;
                currentAnimation_->reset();
                return true;
            }
            return false;
        }

        // Update current animation
        const AnimationFrame& update(float deltaTime)
        {
            static AnimationFrame defaultFrame;
            if (currentAnimation_)
            {
                return currentAnimation_->update(deltaTime);
            }
            return defaultFrame;
        }

        // Get current frame without updating
        const AnimationFrame& getCurrentFrame() const
        {
            static AnimationFrame defaultFrame;
            if (currentAnimation_)
            {
                return currentAnimation_->getCurrentFrame();
            }
            return defaultFrame;
        }

        EnoAnimation* getCurrentAnimation() { return currentAnimation_; }
        const std::string& getCurrentAnimationName() const { return currentAnimationName_; }

        //yunqi moved private data members out for debug
        std::unordered_map<std::string, EnoAnimation> animations_;
        EnoAnimation* currentAnimation_;
        std::string currentAnimationName_;
    private:
    };



    //===========================  Yun Qi's implementation ==========================
    class AnimationControllerRegistry {
    public:
        // controllerID -> actual controller
        std::unordered_map<uint32_t, Animation::AnimationController> controllers; //this is where the entity's controller will be stored

        uint32_t createController() {
            uint32_t id = nextID++;
            controllers[id] = Animation::AnimationController();
            return id;
        }

        Animation::AnimationController* get(uint32_t id) {
            auto it = controllers.find(id);
            if (it != controllers.end()) return &it->second;
            return nullptr;
        }

        void remove(uint32_t id) {
            controllers.erase(id);
        }

    private:
        uint32_t nextID = 1;
    };




    // Helper function to create animations from sprite sheets
    inline EnoAnimation createSpriteSheetAnimation(
        int frameWidth, int frameHeight,
        int sheetWidth, int sheetHeight,
        int startFrame, int frameCount,
        float frameDuration)
    {
        EnoAnimation animation;

        int framesPerRow = sheetWidth / frameWidth;

        // Add padding to prevent sampling adjacent frames
        float paddingPixels = 1.f;  // 1 pixel padding 

        for (int i = 0; i < frameCount; ++i)
        {
            int frameIndex = startFrame + i;
            int row = frameIndex / framesPerRow;
            int col = frameIndex % framesPerRow;

            // Add padding to UV coordinates to avoid bleeding
            float uvStartX = (static_cast<float>(col * frameWidth) + paddingPixels) / static_cast<float>(sheetWidth);
            float uvStartY = (static_cast<float>(row * frameHeight) + paddingPixels) / static_cast<float>(sheetHeight);
            float uvEndX = (static_cast<float>((col + 1) * frameWidth) - paddingPixels) / static_cast<float>(sheetWidth);
            float uvEndY = (static_cast<float>((row + 1) * frameHeight) - paddingPixels) / static_cast<float>(sheetHeight);

            AnimationFrame frame(
                { uvStartX, uvStartY },
                { uvEndX, uvEndY },
                frameDuration
            );

            animation.addFrame(frame);
        }

        return animation;
    }


    inline EnoAnimation createAnimationFromTexture(const Graphics::TextureData& tex, float speed = 0.1f)
    {
        return createSpriteSheetAnimation(
            tex.frameWidth,
            tex.frameHeight,
            tex.texture->getWidth(),
            tex.texture->getHeight(),
            0,
            tex.rows * tex.cols,
            speed
        );
    }


}