using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Input
    {
        private static InputKey curKey;
        private static int curScancode;
        private static InputAction curAction;
        private static int curMods;

        public static bool isPressed() => curAction == InputAction.PRESS;
        public static bool isReleased() => curAction == InputAction.RELEASE;

        [UnmanagedCallersOnly]
        private static void InputBoarding(int key, int scancode, int action, int mods)
        {
            Console.WriteLine($"Assigning Input! Key = {key}, Scancode = { scancode }, Action = { action }, Mods = { mods }");
            
            curKey = (InputKey)key;
            curScancode = scancode;
            curAction = (InputAction)action;
            curMods = mods;
        }

        public enum InputAction : int
        {
            RELEASE = 0,
            PRESS = 1,
        }

        public enum InputKey : int
        {
            Num0 = 48, Num1, Num2,
            Num3, Num4, Num5, Num6,
            Num7, Num8, Num9,

            A = 65, B, C, D, E,
            F, G, H, I, J, K, L,
            M, N, O, P, Q, R, S,
            T, U, V, W, X, Y, Z
        }
    }
}