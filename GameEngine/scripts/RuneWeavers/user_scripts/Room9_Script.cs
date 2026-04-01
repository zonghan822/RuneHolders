namespace Eno
{
    public class Room9_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room9_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;

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
            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door9to10");
                UnlockDoor("Door9to11");
                UnlockDoor("Door9to8");
                UnlockDoor("Door8to9");
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
                UnlockDoor("Door9to10");
                UnlockDoor("Door9to11");
                UnlockDoor("Door9to8");
                UnlockDoor("Door8to9");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door8to9");
                LockDoor("Door9to8");
                LockDoor("Door9to10");
                LockDoor("Door9to11");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("SlugBlue", 8899f, -4686f);
                    break;
                case 2:
                    SpawnPrefab("SlugBlue", 8610f, -4686f);
                    SpawnPrefab("SlugBlue", 8299f, -4686f);
                    break;
            }
        }

        private void LockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Locked");
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocking");
        }
    }
}
