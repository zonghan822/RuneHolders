#include "eno_bridge.hpp"

static Engine::EnoEngine* gEngine = nullptr;

namespace Bridge
{
    extern "C" {
        ENGINE_API void __stdcall Bridge_Init(void* enginePtr)
        {
            gEngine = reinterpret_cast<Engine::EnoEngine*>(enginePtr);
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }
        }

        ENGINE_API bool __stdcall IsKeyPressed(int key)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }
            auto input = gEngine->getInputManager();
            return input.isKeyDown(key);
        }

        ENGINE_API bool __stdcall IsKeyHeld(int key)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }
            auto input = gEngine->getInputManager();
            return input.isKeyPressed(key);
        }

        ENGINE_API bool __stdcall IsKeyReleased(int key)
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return false;
            }
            auto input = gEngine->getInputManager();
            return input.isKeyUp(key);
        }

        ENGINE_API int __stdcall ModsApplied()
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return 0;
            }
            auto input = gEngine->getInputManager();
            return input.currentMods();
        }

        ENGINE_API void ChangeState(const wchar_t* stateName, uint32_t entityID) 
        {
            if (!gEngine) {
                std::cerr << "[Engine DLL] Bridge not initialized!" << std::endl;
                return;
            }

            std::wstring wState(stateName);
            std::string state(wState.begin(), wState.end());

            // Request state change through the system
            gEngine->getStateMachineSys().requestStateChange(entityID, state);
        }
    }
}