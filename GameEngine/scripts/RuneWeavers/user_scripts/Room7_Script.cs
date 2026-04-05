namespace Eno
{
    public class Room7_Script : ScriptBase
    {

        private static Room7_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _lockRumbleTimer = 0f;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _instance     = this;
            _currentWave  = 0;
            _wavesStarted = false;
            _roomCleared  = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void ResetForRespawn()
        {
            if (_instance == null || _instance._roomCleared) return;
            _instance._wavesStarted = false;
            _instance._currentWave = 0;
            SilentUnlock("Door7to6");
            SilentUnlock("Door7to8");
            SilentUnlock("Door7to7A");
        }

        private static void SilentUnlock(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocked");
        }

        public static void StartWaves()
        {
            if (_instance == null) return;
            if (_instance._roomCleared) return;
            _instance.SpawnWave(1);
        }

        public void Update()
        {
            if (_lockRumbleTimer > 0f)
            {
                _lockRumbleTimer -= Time.deltaTime;
                if (_lockRumbleTimer <= 0f)
                    Input.setGamepadRumble(0f, 0f);
            }

            if (!_wavesStarted || _roomCleared) return;

            if (GetAllEnemies().Count == 0)
            {
                if (_currentWave < 2)
                    SpawnWave(_currentWave + 1);
                else
                {
                    _roomCleared = true;
                    BGMforLevel1_Script.SetLayer2Combat(false);
                    UnlockDoor("Door7to8");
                    UnlockDoor("Door7to6");
                    UnlockDoor("Door7to7A");
                }
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                BGMforLevel1_Script.SetLayer2Combat(true);
                LockDoor("Door7to6");
                LockDoor("Door7to8");
                LockDoor("Door7to7A");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("SlugRed", 4687f, -4735f);
                    SpawnPrefab("BatYellow",  4860f, -5033f);
                    SpawnPrefab("BatYellow", 5082f, -4635f);
                    break;
                case 2:
                    SpawnPrefab("SlugRed", 4482f, -4925f);
                    SpawnPrefab("SlugRed", 4817f, -4666f);
                    SpawnPrefab("BatYellow", 4483f, -4683f);
                    SpawnPrefab("BatYellow", 5061f, -5005f);
                    break;
            }
        }

        private void LockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
            {
                Bridge.ChangeState(id, "Locked");
                if (_doorAudio != null) { _doorAudio.sound = "Door_Close_01"; _doorAudio.play(); }
            }
            Input.setGamepadRumble(0.5f, 0.5f);
            _lockRumbleTimer = 0.2f;
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
            {
                Bridge.ChangeState(id, "Unlocking");
                if (_doorAudio != null) { _doorAudio.sound = "Door_Open_02"; _doorAudio.play(); }
            }
        }
    }
}
