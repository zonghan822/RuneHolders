namespace Eno
{
    public class Room3_Script : ScriptBase
    {
        private const float Wave2Delay = 10f;

        private static Room3_Script _instance;

        private bool  _wavesStarted  = false;
        private bool  _wave2Spawned  = false;
        private bool  _roomCleared   = false;
        private float _wave2Timer    = 0f;

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

            // Count down to wave 2
            if (!_wave2Spawned)
            {
                _wave2Timer += Time.deltaTime;
                if (_wave2Timer >= Wave2Delay)
                {
                    _wave2Spawned = true;
                    SpawnWave2();
                }
                return; // don't check clear until wave 2 is out
            }

            // Once wave 2 is spawned, wait for all enemies to die
            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor3to4();
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            SpawnPrefab("SlugRed", 955f, -2826f);
        }

        private void SpawnWave2()
        {
            SpawnPrefab("SlugRed",  410f, -2693f);
            SpawnPrefab("SlugRed", 1503f, -2693f);
        }

        private void UnlockDoor3to4()
        {
            uint doorID = Bridge.GetNode("Door3to4");
            if (doorID != 0 && doorID != uint.MaxValue)
                Bridge.ChangeState(doorID, "Unlocking");
        }
    }
}
