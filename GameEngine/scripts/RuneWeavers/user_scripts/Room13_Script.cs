namespace Eno
{
    public class Room13_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room13_Script? _instance;

        private int _currentWave = 0;  // 0 = none spawned yet
        private bool _wavesStarted = false;
        private bool _roomCleared = false;
        private float _waveTimer = 0f;

        public void Begin()
        {
            _instance = this;
            _currentWave = 0;
            _wavesStarted = false;
            _roomCleared = false;
            _waveTimer = 0f;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;
            if (_instance._roomCleared) return;
            _instance.SpawnWave(1);
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
                return; // don't check clear until gate wave fires
            }

            // All waves spawned => wait for enemies to die
            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door13to12");
                UnlockDoor("Door13to14");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door13to12");
                LockDoor("Door13to14");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    NullflyBlue_Script.PendingRuneIndex = 1; // ban SE
                    SpawnPrefab("NullflyBlue", 8452f, -8306f);
                    NullflyBlue_Script.PendingRuneIndex = 2; // ban E
                    SpawnPrefab("NullflyBlue", 9013f, -8107f);
                    SpawnPrefab("SlugRed", 8818f, -7925f);
                    SpawnPrefab("SlugRed", 8351f, -8040f);
                    SpawnPrefab("BatYellow", 8642f, -8189f);
                    break;
                case 2:
                    // gate wave — no new enemies, just triggers the clear check
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
