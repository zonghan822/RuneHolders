#include "eno_bridge.hpp"


namespace Bridge
{
	EnoBridge* bridge_ = nullptr;

	// ================= Input =================
	extern "C" __declspec(dllexport)
		bool __stdcall IsKeyReleased(int key)
	{
		return bridge_->input().isKeyUp(key);
	}

	extern "C" __declspec(dllexport)
		bool __stdcall IsKeyPressed(int key)
	{
		return bridge_->input().isKeyPressed(key);
	}

	extern "C" __declspec(dllexport)
		bool __stdcall IsKeyHeld(int key)
	{
		return bridge_->input().isKeyDown(key);
	}

	extern "C" __declspec(dllexport)
		int __stdcall ModsApplied()
	{
		return bridge_->input().currentMods();
	}
}