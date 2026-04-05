/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   51%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   26%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   11%
* @brief        Master bridge header — declares all C-export functions grouped by system.
*//*========================================================================================*/



#pragma once

#include "../engine/eno_engine.hpp"

#if defined(ENGINE_BUILD_DLL)
    #define ENGINE_API __declspec(dllexport)
#elif defined(ENGINE_USE_DLL)
    #define ENGINE_API __declspec(dllimport)
#else
    #define ENGINE_API
#endif

// Global engine pointer — defined once in eno_bridge_core.cpp,
// used across all bridge translation units.
extern Engine::EnoEngine* gEngine;

namespace Bridge {

    extern "C" {

        // =========================================================================
        // Core / Initialisation
        // Implemented in: eno_bridge_core.cpp
        // =========================================================================
        /** @brief Initializes the bridge with a pointer to the engine instance. */
        ENGINE_API void    __stdcall Bridge_Init(void* enginePtr);

        // =========================================================================
        // Time
        // Implemented in: eno_bridge_core.cpp
        // =========================================================================
        /** @brief Returns the scaled delta time (affected by time scale) in seconds. */
        ENGINE_API float   __stdcall GetDeltaTime();
        /** @brief Returns the real (wall-clock) delta time, unaffected by time scale. */
        ENGINE_API float   __stdcall GetUnscaledDeltaTime();
        /** @brief Sets the global time scale (1.0 = normal, <1.0 = slow motion, 0 = frozen). */
        ENGINE_API void    __stdcall SetTimeScale(float scale);
        /** @brief Returns the current global time scale factor. */
        ENGINE_API float   __stdcall GetTimeScale();

        // =========================================================================
        // Node Management
        // Implemented in: eno_bridge_node.cpp
        // =========================================================================
        /** @brief Creates a new scene node of the given type and returns its ID. */
        ENGINE_API uint32_t __stdcall CreateNode(uint32_t type);
        /** @brief Destroys a scene node by its ID, freeing its resources. */
        ENGINE_API void __stdcall FreeNode(uint32_t id);
        /** @brief Finds a scene node by name and returns its ID, or UINT32_MAX if not found. */
        ENGINE_API uint32_t __stdcall GetNode(const char* name);
        /** @brief Returns the child node ID at the given index under a parent node. */
        ENGINE_API uint32_t __stdcall GetChild(uint32_t parentID, uint32_t index);
        /** @brief Attaches a child node to a parent node in the scene hierarchy. */
        ENGINE_API void __stdcall AddChild(uint32_t cid, uint32_t pid);
        /** @brief Removes a node from its parent, making it a root-level node. */
        ENGINE_API void __stdcall Unparent(uint32_t id);
        /** @brief Moves a child node from its current parent to a new parent. */
        ENGINE_API void __stdcall Reparent(uint32_t cid, uint32_t pid);
        /** @brief Returns the parent node ID, or UINT32_MAX if the node has no parent. */
        ENGINE_API uint32_t __stdcall GetParentNode(uint32_t id);
        /** @brief Returns the type identifier of the specified node. */
        ENGINE_API uint32_t __stdcall GetNodeType(uint32_t id);
        /** @brief Returns the number of direct children of the specified node. */
        ENGINE_API uint32_t __stdcall GetChildCount(uint32_t id);
        /** @brief Fills outIDs with scene object IDs of nodes matching nodeType; returns the count written. */
        ENGINE_API uint32_t __stdcall GetNodesByType(uint32_t nodeType, uint32_t* outIDs, uint32_t maxCount);

        // Spawn a user prefab at (x, y); parentID = UINT32_MAX for no parent.
        // Returns the sceneObjectID of the new node, or UINT32_MAX on failure.
        ENGINE_API uint32_t __stdcall SpawnPrefab(const char* prefabName, float x, float y, uint32_t parentID);

        // ENGINE_API uint32_t __stdcall SpawnNode(const char* presetName, const char* nodeName, uint32_t parentID);
        // ENGINE_API void     __stdcall DestroyNode(uint32_t sceneObjectID);

        // =========================================================================
        // Transform Component
        // Implemented in: eno_bridge_transform.cpp
        // =========================================================================
        /** @brief Returns the world-space X position of a scene object. */
        ENGINE_API float   __stdcall GetPositionX(uint32_t sceneObjectID);
        /** @brief Returns the world-space Y position of a scene object. */
        ENGINE_API float   __stdcall GetPositionY(uint32_t sceneObjectID);
        /** @brief Sets the world-space position of a scene object. */
        ENGINE_API void    __stdcall SetPosition(uint32_t sceneObjectID, float x, float y);
        /** @brief Sets the local (parent-relative) position of a scene object. */
        ENGINE_API void    __stdcall SetLocalPosition(uint32_t sceneObjectID, float x, float y);

        /** @brief Returns the X component of the scene object's rotation. */
        ENGINE_API float   __stdcall GetRotationX(uint32_t sceneObjectID);
        /** @brief Returns the Y component of the scene object's rotation. */
        ENGINE_API float   __stdcall GetRotationY(uint32_t sceneObjectID);
        /** @brief Sets the rotation of a scene object. */
        ENGINE_API void    __stdcall SetRotation(uint32_t sceneObjectID, float x, float y);

        /** @brief Returns the X component of the scene object's scale. */
        ENGINE_API float   __stdcall GetScaleX(uint32_t sceneObjectID);
        /** @brief Returns the Y component of the scene object's scale. */
        ENGINE_API float   __stdcall GetScaleY(uint32_t sceneObjectID);
        /** @brief Sets the scale of a scene object. */
        ENGINE_API void    __stdcall SetScale(uint32_t sceneObjectID, float x, float y);

        // =========================================================================
        // Physics Component
        // Implemented in: eno_bridge_physics.cpp
        // =========================================================================
        /** @brief Returns the X velocity of a scene object's physics body. */
        ENGINE_API float   __stdcall GetVelocityX(uint32_t sceneObjectID);
        /** @brief Returns the Y velocity of a scene object's physics body. */
        ENGINE_API float   __stdcall GetVelocityY(uint32_t sceneObjectID);
        /** @brief Sets the velocity of a scene object's physics body. */
        ENGINE_API void    __stdcall SetVelocity(uint32_t sceneObjectID, float x, float y);

        /** @brief Applies an instantaneous force to a scene object's physics body. */
        ENGINE_API void    __stdcall AddForce(uint32_t sceneObjectID, float x, float y);

        /** @brief Returns the mass of a scene object's physics body. */
        ENGINE_API float   __stdcall GetMass(uint32_t sceneObjectID);
        /** @brief Sets the mass of a scene object's physics body. */
        ENGINE_API void    __stdcall SetMass(uint32_t sceneObjectID, float mass);

        /** @brief Returns whether gravity is enabled for a scene object's physics body. */
        ENGINE_API bool    __stdcall GetUseGravity(uint32_t sceneObjectID);
        /** @brief Enables or disables gravity for a scene object's physics body. */
        ENGINE_API void    __stdcall SetUseGravity(uint32_t sceneObjectID, bool useGravity);

        /** @brief Returns whether a scene object's physics body is kinematic (not affected by forces). */
        ENGINE_API bool    __stdcall GetIsKinematic(uint32_t sceneObjectID);
        /** @brief Sets whether a scene object's physics body is kinematic. */
        ENGINE_API void    __stdcall SetIsKinematic(uint32_t sceneObjectID, bool kinematic);

        // =========================================================================
        // Visibility Component
        // Implemented in: eno_bridge_visibility.cpp
        // =========================================================================
        /** @brief Returns the Z-index (draw order) of a scene object. */
        ENGINE_API uint32_t __stdcall GetZIndex(uint32_t sceneObjectID);
        /** @brief Sets the Z-index (draw order) of a scene object. */
        ENGINE_API void     __stdcall SetZIndex(uint32_t sceneObjectID, uint32_t z);
        /** @brief Returns whether a scene object is currently visible. */
        ENGINE_API bool     __stdcall GetVisibility(uint32_t sceneObjectID);
        /** @brief Sets the visibility of a scene object. */
        ENGINE_API void     __stdcall SetVisibility(uint32_t sceneObjectID, bool visible);

        // =========================================================================
        // Canvas Component
        // Implemented in: eno_bridge_canvas.cpp
        // =========================================================================
        /** @brief Returns whether a canvas UI element is active. */
        ENGINE_API bool __stdcall GetCanvasActive(uint32_t sceneObjectID);
        /** @brief Activates or deactivates a canvas UI element. */
        ENGINE_API void __stdcall SetCanvasActive(uint32_t sceneObjectID, bool active);

        // =========================================================================
        // Input — Keyboard
        // Implemented in: eno_bridge_input.cpp
        // =========================================================================
        /** @brief Returns true if the key was pressed this frame. */
        ENGINE_API bool    __stdcall IsKeyPressed(int key);
        /** @brief Returns true if the key is currently held down. */
        ENGINE_API bool    __stdcall IsKeyHeld(int key);
        /** @brief Returns true if the key was released this frame. */
        ENGINE_API bool    __stdcall IsKeyReleased(int key);
        /** @brief Returns the currently active modifier key bitmask (shift, ctrl, alt, etc.). */
        ENGINE_API int     __stdcall ModsApplied();

        // =========================================================================
        // Input — Mouse
        // Implemented in: eno_bridge_input.cpp
        // =========================================================================
        /** @brief Returns true if the mouse button was pressed this frame. */
        ENGINE_API bool    __stdcall IsMousePressed(int button);
        /** @brief Returns true if the mouse button is currently held down. */
        ENGINE_API bool    __stdcall IsMouseHeld(int button);
        /** @brief Returns true if the mouse button was released this frame. */
        ENGINE_API bool    __stdcall IsMouseReleased(int button);
        /** @brief Returns the mouse cursor X position in screen space. */
        ENGINE_API float   __stdcall GetMouseX();
        /** @brief Returns the mouse cursor Y position in screen space. */
        ENGINE_API float   __stdcall GetMouseY();
        /** @brief Returns the mouse cursor X position converted to world space. */
        ENGINE_API float   __stdcall GetMouseWorldX();
        /** @brief Returns the mouse cursor Y position converted to world space. */
        ENGINE_API float   __stdcall GetMouseWorldY();

        // =========================================================================
        // Input — Gamepad
        // Implemented in: eno_bridge_input.cpp
        // =========================================================================
        /** @brief Returns true if a gamepad is connected and recognized. */
        ENGINE_API bool    __stdcall IsGamepadConnected();
        /** @brief Returns the value of a gamepad axis in [-1, 1]. */
        ENGINE_API float   __stdcall GetGamepadAxis(int axis);
        /** @brief Returns true if a gamepad button was pressed this frame. */
        ENGINE_API bool    __stdcall IsGamepadButtonPressed(int button);
        /** @brief Returns true if a gamepad button is currently held down. */
        ENGINE_API bool    __stdcall IsGamepadButtonHeld(int button);
        /** @brief Returns true if a gamepad button was released this frame. */
        ENGINE_API bool    __stdcall IsGamepadButtonReleased(int button);
        /** @brief Sets gamepad vibration intensity for left and right motors (0.0-1.0). */
        ENGINE_API void    __stdcall SetGamepadRumble(float leftMotor, float rightMotor);

        // =========================================================================
        // Input — Mode
        // Implemented in: eno_bridge_input.cpp
        // =========================================================================
        ENGINE_API int     __stdcall GetInputMode(); // 0 = MouseKeyboard, 1 = Controller

        // =========================================================================
        // Audio Component
        // Implemented in: eno_bridge_audio.cpp
        // =========================================================================
        /** @brief Plays the audio clip attached to a scene object. */
        ENGINE_API void    __stdcall AudioPlay(uint32_t sceneObjectID);
        /** @brief Stops the audio clip attached to a scene object. */
        ENGINE_API void    __stdcall AudioStop(uint32_t sceneObjectID);
        /** @brief Returns the volume of the audio component on a scene object (0-100). */
        ENGINE_API float   __stdcall GetAudioVolume(uint32_t sceneObjectID);
        /** @brief Sets the volume of the audio component on a scene object (0-100). */
        ENGINE_API void    __stdcall SetAudioVolume(uint32_t sceneObjectID, float volume);
        /** @brief Returns the pitch multiplier of the audio component on a scene object. */
        ENGINE_API float   __stdcall GetAudioPitch(uint32_t sceneObjectID);
        /** @brief Sets the pitch multiplier of the audio component on a scene object. */
        ENGINE_API void    __stdcall SetAudioPitch(uint32_t sceneObjectID, float pitch);

        // Pause/resume all audio channel groups (BGM + SFX) — used by in-game pause
        ENGINE_API void    __stdcall SetAudioPaused(bool paused);

        // Channel group volume (groupName: "BGM" or "SFX"), range 0-100
        ENGINE_API void    __stdcall SetGroupVolume(const char* groupName, float volume);
        ENGINE_API float   __stdcall GetGroupVolume(const char* groupName);

        // Apply a low-pass filter to a channel group. cutoff in Hz (10-22000); pass 0 to remove.
        ENGINE_API void    __stdcall SetGroupLowPass(const char* groupName, float cutoff);

        // Master (FMOD master channel group) volume, range 0-100
        ENGINE_API void    __stdcall SetMasterVolume(float volume);
        ENGINE_API float   __stdcall GetMasterVolume();

        // Persist audio volumes to config file and reload them
        ENGINE_API void    __stdcall SaveAudioVolumes(float master, float bgm, float sfx);
        ENGINE_API float   __stdcall LoadAudioVolume(const char* key, float defaultVal);

        // =========================================================================
        // Animation Component
        // Implemented in: eno_bridge_animation.cpp
        // =========================================================================
        /** @brief Switches the active animation clip on a scene object by name. */
        ENGINE_API void        __stdcall SetAnimation(uint32_t sceneObjectID, const char* animName);
        /** @brief Returns the name of the currently playing animation clip. */
        ENGINE_API const char* __stdcall GetCurrentAnimation(uint32_t sceneObjectID);
        /** @brief Sets whether the current animation should loop. */
        ENGINE_API void        __stdcall SetAnimationLooping(uint32_t sceneObjectID, bool loop);
        /** @brief Sets the duration of each animation frame in seconds. */
        ENGINE_API void        __stdcall SetAnimationFrameDuration(uint32_t sceneObjectID, float frameDuration);
        /** @brief Returns the total number of frames in the current animation clip. */
        ENGINE_API uint32_t    __stdcall GetAnimationFrameCount(uint32_t sceneObjectID);

        // =========================================================================
        // State Machine
        // Implemented in: eno_bridge_state.cpp
        // =========================================================================
        /** @brief Requests a state machine transition to the named state for a scene object. */
        ENGINE_API void    __stdcall ChangeState(uint32_t sceneObjectID, const wchar_t* stateName);

        // =========================================================================
        // Combat — Health / Damage
        // Implemented in: eno_bridge_combat.cpp
        // =========================================================================
        /** @brief Returns the current health of a scene object. */
        ENGINE_API float   __stdcall GetHealth(uint32_t sceneObjectID);
        /** @brief Sets the current health of a scene object. */
        ENGINE_API void    __stdcall SetHealth(uint32_t sceneObjectID, float health);
        /** @brief Returns the maximum health of a scene object. */
        ENGINE_API float   __stdcall GetMaxHealth(uint32_t sceneObjectID);
        /** @brief Sets the maximum health of a scene object. */
        ENGINE_API void    __stdcall SetMaxHealth(uint32_t sceneObjectID, float maxHealth);
        /** @brief Returns true if the scene object's health is zero or below. */
        ENGINE_API bool    __stdcall IsDead(uint32_t sceneObjectID);
        /** @brief Heals a scene object by the specified amount, clamped to max health. */
        ENGINE_API void    __stdcall HealEntity(uint32_t sceneObjectID, float amount);

        // =========================================================================
        // Video Component
        // Implemented in: eno_bridge_video.cpp
        // =========================================================================
        /** @brief Starts or resumes video playback on a scene object. */
        ENGINE_API void    __stdcall VideoPlay(uint32_t sceneObjectID);
        /** @brief Pauses video playback on a scene object. */
        ENGINE_API void    __stdcall VideoPause(uint32_t sceneObjectID);
        /** @brief Stops video playback and resets to the beginning. */
        ENGINE_API void    __stdcall VideoStop(uint32_t sceneObjectID);
        /** @brief Returns true if the video on the scene object is currently playing. */
        ENGINE_API bool    __stdcall IsVideoPlaying(uint32_t sceneObjectID);
        /** @brief Returns the current playback time of the video in seconds. */
        ENGINE_API float   __stdcall GetVideoTime(uint32_t sceneObjectID);
        /** @brief Returns the total duration of the video in seconds. */
        ENGINE_API float   __stdcall GetVideoDuration(uint32_t sceneObjectID);
        /** @brief Sets whether the video should loop when it reaches the end. */
        ENGINE_API void    __stdcall SetVideoLoop(uint32_t sceneObjectID, bool loop);
        /** @brief Returns whether the video is set to loop. */
        ENGINE_API bool    __stdcall GetVideoLoop(uint32_t sceneObjectID);

        // =========================================================================
        // Render Component
        // Implemented in: eno_bridge_render.cpp
        // =========================================================================
        /** @brief Returns the render width of a scene object's sprite/quad. */
        ENGINE_API float   __stdcall GetRenderWidth(uint32_t sceneObjectID);
        /** @brief Returns the render height of a scene object's sprite/quad. */
        ENGINE_API float   __stdcall GetRenderHeight(uint32_t sceneObjectID);
        /** @brief Sets the render size (width and height) of a scene object's sprite/quad. */
        ENGINE_API void    __stdcall SetRenderSize(uint32_t sceneObjectID, float w, float h);
        /** @brief Changes the texture on a scene object by asset name, keeping the current size. */
        ENGINE_API void    __stdcall SetTexturePath(uint32_t sceneObjectID, const char* textureName);
        /** @brief Changes the texture and resizes the sprite to match the new texture dimensions. */
        ENGINE_API void    __stdcall SetTexturePathAndResize(uint32_t sceneObjectID, const char* textureName);
        /** @brief Returns the starting U coordinate of the UV rect. */
        ENGINE_API float   __stdcall GetUVStartX(uint32_t sceneObjectID);
        /** @brief Returns the starting V coordinate of the UV rect. */
        ENGINE_API float   __stdcall GetUVStartY(uint32_t sceneObjectID);
        /** @brief Returns the ending U coordinate of the UV rect. */
        ENGINE_API float   __stdcall GetUVEndX(uint32_t sceneObjectID);
        /** @brief Returns the ending V coordinate of the UV rect. */
        ENGINE_API float   __stdcall GetUVEndY(uint32_t sceneObjectID);
        /** @brief Sets the UV coordinates for texture mapping on a scene object. */
        ENGINE_API void    __stdcall SetUV(uint32_t sceneObjectID, float startX, float startY, float endX, float endY);

        // =========================================================================
        // Collision
        // Implemented in: eno_bridge_collision.cpp
        // =========================================================================
        /** @brief Returns true if the point (x, y) overlaps the scene object's collider. */
        ENGINE_API bool    __stdcall IsOverlappingPoint(uint32_t sceneObjectID, float x, float y);
        /** @brief Returns true if two scene objects' colliders are currently overlapping. */
        ENGINE_API bool    __stdcall IsColliding(uint32_t sceneObjectIDA, uint32_t sceneObjectIDB);
        /** @brief Returns whether a scene object's collider is active. */
        ENGINE_API bool    __stdcall GetColliderActive(uint32_t sceneObjectID);
        /** @brief Activates or deactivates a scene object's collider. */
        ENGINE_API void    __stdcall SetColliderActive(uint32_t sceneObjectID, bool active);

        // =========================================================================
        // Text Component
        // Implemented in: eno_bridge_render.cpp
        // =========================================================================
        /** @brief Returns the text string displayed by a text component. */
        ENGINE_API const char* __stdcall GetText(uint32_t sceneObjectID);
        /** @brief Sets the text string displayed by a text component. */
        ENGINE_API void        __stdcall SetText(uint32_t sceneObjectID, const char* text);
        /** @brief Returns the font size of a text component. */
        ENGINE_API float       __stdcall GetFontSize(uint32_t sceneObjectID);
        /** @brief Sets the font size of a text component. */
        ENGINE_API void        __stdcall SetFontSize(uint32_t sceneObjectID, float size);

        // =========================================================================
        // Window — resolution, vsync, fullscreen, FPS, pause, close
        // Implemented in: eno_bridge_window.cpp
        // =========================================================================
        /** @brief Returns the current window width in pixels. */
        ENGINE_API int   __stdcall GetWindowWidth();
        /** @brief Returns the current window height in pixels. */
        ENGINE_API int   __stdcall GetWindowHeight();

        /** @brief Sets the window resolution to the specified width and height. */
        ENGINE_API void  __stdcall SetResolution(int width, int height);
        /** @brief Steps up to the next resolution in the resolution table. */
        ENGINE_API void  __stdcall IncreaseResolution();
        /** @brief Steps down to the previous resolution in the resolution table. */
        ENGINE_API void  __stdcall DecreaseResolution();

        /** @brief Enables or disables vertical synchronization. */
        ENGINE_API void  __stdcall SetVSync(bool enabled);
        /** @brief Toggles between fullscreen and windowed mode. */
        ENGINE_API void  __stdcall ToggleFullscreen();

        /** @brief Returns the current frames per second. */
        ENGINE_API float __stdcall GetFPS();

        /** @brief Pauses or resumes the game runtime (physics, AI, collision). */
        ENGINE_API void  __stdcall SetPaused(bool paused);
        /** @brief Returns true if the game is currently paused. */
        ENGINE_API bool  __stdcall GetIsPaused();

        /** @brief Returns true if the window has been requested to close. */
        ENGINE_API bool  __stdcall GetShouldClose();
        /** @brief Requests the engine to close the window at the end of the frame. */
        ENGINE_API void  __stdcall RequestClose();

        // =========================================================================
        // Particle System
        // Implemented in: eno_bridge_particle.cpp
        // =========================================================================
        /** @brief Emits a one-shot burst of particles from a particle emitter. */
        ENGINE_API void  __stdcall ParticleEmitBurst(uint32_t sceneObjectID, int count);
        /** @brief Starts continuous particle emission on a scene object. */
        ENGINE_API void  __stdcall ParticleStartEmission(uint32_t sceneObjectID);
        /** @brief Stops continuous particle emission on a scene object. */
        ENGINE_API void  __stdcall ParticleStopEmission(uint32_t sceneObjectID);
        /** @brief Sets the number of particles emitted per second. */
        ENGINE_API void  __stdcall SetParticleEmissionRate(uint32_t sceneObjectID, float rate);
        /** @brief Returns the maximum number of particles allowed for this emitter. */
        ENGINE_API int   __stdcall GetParticleMaxParticles(uint32_t sceneObjectID);
        /** @brief Sets the maximum number of particles allowed for this emitter. */
        ENGINE_API void  __stdcall SetParticleMaxParticles(uint32_t sceneObjectID, int max);
        /** @brief Sets the min and max lifetime range for newly spawned particles. */
        ENGINE_API void  __stdcall SetParticleLifetime(uint32_t sceneObjectID, float minLife, float maxLife);
        /** @brief Sets the min and max size range for newly spawned particles. */
        ENGINE_API void  __stdcall SetParticleSize(uint32_t sceneObjectID, float minSize, float maxSize);
        /** @brief Sets the min and max initial velocity range for newly spawned particles. */
        ENGINE_API void  __stdcall SetParticleVelocity(uint32_t sceneObjectID, float minX, float minY, float maxX, float maxY);
        /** @brief Sets the starting color (RGBA) for newly spawned particles. */
        ENGINE_API void  __stdcall SetParticleColorStart(uint32_t sceneObjectID, float r, float g, float b, float a);
        /** @brief Sets the ending color (RGBA) that particles fade to over their lifetime. */
        ENGINE_API void  __stdcall SetParticleColorEnd(uint32_t sceneObjectID, float r, float g, float b, float a);
        /** @brief Sets the gravity force applied to particles. */
        ENGINE_API void  __stdcall SetParticleGravity(uint32_t sceneObjectID, float gx, float gy);
        /** @brief Sets the drag coefficient applied to particle velocity. */
        ENGINE_API void  __stdcall SetParticleDrag(uint32_t sceneObjectID, float drag);
        /** @brief Returns true if the particle emitter is currently emitting. */
        ENGINE_API bool  __stdcall GetParticleIsEmitting(uint32_t sceneObjectID);
    }
}
