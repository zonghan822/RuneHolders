namespace Eno
{
    public class Room13A_Script : ScriptBase
    {
        private const float WaveDelay   = 10f;
        private const int   TotalWaves  = 1;

        private static readonly string[] RoomDoors =
        {
            "Door13Ato12",
            "Door13Ato14A",
        };

        private static Room13A_Script? _instance;
        public static bool _popupActivated = false;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;

        public void Begin()
        {
            _instance       = this;
            _popupActivated = false;
            _currentWave    = 0;
            _wavesStarted   = false;
            _roomCleared    = false;
            _waveTimer      = 0f;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;

            if (!_popupActivated)
            {
                _popupActivated = true;
                uint popupID = Bridge.GetNode("EnemyInfoPopUp");
                if (popupID != 0 && popupID != uint.MaxValue)
                {
                    Bridge.SetTexturePath(popupID, "Enemy-Nullfly-Frozen-Sheet");
                    Bridge.SetVisibility(popupID, true);
                    Bridge.SetButtonActive(popupID, true);
                }
                uint popupBgID = Bridge.GetNode("EnemyInfoPopUpBG");
                if (popupBgID != 0 && popupBgID != uint.MaxValue)
                    Bridge.SetVisibility(popupBgID, true);
                uint enemyInfoID = Bridge.GetNode("EnemyInfo");
                if (enemyInfoID != 0 && enemyInfoID != uint.MaxValue)
                    Bridge.SetTexturePath(enemyInfoID, "UI-EnemyPopUp-BlueNullfly");
            }

            _instance.SpawnWave(1);
        }

        public void Update()
        {
            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoors();
                return;
            }

            if (!_wavesStarted || _roomCleared) return;

            if (_currentWave < TotalWaves)
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
                UnlockDoors();
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoors();
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    NullflyBase_Script.PendingRuneIndex = 0; // ban NE
                    SpawnPrefab("NullflyBlue", 6415f, -7047f);
                    SpawnPrefab("SlugRed",     6415f, -6890f);
                    SpawnPrefab("SlugBlue",    6625f, -6890f);
                    break;
                // case 2: TODO
                // case 3: TODO
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
