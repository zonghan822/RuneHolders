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
            Console.WriteLine($"[AudioSettings] Begin — ID={ID}, pos=({Bridge.GetPositionX(ID)}, {Bridge.GetPositionY(ID)})");
        }

        public void Update()
        {
            float mx = Bridge.GetMouseWorldX();
            float my = Bridge.GetMouseWorldY();

            if (Bridge.IsMousePressed(0))
            {
                float nodeX = Bridge.GetPositionX(ID);
                float nodeY = Bridge.GetPositionY(ID);
                float dx = mx - nodeX;
                float dy = my - nodeY;
                float distSq = dx * dx + dy * dy;

                Console.WriteLine($"[AudioSettings] Mouse pressed — mouse=({mx}, {my}), node=({nodeX}, {nodeY}), distSq={distSq}, threshold={BallRadius * BallRadius}");

                if (distSq <= (BallRadius * BallRadius))
                {
                    _dragging = true;
                    Console.WriteLine("[AudioSettings] CLICKED ON NODE — drag started");
                }
                else
                {
                    Console.WriteLine("[AudioSettings] Click missed node");
                }
            }

            if (Bridge.IsMouseHeld(0) && _dragging)
            {
                float clampedX = MathF.Max(SliderMinX, MathF.Min(SliderMaxX, mx));
                Bridge.SetPosition(ID, clampedX, Bridge.GetPositionY(ID));
                Console.WriteLine($"[AudioSettings] Dragging — mouseX={mx}, clampedX={clampedX}");
            }

            if (Bridge.IsMouseReleased(0) && _dragging)
            {
                Console.WriteLine("[AudioSettings] Mouse released — drag stopped");
                _dragging = false;
            }
        }
    }
}
