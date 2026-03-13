/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_scene.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        Scene bridge — change scene from C# scripts.
*//*========================================================================================*/



#include "eno_bridge.hpp"

namespace Bridge
{
    extern "C"
    {
        // Queue a scene change to be executed at the end of the current frame,
        // after all scripts have finished updating.
        // sceneName should be the scene filename, e.g. "Level.txt".
        ENGINE_API void __stdcall ChangeScene(const char* sceneName)
        {
            if (!gEngine || !sceneName) return;
            gEngine->requestSceneChange(sceneName);
        }

        // Returns the name of the currently loaded scene.
        ENGINE_API void __stdcall GetCurrentScene(char* outBuffer, int bufferSize)
        {
            if (!gEngine || !outBuffer || bufferSize <= 0) return;
            const std::string& name = gEngine->getEnoEditor().getCurrentScene().getSceneName();
            strncpy_s(outBuffer, bufferSize, name.c_str(), bufferSize - 1);
        }
    }
}
