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

    // GLFW gamepad axis indices
    public enum GamepadAxis : int
    {
        LeftX       = 0,
        LeftY       = 1,
        RightX      = 2,
        RightY      = 3,
        LeftTrigger = 4,
        RightTrigger = 5,
    }

    // GLFW gamepad button indices
    public enum GamepadButton : int
    {
        A            = 0,
        B            = 1,
        X            = 2,
        Y            = 3,
        LeftBumper   = 4,
        RightBumper  = 5,
        Back         = 6,
        Start        = 7,
        Guide        = 8,
        LeftThumb    = 9,
        RightThumb   = 10,
        DPadUp       = 11,
        DPadRight    = 12,
        DPadDown     = 13,
        DPadLeft     = 14,
    }

    // Direct static queries for keyboard and mouse state.
    // For complex action bindings use InputAction / InputManager instead.
    public static class Input
    {
        // ── Keyboard ──────────────────────────────────────────────────────────

        // True on the first frame the key is pressed.
        public static bool isKeyPressed(KeyCode key)  => Bridge.IsKeyPressed(key);

        // True every frame the key is held down.
        public static bool isKeyHeld(KeyCode key)     => Bridge.IsKeyHeld(key);

        // True on the frame the key is released.
        public static bool isKeyReleased(KeyCode key) => Bridge.IsKeyReleased(key);

        // Active modifier flags (Shift, Ctrl, Alt …) as a Mod bitmask.
        public static Mod mods => (Mod)Bridge.ModsApplied();

        // ── Mouse ─────────────────────────────────────────────────────────────
        // button: 0 = left, 1 = right, 2 = middle

        public static bool isMousePressed(int button)  => Bridge.IsMousePressed(button);
        public static bool isMouseHeld(int button)     => Bridge.IsMouseHeld(button);
        public static bool isMouseReleased(int button) => Bridge.IsMouseReleased(button);

        // Screen-space mouse position.
        public static Vector2 mousePosition
            => new Vector2(Bridge.GetMouseX(), Bridge.GetMouseY());

        // World-space mouse position.
        public static Vector2 mouseWorldPosition
            => new Vector2(Bridge.GetMouseWorldX(), Bridge.GetMouseWorldY());

        // ── Gamepad ───────────────────────────────────────────────────────────

        public static bool isGamepadConnected() => Bridge.IsGamepadConnected();

        // Raw axis value in [-1, 1]. Triggers: -1 = released, 1 = fully pressed.
        public static float getGamepadAxis(GamepadAxis axis) => Bridge.GetGamepadAxis((int)axis);

        // Axis value with deadzone applied. Returns 0 if magnitude < deadzone.
        public static float getGamepadAxis(GamepadAxis axis, float deadzone)
        {
            float v = Bridge.GetGamepadAxis((int)axis);
            return MathF.Abs(v) < deadzone ? 0f : v;
        }

        // Left stick as a Vector2 with per-axis deadzone.
        public static Vector2 getLeftStick(float deadzone = 0.2f)
        {
            float x = getGamepadAxis(GamepadAxis.LeftX, deadzone);
            float y = getGamepadAxis(GamepadAxis.LeftY, deadzone);
            return new Vector2(x, y);
        }

        // Right stick as a Vector2 with per-axis deadzone.
        public static Vector2 getRightStick(float deadzone = 0.2f)
        {
            float x = getGamepadAxis(GamepadAxis.RightX, deadzone);
            float y = getGamepadAxis(GamepadAxis.RightY, deadzone);
            return new Vector2(x, y);
        }

        // Right trigger normalized to [0, 1] (GLFW reports [-1, 1]).
        public static float getRightTrigger()
        {
            float raw = Bridge.GetGamepadAxis((int)GamepadAxis.RightTrigger);
            return (raw + 1f) * 0.5f;  // map -1..1 to 0..1
        }

        public static bool isGamepadButtonPressed(GamepadButton button)  => Bridge.IsGamepadButtonPressed((int)button);
        public static bool isGamepadButtonHeld(GamepadButton button)     => Bridge.IsGamepadButtonHeld((int)button);
        public static bool isGamepadButtonReleased(GamepadButton button) => Bridge.IsGamepadButtonReleased((int)button);

        /// Set gamepad rumble/vibration via XInput.
        public static void setGamepadRumble(float leftMotor, float rightMotor)
            => Bridge.SetGamepadRumble(leftMotor, rightMotor);
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
            var mods = (Mod)Bridge.ModsApplied();

            // Check if the main key is pressed or held
            bool keyHeld = Bridge.IsKeyHeld(Key);
            bool keyPressed = Bridge.IsKeyPressed(Key);

            // Check if the required modifier flag(s) are active (if any)
            bool modActive = Mod == null || (mods & Mod.Value) == Mod.Value;

            // Works regardless of order (mod first or key first)
            return modActive && (keyPressed || (keyHeld && modActive));
        }
    }
}