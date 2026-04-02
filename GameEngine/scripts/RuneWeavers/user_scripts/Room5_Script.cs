namespace Eno
{
    public class Room5_Script : ScriptBase
    {
        private const float WaveDelay = 3f;

        private static Room5_Script? _instance;
        private static bool _popupActivated = false;

        private int   _currentWave  = 0;  // 0 = none spawned yet
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _waveTimer    = 0f;
        private float _lockRumbleTimer = 0f;

        public void Begin()
        {
            _instance     = this;
            _popupActivated = false;
            _currentWave  = 0;
            _wavesStarted = false;
            _roomCleared  = false;
            _waveTimer    = 0f;
        }

        public static void StartWaves()
        {
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
                UnlockDoor("Door5to6");
                UnlockDoor("Door5to4");
                UnlockDoor("Door5to5A");
            }

            if (!_wavesStarted || _roomCleared) return;

            if (_currentWave < 2)
            {
                _waveTimer += Time.deltaTime;
                if (_waveTimer >= WaveDelay)
                {
                    _waveTimer = 0f;
                    SpawnWave(_currentWave + 1);
                }
                return; // don't check clear until all waves out
            }

            // All waves spawned — wait for enemies to die
            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                UnlockDoor("Door5to6");
                UnlockDoor("Door5to4");
                UnlockDoor("Door5to5A");
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                LockDoor("Door5to6");
                LockDoor("Door5to4");
                LockDoor("Door5to5A");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    SpawnPrefab("BatYellow",  635f, -4630f);
                    break;
                case 2:
                    SpawnPrefab("BatYellow",  956f, -5000f);
                    SpawnPrefab("SlugRed",   1138f, -4682f);
                    break;
            }
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
