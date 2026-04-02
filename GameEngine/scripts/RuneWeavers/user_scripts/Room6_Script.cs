namespace Eno
{
    public class Room6_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room6_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;
        private float _lockRumbleTimer = 0f;

        public void Begin()
        {
            _instance       = this;
            _currentWave    = 0;
            _wavesStarted   = false;
            _roomCleared    = false;
            _waveTimer      = 0f;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;

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

            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door6to7");
                UnlockDoor("Door6to5");
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
                UnlockDoor("Door6to7");
                UnlockDoor("Door6to5");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door6to7");
                LockDoor("Door6to5");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("BatYellow", 2721f, -4655f);
                    SpawnPrefab("BatYellow", 3048f, -4975f);
                    break;
                case 2:
                    SpawnPrefab("BatYellow", 2993f, -4712f);
                    SpawnPrefab("SlugRed", 2536f, -4860f);
                    SpawnPrefab("SlugRed", 2898f, -4955f);
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
