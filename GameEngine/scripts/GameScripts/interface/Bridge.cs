using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Bridge
    {
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsKeyReleased(KeyCode key);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsKeyPressed(KeyCode key);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsKeyHeld(KeyCode key);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int ModsApplied();

        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern bool SetEntityAnimation(uint entityID, string animationName);

        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr GetEntityAIState(uint entityID);

        // Helper to convert the returned pointer to a string
        public static string GetAIState(uint entityID)
        {
            IntPtr ptr = GetEntityAIState(entityID);
            if (ptr == IntPtr.Zero)
                return "";
            return Marshal.PtrToStringUni(ptr) ?? "";
        }
    }
}
