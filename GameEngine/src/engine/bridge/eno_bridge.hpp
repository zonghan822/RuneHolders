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

        // ================= Input =================
        ENGINE_API bool __stdcall IsKeyPressed(int key);
        ENGINE_API bool __stdcall IsKeyHeld(int key);
        ENGINE_API bool __stdcall IsKeyReleased(int key);
        ENGINE_API int  __stdcall ModsApplied();
    }
}

