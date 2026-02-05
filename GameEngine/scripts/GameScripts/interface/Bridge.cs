using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Bridge
    {
        [DllImport("Engine_shared.dll")]
        public static extern bool IsKeyReleased(KeyCode key);

        [DllImport("Engine_shared.dll")]
        public static extern bool IsKeyPressed(KeyCode key);

        [DllImport("Engine_shared.dll")]
        public static extern bool IsKeyHeld(KeyCode key);

        [DllImport("Engine_shared.dll")]
        public static extern int ModsApplied();

        [DllImport("Engine_shared.dll")]
        public static extern bool SetEntityAnimation(uint entityID, string animationName);

        [DllImport("Engine_shared.dll")]
        public static extern int ChangeState();
    }
}
