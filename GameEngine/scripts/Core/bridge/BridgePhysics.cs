using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Physics Component  (implemented in eno_bridge_physics.cpp)
        // -----------------------------------------------------------------------

        // Velocity
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetVelocityX(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetVelocityY(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetVelocity(uint sceneObjectID, float x, float y);

        // Forces
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AddForce(uint sceneObjectID, float x, float y);

        // Mass
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetMass(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetMass(uint sceneObjectID, float mass);

        // Gravity
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool GetUseGravity(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetUseGravity(uint sceneObjectID, [MarshalAs(UnmanagedType.I1)] bool useGravity);

        // Kinematic
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool GetIsKinematic(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetIsKinematic(uint sceneObjectID, [MarshalAs(UnmanagedType.I1)] bool kinematic);
    }
}
