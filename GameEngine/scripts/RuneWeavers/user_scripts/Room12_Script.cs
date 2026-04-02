namespace Eno
{
    public class Room12_Script : ScriptBase
    {
        private const float WaveDelay = 10f;

        private static Room12_Script? _instance;

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
            _instance?.SpawnWave(1);
        }

        public void Update()
        {
            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door12to11");
                UnlockDoor("Door12to13");
            }

            if (!_wavesStarted || _roomCleared) return;

            if (_currentWave < 3)
            {
                _waveTimer += Time.deltaTime;
                if (_waveTimer >= WaveDelay)
                {
                    _waveTimer = 0f;
                    SpawnWave(_currentWave + 1);
                }
                return; // don't check clear until all waves out
            }

            // All waves spawned — wait for enemies to die
            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door12to11");
                UnlockDoor("Door12to13");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door12to11");
                LockDoor("Door12to13");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("SlugYellow", 957f, -4946f);
                    break;
                case 2:
                    SpawnPrefab("SlugYellow", 654f, -4872f);
                    SpawnPrefab("SlugYellow", 1311f, -4764f);
                    break;
                case 3:
                    SpawnPrefab("SlugRed", 1117f, -4937f);
                    SpawnPrefab("SlugYellow", 790f, -4688f);
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
