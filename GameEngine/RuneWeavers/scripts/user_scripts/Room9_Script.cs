namespace Eno
{
    public class Room9_Script : ScriptBase
    {

        private static Room9_Script? _instance;

        private int   _currentWave  = 0;
        private bool  _wavesStarted = false;
        private bool  _roomCleared  = false;
        private float _lockRumbleTimer = 0f;
        private AudioNode? _doorAudio = null;
        private uint  _popupID     = 0;
        private static bool _popupActivated = false;

        public void Begin()
        {
            _instance     = this;
            _popupActivated = false;
            _currentWave  = 0;
            _wavesStarted = false;
            _roomCleared  = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void ResetForRespawn()
        {
            if (_instance == null || _instance._roomCleared) return;
            _instance._wavesStarted = false;
            _instance._currentWave = 0;
            SilentUnlock("Door9to8");
            SilentUnlock("Door9to10");
            SilentUnlock("Door9to11");
        }

        private static void SilentUnlock(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocked");
        }

        public static void ShowPopup()
        {
            if (_instance == null || !_popupActivated) return;
            if (_instance._popupID != 0 && _instance._popupID != uint.MaxValue)
            {
                Bridge.SetVisibility(_instance._popupID, true);
                Bridge.SetButtonActive(_instance._popupID, true);
            }
        }

        public static void StartWaves()
        {
            if (_instance == null) return;
            if (_instance._roomCleared) return;

            if (!_popupActivated)
            {
                _popupActivated = true;
                EnemyInfoController.PopupKeyboardTexture   = "UI-EnemyPopUp-BlueNullfly";
                EnemyInfoController.PopupControllerTexture = "UI-EnemyPopUp-Nullfly-Controller";

                _instance._popupID = Bridge.GetNode("EnemyInfoPopUp");
                if (_instance._popupID != 0 && _instance._popupID != uint.MaxValue)
                {
                    Bridge.SetVisibility(_instance._popupID, true);
                    Bridge.SetButtonActive(_instance._popupID, true);
                }
            }

            _instance.SpawnWave(1);
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
                    ? "UI-EnemyIcon-Nullfly-Controller"
                    : "UI-EnemyIcon-Nullfly";
                Bridge.SetTexturePath(_popupID, icon);
            }

            if (!_wavesStarted || _roomCleared) return;

            if (GetAllEnemies().Count == 0)
            {
                if (_currentWave < 2)
                    SpawnWave(_currentWave + 1);
                else
                {
                    _roomCleared = true;
                    BGMforLevel1_Script.SetLayer2Combat(false);
                    UnlockDoor("Door9to10");
                    UnlockDoor("Door9to11");
                    UnlockDoor("Door9to8");
                }
            }
        }

        private void SpawnWave(int wave)
        {
            if (!_wavesStarted)
            {
                _wavesStarted = true;
                BGMforLevel1_Script.SetLayer2Combat(true);
                LockDoor("Door9to10");
                LockDoor("Door9to11");
                LockDoor("Door9to8");
            }

            _currentWave = wave;

            switch (wave)
            {
                case 1:
                    NullflyBlue_Script.PendingRuneIndex = 2; // ban SE
                    SpawnPrefab("NullflyBlue", 8645f, -4831f);
                    break;
                case 2:
                    NullflyBlue_Script.PendingRuneIndex = 1; // ban E
                    SpawnPrefab("NullflyBlue", 9022f, -4993f);
                    SpawnPrefab("SlugRed", 8447f, -4735f);
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
