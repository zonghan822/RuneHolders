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
    }
}
