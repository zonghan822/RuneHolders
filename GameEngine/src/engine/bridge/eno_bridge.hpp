#pragma once

// Input
#include "../engine/eno_engine.hpp"

#if defined(ENGINE_BUILD_DLL)
    #define ENGINE_API __declspec(dllexport)
# elif defined(ENGINE_USE_DLL)
    #define ENGINE_API __declspec(dllimport)
#else
    #define ENGINE_API
#endif

namespace Bridge {

    extern "C" {
        ENGINE_API void __stdcall Bridge_Init(void* enginePtr);

        // ================= Time =================
        ENGINE_API float __stdcall GetDeltaTime();

        // ================= Input =================
        ENGINE_API bool __stdcall IsKeyPressed(int key);
        ENGINE_API bool __stdcall IsKeyHeld(int key);
        ENGINE_API bool __stdcall IsKeyReleased(int key);
        ENGINE_API int  __stdcall ModsApplied();

        // ============= State Change ==============
        ENGINE_API void __stdcall ChangeState(uint32_t entityID, const wchar_t* stateName);
        
        // ================= Animation =================
        ENGINE_API bool __stdcall SetEntityAnimation(uint32_t entityID, const wchar_t* animationName);

        // ================= AI =================
        ENGINE_API const wchar_t* __stdcall GetEntityAIState(uint32_t entityID);

        // ================= Scene =================
        ENGINE_API void __stdcall LoadScene(const wchar_t* sceneName);

        // ============= AI Pathfinding =============
        ENGINE_API bool __stdcall GetPlayerPosition(float* outX, float* outY);
        ENGINE_API bool __stdcall GetEntityPosition(uint32_t entityID, float* outX, float* outY);
        ENGINE_API bool __stdcall SetEntityVelocity(uint32_t entityID, float vx, float vy);
        ENGINE_API bool __stdcall RequestPathToPosition(uint32_t entityID, float targetX, float targetY, bool allowDiagonal);
        ENGINE_API bool __stdcall GetNextWaypoint(uint32_t entityID, float* outX, float* outY);
        ENGINE_API float __stdcall CalculateDistance(float x1, float y1, float x2, float y2);
        ENGINE_API bool __stdcall MoveTowards(uint32_t entityID, float targetX, float targetY, float speed);
        ENGINE_API bool __stdcall IsNearPosition(uint32_t entityID, float targetX, float targetY, float threshold);
        ENGINE_API bool __stdcall ClearPath(uint32_t entityID);
        ENGINE_API bool __stdcall GetAIComponent(uint32_t entityID, float* outAwarenessRadius, float* outChaseSpeed,
            float* outAttackRange, float* outWindupTime, float* outAttackDuration, float* outHurtDuration,
            int* outHealth, bool* outAllowDiagonal);
        ENGINE_API bool __stdcall SetAIHealth(uint32_t entityID, int health);
        ENGINE_API bool __stdcall GetAIHealth(uint32_t entityID, int* outHealth);
        ENGINE_API bool __stdcall FindAndTriggerRuptureMark(uint32_t entityID, int spellType, float* outDamage);

        // ============= Entity Management =============
        ENGINE_API bool __stdcall DestroyEntity(uint32_t entityID);

        // ============= Entity Transform =============
        ENGINE_API bool __stdcall SetEntityPosition(uint32_t entityID, float x, float y);
        ENGINE_API bool __stdcall SetPlayerPosition(float x, float y);

        // ================= Player Scripting =================
        ENGINE_API bool __stdcall SetPlayerScriptedMovement(bool enabled);
        ENGINE_API bool __stdcall SetPlayerVelocity(float vx, float vy);

        // ================= Camera =================
        ENGINE_API bool __stdcall GetCameraPosition(float* outX, float* outY);
        ENGINE_API bool __stdcall SetCameraPosition(float x, float y);

        // ================= Audio ================= 
        ENGINE_API void __stdcall PlayInstantAudio(const wchar_t* groupName, const wchar_t* soundName);

        // ============= Entity Queries =============
        ENGINE_API uint32_t __stdcall FindEntityByName(const wchar_t* name);

        // ============= Entity State =============
        ENGINE_API bool __stdcall SetEntityVisible(uint32_t entityID, bool visible);
        ENGINE_API bool __stdcall SetEntityScale(uint32_t entityID, float x, float y);
        ENGINE_API bool __stdcall GetEntityScale(uint32_t entityID, float* outX, float* outY);

        // ============= Game Pause =============
        ENGINE_API void __stdcall PauseGame();
        ENGINE_API void __stdcall ResumeGame();
        ENGINE_API bool __stdcall IsGamePaused();

        // ============= Text =============
        ENGINE_API bool __stdcall SetEntityText(uint32_t entityID, const wchar_t* text);

        // ============= Assets =============
        ENGINE_API const wchar_t* __stdcall GetAssetsDir();

        // ============= Combat =============
        ENGINE_API float __stdcall GetEntityHealth(uint32_t entityID);
        ENGINE_API bool  __stdcall SetEntityHealth(uint32_t entityID, float hp);
        ENGINE_API bool  __stdcall AddEntityDamage(uint32_t entityID, float damage);

        // ============= Mouse =============
        ENGINE_API bool __stdcall GetMouseWorldPosition(float* outX, float* outY);
    }
}

