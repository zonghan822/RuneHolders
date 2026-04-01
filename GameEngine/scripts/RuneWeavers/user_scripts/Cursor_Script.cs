using System;

namespace Eno
{
    public class Cursor_Script : ScriptBase
    {
        // Movement speed in world units per second (right stick)
        public float cursorSpeed = 400f;

        // Static accessor so RuneSystem can read cursor world position
        public static float CursorX { get; set; }
        public static float CursorY { get; set; }

        private Sprite2D? _self;

        // Previous mouse world position — used to detect mouse movement
        private float _prevMouseX = 0f;
        private float _prevMouseY = 0f;

        public void Begin()
        {
            _self = Self<Sprite2D>();
            if (_self == null) return;
            _prevMouseX = Input.mouseWorldPosition.x;
            _prevMouseY = Input.mouseWorldPosition.y;
            CursorX = _prevMouseX;
            CursorY = _prevMouseY;
        }

        public void Update()
        {
            if (_self == null) return;

            // Check if mouse moved this frame
            float mx = Input.mouseWorldPosition.x;
            float my = Input.mouseWorldPosition.y;
            if (mx != _prevMouseX || my != _prevMouseY)
            {
                CursorX = mx;
                CursorY = my;
                _prevMouseX = mx;
                _prevMouseY = my;
            }

            // Move cursor with right stick
            Vector2 stick = Input.getRightStick(0.15f);
            if (stick.x != 0f || stick.y != 0f)
            {
                CursorX += stick.x * cursorSpeed * Time.unscaledDeltaTime;
                CursorY += stick.y * cursorSpeed * Time.unscaledDeltaTime;
            }

            _self.position = new Vector2(CursorX, CursorY);
        }

        public void Default_enter() { }
        public void Default_update() { }
        public void Default_exit() { }
    }
}
