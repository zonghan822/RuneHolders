namespace Eno
{
    public class Room3_Script : ScriptBase
    {
        private const float Wave2Delay = 10f;

        private static Room3_Script _instance;

        private bool  _wavesStarted = false;
        private bool  _wave2Spawned = false;
        private bool  _roomCleared  = false;
        private float _wave2Timer   = 0f;

        public void Begin()
        {
            _instance = this;
        }

        public static void StartWaves()
        {
            _instance?.SpawnWave1();
        }

        public void Update()
        {
            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door3to4");
                UnlockDoor("Door4to3");
                UnlockDoor("Door3to2");
                UnlockDoor("Door2to3");
            }

            if (!_wavesStarted || _roomCleared) return;

            if (!_wave2Spawned)
            {
                _wave2Timer += Time.deltaTime;
                if (_wave2Timer >= Wave2Delay)
                {
                    _wave2Spawned = true;
                    SpawnWave2();
                }
                return;
            }

            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door3to4");
                UnlockDoor("Door4to3");
                UnlockDoor("Door3to2");
                UnlockDoor("Door2to3");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            LockDoor("Door2to3");
            LockDoor("Door3to2");
            LockDoor("Door3to4");
            LockDoor("Door4to3");
            SpawnPrefab("SlugRed", 955f, -2826f);
        }

        private void SpawnWave2()
        {
            SpawnPrefab("SlugRed",  410f, -2693f);
            SpawnPrefab("SlugRed", 1503f, -2693f);
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
