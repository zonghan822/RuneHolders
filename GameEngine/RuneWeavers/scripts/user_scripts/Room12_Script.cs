namespace Eno
{
    public class Room12_Script : ScriptBase
    {

        private static Room12_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _instance = this;
            _currentWave = 0;
            _wavesStarted = false;
            _roomCleared = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void ResetForRespawn()
        {
            if (_instance == null || _instance._roomCleared) return;
            _instance._wavesStarted = false;
            _instance._currentWave = 0;
            SilentUnlock("Door12to11");
            SilentUnlock("Door12to13");
        }

        private static void SilentUnlock(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocked");
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

            if (GetAllEnemies().Count == 0)
            {
                if (_currentWave < 2)
                    SpawnWave(_currentWave + 1);
                else
                {
                    _roomCleared = true;
                    UnlockDoor("Door12to11");
                    UnlockDoor("Door12to13");
                }
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
                    NullflyBlue_Script.PendingRuneIndex = 2; // ban SE
                    SpawnPrefab("NullflyBlue", 8351f, -7156f);
                    NullflyBlue_Script.PendingRuneIndex = 1; // ban E
                    SpawnPrefab("NullflyBlue", 8955f, -7008f);
                    SpawnPrefab("SlugRed", 8636f, -7000f);
                    break;
                case 2:
                    NullflyBlue_Script.PendingRuneIndex = 1; // ban E
                    SpawnPrefab("NullflyBlue", 8105f, -7002f);
                    NullflyBlue_Script.PendingRuneIndex = 2; // ban SE
                    SpawnPrefab("NullflyBlue", 9176f, -7218f);
                    SpawnPrefab("SlugRed", 8953f, -6821f);
                    SpawnPrefab("SlugRed", 8444f, -7129f);
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
