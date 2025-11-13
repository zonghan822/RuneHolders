#include "eno_bridge.hpp"


namespace Bridge
{
	EnoBridge* bridge_ = nullptr;

	// ================= Input =================
	extern "C" __declspec(dllexport)
		bool __stdcall IsKeyReleased(Input::KeyCode key)
	{
		return bridge_->input().isKeyReleased(key);
	}

	extern "C" __declspec(dllexport)
		bool __stdcall IsKeyPressed(Input::KeyCode key)
	{
		return bridge_->input().isKeyPressed(key);
	}

	extern "C" __declspec(dllexport)
		bool __stdcall IsKeyHeld(Input::KeyCode key)
	{
		return bridge_->input().isKeyHeld(key);
	}

	extern "C" __declspec(dllexport)
		int __stdcall ModsApplied()
	{
		return bridge_->input().currentMods();
	}
}