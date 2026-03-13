/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   80%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   13%
* @brief        Core bridge — engine initialisation and time.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../file/file_system.hpp"

// Global engine pointer — declared extern in eno_bridge.hpp,
// defined exactly once here so all bridge .cpp files share it.
Engine::EnoEngine* gEngine = nullptr;

namespace Bridge
{
    extern "C"
    {
        ENGINE_API void __stdcall Bridge_Init(void* enginePtr)
        {
            gEngine = reinterpret_cast<Engine::EnoEngine*>(enginePtr);
            if (!gEngine)
                std::cerr << "[Bridge] Bridge_Init: engine pointer is null!\n";

            // The DLL has its own copy of FileSystem statics (separate from the EXE's).
            // Initialize them here so GetScriptsDir() works correctly inside the DLL
            // (e.g. when SpawnPrefab -> attachScriptByFilename -> EnoScript uses the path).
            // Use the app name stored in the engine (set via EnoEngine::setAppName before setup).
            FileSystem::Initialize(gEngine->getAppName());
        }

        ENGINE_API float __stdcall GetDeltaTime()
        {
            if (!gEngine) return 0.0f;
            return static_cast<float>(gEngine->getDeltaTime());  // already scaled by timeScale_
        }

        ENGINE_API float __stdcall GetUnscaledDeltaTime()
        {
            if (!gEngine) return 0.0f;
            return static_cast<float>(gEngine->getUnscaledDeltaTime());
        }

        ENGINE_API void __stdcall SetTimeScale(float scale)
        {
            if (!gEngine) return;
            gEngine->setTimeScale(scale);
        }

        ENGINE_API float __stdcall GetTimeScale()
        {
            if (!gEngine) return 1.0f;
            return gEngine->getTimeScale();
        }
    }
}
