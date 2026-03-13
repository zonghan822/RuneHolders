using System;

namespace Eno
{
    public class Door18Cto20C_Script : ScriptBase
    {
        [Port] public string PlayerName    = "Player";
        [Port] public string CameraName    = "Camera";
        [Port] public float  PanSpeed      = 800f;
        [Port] public float  TriggerRadius = 64f;

        private const float PlayerDestX =  9044f;
        private const float PlayerDestY = -10327f;
        private const float CamDestX    =  8636f;
        private const float CamDestY    = -10302f;

        private uint  _playerID    = 0;
        private uint  _cameraID    = 0;
        private float _unlockTimer = 0f;
        private bool  _panning     = false;

        public void Begin()
        {
            _playerID = Bridge.GetNode(PlayerName);
            _cameraID = Bridge.GetNode(CameraName);
        }

        public void Update()
        {
            if (!_panning || _cameraID == uint.MaxValue || _cameraID == 0) return;

            if (_playerID != 0 && _playerID != uint.MaxValue)
            {
                Bridge.SetPosition(_playerID, PlayerDestX, PlayerDestY);
                Bridge.SetVelocity(_playerID, 0f, 0f);
            }

            float cx = Bridge.GetPositionX(_cameraID);
            float cy = Bridge.GetPositionY(_cameraID);
            float dx = CamDestX - cx;
            float dy = CamDestY - cy;
            float dist = MathF.Sqrt(dx * dx + dy * dy);
            float step = PanSpeed * Time.deltaTime;

            if (dist <= step)
            {
                Bridge.SetPosition(_cameraID, CamDestX, CamDestY);
                _panning = false;
            }
            else
            {
                float inv = step / dist;
                Bridge.SetPosition(_cameraID, cx + dx * inv, cy + dy * inv);
            }
        }

        public void Locked_enter()
        {
            Bridge.SetVisibility(ID, true);
            Bridge.SetColliderActive(ID, true);
        }
        public void Locked_update() { }
        public void Locked_exit()   { }

        public void Unlocking_enter()  { _unlockTimer = 0f; }
        public void Unlocking_update()
        {
            _unlockTimer += Time.deltaTime;
            if (_unlockTimer >= 0.5f)
                Bridge.ChangeState(ID, "Unlocked");
        }
        public void Unlocking_exit() { }

        public void Unlocked_enter()
        {
            Bridge.SetVisibility(ID, false);
            Bridge.SetColliderActive(ID, false);
        }
        public void Unlocked_update()
        {
            if (_panning || _playerID == 0 || _playerID == uint.MaxValue) return;

            float doorX = Bridge.GetPositionX(ID);
            float doorY = Bridge.GetPositionY(ID);
            float px    = Bridge.GetPositionX(_playerID);
            float py    = Bridge.GetPositionY(_playerID);
            float dx    = px - doorX;
            float dy    = py - doorY;

            if (dx * dx + dy * dy > TriggerRadius * TriggerRadius) return;

            Bridge.SetPosition(_playerID, PlayerDestX, PlayerDestY);
            Bridge.SetVelocity(_playerID, 0f, 0f);
            _panning = true;
        }
        public void Unlocked_exit() { }
    }
}
