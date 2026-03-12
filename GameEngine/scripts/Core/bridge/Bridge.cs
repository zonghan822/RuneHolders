using System.Runtime.InteropServices;

// Bridge is a partial static class — each category lives in its own file:
//   BridgeNode.cs       — node lookup, spawn, destroy
//   BridgeTransform.cs  — position, rotation, scale
//   BridgePhysics.cs    — velocity, forces, mass, gravity, kinematic
//   BridgeVisibility.cs — z-index, visible flag
//   BridgeInput.cs      — keyboard and mouse
//   BridgeAudio.cs      — play, stop, volume, pitch
//   BridgeAnimation.cs  — set / get current animation
//   BridgeState.cs      — state machine transitions
//   BridgeCombat.cs     — health, heal, death
//   BridgeRender.cs     — render size, text content, font size

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Core / Init  (implemented in eno_bridge.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void Bridge_Init(IntPtr enginePtr);

        // -----------------------------------------------------------------------
        // Time  (implemented in eno_bridge.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetDeltaTime();

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetUnscaledDeltaTime();

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetTimeScale(float scale);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetTimeScale();
    }
}
