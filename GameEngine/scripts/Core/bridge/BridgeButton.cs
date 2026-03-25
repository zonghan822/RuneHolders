using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Button Component  (implemented in eno_bridge_button.cpp)
        // -----------------------------------------------------------------------

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool GetButtonActive(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetButtonActive(uint sceneObjectID, [MarshalAs(UnmanagedType.I1)] bool active);
    }
}
