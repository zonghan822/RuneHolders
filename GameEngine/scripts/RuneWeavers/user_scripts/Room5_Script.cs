namespace Eno
{
    public class Room5_Script : ScriptBase
    {

        private static Room5_Script? _instance;
        private static bool _popupActivated = false;

        private int   _currentWave  = 0;  // 0 = none spawned yet
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private const float WaveDelay = 5f;
        private float _waveTimer    = 0f;
        private float _lockRumbleTimer = 0f;
        private uint  _popupID         = 0;

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
            if (_instance == null) return;
            if (_instance._roomCleared) return;

            if (!_popupActivated)
            {
                _popupActivated = true;
                EnemyInfoController.PopupKeyboardTexture   = "UI-EnemyPopUp-YellowBat";
                EnemyInfoController.PopupControllerTexture = "UI-EnemyPopUp-YellowBat-Controller";

                _instance._popupID = Bridge.GetNode("EnemyInfoPopUp");
                if (_instance._popupID != 0 && _instance._popupID != uint.MaxValue)
                {
                    Bridge.SetVisibility(_instance._popupID, true);
                    Bridge.SetButtonActive(_instance._popupID, true);
                }
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

            if (_popupID != 0 && _popupID != uint.MaxValue)
            {
                string icon = Input.inputMode == InputMode.Controller
                    ? "UI-EnemyIcon-Bat-Controller"
                    : "UI-EnemyIcon-Bat";
                Bridge.SetTexturePath(_popupID, icon);
            }

            if (!_wavesStarted || _roomCleared) return;

            _waveTimer += Time.deltaTime;

            if (GetAllEnemies().Count == 0 || _waveTimer >= WaveDelay)
            {
                _waveTimer = 0f;
                if (_currentWave < 2)
                    SpawnWave(_currentWave + 1);
                else
                {
                    _roomCleared = true;
                    BGMforLevel1_Script.SetLayer2Combat(false);
                    UnlockDoor("Door5to6");
                    UnlockDoor("Door5to4");
                    UnlockDoor("Door5to5A");
                }
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                BGMforLevel1_Script.SetLayer2Combat(true);
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
