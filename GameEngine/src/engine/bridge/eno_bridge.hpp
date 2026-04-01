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
        ENGINE_API void    __stdcall Bridge_Init(void* enginePtr);

        // =========================================================================
        // Time
        // Implemented in: eno_bridge_core.cpp
        // =========================================================================
        ENGINE_API float   __stdcall GetDeltaTime();
        ENGINE_API float   __stdcall GetUnscaledDeltaTime();
        ENGINE_API void    __stdcall SetTimeScale(float scale);
        ENGINE_API float   __stdcall GetTimeScale();

        // =========================================================================
        // Node Management
        // Implemented in: eno_bridge_node.cpp
        // =========================================================================
        ENGINE_API uint32_t __stdcall CreateNode(uint32_t type);
        ENGINE_API void __stdcall FreeNode(uint32_t id);
        ENGINE_API uint32_t __stdcall GetNode(const char* name);
        ENGINE_API uint32_t __stdcall GetChild(uint32_t parentID, uint32_t index);
        ENGINE_API void __stdcall AddChild(uint32_t cid, uint32_t pid);
        ENGINE_API void __stdcall Unparent(uint32_t id);
        ENGINE_API void __stdcall Reparent(uint32_t cid, uint32_t pid);
        ENGINE_API uint32_t __stdcall GetParentNode(uint32_t id);
        ENGINE_API uint32_t __stdcall GetNodeType(uint32_t id);
        ENGINE_API uint32_t __stdcall GetChildCount(uint32_t id);
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
        ENGINE_API float   __stdcall GetPositionX(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetPositionY(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetPosition(uint32_t sceneObjectID, float x, float y);
        ENGINE_API void    __stdcall SetLocalPosition(uint32_t sceneObjectID, float x, float y);

        ENGINE_API float   __stdcall GetRotationX(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetRotationY(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetRotation(uint32_t sceneObjectID, float x, float y);

        ENGINE_API float   __stdcall GetScaleX(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetScaleY(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetScale(uint32_t sceneObjectID, float x, float y);

        // =========================================================================
        // Physics Component
        // Implemented in: eno_bridge_physics.cpp
        // =========================================================================
        ENGINE_API float   __stdcall GetVelocityX(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetVelocityY(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetVelocity(uint32_t sceneObjectID, float x, float y);

        ENGINE_API void    __stdcall AddForce(uint32_t sceneObjectID, float x, float y);

        ENGINE_API float   __stdcall GetMass(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetMass(uint32_t sceneObjectID, float mass);

        ENGINE_API bool    __stdcall GetUseGravity(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetUseGravity(uint32_t sceneObjectID, bool useGravity);

        ENGINE_API bool    __stdcall GetIsKinematic(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetIsKinematic(uint32_t sceneObjectID, bool kinematic);

        // =========================================================================
        // Visibility Component
        // Implemented in: eno_bridge_visibility.cpp
        // =========================================================================
        ENGINE_API uint32_t __stdcall GetZIndex(uint32_t sceneObjectID);
        ENGINE_API void     __stdcall SetZIndex(uint32_t sceneObjectID, uint32_t z);
        ENGINE_API bool     __stdcall GetVisibility(uint32_t sceneObjectID);
        ENGINE_API void     __stdcall SetVisibility(uint32_t sceneObjectID, bool visible);

        // =========================================================================
        // Canvas Component
        // Implemented in: eno_bridge_canvas.cpp
        // =========================================================================
        ENGINE_API bool __stdcall GetCanvasActive(uint32_t sceneObjectID);
        ENGINE_API void __stdcall SetCanvasActive(uint32_t sceneObjectID, bool active);

        // =========================================================================
        // Input — Keyboard
        // Implemented in: eno_bridge_input.cpp
        // =========================================================================
        ENGINE_API bool    __stdcall IsKeyPressed(int key);
        ENGINE_API bool    __stdcall IsKeyHeld(int key);
        ENGINE_API bool    __stdcall IsKeyReleased(int key);
        ENGINE_API int     __stdcall ModsApplied();

        // =========================================================================
        // Input — Mouse
        // Implemented in: eno_bridge_input.cpp
        // =========================================================================
        ENGINE_API bool    __stdcall IsMousePressed(int button);
        ENGINE_API bool    __stdcall IsMouseHeld(int button);
        ENGINE_API bool    __stdcall IsMouseReleased(int button);
        ENGINE_API float   __stdcall GetMouseX();
        ENGINE_API float   __stdcall GetMouseY();
        ENGINE_API float   __stdcall GetMouseWorldX();
        ENGINE_API float   __stdcall GetMouseWorldY();

        // =========================================================================
        // Audio Component
        // Implemented in: eno_bridge_audio.cpp
        // =========================================================================
        ENGINE_API void    __stdcall AudioPlay(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall AudioStop(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetAudioVolume(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetAudioVolume(uint32_t sceneObjectID, float volume);
        ENGINE_API float   __stdcall GetAudioPitch(uint32_t sceneObjectID);
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
        ENGINE_API void        __stdcall SetAnimation(uint32_t sceneObjectID, const char* animName);
        ENGINE_API const char* __stdcall GetCurrentAnimation(uint32_t sceneObjectID);
        ENGINE_API void        __stdcall SetAnimationLooping(uint32_t sceneObjectID, bool loop);
        ENGINE_API void        __stdcall SetAnimationFrameDuration(uint32_t sceneObjectID, float frameDuration);
        ENGINE_API uint32_t    __stdcall GetAnimationFrameCount(uint32_t sceneObjectID);

        // =========================================================================
        // State Machine
        // Implemented in: eno_bridge_state.cpp
        // =========================================================================
        ENGINE_API void    __stdcall ChangeState(uint32_t sceneObjectID, const wchar_t* stateName);

        // =========================================================================
        // Combat — Health / Damage
        // Implemented in: eno_bridge_combat.cpp
        // =========================================================================
        ENGINE_API float   __stdcall GetHealth(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetHealth(uint32_t sceneObjectID, float health);
        ENGINE_API float   __stdcall GetMaxHealth(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetMaxHealth(uint32_t sceneObjectID, float maxHealth);
        ENGINE_API bool    __stdcall IsDead(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall HealEntity(uint32_t sceneObjectID, float amount);

        // =========================================================================
        // Video Component
        // Implemented in: eno_bridge_video.cpp
        // =========================================================================
        ENGINE_API void    __stdcall VideoPlay(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall VideoPause(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall VideoStop(uint32_t sceneObjectID);
        ENGINE_API bool    __stdcall IsVideoPlaying(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetVideoTime(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetVideoDuration(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetVideoLoop(uint32_t sceneObjectID, bool loop);
        ENGINE_API bool    __stdcall GetVideoLoop(uint32_t sceneObjectID);

        // =========================================================================
        // Render Component
        // Implemented in: eno_bridge_render.cpp
        // =========================================================================
        ENGINE_API float   __stdcall GetRenderWidth(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetRenderHeight(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetRenderSize(uint32_t sceneObjectID, float w, float h);
        ENGINE_API void    __stdcall SetTexturePath(uint32_t sceneObjectID, const char* textureName);
        ENGINE_API float   __stdcall GetUVStartX(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetUVStartY(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetUVEndX(uint32_t sceneObjectID);
        ENGINE_API float   __stdcall GetUVEndY(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetUV(uint32_t sceneObjectID, float startX, float startY, float endX, float endY);

        // =========================================================================
        // Collision
        // Implemented in: eno_bridge_collision.cpp
        // =========================================================================
        ENGINE_API bool    __stdcall IsOverlappingPoint(uint32_t sceneObjectID, float x, float y);
        ENGINE_API bool    __stdcall IsColliding(uint32_t sceneObjectIDA, uint32_t sceneObjectIDB);
        ENGINE_API bool    __stdcall GetColliderActive(uint32_t sceneObjectID);
        ENGINE_API void    __stdcall SetColliderActive(uint32_t sceneObjectID, bool active);

        // =========================================================================
        // Text Component
        // Implemented in: eno_bridge_render.cpp
        // =========================================================================
        ENGINE_API const char* __stdcall GetText(uint32_t sceneObjectID);
        ENGINE_API void        __stdcall SetText(uint32_t sceneObjectID, const char* text);
        ENGINE_API float       __stdcall GetFontSize(uint32_t sceneObjectID);
        ENGINE_API void        __stdcall SetFontSize(uint32_t sceneObjectID, float size);

        // =========================================================================
        // Window — resolution, vsync, fullscreen, FPS, pause, close
        // Implemented in: eno_bridge_window.cpp
        // =========================================================================
        ENGINE_API int   __stdcall GetWindowWidth();
        ENGINE_API int   __stdcall GetWindowHeight();

        ENGINE_API void  __stdcall SetResolution(int width, int height);
        ENGINE_API void  __stdcall IncreaseResolution();
        ENGINE_API void  __stdcall DecreaseResolution();

        ENGINE_API void  __stdcall SetVSync(bool enabled);
        ENGINE_API void  __stdcall ToggleFullscreen();

        ENGINE_API float __stdcall GetFPS();

        ENGINE_API void  __stdcall SetPaused(bool paused);
        ENGINE_API bool  __stdcall GetIsPaused();

        ENGINE_API bool  __stdcall GetShouldClose();
        ENGINE_API void  __stdcall RequestClose();

        // =========================================================================
        // Particle System
        // Implemented in: eno_bridge_particle.cpp
        // =========================================================================
        ENGINE_API void  __stdcall ParticleEmitBurst(uint32_t sceneObjectID, int count);
        ENGINE_API void  __stdcall ParticleStartEmission(uint32_t sceneObjectID);
        ENGINE_API void  __stdcall ParticleStopEmission(uint32_t sceneObjectID);
        ENGINE_API void  __stdcall SetParticleEmissionRate(uint32_t sceneObjectID, float rate);
        ENGINE_API int   __stdcall GetParticleMaxParticles(uint32_t sceneObjectID);
        ENGINE_API void  __stdcall SetParticleMaxParticles(uint32_t sceneObjectID, int max);
        ENGINE_API void  __stdcall SetParticleLifetime(uint32_t sceneObjectID, float minLife, float maxLife);
        ENGINE_API void  __stdcall SetParticleSize(uint32_t sceneObjectID, float minSize, float maxSize);
        ENGINE_API void  __stdcall SetParticleVelocity(uint32_t sceneObjectID, float minX, float minY, float maxX, float maxY);
        ENGINE_API void  __stdcall SetParticleColorStart(uint32_t sceneObjectID, float r, float g, float b, float a);
        ENGINE_API void  __stdcall SetParticleColorEnd(uint32_t sceneObjectID, float r, float g, float b, float a);
        ENGINE_API void  __stdcall SetParticleGravity(uint32_t sceneObjectID, float gx, float gy);
        ENGINE_API void  __stdcall SetParticleDrag(uint32_t sceneObjectID, float drag);
        ENGINE_API bool  __stdcall GetParticleIsEmitting(uint32_t sceneObjectID);
    }
}
