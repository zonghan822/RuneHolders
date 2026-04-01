namespace Eno
{
    public class Room6_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room6_Script? _instance;
        private static bool _popupActivated = false;

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
                    Bridge.SetTexturePath(popupID, "Enemy-Bat-Frozen-Sheet");
                    Bridge.SetVisibility(popupID, true);
                    Bridge.SetButtonActive(popupID, true);
                }
                uint popupBgID = Bridge.GetNode("EnemyInfoPopUpBG");
                if (popupBgID != 0 && popupBgID != uint.MaxValue)
                    Bridge.SetVisibility(popupBgID, true);
                uint enemyInfoID = Bridge.GetNode("EnemyInfo");
                if (enemyInfoID != 0 && enemyInfoID != uint.MaxValue)
                    Bridge.SetTexturePath(enemyInfoID, "UI-EnemyPopUp-YellowBat");
            }

            _instance.SpawnWave(1);
        }

        public void Update()
        {
            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                UnlockDoor("Door6to7");
                UnlockDoor("Door6to5");
                UnlockDoor("Door5to6");
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
                UnlockDoor("Door6to7");
                UnlockDoor("Door6to5");
                UnlockDoor("Door5to6");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door5to6");
                LockDoor("Door6to5");
                LockDoor("Door6to7");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("BatYellow",    3207f, -4881f);
                    break;
                case 2:
                    SpawnPrefab("BatYellow", 2633f, -5008f);
                    SpawnPrefab("BatRed",    3057f, -4700f);
                    break;
                case 3:
                    SpawnPrefab("BatRed",    2623f, -4746f);
                    SpawnPrefab("BatYellow", 3074f, -5017f);
                    break;
            }
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
