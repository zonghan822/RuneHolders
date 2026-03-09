using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Eno
{
    // Must follow GLFW's key values
    // Source: https://www.glfw.org/docs/3.3/group__mods.html
    public enum KeyCode : int
    {
        _NULL = 0,

        // Num Keys
        Num0 = 48, Num1, Num2,
        Num3, Num4, Num5, Num6,
        Num7, Num8, Num9,

        // Special Keys
        Minus = 45, Equal = 61, 
        Left_Bracket = 91, Right_Bracket = 93, 
        Backslash = 92, Semicolon = 59, Apostrophe = 39, 
        Comma = 44, Period = 46, Slash = 47, Grave_Accent = 96,

        // Alpha keys
        A = 65, B, C, D, E,
        F, G, H, I, J, K, L,
        M, N, O, P, Q, R, S,
        T, U, V, W, X, Y, Z,

        // Function Keys
        F1 = 290, F2, F3, F4, F5, F6,
        F7, F8, F9, F10, F11, F12,

        // Modifier Keys
        L_Shift = 340, L_Control, L_Alt, L_Super,
        R_Shift, R_Control, R_Alt, R_Super,

        // Standard Keys
        Space = 32, Escape = 256, Enter = 257, Tab = 258, Backspace = 259
    }

    // Source: https://www.glfw.org/docs/3.3/group__mods.html
    [Flags]
    public enum Mod
    {
        None        = 0,
        Shift       = 1 << 0,
        Control     = 1 << 1,
        Alt         = 1 << 2,
        Super       = 1 << 3,
        CapsLock    = 1 << 4,
        NumLock     = 1 << 5
    }

    public class InputManager
    {
        public static InputManager Instance { 
            get
            {
                if (_instance == null)
                {
                    _instance = new InputManager();
                }

                return _instance;
            }
            set => Instance = value;
        }

        private static InputManager? _instance;

        private List<InputAction> actions = new();
        public void Register(InputAction action)
        {
            if (!actions.Contains(action))
                actions.Add(action);
        }
        public void Unregister(InputAction action) => actions.Remove(action);

        public void Clear() => actions.Clear();

        public void Update()
        {
            foreach(var action in actions)
            {
                action.Calls();
            }
        }
    }

    // For custom inputs
    // Defines a single action which can be triggered multiple ways through specified bindings
    // Inspiration:
    // https://docs.unity3d.com/Packages/com.unity.inputsystem@1.0/manual/Actions.html
    // https://docs.unity3d.com/Packages/com.unity.inputsystem@1.4/manual/ActionBindings.html#one-modifier
    public class InputAction
    {
        public string Name;
        public bool state = false;
        private List<InputBinding> bindings = new();

        public event Action? when;

        // Constructor
        public InputAction(string name)
        {
            this.Name = name;
        }

        // State changes
        public void Toggle()
        {
            state = !state;

            if (state)
                InputManager.Instance.Register(this);
            else
                InputManager.Instance.Register(this);
        }

        public void Enable()
        {
            state = true;
            ScriptBase.InputMng.Register(this);
        }
        
        public void Disable()
        {
            state = false;
            ScriptBase.InputMng.Unregister(this);
        }

        // Bindings
        // Modifiers

        // Add 1 binding
        public InputAction AddBinding(KeyCode key, Mod? mod = null)
        {
            bindings.Add(new InputBinding(key, mod));
            return this;
        }

        // // Add multiple bindings
        // public AddCompositeBinding(string binding)
        // {

        // }

        // Callback
        internal void Calls()
        {
            // Trigger each callback stored in each binding
            foreach (var b in bindings)
            {
                if (b.Trigger())
                {
                    when?.Invoke();
                    break;
                }
            }
        }
    }

    public struct InputBinding
    {
        public KeyCode Key;
        public Mod? Mod;

        public InputBinding(KeyCode key, Mod? mod = null)
        {
            Key = key;
            Mod = mod;
        }

        public bool Trigger()
        {
            // Get current active modifiers from engine
            var mods = (Mod)Input.ModsApplied();

            // Check if the main key is pressed or held
            bool keyHeld = Input.IsKeyHeld(Key);
            bool keyPressed = Input.IsKeyPressed(Key);

            // Check if the required modifier flag(s) are active (if any)
            bool modActive = Mod == null || (mods & Mod.Value) == Mod.Value;

            // Works regardless of order (mod first or key first)
            return modActive && (keyPressed || (keyHeld && modActive));
        }
    }
}