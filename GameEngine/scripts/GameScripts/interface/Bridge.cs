using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Bridge
    {
        [DllImport("EnoEngine.exe")]
        public static extern bool IsKeyReleased(KeyCode key);

        [DllImport("EnoEngine.exe")]
        public static extern bool IsKeyPressed(KeyCode key);

        [DllImport("EnoEngine.exe")]
        public static extern bool IsKeyHeld(KeyCode key);

        [DllImport("EnoEngine.exe")]
        public static extern int ModsApplied();

        [DllImport("EnoEngine.exe", CharSet = CharSet.Unicode)]
        public static extern bool SetEntityAnimation(uint entityID, string animationName);

        [DllImport("EnoEngine.exe", CharSet = CharSet.Unicode)]
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
