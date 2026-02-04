using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Bridge
    {
        [DllImport("Engine.dll")]
        public static extern bool IsKeyReleased(KeyCode key);

        [DllImport("Engine.dll")]
        public static extern bool IsKeyPressed(KeyCode key);

        [DllImport("Engine.dll")]
        public static extern bool IsKeyHeld(KeyCode key);

        [DllImport("Engine.dll")]
        public static extern int ModsApplied();
    }
}
