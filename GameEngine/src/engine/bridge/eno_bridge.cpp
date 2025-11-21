#include "eno_bridge.hpp"


namespace Bridge
{
    static EnoBridge* gBridge = nullptr;

    void setBridge(EnoBridge* bridge)
    {
        gBridge = bridge;
    }

    EnoBridge* getBridge()
    {
        return gBridge;
    }

    // ================= Input =================

    extern "C" __declspec(dllexport)
        bool __stdcall IsKeyReleased(int key)
    {
        if (!gBridge) {
            std::cerr << "Input called too early!" << std::endl;
            return false;
        }
        return gBridge->input().isKeyUp(key);
    }

    extern "C" __declspec(dllexport)
        bool __stdcall IsKeyPressed(int key)
    {
        if (!gBridge) {
            std::cerr << "Input called too early!" << std::endl;
            return false;
        }
        return gBridge->input().isKeyDown(key);
    }

    extern "C" __declspec(dllexport)
        bool __stdcall IsKeyHeld(int key)
    {
        if (!gBridge) {
            std::cerr << "Input called too early!" << std::endl;
            return false;
        }
        return gBridge->input().isKeyPressed(key);
    }

    extern "C" __declspec(dllexport)
        int __stdcall ModsApplied()
    {
        if (!gBridge) {
            std::cerr << "Input called too early!" << std::endl;
            return false;
        }
        return gBridge->input().currentMods();
    }
}