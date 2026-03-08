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

        // ================= Node =================
        ENGINE_API uint32_t __stdcall GetNode(const char* name);
        ENGINE_API uint32_t __stdcall GetChild(uint32_t parentID, uint32_t index);
        ENGINE_API uint32_t __stdcall GetNodeType(uint32_t id);

        // ================= Time =================
        ENGINE_API float __stdcall GetDeltaTime();

        // ============= Visibility Component =============
        ENGINE_API uint32_t __stdcall GetZIndex(uint32_t nodeID);
        ENGINE_API void __stdcall SetZIndex(uint32_t nodeID, uint32_t z);

        // ================= Input =================
        ENGINE_API bool __stdcall IsKeyPressed(int key);
        ENGINE_API bool __stdcall IsKeyHeld(int key);
        ENGINE_API bool __stdcall IsKeyReleased(int key);
        ENGINE_API int  __stdcall ModsApplied();

        // ============= State Change ==============
        ENGINE_API void __stdcall ChangeState(uint32_t entityID, const wchar_t* stateName);
    }
}

