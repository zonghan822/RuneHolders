using System;
using System.Runtime.InteropServices;

namespace Eno
{
    // Must follow GLFW's action values
    // Source: ChatGPT, I can't for the life of me find the
    // docs for action's int values
    public enum InputAction : int
    {
        None = 4,
        Release = 0,
        Press = 1,
        Down = 2
    }

    // Must follow GLFW's key values
    // Source: https://www.glfw.org/docs/3.3/group__mods.html
    public enum InputKey : int
    {
        Keycode_NULL,

        Keycode_0 = 48, Keycode_1, Keycode_2,
        Keycode_3, Keycode_4, Keycode_5, Keycode_6,
        Keycode_7, Keycode_8, Keycode_9,

        Keycode_A = 65, Keycode_B, Keycode_C, Keycode_D, Keycode_E,
        Keycode_F, Keycode_G, Keycode_H, Keycode_I, Keycode_J, Keycode_K, Keycode_L,
        Keycode_M, Keycode_N, Keycode_O, Keycode_P, Keycode_Q, Keycode_R, Keycode_S,
        Keycode_T, Keycode_U, Keycode_V, Keycode_W, Keycode_X, Keycode_Y, Keycode_Z,

        Keycode_F1 = 290, Keycode_F2, Keycode_F3, Keycode_F4, Keycode_F5, Keycode_F6,
        Keycode_F7, Keycode_F8, Keycode_F9, Keycode_F10, Keycode_F11, Keycode_F12
    }

    // Source: https://www.glfw.org/docs/3.3/group__mods.html
    //[Flags]
    public enum InputMod
    {
        None        = 0,
        Shift       = 1 << 0,
        Control     = 1 << 1,
        Alt         = 1 << 2,
        Super       = 1 << 3,
        CapsLock    = 1 << 4,
        NumLock     = 1 << 5
    }

    public static class Input
    {
        private struct KeyState
        {
            public bool isHeld;
            public bool pressedThisFrame;
            public bool releasedThisFrame;
        }

        private static Dictionary<InputKey, KeyState> keys = new();
        
        // Key dict initialization
        static Input()
        {
            // For each value in the Enum InputKey,
            // initialize with a new state
            foreach(InputKey key in Enum.GetValues(typeof(InputKey)))
            {
                keys[key] = new KeyState();
            }
        }

        // [Engine-Level] Used to receive raw input from engine end
        [UnmanagedCallersOnly]
        private static void InputBoarding(int key, int scancode, int action, int mods)
        {
            // Account for keys not mapped
            if (!keys.ContainsKey((InputKey)key))
            {
                Console.WriteLine($"{ key } is currently not supported!");
                return;
            }

            var state = keys[(InputKey)key];

            // When input is received, save the state
            if (action == (int)InputAction.Press)
            {
                state.isHeld = true;
                state.pressedThisFrame = true;
            }
            else if (action == (int)InputAction.Release)
            {
                state.isHeld = false;
                state.releasedThisFrame = true;
            }

            keys[(InputKey)key] = state;
        }

        // [Engine-Level] Used to reset input state when a frame ends on engine side
        [UnmanagedCallersOnly]
        private static void noInputReceived()
        {
            var keyList = keys.Keys.ToList();
            foreach(var k in keyList)
            {
                var state = keys[k];
                state.pressedThisFrame = false;
                state.releasedThisFrame = false;
                keys[k] = state;
            }
        }

        // [User-Level] interface for checking a key release state 
        public static bool GetKeyReleased(InputKey key) => keys[key].releasedThisFrame;

        // [User-Level] interface for checking a key pressed state 
        public static bool GetKeyPressed(InputKey key) => keys[key].pressedThisFrame;

        // [User-Level] interface for checking a key held down state 
        public static bool GetKeyHeld(InputKey key) => keys[key].isHeld;
    }

    // For custom inputs
    // Will inquire the Input class for raw input
    // and check if custom input meets criteria
    // Inspired by Unity's syntax: https://docs.unity3d.com/Packages/com.unity.inputsystem@1.0/manual/Actions.html
    //
    // Do I need [Serializable]? I think I need it if I want to send this information over to
    // engine side, which I think I will in the future.
    public class CustomInput
    {
        public InputKey key;
        public InputAction action;
        public InputMod mod;

        public bool state = false;

        public CustomInput(InputKey key, InputAction action, InputMod mod)
        {
            this.key = key;
            this.action = action;
            this.mod = mod;
        }

        // State changes
        public void Toggle() => state ^= !state;
        public void Enable() => state = true;
        public void Disable() => state = false;

        // Action performed check
        // Press check
        public bool Pressed() => state && Input.GetKeyPressed(key);

        // Release check
        public bool Released() => state && Input.GetKeyReleased(key);

        // Hold check
        public bool Held() => state && Input.GetKeyHeld(key);
    }
}