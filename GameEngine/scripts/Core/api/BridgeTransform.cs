using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Transform Component  (implemented in eno_bridge_transform.cpp)
        // -----------------------------------------------------------------------

        // Position
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetPositionX(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetPositionY(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetPosition(uint sceneObjectID, float x, float y);

        // Rotation  (engine stores rotation as Vec2 — x and y components)
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetRotationX(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetRotationY(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetRotation(uint sceneObjectID, float x, float y);

        // Scale
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetScaleX(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetScaleY(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetScale(uint sceneObjectID, float x, float y);
    }
}
