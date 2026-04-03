namespace Eno
{
    public class Room2_Script : ScriptBase
    {
        private static Room2_Script? _instance;
        private float _lockRumbleTimer = 0f;
        private bool  _roomCleared    = false;
        private bool  _wasEverCleared = false;
        private bool  _wavesStarted   = false;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _instance      = this;
            _roomCleared   = false;
            _wasEverCleared = false;
            _wavesStarted  = false;
            DummyRed_Script.WasHit = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;
            DummyRed_Script.WasHit = false;
            _instance.SpawnWave1();
        }

        public static void CleanupDummies()
        {
            DummyRed_Script.Cleanup();
        }

        public void Update()
        {
            if (_lockRumbleTimer > 0f)
            {
                _lockRumbleTimer -= Time.deltaTime;
                if (_lockRumbleTimer <= 0f)
                    Input.setGamepadRumble(0f, 0f);
            }

            if (!_wavesStarted || _roomCleared) return;
            if (DummyRed_Script.WasHit || GetAllEnemies().Count == 0)
            {
                _roomCleared    = true;
                _wasEverCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                UnlockDoor("Door2to1");
                UnlockDoor("Door2to3");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            if (!_wasEverCleared)
            {
                BGMforLevel1_Script.SetLayer2Combat(true);
                LockDoor("Door2to1");
                LockDoor("Door2to3");
            }
            _roomCleared = false;
            SpawnPrefab("DummyRed", 972f, -1700f);
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
