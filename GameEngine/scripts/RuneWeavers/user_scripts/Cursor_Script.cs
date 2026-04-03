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

        private float _prevScreenX = 0f;
        private float _prevScreenY = 0f;
        private float _prevWorldX  = 0f;
        private float _prevWorldY  = 0f;

        public void Begin()
        {
            _self = Self<Sprite2D>();
            if (_self == null) return;
            _prevScreenX = Input.mousePosition.x;
            _prevScreenY = Input.mousePosition.y;
            _prevWorldX  = Input.mouseWorldPosition.x;
            _prevWorldY  = Input.mouseWorldPosition.y;
            CursorX = _prevWorldX;
            CursorY = _prevWorldY;
        }

        public void Update()
        {
            if (_self == null) return;

            float sx = Input.mousePosition.x;
            float sy = Input.mousePosition.y;
            float wx = Input.mouseWorldPosition.x;
            float wy = Input.mouseWorldPosition.y;

            if (PauseManager.PauseMenuOpen)
            {
                // Keep prev values in sync so there's no jump when unpausing
                _prevScreenX = sx;
                _prevScreenY = sy;
                _prevWorldX  = wx;
                _prevWorldY  = wy;
                return;
            }

            if (sx != _prevScreenX || sy != _prevScreenY)
            {
                // Mouse physically moved — cursor follows mouse
                CursorX = wx;
                CursorY = wy;
            }
            else if (wx != _prevWorldX || wy != _prevWorldY)
            {
                // Camera shifted (room change) — offset cursor by the same world delta
                // so it stays at the same screen position
                CursorX += wx - _prevWorldX;
                CursorY += wy - _prevWorldY;
            }

            _prevScreenX = sx;
            _prevScreenY = sy;
            _prevWorldX  = wx;
            _prevWorldY  = wy;

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
