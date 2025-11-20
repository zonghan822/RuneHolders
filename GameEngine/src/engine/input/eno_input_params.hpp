#pragma once

namespace Input
{
    // ========================= Based on GLFW =========================

    // Naming for all enums are matched with C# end naming
    enum InputAction : int
    {
        None = 4,
        Release = 0,
        Press = 1,
        Down = 2
    };

    enum KeyCode : int
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
    };

    enum Mod
    {
        Default =       0,
        Shift =         1 << 0,
        Control =       1 << 1,
        Alt =           1 << 2,
        Super =         1 << 3,
        CapsLock =      1 << 4,
        NumLock =       1 << 5
    };
}