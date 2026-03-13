namespace Eno
{
    public class Room6_Script : ScriptBase
    {
        private const float WaveDelay = 10f;

        private static Room6_Script _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;

        public void Begin()
        {
            _instance = this;
        }

        public static void StartWaves()
        {
            _instance?.SpawnWave(1);
        }

        public void Update()
        {
            if (!_wavesStarted || _roomCleared) return;

            if (_currentWave < 3)
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
                UnlockDoor("Door6to7");
                UnlockDoor("Door6to5");
                UnlockDoor("Door5to6");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door5to6");
                LockDoor("Door6to5");
                LockDoor("Door6to7");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("BatRed",    3207f, -4881f);
                    break;
                case 2:
                    SpawnPrefab("BatYellow", 2633f, -5008f);
                    SpawnPrefab("BatRed",    3057f, -4700f);
                    break;
                case 3:
                    SpawnPrefab("BatRed",    2623f, -4746f);
                    SpawnPrefab("BatYellow", 3074f, -5017f);
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
