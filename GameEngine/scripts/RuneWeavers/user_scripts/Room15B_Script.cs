namespace Eno
{
    public class Room15B_Script : ScriptBase
    {
        private const float WavePauseTime = 2f;

        private static readonly string[] RoomDoors =
        {
            "Door15Bto14B",
            "Door15Bto16B",
            "Door15Bto16C",
        };

        private static Room15B_Script _instance;

        private bool  _wavesStarted = false;
        private bool  _enemiesLive  = false;
        private bool  _pausing      = false;
        private float _pauseTimer   = 0f;
        private int   _waveIndex    = 0;

        public void Begin()
        {
            _instance = this;
        }

        public static void StartWaves()
        {
            if (_instance != null && !_instance._wavesStarted)
                _instance.BeginWaves();
        }

        private void BeginWaves()
        {
            LockDoors();
            SpawnWave(0);
            _wavesStarted = true;
            _enemiesLive  = true;
        }

        public void Update()
        {
            if (!_wavesStarted) return;

            if (_pausing)
            {
                _pauseTimer += Time.deltaTime;
                if (_pauseTimer >= WavePauseTime)
                {
                    _pausing = false;
                    _waveIndex++;
                    SpawnWave(_waveIndex);
                    _enemiesLive = true;
                }
                return;
            }

            if (!_enemiesLive) return;
            if (GetAllEnemies().Count > 0) return;

            _enemiesLive = false;

            if (_waveIndex >= 2)
            {
                UnlockDoors();
            }
            else
            {
                _pausing    = true;
                _pauseTimer = 0f;
            }
        }

        private void SpawnWave(int index)
        {
            float cx = Bridge.GetPositionX(ID);
            float cy = Bridge.GetPositionY(ID);

            switch (index)
            {
                case 0:
                    SpawnPrefab("NullflyYellowNE", cx - 150f, cy);
                    SpawnPrefab("SlugRed",         cx + 150f, cy);
                    break;
                case 1:
                    SpawnPrefab("NullflyRedW",     cx - 150f, cy);
                    SpawnPrefab("SlugYellow",      cx + 150f, cy);
                    break;
                case 2:
                    SpawnPrefab("NullflyBlueSE",   cx - 200f, cy);
                    SpawnPrefab("BatYellow",       cx,        cy);
                    SpawnPrefab("SlugBlue",        cx + 200f, cy);
                    break;
            }
        }

        private void LockDoors()
        {
            foreach (string name in RoomDoors)
            {
                uint id = Bridge.GetNode(name);
                if (id != 0 && id != uint.MaxValue)
                    Bridge.ChangeState(id, "Locked");
            }
        }

        private void UnlockDoors()
        {
            foreach (string name in RoomDoors)
            {
                uint id = Bridge.GetNode(name);
                if (id != 0 && id != uint.MaxValue)
                    Bridge.ChangeState(id, "Unlocking");
            }
        }
    }
}
