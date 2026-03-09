/*============================================================================================
* @file         eno_bridge_window.cpp
* @brief        Window bridge — resolution, vsync, fullscreen, FPS, pause, and close control.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"

namespace Bridge
{
    extern "C"
    {
        // ============= Window Size =============

        ENGINE_API int __stdcall GetWindowWidth()
        {
            if (!gEngine) return 0;
            return gEngine->getWindowManager().getWindowSize().first;
        }

        ENGINE_API int __stdcall GetWindowHeight()
        {
            if (!gEngine) return 0;
            return gEngine->getWindowManager().getWindowSize().second;
        }

        // ============= Resolution =============

        ENGINE_API void __stdcall SetResolution(int width, int height)
        {
            if (!gEngine) return;
            gEngine->getWindowManager().setResolution(width, height);
        }

        ENGINE_API void __stdcall IncreaseResolution()
        {
            if (!gEngine) return;
            gEngine->getWindowManager().increaseResolution();
        }

        ENGINE_API void __stdcall DecreaseResolution()
        {
            if (!gEngine) return;
            gEngine->getWindowManager().decreaseResolution();
        }

        // ============= VSync =============

        ENGINE_API void __stdcall SetVSync(bool enabled)
        {
            if (!gEngine) return;
            gEngine->getWindowManager().setVSync(enabled);
        }

        // ============= Fullscreen =============

        ENGINE_API void __stdcall ToggleFullscreen()
        {
            if (!gEngine) return;
            gEngine->getWindowManager().toggleFullscreen();
        }

        // ============= FPS =============

        ENGINE_API float __stdcall GetFPS()
        {
            if (!gEngine) return 0.0f;
            return static_cast<float>(gEngine->getFPS());
        }

        // ============= Pause =============

        ENGINE_API void __stdcall SetPaused(bool paused)
        {
            if (!gEngine) return;
            gEngine->setPaused(paused);
        }

        ENGINE_API bool __stdcall GetIsPaused()
        {
            if (!gEngine) return false;
            return gEngine->getIsPaused();
        }

        // ============= Close =============

        ENGINE_API bool __stdcall GetShouldClose()
        {
            if (!gEngine) return false;
            return gEngine->shouldClose();
        }

        ENGINE_API void __stdcall RequestClose()
        {
            if (!gEngine) return;
            gEngine->requestClose();
        }
    }
}
