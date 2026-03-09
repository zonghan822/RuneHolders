using System;

namespace Eno
{
    // Attach to a slider ball entity.
    // Click and drag the node left/right; position is clamped to X [0, 500].
    public class AudioSettings_Script : ScriptBase
    {
        private const float SliderMinX = 0f;
        private const float SliderMaxX = 500f;
        private const float BallRadius  = 25f;

        private bool _dragging = false;

        public void Begin()
        {
        }

        public void Update()
        {
            float mx = Bridge.GetMouseX();
            float my = Bridge.GetMouseY();

            if (Bridge.IsMousePressed(0))
            {
                float dx = mx - Bridge.GetPositionX(ID);
                float dy = my - Bridge.GetPositionY(ID);
                if ((dx * dx + dy * dy) <= (BallRadius * BallRadius))
                    _dragging = true;
            }

            if (Bridge.IsMouseHeld(0) && _dragging)
            {
                float clampedX = MathF.Max(SliderMinX, MathF.Min(SliderMaxX, mx));
                Bridge.SetPosition(ID, clampedX, Bridge.GetPositionY(ID));
            }

            if (Bridge.IsMouseReleased(0))
                _dragging = false;
        }
    }
}
