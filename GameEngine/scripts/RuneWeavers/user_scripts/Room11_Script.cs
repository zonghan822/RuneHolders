namespace Eno
{
    public class Room11_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room11_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;
        private float _lockRumbleTimer = 0f;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _instance     = this;
            _currentWave  = 0;
            _wavesStarted = false;
            _roomCleared  = false;
            _waveTimer    = 0f;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
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
                BGMforLevel1_Script.SetLayer2Combat(false);
                UnlockDoor("Door11to12");
                UnlockDoor("Door11to9");
                UnlockDoor("Door9to11");
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
                return;
            }

            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                UnlockDoor("Door11to12");
                UnlockDoor("Door11to9");
                UnlockDoor("Door9to11");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                BGMforLevel1_Script.SetLayer2Combat(true);
                LockDoor("Door9to11");
                LockDoor("Door11to9");
                LockDoor("Door11to12");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    NullflyBlue_Script.PendingRuneIndex = 2; // ban SE
                    SpawnPrefab("NullflyBlue", 8645f, -6126f);
                    SpawnPrefab("SlugRed", 8875f, -5715f);
                    SpawnPrefab("BatYellow", 8476f, -5956f);
                    break;
                case 2:
                    NullflyBlue_Script.PendingRuneIndex = 1; // ban E
                    SpawnPrefab("NullflyBlue", 8219f, -5995f);
                    SpawnPrefab("SlugRed", 8481f, -6128f);
                    SpawnPrefab("SlugRed", 8863f, -6127f);
                    SpawnPrefab("BatYellow", 8635f, -5781f);
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
