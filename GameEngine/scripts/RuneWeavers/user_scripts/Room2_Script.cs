namespace Eno
{
    public class Room2_Script : ScriptBase
    {
        private static Room2_Script? _instance;
        private float _lockRumbleTimer = 0f;
        private bool  _roomCleared  = false;
        private bool  _wavesStarted = false;

        public void Begin()
        {
            _instance     = this;
            _roomCleared  = false;
            _wavesStarted = false;
            DummyRed_Script.WasHit = false;
        }

        public static void StartWaves()
        {
            _instance?.SpawnWave1();
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
            if (DummyRed_Script.WasHit)
            {
                _roomCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                UnlockDoor("Door2to1");
                UnlockDoor("Door2to3");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            BGMforLevel1_Script.SetLayer2Combat(true);
            LockDoor("Door2to1");
            LockDoor("Door2to3");
            SpawnPrefab("DummyRed", 972f, -1700f);
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
