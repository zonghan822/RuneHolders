namespace Eno
{
    public class Room13_Script : ScriptBase
    {
        private const float WaveDelay = 10f;

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
            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door5to6");
                UnlockDoor("Door5to4");
                UnlockDoor("Door4to5");
                UnlockDoor("Door5to5A");
            }

            if (!_wavesStarted || _roomCleared) return;

            if (_currentWave < 1)
            {
                _waveTimer += Time.deltaTime;
                if (_waveTimer >= WaveDelay)
                {
                    _waveTimer = 0f;
                    SpawnWave(_currentWave + 1);
                }
                return; // don't check clear until all waves out
            }

            // All waves spawned � wait for enemies to die
            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door5to6");
                UnlockDoor("Door5to4");
                UnlockDoor("Door4to5");
                UnlockDoor("Door5to5A");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door4to5");
                LockDoor("Door5to4");
                LockDoor("Door5to6");
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
