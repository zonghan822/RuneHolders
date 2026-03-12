using System;

namespace Eno
{
    // DEBUG ONLY — press the TestKey to instantly trigger a camera pan.
    // Attach to any entity in the scene.
    // Remove (or disable) before shipping.
    public class CameraPanTest_Script : ScriptBase
    {
        [Port] public string CameraName  = "Camera";

        // World-space centre of the next fighting ground.
        [Port] public float PanTargetX   = 956f;
        [Port] public float PanTargetY   = -3816.756f;

        // Units per second.
        [Port] public float PanSpeed     = 1000f;

        // Key to press to trigger the pan.
        [Port] public KeyCode TestKey    = KeyCode.T;

        private enum State { Idle, Panning }
        private State _state = State.Idle;

        private uint _cameraID = 0;

        public void Begin()
        {
            _cameraID = Bridge.GetNode(CameraName);
        }

        public void Update()
        {
            if (_state == State.Idle)
            {
                if (Input.isKeyPressed(TestKey))
                    _state = State.Panning;
            }
            else
            {
                Pan();
            }
        }

        private void Pan()
        {
            if (_cameraID == 0) { _state = State.Idle; return; }

            float cx = Bridge.GetPositionX(_cameraID);
            float cy = Bridge.GetPositionY(_cameraID);

            float dx = PanTargetX - cx;
            float dy = PanTargetY - cy;
            float dist = MathF.Sqrt(dx * dx + dy * dy);

            float step = PanSpeed * Time.deltaTime;

            if (dist <= step)
            {
                Bridge.SetPosition(_cameraID, PanTargetX, PanTargetY);
                _state = State.Idle;   // ready to pan again if you press again
            }
            else
            {
                float inv = step / dist;
                Bridge.SetPosition(_cameraID, cx + dx * inv, cy + dy * inv);
            }
        }
    }
}
