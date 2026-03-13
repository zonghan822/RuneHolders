/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_input.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   40%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   40%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   20%
* @brief        Input bridge — keyboard and mouse state queries.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../graphics/graphics.hpp"
#include "../component/components/camera_component.hpp"

namespace Bridge
{
    extern "C"
    {
        // Shared helper: converts current mouse screen position to world coordinates.
        static Math::eVec2<float> mouseToWorld()
        {
            float cameraX = 0.f, cameraY = 0.f, vpW = 1920.f, vpH = 1080.f;
            auto& entityMng = gEngine->getEntityManager();
            auto cams = entityMng.findEntity<ECSComponents::CameraComponent>();
            if (!cams.empty())
            {
                auto& cam = cams[0].get<ECSComponents::CameraComponent>(entityMng);
                cameraX = cam.position.x - cam.viewportWidth  * 0.5f;
                cameraY = cam.position.y - cam.viewportHeight * 0.5f;
                vpW     = cam.viewportWidth;
                vpH     = cam.viewportHeight;
            }

            auto viewportMin = gEngine->getViewportMin();
            auto viewportMax = gEngine->getViewportMax();
            if (viewportMax.x == 0.f && viewportMax.y == 0.f)
            {
                // Use the window size cached at startup in the EXE.
                // GLFW is statically linked to the EXE only; calling getWindowSize() from
                // the DLL always returns 0 because the DLL's GLFW has no windows.
                auto [winW, winH] = gEngine->getCachedWindowSize();
                viewportMax = { static_cast<float>(winW), static_cast<float>(winH) };
            }

            auto mouse = gEngine->getInputManager().getMousePosition();
            return Graphics::screenToWorld(
                { static_cast<float>(mouse.first), static_cast<float>(mouse.second) },
                viewportMin, viewportMax, vpW, vpH, cameraX, cameraY
            );
        }
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

        ENGINE_API float __stdcall GetMouseWorldX()
        {
            if (!gEngine) return 0.0f;
            return mouseToWorld().x;
        }

        ENGINE_API float __stdcall GetMouseWorldY()
        {
            if (!gEngine) return 0.0f;
            return mouseToWorld().y;
        }
    }
}
