using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Animation Component  (implemented in eno_bridge_animation.cpp)
        // -----------------------------------------------------------------------

        // animName must match a name registered in AnimationControllerComponent.
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAnimation(uint sceneObjectID, string animName);

        // Returns the name of the currently playing animation (ANSI string pointer).
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string GetCurrentAnimation(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAnimationLooping(uint sceneObjectID, bool loop);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAnimationFrameDuration(uint sceneObjectID, float frameDuration);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetAnimationFrameCount(uint sceneObjectID);
    }
}
