using System.Runtime.InteropServices;
using System.Text;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Scene  (implemented in eno_bridge_scene.cpp)
        // -----------------------------------------------------------------------

        // Queue a scene change. Executed at the end of the current frame.
        // sceneName is the scene filename, e.g. "Level.txt".
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void ChangeScene(string sceneName);

        // Write the current scene name into outBuffer.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void GetCurrentScene(
            [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] byte[] outBuffer,
            int bufferSize);
    }
}
