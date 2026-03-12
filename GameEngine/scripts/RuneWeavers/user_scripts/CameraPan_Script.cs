using System;

namespace Eno
{
    // Attach to an invisible entity placed at the doorway (the trigger point).
    // When the player walks within TriggerRadius, the camera pans from its
    // current position to (PanTargetX, PanTargetY) at PanSpeed units/sec.
    // Player input is frozen for the duration of the pan.
    public class CameraPan_Script : ScriptBase
    {
        [Port] public string PlayerName = "Player";
        [Port] public string CameraName = "Camera";

        // How close the player must get to this entity to fire the pan.
        [Port] public float TriggerRadius = 80f;

        // World-space position the camera pans to (centre of the next room).
        [Port] public float PanTargetX = 0f;
        [Port] public float PanTargetY = 0f;

        // Units per second.
        [Port] public float PanSpeed = 800f;

        private enum State { Waiting, Panning, Done }
        private State _state = State.Waiting;

        private uint _playerID = 0;
        private uint _cameraID = 0;

        public void Begin()
        {
            _playerID = Bridge.GetNode(PlayerName);
            _cameraID = Bridge.GetNode(CameraName);
        }

        public void Update()
        {
            switch (_state)
            {
                case State.Waiting: Waiting(); break;
                case State.Panning: Panning(); break;
            }
        }

        private void Waiting()
        {
            if (_playerID == 0) return;

            float px = Bridge.GetPositionX(_playerID);
            float py = Bridge.GetPositionY(_playerID);
            float tx = Bridge.GetPositionX(ID);
            float ty = Bridge.GetPositionY(ID);

            float dx = px - tx;
            float dy = py - ty;
            if ((dx * dx + dy * dy) <= TriggerRadius * TriggerRadius)
                _state = State.Panning;
        }

        private void Panning()
        {
            // Freeze the player.
            if (_playerID != 0)
                Bridge.SetVelocity(_playerID, 0f, 0f);

            if (_cameraID == 0)
            {
                _state = State.Done;
                return;
            }

            float cx = Bridge.GetPositionX(_cameraID);
            float cy = Bridge.GetPositionY(_cameraID);

            float dx = PanTargetX - cx;
            float dy = PanTargetY - cy;
            float dist = MathF.Sqrt(dx * dx + dy * dy);

            float step = PanSpeed * Time.deltaTime;

            if (dist <= step)
            {
                // Snap to target and finish.
                Bridge.SetPosition(_cameraID, PanTargetX, PanTargetY);
                _state = State.Done;
            }
            else
            {
                float inv = step / dist;
                Bridge.SetPosition(_cameraID, cx + dx * inv, cy + dy * inv);
            }
        }
    }
}
