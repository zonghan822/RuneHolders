using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Canvas Component  (implemented in eno_bridge_canvas.cpp)
        // -----------------------------------------------------------------------

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool GetCanvasActive(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetCanvasActive(uint sceneObjectID, [MarshalAs(UnmanagedType.I1)] bool active);

    }
}
