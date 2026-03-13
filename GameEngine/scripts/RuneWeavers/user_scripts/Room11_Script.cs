namespace Eno
{
    public class Room11_Script : ScriptBase
    {
        private const float WaveDelay = 10f;

        private static Room11_Script _instance;

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
                UnlockDoor("Door11to9");
                UnlockDoor("Door9to11");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door9to11");
                LockDoor("Door11to9");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("SlugRed",    8963f, -6018f);
                    SpawnPrefab("SlugYellow", 8643f, -6112f);
                    SpawnPrefab("SlugBlue",   8288f, -6078f);
                    break;
                case 2:
                    SpawnPrefab("SlugRed",  8468f, -5945f);
                    SpawnPrefab("SlugRed",  8828f, -5945f);
                    SpawnPrefab("BatBlue",  8291f, -5788f);
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
