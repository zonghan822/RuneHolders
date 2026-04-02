namespace Eno
{
    public class Room7_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room7_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;
        private float _lockRumbleTimer = 0f;

        public void Begin()
        {
            _instance     = this;
            _currentWave  = 0;
            _wavesStarted = false;
            _roomCleared  = false;
            _waveTimer    = 0f;
        }

        public static void StartWaves()
        {
            _instance?.SpawnWave(1);
        }

        public void Update()
        {
            if (_lockRumbleTimer > 0f)
            {
                _lockRumbleTimer -= Time.deltaTime;
                if (_lockRumbleTimer <= 0f)
                    Input.setGamepadRumble(0f, 0f);
            }

            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door7to8");
                UnlockDoor("Door7to6");
                UnlockDoor("Door7to7A");
            }

            if (!_wavesStarted || _roomCleared) return;

            if (_currentWave < 2)
            {
                _waveTimer += Time.deltaTime;
                if (_waveTimer >= WaveDelay)
                {
                    _waveTimer = 0f;
                    SpawnWave(_currentWave + 1);
                }
                return;
            }

            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door7to8");
                UnlockDoor("Door7to6");
                UnlockDoor("Door7to7A");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
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
                Bridge.ChangeState(id, "Locked");
            Input.setGamepadRumble(0.5f, 0.5f);
            _lockRumbleTimer = 0.2f;
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocking");
        }
    }
}
