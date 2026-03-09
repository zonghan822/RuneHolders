/*============================================================================================
* @file         eno_bridge_input.cpp
* @brief        Input bridge — keyboard and mouse state queries.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"

namespace Bridge
{
    extern "C"
    {
        // ============= Keyboard =============

        ENGINE_API bool __stdcall IsKeyPressed(int key)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isKeyDown(key);
        }

        ENGINE_API bool __stdcall IsKeyHeld(int key)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isKeyPressed(key);
        }

        ENGINE_API bool __stdcall IsKeyReleased(int key)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isKeyUp(key);
        }

        ENGINE_API int __stdcall ModsApplied()
        {
            if (!gEngine) return 0;
            return gEngine->getInputManager().currentMods();
        }

        // ============= Mouse =============

        ENGINE_API bool __stdcall IsMousePressed(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMouseDown(button);
        }

        ENGINE_API bool __stdcall IsMouseHeld(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMousePressed(button);
        }

        ENGINE_API bool __stdcall IsMouseReleased(int button)
        {
            if (!gEngine) return false;
            return gEngine->getInputManager().isMouseUp(button);
        }

        ENGINE_API float __stdcall GetMouseX()
        {
            if (!gEngine) return 0.0f;
            return static_cast<float>(gEngine->getInputManager().getMousePosition().first);
        }

        ENGINE_API float __stdcall GetMouseY()
        {
            if (!gEngine) return 0.0f;
            return static_cast<float>(gEngine->getInputManager().getMousePosition().second);
        }
    }
}
