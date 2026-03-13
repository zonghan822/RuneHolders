namespace Eno
{
    public class Room2_Script : ScriptBase
    {
        private static Room2_Script? _instance;
        private bool  _roomCleared  = false;
        private bool  _wavesStarted = false;

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
            if (!_wavesStarted || _roomCleared) return;
            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door2to1");
                UnlockDoor("Door2to3");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            LockDoor("Door2to1");
            LockDoor("Door2to3");
            SpawnPrefab("DummyRed", 960f, -1720f);
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
